/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-08 10:26:42
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-09 11:28:37
 * @FilePath: /OS/chapter14/14.4/fs/file.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "file.h"
#include "stdint.h"
#include "stdio-kernel.h"
#include "stdio.h"
#include "thread.h"
#include "bitmap.h"
#include "ide.h"
#include "fs.h"
/*文件表*/
struct file file_table[MAX_FILE_OPEN];

/*从文件表file_table中获取一个空闲位，成功返回下标，失败返回-1*/
int32_t get_free_slot_in_global(void){
    uint32_t fd_idx = 3;
    while(fd_idx< MAX_FILE_OPEN){
        if(file_table[fd_idx].fd_inode == NULL){
            break;
        }
        fd_idx++;
    }
    if(fd_idx==MAX_FILE_OPEN){
        printk("exceed max open files\n");
        return -1;
    }
    return fd_idx;
}

/*将全局描述符下标安装到进程或线程自己的文件描述符数组fd_table中，成功返回小标，失败返回-1*/
int32_t pcb_fd_install(int32_t globa_fd_idx){
    struct task_struct* cur = running_thread();
    uint8_t local_fd_idx = 3;   //跨过stdin,stout,stderr
    while (local_fd_idx<MAX_FILES_OPEN_PER_PROC)
    {
        if(cur->fd_table[local_fd_idx] == -1){
            //-1表示free_slot，可以用
            cur->fd_table[local_fd_idx] = globa_fd_idx;
            break;
        }
        local_fd_idx++;
    }
    if(local_fd_idx==MAX_FILES_OPEN_PER_PROC){
        printk("exceed max open file_per_proc\n");
        return -1;
    }
    return local_fd_idx;
}

/*分配一个i结点，返回i结点号*/
int32_t inode_bitmap_alloc(struct partition* part){
    int32_t bit_idx = bitmap_scan(&part->inode_bitmap,1);
    if(bit_idx==-1) return -1;
    
    bitmap_set(&part->inode_bitmap,bit_idx,1);
    return bit_idx;
}

/*分配1个扇区，返回其扇区地址*/
int32_t block_bitmap_alloc(struct partition* part){
    int32_t bit_idx = bitmap_scan(&part->block_bitmap,1);
    if(bit_idx==-1) return -1;
    
    bitmap_set(&part->inode_bitmap,bit_idx,1);
    /*和inode_bitmap_alloc不同，此处返回的不是位图索引，而是具体可用的扇区地址*/
    return (part->sb->data_start_lba+bit_idx);
}

/*将内存中bitmao第bit_idx位所在的512字节同步到硬盘*/
void bitmap_sync(struct partition* part, uint32_t bit_idx, uint8_t btmp){
    uint32_t off_sec = bit_idx / 4096;  //本i结点缩影相对于位图的扇区偏移量，4096 是因为一个扇区的大小通常为 512 字节 * 8 = 4096 位。因此，bit_idx 除以 4096 得到的结果是该位所在的扇区索引
    uint32_t off_size = off_sec * BLOCK_SIZE;   //本i结点索引相对于位图的字节偏移量，乘以BLOCK_SIZE是因为位图中的每一位都表示一个块，而每个块的大小为 512 字节。所以，通过将 off_sec 乘以 512，可以得到位图中目标位所在的字节偏移量。

    uint32_t sec_lba;
    uint8_t* bitmap_off;

    /*需要被同步到硬盘的位图只有inode_bitmap和block_bitmap*/
    switch(btmp){
        case INODE_BITMAP:
            sec_lba = part->sb->inode_bitmap_lba + off_sec;
            bitmap_off = part->inode_bitmap.bits + off_size;
            break;
        
        case BLOCK_BITMAP:
            sec_lba = part->sb->block_bitmap_lba + off_sec;
            bitmap_off = part->block_bitmap.bits + off_size;
            break;
    }
    ide_write(part->my_disk,sec_lba,bitmap_off,1);
}