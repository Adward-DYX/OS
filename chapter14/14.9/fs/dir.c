/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-08 11:05:56
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-11 14:05:07
 * @FilePath: /OS/chapter14/14.4/fs/dir.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "dir.h"
#include "ide.h"
#include "fs.h"
#include "global.h"
#include "memory.h"
#include "inode.h"
#include "stdio-kernel.h"
#include "stdint.h"
#include "string.h"
#include "file.h"
#include "debug.h"

struct dir root_dir;

/*打开根目录*/
void open_root_dir(struct partition* part) {
   root_dir.inode = inode_open(part, part->sb->root_inode_no);
   root_dir.dir_pos = 0;
}

/*在分区part上打开i结点为inode_no的目录并返回目录指针*/
struct dir* dir_open(struct partition* part, uint32_t inode_no){
    struct dir* pdir = (struct dir*)sys_malloc(sizeof(struct dir));
    pdir->inode = inode_open(part,inode_no);
    pdir->dir_pos = 0;
    return pdir;
}

/*在part分区内的pdir目录内寻址名为name的文件或目录，找到后返回ture并将其目录项存入dir_e，否则返回false，inode中指向的地方也是存储的dir结构*/
bool search_dir_entry(struct partition* part, struct dir* pdir, const char* name, struct dir_entry* dir_e){
    uint32_t block_cnt = 140;   //12个直接块+128个一级间接块=140

    /*12个直接块大小+128个间接块，供560字节*/
    uint32_t* all_blocks = (uint32_t*)sys_malloc(48+512);
    if(all_blocks == NULL){
        printk("search_dir_entry:sys_malloc for all_blocks failed");
        return false;
    }

    uint32_t block_idx = 0;
    while(block_idx < 12){
        all_blocks[block_idx] = pdir->inode->i_sectors[block_idx];
        block_idx++;
    }
    block_idx=0;

    if(pdir->inode->i_sectors[12]!=0){  //含有一级间接块表
        ide_read(part->my_disk,pdir->inode->i_sectors[12],all_blocks+12,1);
    }
    /*至此，all_blocks存储的是目录pdir的所有扇区地址*/

    /*写目录项的时候已经保证目录项不夸扇区，这样读目录项容易处理，值申请容纳一个扇区的内存*/
    uint8_t* buf = (uint8_t*)sys_malloc(SECTOR_SIZE);
    struct dir_entry* p_de = (struct dir_entry*)buf;
    //p_de为指向目录项的指针，值为buf起始地址
    uint32_t dir_entry_size = part->sb->dir_entry_size;
    uint32_t dir_entry_cnt = SECTOR_SIZE / dir_entry_size;  //1扇区可以容纳的目录项个数

    /*开始在所有块中查找目录项*/
    while(block_idx < block_cnt){
        /*块地址为0时表示该块中无数据，继续在其他块中找*/
        if(all_blocks[block_idx]==0){
            block_idx++;
            continue;
        }
        ide_read(part->my_disk,all_blocks[block_idx],buf,1);
        uint32_t dir_entry_idx = 0;
        /*遍历扇区中所有目录项*/
        while(dir_entry_idx < dir_entry_cnt){
            /*若找到了， 就直接复制整个目录项*/
            if(!strcmp(p_de->filename,name)){
                memcpy(dir_e,p_de,dir_entry_size);
                sys_free(buf);
                sys_free(all_blocks);
                return true;
            }
            dir_entry_idx++;
            p_de++;
        }
        block_idx++;
        p_de = (struct dir_entry*)buf;  //此时p_de已经指向扇区内最后一个完整目录项，需要恢复p_de指向为buf
        memset(buf,0,SECTOR_SIZE);
    }
    sys_free(buf);
    sys_free(all_blocks);
    return false;
}

/*关闭目录*/
void dir_close(struct dir* dir){
    /***************** 根目录不能关闭 ******************************/
    /*1 根目录自打开后就不应该关闭，否则还需要再次open_root_dir()*/
    /*2 root_dir锁在的内存是低端1MB之内，并非在堆中，free会出现问题*/
    if(dir == &root_dir){
        /*不做任何处理直接返回*/
        return;
    }
    inode_close(dir->inode);
    sys_free(dir);
}

/*在内存中初始化目录项p_de*/
void create_dir_entry(char* filename, uint32_t inode_no, uint8_t file_type, struct dir_entry* p_de){
    ASSERT(strlen(filename)<=MAX_FILE_NAME_LEN);
    /*初始化目录项*/
    memcpy(p_de->filename,filename,strlen(filename));
    p_de->i_no = inode_no;
    p_de->f_type = file_type;
}

