/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-07 10:26:42
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-12 18:01:58
 * @FilePath: /OS/chapter14/14.2/fs/inode.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __FS_INODE_H
#define __FS_INODE_H

#include "stdint.h"
#include "list.h"
#include "ide.h"


/*inode结构*/
struct inode{
    uint32_t i_no;  //inode编号
    /*当次inode是文件时，i_size是指文件大小，若是目录，则是指该目录下所有目录项大小之和*/
    uint32_t i_size;

    uint32_t i_open_cnts;   //记录此文件被打开的次数
    bool write_deny;    //写文件不能并行，进程写文件前检查此标识

    /* 咱们的块大小就是 1 扇区 ,i_sectors[0-11]是直接块，i_sectors[12]用来存储一级间接块指针*/
    /*不过稍微不同的是扇区大小是 512 字节，并且块地址用 4 字节来表示，因此咱们支持的一级间接块数量是 128 个，即咱们总共支持 128+12= 140 个块（扇区）*/
    uint32_t i_sectors[13];
    /*由于 inode 是从硬盘上保存的 ， 文件被打开时， 肯定是先要从硬盘上载入其 inode，但硬盘比较
    慢， 为了避免下次再打开该文件时还要从硬盘上重复载入 inode，应该在该文件第一次被打开时就将其 inode
    加入到 内存缓存中，每次打开一个文件时，先在此缓冲中查找相关的 inode ， 如果有就直接使用， 否则再从硬
    盘上读取 inode，然后再加入此缓存。 这个内存缓存就是“己打开的 inode 队列”*/
    struct list_elem inode_tag;
};

void inode_sync(struct partition* part,struct inode* inode, void* io_buf);
struct inode* inode_open(struct partition* part, uint32_t inode_no);
void inode_close(struct inode* inode);
void inode_init(uint32_t inode_no, struct inode* new_inode);
void inode_release(struct partition* part, uint32_t inode_no);
#endif // !__FS_INODE_H
