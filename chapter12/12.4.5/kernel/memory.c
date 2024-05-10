/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-09 14:26:27
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-28 13:40:36
 * @FilePath: /OS/chapter8/8.4/kernel/memory.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "memory.h"
#include "bitmap.h"
#include "stdint.h"
#include "global.h"
#include "debug.h"
#include "string.h"
#include "print.h"
#include "sync.h"

#define PG_SIZE 4096

/***********************************位图地址************************************
 * 因为 Oxc009f000 是内核主线程栈顶， Oxc009e000 是内核主线程的pcb。
 * 一个页框大小的位图可表示 128MB 内存，位图位置安排在地址。xc009a000
 * 这样本系统最大支持 4 个页框的位图，即 512MB
********************************************************************************/
#define MEM_BITMAP_BASE 0xc009a000
/* 0xcOOOOOOO 是内核从虚拟地址 3G 起
 * 0xcOOOOOOO 是内核从虚拟地址 3G起 OxlOOOOO 意指跨过低端 lMB 内存，使虚拟地址在逻辑上连续
*/
#define K_HEAP_START 0xc0100000

#define PDE_IDX(addr) ((addr&0xffc00000)>>22) //高10位
#define PTE_IDX(addr) ((addr&0x003ff000)>>12) //中10位

/*存池结构，生成两个实例用于管理内核内存池和用户内存池*/
struct pool{
    struct bitmap pool_bitmap;   //本内存池周到的位图结构， 用于管理物理内存
    uint32_t phy_addr_start;    //本内存池所管理物理内存的起始地址
    uint32_t pool_size;      //本内存池字节容量
    struct lock lock;
};

/*内存仓库*/
struct arena{
    struct mem_block_desc* desc;    //此arena关联的mem_block_desc
    /*large为ture时，cnt表示的是页框数,一个页框4096字节。否则cnt表示空闲mem_block数量*/
    uint32_t cnt;
    bool large;
};

struct mem_block_desc k_block_descs[DESC_CNT];  //内核内存块描述符数组
struct pool kernel_pool, user_pool;  //生成内核内存池和用户内存池
struct virtual_addr kernel_vaddr;    //此结构用来给内核分配虚拟地址


/*在 pf 表示的虚拟内存池中申请 pg_cnt 个虚拟页，成功则返回虚拟页的起始地址，失败则返回 NULL*/
static void* vaddr_get(enum pool_flags pf, uint32_t pg_cnt){
    int vaddr_start = 0, bit_idx_start = -1;
    uint32_t cnt = 0;
    if(pf==PF_KERNEL){
        bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap,pg_cnt); //  返回的是成功位置的下标需要页大小
        if(bit_idx_start == -1) return NULL;
        while(cnt < pg_cnt){
            bitmap_set(&kernel_vaddr.vaddr_bitmap,bit_idx_start+cnt++,1);
        }
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
    }else{
        //用户内存池
        struct task_struct* cur = running_thread();
        bit_idx_start = bitmap_scan(&cur->userprog_vaddr.vaddr_bitmap,pg_cnt); //  返回的是成功位置的下标需要页大小
        if(bit_idx_start == -1) return NULL;
        while(cnt<pg_cnt){
            bitmap_set(&cur->userprog_vaddr.vaddr_bitmap,bit_idx_start+cnt++,1);
        }
        vaddr_start = cur->userprog_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
        /*(0xc0000000 - PG_SIZE)作为用户3级栈已经在start_process被分配了*/
        ASSERT((uint32_t)vaddr_start < (0xc0000000 - PG_SIZE));
    }
    return (void*)vaddr_start;
}

/*得看书205页*/
/*到虚拟地址 vaddr 对应的 pte 指针*/
uint32_t* pte_ptr(uint32_t vaddr){
    /*访问到页表自己 再用页目录项 pde （页目录内页袤的索引）作为pte的索引访问到页表 再用pte的索引作为页内偏移*/
    uint32_t* pte = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000)>>10) + PTE_IDX(vaddr) * 4); //这里得到的是新的虚拟地址 能够访问到它的页表物理地址
    return pte;
}

/*得到虚拟地址 vaddr 对应的 pde 的指针*/
uint32_t* pde_ptr (uint32_t vaddr){
    /* Oxfffff 用来访问到页表本身所在的地址 */
    uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr)*4);
    return pde;
}

