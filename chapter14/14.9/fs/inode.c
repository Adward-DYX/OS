/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-08 09:08:57
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-09 11:30:05
 * @FilePath: /OS/chapter14/14.4/fs/inode.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "inode.h"
#include "stdint.h"
#include "global.h"
#include "debug.h"
#include "ide.h"
#include "list.h"
#include "thread.h"
#include "memory.h"
#include "string.h"


/*用来存储inode位置*/
struct inode_position{
    bool two_swc;   //inode是否跨扇区
    uint32_t sec_lba;   //inode所在的扇区编号
    uint32_t off_size;  //inode在扇区内的字节偏移量
};

/*获取inode所在的扇区和扇区内的偏移量*/
static void inode_locate(struct partition* part, uint32_t inode_no, struct inode_position* inode_pos){
    /*inode_table在硬盘上是连续的*/
    ASSERT(inode_no < 4096);
    uint32_t inode_table_lba = part->sb->inode_table_lba;

    uint32_t inode_size = sizeof(struct inode);
    uint32_t off_size = inode_no * inode_size;//第inode_no号i结点相对于inode_table_lba的字节偏移量
    uint32_t off_sec = off_size / 512; //第inode_no号i结点相对于inode_table_lba的扇区偏移量
    uint32_t off_size_in_sec = off_size % 512; //待查找的inode所在扇区中的起始地址
    
    /*判断i结点是否跨域2个扇区*/
    uint32_t left_in_sec = 512 - off_size_in_sec;
    if(left_in_sec < inode_size){
        //若扇区内剩下的空间不足以容纳一个inode ，必然是i结点跨越了2个扇区
        inode_pos->two_swc = true;
    }else{  //否则，所查找的 inode 未跨扇区
        inode_pos->two_swc = false;
    }
    inode_pos->sec_lba = inode_table_lba + off_sec;
    inode_pos->off_size = off_size_in_sec;
}

/*将inode写入到分区part*/
void inode_sync(struct partition* part,struct inode* inode, void* io_buf){
    //io_buf是用于硬盘IO的缓冲区
    uint8_t inode_no = inode->i_no;
    struct inode_position inode_pos;
    inode_locate(part,inode_no,&inode_pos); //inode位置信息存入inode_pos
    ASSERT(inode_pos.sec_lba <= (part->start_lba + part->sec_cnt));

    /*硬盘中的inode中的成员inode_tag和i_open_cnts是不需要的，他们只在内存中记录链表位置和被多少进程共享*/
    struct inode pure_inode;
    memcpy(&pure_inode,inode,sizeof(struct inode));
    
    /*以下inode的三个成员只存在于内存中，现在将inode同步到硬盘，清理掉这三项中就好了*/
    pure_inode.i_open_cnts = 0;
    pure_inode.write_deny = false;  //置为false，以保证在硬盘中读出时为可写
    pure_inode.inode_tag.prev = pure_inode.inode_tag.next = NULL;

    char* inode_buf = (char*)io_buf;
    if(inode_pos.two_swc){
        //／若是跨了两个扇区，就要读出两个扇区再写入两个扇区

        /*读写硬盘是以扇区为单位，若写入的数据小于一扇区，要将原硬盘上的内容先读出来离和新数据拼成一扇区后再写入*/
        ide_read(part->my_disk,inode_pos.sec_lba,inode_buf,2);  //inode在format中写入硬盘时是连续写入的，所以读入2块扇区
        
        /*开始将待写入的 inode 拼入到这 2 个扇区中的相应位置＊*/
        memcpy((inode_buf + inode_pos.off_size), &pure_inode, sizeof(struct inode));

        /*将拼接好的数据在写入磁盘*/
        ide_write(part->my_disk,inode_pos.sec_lba,inode_buf,2);
    }else{
        ide_read(part->my_disk,inode_pos.sec_lba,inode_buf,1);
        memcpy((inode_buf + inode_pos.off_size), &pure_inode, sizeof(struct inode));
        ide_write(part->my_disk,inode_pos.sec_lba,inode_buf,1);
    }
}

