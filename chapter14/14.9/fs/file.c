/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-08 10:26:42
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-12 17:58:17
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
#include "inode.h"
#include "super_block.h"
#include "dir.h"
#include "global.h"
#include "debug.h"

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
    
    bitmap_set(&part->block_bitmap,bit_idx,1);
    /*和inode_bitmap_alloc不同，此处返回的不是位图索引，而是具体可用的扇区地址*/
    return (part->sb->data_start_lba + bit_idx);
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

/*创建文件，若成功则返回文件描述符，否则返回-1*/
int32_t file_create(struct dir* parent_dir, char* filename, uint8_t flag){
    /*后续操作的公共缓冲区*/
    void* io_buf = sys_malloc(1024);
    if(io_buf==NULL){
        printk("in file_creat:sys_malloc for io_buf failed");
        return -1;
    }

    uint8_t rollback_step = 0;  //用于操作失败时回滚个各资源状态

    /*为新文件分配inode*/
    int32_t inode_no = inode_bitmap_alloc(cur_part);
    if(inode_no==-1){
        printk("in file_creat: allocate inode failed\n");
        return -1;
    }

    /**
     * 此inode要从堆中申请内存，不可生产局部变量（函数退出时会释放），
     * 因此file_table数组中的文件描述符的inode指正要指向他
    */
    struct inode* new_file_inode = (struct inode*)sys_malloc(sizeof(struct inode));
    if(new_file_inode == NULL){
        printk("file_create:sys_malloc for inode failded\n");
        rollback_step = 1;
        goto rollback;
    }
    inode_init(inode_no,new_file_inode);    //初始化i结点

    /*返回的是file_table数组的下标*/
    int fd_idx = get_free_slot_in_global();
    if(fd_idx == -1){
        printk("exceed max open files\n");
        rollback_step = 2;
        goto rollback;
    }

    file_table[fd_idx].fd_inode = new_file_inode;
    file_table[fd_idx].fd_pos = 0;
    file_table[fd_idx].fd_flag = flag;
    file_table[fd_idx].fd_inode->write_deny = false;

    struct dir_entry new_dir_entry;
    memset(&new_dir_entry,0,sizeof(struct dir_entry));

    create_dir_entry(filename,inode_no,FT_REGULAR,&new_dir_entry);  //create_dir_entry只是内存操作不出意外，不会返回失败

    /*同步内存数据到硬盘*/
    /*a在目录parent_dir下安装目录项new_dir_entry,写入硬盘后返回true，否则false*/
    if(!sync_dir_entry(parent_dir,&new_dir_entry,io_buf)){
        printk("sync dir_entry to disk failed\n");
        rollback_step=3;
        goto rollback;
    }

    memset(io_buf,0,1024);
    /*b将父目录i节点的内容同步到硬盘*/
    inode_sync(cur_part,parent_dir->inode,io_buf);

    memset(io_buf,0,1024);
    /*c将新创建文件的i节点内容同步到硬盘*/
    inode_sync(cur_part,new_file_inode,io_buf);
    
    /*d将inode_bitmap位图同步到硬盘*/
    bitmap_sync(cur_part,inode_no,INODE_BITMAP);

    /*e将创建的文件i节点添加到open_inodes链表*/
    list_push(&cur_part->open_inodes,&new_file_inode->inode_tag);
    new_file_inode->i_open_cnts=1;

    sys_free(io_buf);
    return pcb_fd_install(fd_idx);

    /*创建文件需要创建相关的多个资源，若某部失败则会指向到下面的回滚操作*/
rollback:
    switch(rollback_step){
        case 3:
            /*失败时，将file_table中的相应位清空*/
            memset(&file_table[fd_idx],0,sizeof(struct file));
        case 2:
            sys_free(new_file_inode);
        case 1:
            /*如果新文件的i节点创建失败，之前位图中分配的inode_no也要恢复*/
            bitmap_set(&cur_part->inode_bitmap,inode_no,0);
            break;
    }
    sys_free(io_buf);
    return -1;
}