/*在 m_pool 指向的物理内存池中分配 1 个物理页，成功则返回页框的物理地址，失败则返回 NULL */
static void* palloc(struct pool* m_pool){
    /*扫描或设置位图要保证原子操作*/
    int bit_idx = bitmap_scan(&m_pool->pool_bitmap,1);  //找到一个物理页
    if(bit_idx == -1)
        return NULL;
    bitmap_set(&m_pool->pool_bitmap,bit_idx,1); //将这个位置1,表示以用
    uint32_t page_phyaddr = ((bit_idx * PG_SIZE) + m_pool->phy_addr_start);
    return (void*)page_phyaddr;
}

/*表中添加虚拟地址＿vaddr 与物理地址＿page_phyaddr 的映射*/
static void page_table_add(void* _vaddr, void* _page_phyaddr){
    uint32_t vaddr = (uint32_t)_vaddr, page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t* pde = pde_ptr(vaddr);
    uint32_t* pte = pte_ptr(vaddr);

    /** 注意
     * 执行＊pte ，会访问到空的pde。所以确保pde创建完成后才能执行＊pte,
     * 否则会引发 page_fault。因此在＊pde 为 0 时，
     * pte 只能出现在下面 else 语句块中的＊ pde 后面。
    */
   /*先在页目录内判断 目录项的 p 位，若为 1 ，则表示该表已存在*/
   if(*pde & 0x00000001){
    //页目录项和页表项的第 0 位为 p ，此处判断目录项是否存在
    ASSERT(!(*pte & 0x00000001));
    if(!(*pte & 0x00000001)){
        //只要是创建页表， pte 就应该不存在，多判断一下放心
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }else{  //／目前应该不会执行到这，因为上面的 ASSERT 会先执行
        PANIC("pte repeat");
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
   }else{//页吕录项不存在，所以要先创建页目录再创建页表项
    //页表中用到的页框一律从内核空间分配
    uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);
    *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);

    /** 分配到的物理页地址 pde_phyaddr 对应的物理内存清 O,
     * 避免里面的陈旧数据变成了页表项，从而让页表混乱。
     * 访问到 pde 对应的物理地址，用 pte 取高 20 位便可。
     * 因为 pte 基于该 pde 对应的物理地址内再寻址，
     * 把低 12 位置。便是该 pde 对应的物理页的起始＊
    */
    memset((void*)((int)pte & 0xfffff000),0,PG_SIZE); //这里要的地址必须是pte的虚拟地址
    ASSERT(!(*pte & 0x00000001));
    *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
   }
}

/*分配 pg_cnt 个页空间，成功则返回起始虚拟地址，失败时返回 NULL*/
void* malloc_page(enum pool_flags pf, uint32_t pg_cnt){
    ASSERT(pg_cnt > 0 && pg_cnt < 3840);

    /** malloc__page 的原理是三个动作的合成：
     * 1 通过 vaddr_get 在虚拟内存池中申请虚拟地址
     * 2 通过 palloc 在物理内存池中申请物理页
     * 3 通过 page_table_add 将以上得到的虚拟地址和物理地址在页表中完成映射
    */

    void* vaddr_start = vaddr_get(pf,pg_cnt);
    if(vaddr_start==NULL)   return NULL;

    uint32_t vaddr = (uint32_t)vaddr_start, cnt = pg_cnt;
    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;

    /*因为虚拟地址是连续的，但物理地址可以是不连续的，所以逐个做映射*/
    while(cnt-->0){
        void* page_phyaddr = palloc(mem_pool);
        if(page_phyaddr == NULL)    //／失败时要将曾经已申请的虚拟地址和物理页全部回滚，在将来完成内存回收时再补充
            return NULL;
        page_table_add((void*)vaddr,page_phyaddr);  //在页表中映射
        vaddr+=PG_SIZE;
    }
    return vaddr_start;
}

/*从内核物理内存池中申请 1 页内存，成功则返回其虚拟地址，失败则返回 NULL */
void* get_kernel_pages(uint32_t pg_cnt){
    lock_acquire(&kernel_pool.lock);
    void* vaddr = malloc_page(PF_KERNEL, pg_cnt);
    if(vaddr != NULL)   //若分配的地址不为空，将页框清0后返回
        memset(vaddr,0,pg_cnt*PG_SIZE);
    lock_release(&kernel_pool.lock);
    return vaddr;
}

