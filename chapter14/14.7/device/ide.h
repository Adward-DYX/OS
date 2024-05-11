/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-28 17:37:39
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-07 20:09:34
 * @FilePath: /OS/chapter13/13.2/device/ide.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __DEVICE_IDE_H
#define __DEVICE_IDE_H

#include "stdint.h"
#include "list.h"
#include "bitmap.h"
#include "global.h"
#include "thread.h"
#include "sync.h"
#include "super_block.h"

struct partition{
    uint32_t start_lba; //起始扇区
    uint32_t sec_cnt;   //扇区数
    struct disk* my_disk;   //分区所属的硬盘
    struct list_elem part_tag;  //用于队列中的标记 
    char name[8];       //分区名称
    struct super_block* sb; //本分区的超级块
    struct bitmap block_bitmap; //块位图
    struct bitmap inode_bitmap; //i结点位图
    struct list open_inodes;    //本分区打开的i结点队列
};

/*硬盘结构*/
struct disk{
    char name[8];   //本硬盘的名称
    struct ide_channel* my_channel;     //此块硬盘归属于那个ide通道
    uint8_t dev_no;     //本硬盘是主0，还是从1
    struct partition prim_parts[4];     //主分区顶多只有4个
    struct partition logic_parts[8];    //逻辑分区数量无限，但总得有个上限支持，这里设置为8
};

/*ata通道*/
//port_base咱们这里只处理两个通道的主板，每个通道的
//端口范围是不一样的，通道 I (Primary通道）的命令块寄存器端口范围是 Ox1FO～Ox1F7，控制块寄存器
//端口是 0x3F6，通道 2 ( Secondary 通道〉命令块寄存器端口范围是 Ox170～Oxl77 ，控制块寄存器端口是Ox376 
//通道 l 的端口可以以 OxlFO 为基数，其命令块寄存器端口在此基数上分别加上 0～ 7 就可以了，
//控制块寄存器端口在此基数上加上 Ox206，同理，通道 2 的基数就是 Oxl70 
struct ide_channel{
    char name[8];       //本ata通道名称
    uint16_t port_base;     //本通道的起始端口号
    uint8_t irq_no;     //本通道所用的中断号
    struct lock lock;   //通道锁
    bool expecting_intr;    //表示等待硬盘的中断
    struct semaphore disk_done; //用于阻塞、唤醒驱动程序
    struct disk devices[2]; //一个通道上连接两个硬盘，一主一从
};

extern uint8_t channel_cnt;
extern struct ide_channel channels[];
extern struct list partition_list;
void ide_read(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);
void ide_write(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);
void intr_hd_handler(uint8_t irq_no);
void ide_init(void);

#endif // !__DEVICE_IDE_H