/*把buf中的count个字节写入file，成功则返回写入的字节数，失败则返回-1*/
int32_t file_write(struct file* file, const void* buf, uint32_t count){
    if((file->fd_inode->i_size +count)>(BLOCK_SIZE * 140)){ //文件目前最大只支持512*140=71680字节
        printk("exceed max file_size 71680 bytes,write file failed\n");
        return -1;
    }
    uint8_t* io_buf = sys_malloc(512);
    if(io_buf==NULL){
        printk("file_write:sys_malloc for io_buf failed\n");
        return -1;
    }

    uint32_t* all_blocks = (uint32_t*)sys_malloc(BLOCK_SIZE+48);    //用来记录文件所有的块地址，一个地址4字节，一共有140个块所以要560字节内存
    if(all_blocks == NULL){
        printk("file_write: sys_malloc for all_blocks failed\n");
        return -1;
    }

    const uint8_t* src = buf;   //用src指向buf中待写入的数据
    uint32_t bytes_written = 0;     //用来记录已写入数据大小
    uint32_t size_left = count;     //用来记录未写入数据大小
    int32_t block_lba = -1; //用来记录块地址
    uint32_t block_bitmap_idx = 0;  //用来记录block对应于block_bitmap中的索引，作为参数传递给bitmap_sync

    uint32_t sec_idx;   //用来索引扇区
    uint32_t sec_lba;   //扇区地址
    uint32_t sec_off_bytes; //扇区内字节偏移量
    uint32_t sec_left_bytes;    //扇区内剩余字节量
    uint32_t chunk_size;    //每次写入硬盘的数据块大小
    int32_t indirect_block_table;   //用来获取一级间接表地址
    uint32_t block_idx; //块索引

    //判断文件是否是第一次写，如果是，先为其分配一个块
    if(file->fd_inode->i_sectors[0]==0){
        block_lba = block_bitmap_alloc(cur_part);
        if(block_lba == -1){
            printk("file_write:block_bitmap_alloc failed\n");
            return -1;
        }
        file->fd_inode->i_sectors[0]=block_lba;
        
        /*每分配一个块就将位图同步到磁盘*/
        block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
        ASSERT(block_bitmap_idx!=0);
        bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);
    }

    /*写入count字节前，改文件已经占用的块数*/
    uint32_t file_has_used_blocks = file->fd_inode->i_size / BLOCK_SIZE + 1;

    /*存储count字节后该文件将占用的块数*/
    uint32_t file_will_use_blocks = (file->fd_inode->i_size + count) / BLOCK_SIZE + 1;
    ASSERT(file_will_use_blocks <= 140);

    /*通过此增量判断是否要分配扇区，如果增量为0，表示原扇区够用*/
    uint32_t add_blocks = file_will_use_blocks - file_has_used_blocks;
    
    /*将写文件所用到的块地址收集到all_blocks，系统中快大小等于扇区大小，后面都统一在all_blocks中获取写入扇区地址*/
    if(add_blocks==0){
        /*在同一扇区内写入数据，不涉及到分配新扇区*/
        if(file_will_use_blocks<=12){
            //文件数据将在12块之内
            block_idx = file_has_used_blocks - 1;   //指向最后一个已有数据扇区
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
        }else{
            /*未写入新数据之前已经占用了间接块，需要将间接块地址读取出来*/
            ASSERT(file->fd_inode->i_sectors[12]!=0);
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk,indirect_block_table,all_blocks+12,1);
        }
    }else{
        /*若是有增量，变涉及到分配新扇区及是否分配一级间接块表，下面要分三种情况处理*/
        /*第一种情况：12个块直接够用*/
        if(file_will_use_blocks <= 12){
            /*先将剩余空间的可继续用的扇区地址写入all_blocks*/
            block_idx = file_has_used_blocks - 1;
            ASSERT(file->fd_inode->i_sectors[block_idx]!=0);
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];

            /*再将未来要用的扇区分配好后写入all_blocks*/
            block_idx = file_has_used_blocks;   //指向第一个要分配的扇区
            while(block_idx<file_will_use_blocks){
                block_lba = block_bitmap_alloc(cur_part);
                if(block_lba==-1){
                    printk("file_write:block_bitmap_alloc for situation 1 failed\n");
                    return -1;
                }
                /*写文件时，不应该存在块未使用但已经分配扇区的情况，当文件删除时，就会把块地址清0*/
                ASSERT(file->fd_inode->i_sectors[block_idx]==0);    //确保尚未分配扇区地址
                file->fd_inode->i_sectors[block_idx] = all_blocks[block_idx] = block_lba;

                /*每分配一个块就将位图同步到磁盘*/
                block_bitmap_idx = block_lba-cur_part->sb->data_start_lba;
                ASSERT(block_bitmap_idx!=0);
                bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);
                block_idx++;    //分配下一个扇区
            }
        }else if(file_has_used_blocks <= 12 && file_will_use_blocks > 12){
            /*第二种情况：旧数据在12个直接块中，数据将使用间接块*/
            /*先将剩余空间的可继续用的扇区地址写入all_blocks*/
            block_idx = file_has_used_blocks - 1;   //指向旧数据所在的最后一个扇区
            ASSERT(file->fd_inode->i_sectors[block_idx]!=0);
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];

            /*创建一级间接块表*/
            block_lba = block_bitmap_alloc(cur_part);
            if(block_lba==-1){
                printk("file_write:block_bitmap_alloc for situation 2 failed\n");
                return -1;
            }

            ASSERT(file->fd_inode->i_sectors[12]==0);//确保一级块表未分配
            /*分配一级块间接索引表*/
            indirect_block_table = file->fd_inode->i_sectors[12] = block_lba;

            block_idx = file_has_used_blocks;   //第－个未使用的块，即本文件最后一个已经使用的直接块的下一块
            while(block_idx < file_will_use_blocks){
                block_lba = block_bitmap_alloc(cur_part);
                if(block_lba==-1){
                    printk("file_write:block_bitmap_alloc for situation 2 failed\n");
                    return -1;
                }

                if(block_idx <  12){    //新创建的0-11快直接存入all_blocks数组中
                    ASSERT(file->fd_inode->i_sectors[block_idx]==0);    //确保尚未分配扇区地址
                    file->fd_inode->i_sectors[block_idx] = all_blocks[block_idx] = block_lba;
                }else{
                    //间接块只写入到all_blocks数组中，待全部分配完成后一次同步到硬盘
                    all_blocks[block_idx] = block_lba;
                }
                /*每分配一个块就将文图同步到硬盘*/
                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);
                block_idx++;
            }
            ide_write(cur_part->my_disk,indirect_block_table,all_blocks+12,1);   //同步一级间接块表到硬盘
        }else if(file_has_used_blocks > 12){
            /*第三种情况：新数据占间接块*/
            ASSERT(file->fd_inode->i_sectors[12]!=0);   //已经具备了一级间接块
            indirect_block_table = file->fd_inode->i_sectors[12];   //  获取一级间接表地址

            /*已经使用的间接块也将被读入all_blocks，无需单独收录*/
            ide_read(cur_part->my_disk,indirect_block_table,all_blocks+12,1);   //获取所有间接块地址
            block_idx = file_has_used_blocks;   //第一个为使用的间接块，即已经使用的间接块的下一块
            while(block_idx<file_will_use_blocks){
                block_lba = block_bitmap_alloc(cur_part);
                if(block_lba==-1){
                    printk("file_write:block_bitmap_alloc for situation 3 failed\n");
                    return -1;
                }
                all_blocks[block_idx] = block_lba;
                /*每分配一个块就将文图同步到硬盘*/
                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                bitmap_sync(cur_part,block_bitmap_idx,BLOCK_BITMAP);
                block_idx++;
            }
            ide_write(cur_part->my_disk,indirect_block_table,all_blocks+12,1);//同步一级间接块表到硬盘
        }
    }

    /*用到的块地址已经收集到了all_blocks中，下面开是写数据*/
    bool first_write_block = true;  //含有剩余空间的块标识
    file->fd_pos = file->fd_inode->i_size - 1;  //置fd_pos为文件大小-1，下面在写数据时随时更新，这是文件内偏移量
    while(bytes_written < count){   //直到数据写完
        memset(io_buf,0,BLOCK_SIZE);
        sec_idx = file->fd_inode->i_size / BLOCK_SIZE;
        sec_lba = all_blocks[sec_idx];
        sec_off_bytes = file->fd_inode->i_size % BLOCK_SIZE;
        sec_left_bytes = BLOCK_SIZE - sec_off_bytes;

        /*判断此写入硬盘的大小数据*/
        chunk_size = size_left<sec_left_bytes ? size_left : sec_left_bytes;
        if(first_write_block){
            ide_read(cur_part->my_disk,sec_lba,io_buf,1);
            first_write_block = false;
        }
        memcpy(io_buf+sec_off_bytes,src,chunk_size);
        ide_write(cur_part->my_disk,sec_lba,io_buf,1);
        printk("file write at lba 0x%x\n",sec_lba); //调试，完成后去掉
        src+=chunk_size;    //将指针推移到下一个新数据
        file->fd_inode->i_size+=chunk_size; //更新文件大小
        file->fd_pos +=chunk_size;
        bytes_written+=chunk_size;
        size_left -= chunk_size;
    }
    inode_sync(cur_part,file->fd_inode,io_buf);
    sys_free(all_blocks);
    sys_free(io_buf);
    return bytes_written;
}