/*在用户空间申请4k内存，并返回其虚拟地址*/
void* get_user_pages(uint32_t pg_cnt){
    lock_acquire(&user_pool.lock);
    void* vaddr = malloc_page(PF_USER,pg_cnt);
    memset(vaddr,0,pg_cnt*PG_SIZE);
    lock_release(&user_pool.lock);
    return vaddr;
}

/*将地址vaddr与pf池中的物理地址关联起来，仅支持一页空间分配*/
void* get_a_page(enum pool_flags pf, uint32_t vaddr){
    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
    lock_acquire(&mem_pool->lock);

    /*先将虚拟地址对应的位图置1*/
    struct task_struct* cur = running_thread();
    int32_t bit_idx = -1;

/*当前是用户进程申请用户内存，就修改用户进程自己的虚拟地址位图*/
    if(cur->pgdir!=NULL && pf == PF_USER){
        bit_idx = (vaddr - cur->userprog_vaddr.vaddr_start) / PG_SIZE;
        ASSERT(bit_idx>0);
        bitmap_set(&cur->userprog_vaddr.vaddr_bitmap,bit_idx,1);
    }else if(cur->pgdir==NULL && pf == PF_KERNEL){
/*如果是内核线程申请内核内存，就修改 kernel_vaddr */
        bit_idx = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
        ASSERT(bit_idx>0);
        bitmap_set(&kernel_vaddr.vaddr_bitmap,bit_idx,1);
    }else{
        PANIC("get_a_page:not allow kernel alloc userspace or user alloc kernelspace by get_a_page");
    }

    void* page_phyaddr = palloc(mem_pool);
    if(page_phyaddr == NULL)
        return NULL;
    page_table_add((void*)vaddr,page_phyaddr);
    lock_release(&mem_pool->lock);
    return (void*)vaddr;
}

/*得到虚拟地址映射到的物理地址*/
uint32_t addr_v2p(uint32_t vaddr){
    uint32_t* pte = pte_ptr(vaddr);
    /** 
     * (* pte)的值是页表所在的物理页框地址，
     * 去掉其低 12 位的页表项属性＋虚拟地址 vaddr 的低 12 位
    */
   return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));
}

/*初始化内存池*/
static void mem_pool_init(uint32_t all_mem){
    put_str("mem_pool_init start\n");
    uint32_t page_table_size = PG_SIZE * 256;//页表大小：1 页的页目录表＋第 0 和第 768 个页目录项指向同一个页表＋第 769～ 1022 个页 目录项共指向 254 个页表，共 256 个页框
    uint32_t used_mem = page_table_size + 0x100000; //0x100000为低端1MB内存
    uint32_t free_mem = all_mem - used_mem;
    uint16_t all_free_pages = free_mem / PG_SIZE; //1 页为 4KB，不管总内存是不是 4k 的倍数
    //对于以页为单位的内存分配策略，不足 1 页的内存不用考虑了

    uint16_t kernel_free_pages = all_free_pages / 2;
    uint16_t user_free_pages = all_free_pages - kernel_free_pages;

    /*为简化位图操作，余数不处理，坏处是这样做会丢内存。好处是不用做内存的越界检查，因为位图表示的内存少于实际物理内存＊*/
    uint32_t kbm_length = kernel_free_pages / 8;    //kernel Bitmap的长度，位图中的一位表示一页，以字节为单位
    uint32_t ubm_length = user_free_pages / 8;     //user Bitmap长度
    
    uint32_t kp_start = used_mem;   //kernel pool start 内核内存次的起始地址
    uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE; //user pool start 内核内存次的起始地址

    kernel_pool.phy_addr_start = kp_start;
    user_pool.phy_addr_start = up_start;

    kernel_pool.pool_size = kernel_free_pages * PG_SIZE;
    user_pool.pool_size = user_free_pages * PG_SIZE;

    kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;
    user_pool.pool_bitmap.btmp_bytes_len = ubm_length;

    /** 内核内存池和用户内存池位图
     * 位图是全局的数据，长度不固定。
     * 全局或静态的数组需要在编译时知道其长度，
     * 而我们需要根据总内存大小算出需要多少字节，
     * 所以改为指定一块内存来生成位图。
    */
   //内核使用的最高地址是 Oxc009f000，这是主线程的校地址
   //（内核的大小预计为 70KB 左右）
   //32MB内存占用的位图是2KB
   //／内核内存池的位图先定在 MEM_BITMAP_BASE(Oxc009a000 ）处
   kernel_pool.pool_bitmap.bits = (void*)MEM_BITMAP_BASE;
   /*用户内存池的位图紧跟在内核内存池位图之后*/
   user_pool.pool_bitmap.bits = (void*)(MEM_BITMAP_BASE+kbm_length);

   put_str(" kernel_pool_bitmap_start:");
   put_int((int)kernel_pool.pool_bitmap.bits);
   put_str(" kernel_pool_phy_addr_start:");
   put_int(kernel_pool.phy_addr_start);
   put_str("\n");
   put_str(" user_pool_bitmap_start:");
   put_int((int)user_pool.pool_bitmap.bits);
   put_str(" user_pool_phy_addr_start:");
   put_int(user_pool.phy_addr_start);

   /*将位图置0*/
   bitmap_init(&kernel_pool.pool_bitmap);
   bitmap_init(&user_pool.pool_bitmap);

   lock_init(&kernel_pool.lock);
   lock_init(&user_pool.lock);
   /*下面初始化内核虚拟地址的位图，按实际物理内存大小生成数组。*/
   kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length; //用于维护内核堆的虚拟地址，所以要和内核内存池大小一致

   /*位图的数组指向一块未使用的内存，目前定位在内核内存池和用户内存池之外＊*/
   kernel_vaddr.vaddr_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length + ubm_length);
   kernel_vaddr.vaddr_start = K_HEAP_START;
   bitmap_init(&kernel_vaddr.vaddr_bitmap);
   put_str(" mem_pool_ini t done \n");
}

