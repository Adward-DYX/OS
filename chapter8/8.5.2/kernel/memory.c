/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-09 14:26:27
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-10 10:43:14
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
};

struct pool kernel_pool, user_pool;  //生成内核内存池和用户内存池
struct virtual_addr kernel_vaddr;    //此结构用来给内核分配虚拟地址


/*在 pf 表示的虚拟内存池中申请 pg_cnt 个虚拟页，成功则返回虚拟页的起始地址，失败则返回 NULL*/
static void* vaddr_get(enum pool_flags pf, uint32_t pg_cnt){
    int vaddr_start = 0, bit_idx_start = -1;
    uint32_t cnt = 0;
    if(pf==PF_KERNEL){
        bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap,pg_cnt); //  返回的是成功位置的下标需要*页大小
        if(bit_idx_start == -1) return -1;
        while(cnt < pg_cnt){
            bitmap_set(&kernel_vaddr.vaddr_bitmap,bit_idx_start+cnt++,1);
        }
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
    }else{
        //用户内存池，将来实现用户进程在补充
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
    uint32_t* pde = pde_ptr(_vaddr);
    uint32_t* pte = pte_ptr(_vaddr);

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
    void* vaddr = malloc_page(PF_KERNEL, pg_cnt);
    if(vaddr != NULL)   //若分配的地址不为空，将页框清0后返回
        memset(vaddr,0,pg_cnt*PG_SIZE);
    return vaddr;
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

   /*下面初始化内核虚拟地址的位图，按实际物理内存大小生成数组。*/
   kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length; //用于维护内核堆的虚拟地址，所以要和内核内存池大小一致

   /*位图的数组指向一块未使用的内存，目前定位在内核内存池和用户内存池之外＊*/
   kernel_vaddr.vaddr_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length + ubm_length);
   kernel_vaddr.vaddr_start = K_HEAP_START;
   bitmap_init(&kernel_vaddr.vaddr_bitmap);
   put_str(" mem_pool_ini t done \n");
}

/*内存管理部分初始化入口*/
void mem_init(){
    put_str("mem_init start\n");
    uint32_t mem_bytes_total = (*(uint32_t*)(0xb00));
    mem_pool_init(mem_bytes_total);
    put_str("mem_init done\n");
}