/*冲文件file中读取count个字节写入buf，返回读取的字节数，若到文件尾返回-1*/
int32_t file_read(struct file* file, void* buf, uint32_t count){
    uint8_t* buf_dst = (uint8_t*)buf;
    uint32_t size = count, size_left = size;

    /*若要读取的字节数超过了文件可读的剩余量，就用剩余量作为待读取的字节数*/
    if((file->fd_pos + count) > file->fd_inode->i_size){
        size = file->fd_inode->i_size - file->fd_pos;
        size_left = size;
        if(size==0){//若到文件尾，则返回-1
            return -1;
        }   
    }

    uint8_t* io_buf = sys_malloc(BLOCK_SIZE);
    if(io_buf==NULL){
        printk("file_read:sys_malloc for io_buf failed\n");
    }

    uint32_t* all_blocks = (uint32_t*)sys_malloc(BLOCK_SIZE+48);    //用来记录文件所有的块地址
    if(all_blocks == NULL){
        printk("file_read: sys_malloc for all_blocks failed\n");
        return -1;
    }

    uint32_t block_read_start_idx = file->fd_pos / BLOCK_SIZE;  //数据所在块的起始地址
    uint32_t block_read_end_idx = (file->fd_pos+size) / BLOCK_SIZE; //数据所在块的终止地址
    uint32_t read_blocks = block_read_start_idx - block_read_end_idx;   //如果增量为0，表示数据在同一扇区
    ASSERT(block_read_start_idx < 139 && block_read_end_idx < 139);
    int32_t indirect_block_table;   //用来获取一级间接表地址
    uint32_t block_idx; //获取待读的块地址

    /*以下开始构建all_blocks块地址数组，专门存储用到的块地址（本程序中块大小通扇区大小）*/
    if(read_blocks==0){ //在同一扇区内读取数据，不涉及到夸扇区读取
        ASSERT(block_read_end_idx == block_read_start_idx);
        if(block_read_end_idx<12){  //待读的数据在12个直接块内
            block_idx = block_read_end_idx;
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
        }else{  //若用到了一级间接块，需要将表中间接块读进来
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk,indirect_block_table,all_blocks+12,1);
        }
    }else{  //若要读多个块
        /*第一种情况：起始块和终止块属于直接块*/
        if(block_read_end_idx<12){  //数据结束所爱的块属于直接块
            block_idx = block_read_start_idx;
            while(block_idx<block_read_end_idx){
                all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
                block_idx++;
            }
        }else if (block_read_start_idx<12&&block_read_end_idx >= 12){
            /*第二种情况：起始块属于直接块，终止块属于间接块*/
            /*先将直接块地址写入all_blocks*/
            block_idx = block_read_start_idx;
            while(block_idx<12){
                all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
                block_idx++;
            }
            ASSERT(file->fd_inode->i_sectors[12]!=0);   //确保已经分配了一级间接块

            /*在将间接块地址写入all_blocks*/
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk,indirect_block_table,all_blocks+12,1);   //将一级间接块表读进来写入到第 13 个块的位置之后
        }else{
            /*第三种情况：数据在间接块中*/
            ASSERT(file->fd_inode->i_sectors[12]!=0);   //确保已经分配了一级间接块
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk,indirect_block_table,all_blocks+12,1);   //将一级间接块表读进来写入到第 13 个块的位置之后
        }
    }

    /*用到的块地址已经收集到 all_blocks 中，下面开始读数据*/
    uint32_t sec_idx, sec_lba, sec_off_bytes, sec_left_bytes, chunk_size;
    uint32_t bytes_read = 0;
    while(bytes_read<size){     //直到读完为止
        sec_idx = file->fd_pos / BLOCK_SIZE;
        sec_lba = all_blocks[sec_idx];
        sec_off_bytes = file->fd_pos % BLOCK_SIZE;
        sec_left_bytes = BLOCK_SIZE - sec_off_bytes;
        chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;   //待读入的数据大小
        memset(io_buf,0,BLOCK_SIZE);
        ide_read(cur_part->my_disk,sec_lba,io_buf,1);
        memcpy(buf_dst,io_buf+sec_off_bytes,chunk_size);
        
        buf_dst += chunk_size;
        file->fd_pos += chunk_size;
        bytes_read += chunk_size;
        size_left -= chunk_size;
    }
    sys_free(all_blocks);
    sys_free(io_buf);
    return bytes_read;
}