/*未malloc做准备*/
void block_desc_init(struct mem_block_desc* desc_array){
    uint16_t desc_idx, block_size = 16;
    /*初始化每个mem_block_desc描述符*/
    for(desc_idx=0;desc_idx<DESC_CNT;desc_idx++){
        desc_array[desc_idx].block_size = block_size;

        /*初始化arena中的内存块数量*/
        desc_array[desc_idx].blocks_per_arena = (PG_SIZE - sizeof(struct arena)) / block_size;
        list_init(&desc_array[desc_idx].free_list);
        block_size*=2;  //更新为下一个规格
    }
}

/*返回arena中第idx个内存块地址*/
static struct mem_block* arena2block(struct arena* a, uint32_t idex){
    return (struct mem_block*)((uint32_t)a+sizeof(struct arena)+idex*a->desc->block_size);
}

/*返回内存块 b 所在的 arena 地址*/
static struct arena* block2arena(struct mem_block* b){
    return (struct arena*)((uint32_t)b & 0xfffff000);
}

/*在堆中申请size字节内存*/
void* sys_malloc(uint32_t size){
    enum pool_flags PF;
    struct pool* mem_pool;
    uint32_t pool_size;
    struct mem_block_desc* descs;
    struct task_struct* cur_thread = running_thread();

    /*判断用那个内存池*/
    if(cur_thread->pgdir == NULL){//若为内核线程
        PF = PF_KERNEL;
        pool_size = kernel_pool.pool_size;
        mem_pool  = &kernel_pool;
        descs = k_block_descs;
    }else{//用户进程PCB中的pgdir会在为其分配页表时创建
        PF = PF_USER;
        pool_size = user_pool.pool_size;
        mem_pool = &user_pool;
        descs = cur_thread->u_block_desc;
    }

    /*若申请的内存不在内存池容量范围内，则直接返回NULL*/
    if(!(size>0&&size<pool_size)){
        return NULL;
    }
    struct arena* a;
    struct mem_block* b;
    lock_acquire(&mem_pool->lock);

    /*超过最大内存1024，就分配页框*/
    if(size>1024){
        uint32_t page_cnt = DIV_ROUND_UP(size + sizeof(struct arena), PG_SIZE);//向上取整需要的页框数
        
        a = malloc_page(PF,page_cnt);

        if(a!=NULL){
            memset(a,0,page_cnt * PG_SIZE); //将分配的内存清0

            /*对于分配的大块页框，将desc置为NULL，cnt置为页框数，large置为true*/
            a->desc = NULL;
            a->cnt = page_cnt;
            a->large = true;
            lock_release(&mem_pool->lock);
            return (void*)(a+1);    //跨过arena大小把剩下的内存返回
        }else{
            lock_acquire(&mem_pool->lock);
            return NULL;
        }  
    }else{  //若申请的内存小于等于 1024
        //可在各种规格的 mem_block_desc 中去适配
        uint8_t desc_idex;
        /*内存块描述符中匹配合适的内存块规格*/
        for(desc_idex=0;desc_idex<DESC_CNT;desc_idex++){
            if(size<=descs[desc_idex].block_size){
                //从小往大，找到后退出
                break;
            }
        }
        /*若mem_block_desc的free_list中已经没有可用的mem_block,就创建新的arena提供mem_block*/
        if(list_empty(&descs[desc_idex].free_list)){
            a = malloc_page(PF,1);  //分配1页框作为arena
            if(a == NULL){
                lock_release(&mem_pool->lock);
                return NULL;
            }
            memset(a,0,PG_SIZE);
            /*对于分配的小内存块，将desc置为相应内存块描述符，cnt置为arena可以用的内存块数量，large置为false*/
            a->desc = &descs[desc_idex];
            a->large = false;
            a->cnt = descs[desc_idex].blocks_per_arena;
            uint32_t block_idx;

            enum intr_status old_status = intr_disable();

            /*开始将arena拆分为内存块，并添加到内存块描述符的free_list中*/
            for(block_idx=0;block_idx<descs[desc_idex].blocks_per_arena;block_idx++){
                b = arena2block(a,block_idx);
                ASSERT(!elem_find(&a->desc->free_list,&b->free_elem));
                list_append(&a->desc->free_list,&b->free_elem);
            }
            intr_set_status(old_status);
        }
        /*开始分配内存块*/
        b = elem2entry(struct mem_block, free_elem, list_pop(&(descs[desc_idex].free_list)));
        memset(b,0,descs[desc_idex].block_size);
        
        a = block2arena(b); //获取内存块b所在的arena
        a->cnt--;   // 将此 arena 中的空闲内存块数减 1
        lock_release(&mem_pool->lock);
        return (void*)b;
    }
    
}