/*将目录项p_de写入父目录parent_dir中，io_buf由主调函数提供*/
bool sync_dir_entry(struct dir* parent_dir, struct dir_entry* p_de, void* io_buf){
    struct inode* dir_inode = parent_dir->inode;
    uint32_t dir_size = dir_inode->i_size;
    uint32_t dir_entry_size = cur_part->sb->dir_entry_size;
    ASSERT(dir_size % dir_entry_size == 0);//dir_size应该是dir_entry_size的整数倍

    uint32_t dir_entrys_per_sec = (512 / dir_entry_size); //每个扇区最大目录项数目
    int32_t block_lba = -1;

    /*将该目录的所有扇区地址（12个直接块+128个间接块）存入all_blocks*/
    uint8_t block_idx = 0;
    uint32_t all_blocks[140] = {0}; //all_blocks保存目录所有的块

    /*将12个直接块存入all_blocks*/
    while(block_idx < 12){
        all_blocks[block_idx] = dir_inode->i_sectors[block_idx];
        block_idx++;
    }
    if(dir_inode->i_sectors[12]!=0){//若含有一级间接块
    /*仅仅从目录文件中读出了前12个直接块，导致all_blocks数组仅前12项是有非0值的。假设整个目录文件，前12个直接块指向的块已经存满了目录项，那么新的目录项只能存储至一级间接块（i_sectors[12]）指向的块其中某个地址指向的目录文件中。但是代码逻辑，会必然判断出all_blocks[12] == 0，然后进入block_idx == 12，就会重新申请一个块充当一级间接块，就把原有的一级间接块覆盖了。这会导致对之前存储在间接块中的所有目录项的引用丢失，这是一个严重的数据完整性问题。*/
        ide_read(cur_part->my_disk,dir_inode->i_sectors[12],all_blocks+12,1);
    }

    struct dir_entry* dir_e = (struct dir_entry*)io_buf;    //dir_e用来在io_buf中遍历目录项
    
    int32_t block_bitmap_idx = -1;

    /*开始遍历所有块以寻找目录项空位，若已有扇区中没有空闲位，在不超过文件大小的情况下申请新扇区来存储新目录项*/
    block_idx = 0;
    while(block_idx<140){//文件最大支持12个直接块+128个间接块=140
        block_bitmap_idx = -1;
        if(all_blocks[block_idx]==0){   //在三种情况下分配块
            block_lba = block_bitmap_alloc(cur_part);
            if(block_lba==-1){
                printk("alloc block bitmap for sync_dir_entry failed\n");
                return false;
            }

            /*每分配一个块就同步一次block_bitmap*/
            block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
            ASSERT(block_bitmap_idx!=-1);
            bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);

            block_bitmap_idx = -1;
            if(block_idx<12){   //若是直接块
                dir_inode->i_sectors[block_idx] = all_blocks[block_idx] = block_lba;
            }else if(block_idx == 12){  //若是尚未分配一级间接块表(block_idx 等于 12 表示第 0个间接块地址为 0)
                dir_inode->i_sectors[12] = block_lba;   //将上面分配的块作为一级间接块表地址
                block_lba = -1;
                block_lba = block_bitmap_alloc(cur_part);   //在分配一个块作为第0个间接块
                if(block_lba==-1){
                    block_bitmap_idx = dir_inode->i_sectors[12] - cur_part->sb->data_start_lba;
                    bitmap_set(&cur_part->block_bitmap,block_bitmap_idx,0);
                    dir_inode->i_sectors[12]=0;
                    printk("all block bitmap for sync_dir_entry failed\n");
                    return false;
                }
                /*每分配一个块就同步一次block_bitmap*/
                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                ASSERT(block_bitmap_idx!=-1);
                bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);

                all_blocks[12] = block_lba;
                /*把新分配的第0个间接块地址写入一级间接块表*/
                ide_write(cur_part->my_disk,dir_inode->i_sectors[12],all_blocks+12,1);
            }else{  //若是间接块未分配
                all_blocks[block_idx] = block_lba;
                /*把新分配的第(block_idx-12)个间接块写入一级间接块*/
                ide_write(cur_part->my_disk,dir_inode->i_sectors[12],all_blocks+12,1); 
            }

            /*在将新目录p_de写入新分配的间接块*/
            memset(io_buf,0,512);
            memcpy(io_buf,p_de,dir_entry_size);
            ide_write(cur_part->my_disk,all_blocks[block_idx],io_buf,1);//io_buf中的内容写入到新分配到all_blocks[block_idx]指向的新扇区地址中，也就写入到了父目录的目录项中
            dir_inode->i_size+=dir_entry_size;
            return true;
        }

        /*若block_idx块已经存在，将其读进内存，然后再该块总查找空目录项*/
        ide_read(cur_part->my_disk,all_blocks[block_idx],io_buf,1); //写入到了io_buf其实也是相当于写入到了dir_e中,
        /*在扇区中查找空目录项*/
        uint8_t dir_entry_idx = 0;
        while(dir_entry_idx < dir_entrys_per_sec){
            if((dir_e+dir_entry_idx)->f_type == FT_UNKNOWN){//FT_UNKNOWN为0，无论是初始化，或是删除文件后，都会将f_type置位FT_UNKNOWN
                memcpy(dir_e+dir_entry_idx,p_de,dir_entry_size);
                ide_write(cur_part->my_disk,all_blocks[block_idx],io_buf,1);
                dir_inode->i_size+=dir_entry_size;
                return true;
            }
            dir_entry_idx++;
        }
        block_idx++;
    }
    printk("dorectory is full!\n");
    return false;
}