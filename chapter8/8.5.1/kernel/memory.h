/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-09 14:17:52
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-09 16:45:14
 * @FilePath: /OS/chapter8/8.4/kernel/memory.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"

/*虚拟地址池，用于虚拟地址管理*/
struct virtual_addr
{
    /* data */
    struct bitmap vaddr_bitmap; //虚拟地址用到的位图结构
    uint32_t vaddr_start;   //虚拟地址起始地址
};

extern struct pool kernel_pool, user_pool;
void mem_init(void);
#endif // !__KERNEL_MEMORY_H