/*将物理地址 pg_phy_addr 回收到物理内存池*/
void pfree(uint32_t pg_phy_addr){
    struct pool* mem_pool;
    uint32_t bit_idx = 0;
    if(pg_phy_addr>=user_pool.phy_addr_start){  //用户物理内存池
        mem_pool = &user_pool;
        bit_idx = (pg_phy_addr - user_pool.phy_addr_start) / PG_SIZE;
    }else{  //内核物理内存池
        mem_pool = &kernel_pool;
        bit_idx = (pg_phy_addr - kernel_pool.phy_addr_start) / PG_SIZE;
    }
    bitmap_set(&mem_pool->pool_bitmap,bit_idx,0);   //将位图中该位清0
}

/*掉页表中虚拟地址vaddr的映射，只去掉vaddr对应的pte*/
static void page_table_pte_remove(uint32_t vaddr){
    uint32_t* pte = pte_ptr(vaddr);
    *pte &= ~PG_P_1;    //  将pte置为0
    asm volatile("invlpg %0"::"m" (vaddr):"memory");    //更新tlb
}

/*在虚拟地址池中释放以vaddr起始的连续pg_cnt个虚拟页*/
static void vaddr_remove(enum pool_flags pf, void* _vaddr, uint32_t pg_cnt){
    uint32_t bit_idx_start = 0, vaddr = (uint32_t)_vaddr, cnt = 0;

    if(pf == PF_KERNEL) //内核虚拟内存池
    {
        bit_idx_start = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
        while (cnt < pg_cnt)
        {
            bitmap_set(&kernel_vaddr.vaddr_bitmap,bit_idx_start+cnt++,0);
        }
    }else{  //用户虚拟内存池
        struct task_struct* cur_thread = running_thread();
        bit_idx_start = (vaddr - cur_thread->userprog_vaddr.vaddr_start) / PG_SIZE;
        while (cnt<pg_cnt)
        {
            bitmap_set(&cur_thread->userprog_vaddr.vaddr_bitmap,bit_idx_start+cnt++,0);
        }
        
    }   
}

