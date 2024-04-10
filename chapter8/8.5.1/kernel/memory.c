/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-09 14:26:27
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-09 17:33:33
 * @FilePath: /OS/chapter8/8.4/kernel/memory.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "memory.h"
#include "stdint.h"
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
/*存池结构，生成两个实例用于管理内核内存池和用户内存池*/
struct pool{
    struct bitmap pool_bitmap;   //本内存池周到的位图结构， 用于管理物理内存
    uint32_t phy_addr_start;    //本内存池所管理物理内存的起始地址
    uint32_t pool_size;      //本内存池字节容量
};

struct pool kernel_pool, user_pool;  //生成内核内存池和用户内存池
struct virtual_addr kernel_vaddr;    //此结构用来给内核分配虚拟地址

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