/*根据i结点号返回相应的i结点*/
struct inode* inode_open(struct partition* part, uint32_t inode_no){
    /*先在已经打开的inode链表中找inode，次链表是为提速创建的缓冲区*/
    //在mount_partition中初始化的这个队列，一开始里面谁都没有
    struct list_elem* elem = part->open_inodes.head.next;
    struct inode* inode_found;
    
    while (elem!=&part->open_inodes.tail)
    {
        inode_found = elem2entry(struct inode, inode_tag, elem);
        if(inode_found->i_no == inode_no){
            inode_found->i_open_cnts++;
            return inode_found;
        }
        elem = elem->next;
    }

    /*由于open_inodes链表找不到，所以我们就要从硬盘上读入此inode并加入到此链表*/
    struct inode_position inode_pos;
    
    /*inode位置信息会存入inode_pos,包括 inode 所在扇区地址和扇区内的字节偏移量*/
    inode_locate(part,inode_no,&inode_pos);
    /** 
     * 为使通过 sys_malloc 创建的新 inode 被所有任务共享，
     * 需要将 inode 置于内核空间，故需要临时
     * 将 cur_pbc->pgdir 置为 NULL
     * 因为sys_malloc是通过判断这个cur_pbc->pgdir来观察他是否是内核线程，因为只有用户进程才有这用户进程自己的虚拟页表
    */
    struct task_struct* cur = running_thread();
    uint32_t* cur_pagedit_bak = cur->pgdir;
    cur->pgdir = NULL;
    /*以上三行代码完成后下面分配的内存将位于内核空间*/
    inode_found = (struct inode*)sys_malloc(sizeof(struct inode));
    /*恢复pgdir*/
    cur->pgdir = cur_pagedit_bak;

    char* inode_buf;
    if(inode_pos.two_swc){
        //考虑跨扇区的情况
        inode_buf = (char*)sys_malloc(1024);
        /*i结点表是被partition_format函数连续写入扇区的所以下面可以被连续读出来*/
        ide_read(part->my_disk,inode_pos.sec_lba,inode_buf,2);
    }else{
        //否则所查拢的 inode 未跨扇区，一个扇区大小的缓冲区足够
        inode_buf = (char*)sys_malloc(512);
        ide_read(part->my_disk,inode_pos.sec_lba,inode_buf,1);
    }

    memcpy(inode_found,inode_buf+inode_pos.off_size,sizeof(struct inode));
    
    /*因为以后可能要用到此inode，故将其插入到队首便于提前检索到*/
    list_push(&part->open_inodes,&inode_found->inode_tag);
    inode_found->i_open_cnts = 1;
    sys_free(inode_buf);
    return inode_found;
}


/*关闭inode或减少inode的打开数*/
void inode_close(struct inode* inode){
    /*若没有进程再打开此文件，将此inode去掉并释放空间*/
    enum intr_status old_status = intr_disable();
    if(--inode->i_open_cnts == 0){
        list_remove(&inode->inode_tag); //将i结点从part->open_inodes中去掉
        /**
         * inode_open时为实现 inode 被所有进程共享，
         * 已经在 sys_malloc 为 inode 分配了内核空间，
         * 释放工 inode 时也要确保释放的是内核内存池
        */
       struct task_struct* cur = running_thread();
       uint32_t* cur_pagedir_bak = cur->pgdir;
       cur->pgdir = NULL;
       sys_free(inode);
       cur->pgdir = cur_pagedir_bak;
    }
    intr_set_status(old_status);
}

/*初始化new_inode*/
void inode_init(uint32_t inode_no, struct inode* new_inode){
    new_inode->i_no = inode_no;
    new_inode->i_size = 0;
    new_inode->i_open_cnts = 0;
    new_inode->write_deny = false;

    /*初始化块索引数组i_sector*/
    uint8_t sec_idx = 0;
    while(sec_idx<13){
        /*i_sectors[12]为一级间接块地址*/
        new_inode->i_sectors[sec_idx] = 0;
        sec_idx++;
    }
}