/*释放以虚拟地址vaddr为起始的cn个物理页框*/
void mfree_page(enum pool_flags pf, void* _vaddr, uint32_t pg_cnt){
    uint32_t pg_phy_addr;
    uint32_t vaddr = (int32_t)_vaddr, page_cnt = 0;
    ASSERT(pg_cnt >= 1 && vaddr % PG_SIZE == 0);
    pg_phy_addr = addr_v2p(vaddr);    //获得虚拟地址vaddr对应的物理地址

    /*确保释放的物理内存在低端1MB+1KB大小的页目录+1KB大小的页表地址范围外*/
    ASSERT((pg_phy_addr % PG_SIZE) == 0 && pg_phy_addr >= 0x102000);
    
    /*判断pg_phy_addr属于用户物理内存池还是内核物理内存池*/
    if(pg_phy_addr >= user_pool.phy_addr_start){    //位于user_pool内存池
        vaddr -= PG_SIZE;
        while(page_cnt<pg_cnt){
            vaddr+=PG_SIZE;
            pg_phy_addr = addr_v2p(vaddr);

            /*确保物理地址属于用户物理内存池*/
            ASSERT((pg_phy_addr % PG_SIZE) == 0 && pg_phy_addr >= user_pool.phy_addr_start);

            /*现将对应的物理页框归还到内存池*/
            pfree(pg_phy_addr);

            /*再从页表中清除此虚拟地址所在的页表项pte*/
            page_table_pte_remove(pg_phy_addr);

            page_cnt++;
        }
        /*清空虚拟地址的位图中的相应位*/
        vaddr_remove(pf, _vaddr, pg_cnt);
    }else{  //位于kernel_pool内存池
        vaddr -= PG_SIZE;
        while(page_cnt < pg_cnt){
            vaddr+=PG_SIZE;
            pg_phy_addr = addr_v2p(vaddr);
            /*确保待释放的物理内存只属于内核物理内存池*/
            ASSERT((pg_phy_addr % PG_SIZE)==0 && pg_phy_addr>=kernel_pool.phy_addr_start && pg_phy_addr<user_pool.phy_addr_start);

            /*现将对应的物理页框归还到内存池*/
            pfree(pg_phy_addr);

            /*再从页表中清除此虚拟地址所在的页表项pte*/
            page_table_pte_remove(vaddr);

            page_cnt++;
        }
        /*清空虚拟地址的位图中的相应位*/
        vaddr_remove(pf, _vaddr, pg_cnt);
    }
}

/*回收内存ptr*/
void sys_free(void* ptr){
    ASSERT(ptr!=NULL);
    if(ptr!=NULL){
        enum pool_flags PF;
        struct pool* mem_pool;

        /*判断是线程还是进程*/
        if(running_thread()->pgdir == NULL){
            ASSERT((uint32_t)ptr >= K_HEAP_START);
            PF = PF_KERNEL;
            mem_pool = &kernel_pool;
        }else{
            PF = PF_USER;
            mem_pool = &user_pool;
        }

        lock_acquire(&mem_pool->lock);
        struct mem_block* b = ptr;
        struct arena* a = block2arena(b);
        //把mem_block转换为arena，获取元信息
        ASSERT(a->large == 0 || a->large == 1);
        if(a->desc == NULL && a->large == true){    //大于1024的内存
            mfree_page(PF,a,a->cnt);
        }else{      //小于1024的内存
            /*先将内存块回收到free_list*/
            list_append(&a->desc->free_list, &b->free_elem);

            /*在判断此arena中的内存块是否都是空闲，如果是就释放arena*/
            if(++a->cnt == a->desc->blocks_per_arena){
                uint32_t block_idx;
                for(block_idx=0;block_idx<a->desc->blocks_per_arena;block_idx++){
                    struct mem_block* b = arena2block(a,block_idx);
                    ASSERT(elem_find(&a->desc->free_list,&b->free_elem));
                    list_remove(&b->free_elem);
                }
                mfree_page(PF,a,1);
            }
        }
        lock_release(&mem_pool->lock);
    }
}

/*内存管理部分初始化入口*/
void mem_init(){
    put_str("mem_init start\n");
    uint32_t mem_bytes_total = (*(uint32_t*)(0xb00));
    mem_pool_init(mem_bytes_total);//初始化内存池
    block_desc_init(k_block_descs); //初始化 mem_block_desc数组descs ，为malloc做准备
    put_str("mem_init done\n");
}