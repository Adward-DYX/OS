/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-07 10:18:02
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-07 11:24:50
 * @FilePath: /OS/chapter14/14.2/fs/super_block.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __FS_SUPER_BLOCK_H
#define __FS_SUPER_BLOCK_H
#include "global.h"
#include "stdint.h"

/*超级块*/
struct super_block{
    uint32_t magic; //用来标识文件系统类型，支持多文件系统的操作系统通过此标志来识别文件系统的类型
    uint32_t sec_cnt;   //本分区总共的扇区数
    uint32_t inode_cnt; //本分区中ino数量
    uint32_t part_lba_base; //本分区的起始lba地址

    uint32_t block_bitmap_lba;  //块位图本身起始扇区地址
    uint32_t block_bitmap_sects; //扇区位图本身所占用的扇区数量

    uint32_t inode_bitmap_lba;  //i结点位图起始扇区lba地址
    uint32_t inode_bitmap_sects;    //i结点位图占用的扇区数量
    
    uint32_t inode_table_lba;   //i结点表起始扇区lba地址
    uint32_t inode_table_sects; //i结点表占用的扇区数量

    uint32_t data_start_lba;    //数据区开始的第一个扇区号
    uint32_t root_inode_no;    //根目录所在的i节点号
    uint32_t dir_entry_size;    //目录项大小

    uint8_t pad[460];   //加上460字节，凑足512字节1扇区的大小
}__attribute__ ((packed));

#endif // !__FS_SUPER_BLOCK_H
