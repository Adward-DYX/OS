/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-07 11:09:28
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-11 15:16:12
 * @FilePath: /OS/chapter14/14.2/fs/fs.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "fs.h"
#include "dir.h"
#include "inode.h"
#include "super_block.h"
#include "stdint.h"
#include "bitmap.h"
#include "list.h"
#include "global.h"
#include "ide.h"
#include "stdio-kernel.h"
#include "stdio.h"
#include "io.h"
#include "memory.h"
#include "ide.h"
#include "string.h"
#include "debug.h"
#include "file.h"
#include "console.h"


struct partition* cur_part; //默认情况下操作的是那个分区

/*在分区链表中找到名为part_name的分区，并将其赋值给cur_part*/
static bool mount_partition(struct list_elem* pelem, int arg){
    char* part_name = (char*)arg;
    struct partition* part = elem2entry(struct partition, part_tag, pelem);
    if(!strcmp(part->name,part_name)){
        cur_part = part;
        struct disk* hd = cur_part->my_disk;

        /*sb_buf用来存储从硬盘上读入的超级块*/
        struct super_block* sb_buf = (struct super_block*)sys_malloc(SECTOR_SIZE);

        /*在内存中创建分区cur_part的超级块*/
        cur_part->sb = (struct super_block*)sys_malloc(sizeof(struct super_block));
        if(cur_part->sb == NULL){
            PANIC("alloc memory failed!");
        }

        /*读入超级块*/
        memset(sb_buf, 0, SECTOR_SIZE);
        ide_read(hd, cur_part->start_lba+1,sb_buf,1);

        /*把sb_buf中的超级块的信息复制到分区的超级块sb中*/
        memcpy(cur_part->sb,sb_buf,sizeof(struct super_block));

        /********************* 将硬盘上的块位图读入到内存中 ***************************************/
        cur_part->block_bitmap.bits = (uint8_t*)sys_malloc(sb_buf->block_bitmap_sects * SECTOR_SIZE);
        if(cur_part->block_bitmap.bits == NULL)
            PANIC("alloc memory failed!");
        
        cur_part->block_bitmap.btmp_bytes_len = sb_buf->block_bitmap_sects * SECTOR_SIZE;

        /*从硬盘上读入块位图到分区的block_bitmao.bits*/
        ide_read(hd,sb_buf->block_bitmap_lba,cur_part->block_bitmap.bits,sb_buf->block_bitmap_sects);
        /********************************************************************************************/
    
        /********************* 将硬盘上的inode位图读入到内存中 ***************************************/
        cur_part->inode_bitmap.bits = (uint8_t*)sys_malloc(sb_buf->inode_bitmap_sects * SECTOR_SIZE);
        if(cur_part->inode_bitmap.bits == NULL)
            PANIC("alloc memory failed!");
        cur_part->inode_bitmap.btmp_bytes_len = sb_buf->inode_bitmap_sects * SECTOR_SIZE;

        /*从硬盘上读入inode位图到分区的inode_bitmap.bits*/
        ide_read(hd,sb_buf->inode_bitmap_lba,cur_part->inode_bitmap.bits,sb_buf->inode_bitmap_sects);
        /********************************************************************************************/

        list_init(&cur_part->open_inodes);
        printk("mount %s done!\n", part->name);
        
        /**
         * 此处返回true是为了迎合主调函数list_traversal的实现
         * 与函数本身功能无关
         * 只有返回true时list_traversal才会停止遍历
         * 减少了后面元素无意义的遍历
        */
        return true;
    }
    return false;   //使得list_traversal继续遍历
}

/*格式化分区，也就是初始化分区的元信息，创建文件系统*/
static void partition_format(struct partition* part){
    /*blocks_bitmao_init（为方便实现，一个块大小就是一个扇区)*/
    uint32_t boot_sector_sects = 1;
    uint32_t super_block_sects = 1;
    uint32_t inode_bitmap_sects = DIV_ROUND_UP(MAX_FILES_PER_PART,BITS_PER_SECTOR);//I 结点位图占用的扇区数，最多支持 4096 个文件
    uint32_t inode_table_sects = DIV_ROUND_UP(((sizeof(struct inode)*MAX_FILES_PER_PART)),SECTOR_SIZE);
    uint32_t used_sects = boot_sector_sects + super_block_sects + inode_bitmap_sects + inode_table_sects;
    uint32_t free_sects = part->sec_cnt - used_sects;

    /************************** 简单处理块位图占据的扇区数 ************************************************/
    uint32_t block_bitmap_sects;
    block_bitmap_sects = DIV_ROUND_UP(free_sects,BITS_PER_SECTOR);  ///的目的是计算出块位图占用的扇区数。由于每个位表示一个块，因此要以位的数量计算，而不是以字节的数量。所以这里用位数除以 BITS_PER_SECTOR 来计算位图占用的扇区数。
    /*block_bitmap_bit_len是位图中位的长度，也是可用块的数量*/
    uint32_t block_bitmap_bit_len = free_sects - block_bitmap_sects;
    block_bitmap_sects = DIV_ROUND_UP(block_bitmap_bit_len,BITS_PER_SECTOR);
    /********************************************************************************************************/

    /*超级块初始化*/
    struct super_block sb;
    sb.magic = 0x19590318;
    sb.sec_cnt = part->sec_cnt;
    sb.inode_cnt = MAX_FILES_PER_PART;
    sb.part_lba_base = part->start_lba;

    sb.block_bitmap_lba = sb.part_lba_base + 2; //第0块是引导块，第一块是超级块
    sb.block_bitmap_sects = block_bitmap_sects;

    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects;
    sb.inode_bitmap_sects = inode_bitmap_sects;

    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
    sb.inode_table_sects = inode_table_sects;

    sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects;
    sb.root_inode_no = 0;
    sb.dir_entry_size = sizeof(struct dir_entry);

    printk("%s info:\n", part->name);
    printk("   magic:0x%x\n   part_lba_base:0x%x\n   all_sectors:0x%x\n   inode_cnt:0x%x\n   block_bitmap_lba:0x%x\n   block_bitmap_sectors:0x%x\n   inode_bitmap_lba:0x%x\n   inode_bitmap_sectors:0x%x\n   inode_table_lba:0x%x\n   inode_table_sectors:0x%x\n   data_start_lba:0x%x\n", sb.magic, sb.part_lba_base, sb.sec_cnt, sb.inode_cnt, sb.block_bitmap_lba, sb.block_bitmap_sects, sb.inode_bitmap_lba, sb.inode_bitmap_sects, sb.inode_table_lba, sb.inode_table_sects, sb.data_start_lba);

    struct disk* hd = part->my_disk;
    /*************** 1.将超级块写入本分区的1扇区 ************************/
    ide_write(hd,part->start_lba+1,&sb,1);
    printk("    super_block_lba:0x%x\n",part->start_lba+1);

    /** 找出数据量最大的元信息，用其尺寸做存储缓冲区 **/
    uint32_t buf_size = (sb.block_bitmap_sects>=sb.inode_bitmap_sects ? sb.block_bitmap_sects : sb.inode_bitmap_sects);
    buf_size = (buf_size >= sb.inode_table_sects ? buf_size:sb.inode_table_sects) * SECTOR_SIZE;

    uint8_t* buf = (uint8_t*)sys_malloc(buf_size); //申请的内存由内存管理系统清0后返回

    /*************** 2.将块位图初始化并写入sb.block_bitmap_lba ************************/
    /* 初始化块位图block_bitmap */
    buf[0] |= 0x01; //第0个块预留给跟目录，位图中先占位
    uint32_t block_bitmap_last_byte = block_bitmap_bit_len / 8;
    uint32_t block_bitmap_last_bit = block_bitmap_bit_len % 8;
    uint32_t last_size = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE);
    //last_size是位图所在最后一个扇区中，不足一扇区的其余部分

    /* 1 先将位图最后一字节到其所在的扇区的结束全置位1，即超出实际块的部分直接置位已占用 */
    memset(&buf[block_bitmap_last_bit],0xff,last_size);

    /* 2 再将上一步中覆盖的最后一字节内的有效位重新置0 */
    uint8_t bit_idx = 0;
    while(bit_idx <= block_bitmap_last_bit){
        buf[block_bitmap_last_byte] &= ~(1<<bit_idx++);
    }
    ide_write(hd, sb.block_bitmap_lba, buf, sb.block_bitmap_sects);

    /*************** 3.将inode位图初始化并写入sb.inode_bitmap_lba ************************/
    /*先清空缓冲区*/
    memset(buf,0,buf_size);
    buf[0] |= 0x01; //第0个块预留给跟目录
    /**
     * 由于inode_table中共4096个inode，
     * 位图inode_bitmap正好占用1扇区
     * 即inode_bitmap_sects等于1
     * 所以沃土中的位全都代表inode_table中的inode，
     * 无需再像block_bitmap那样单独处理最后一扇区的剩余部分，
     * inode_bitmap所在的扇区中没有多余的无效位。
    */
   ide_write(hd,sb.inode_bitmap_lba,buf,sb.inode_bitmap_sects);
   
    /*************** 4.将inode数组初始化并写入sb.inode_table_lba ************************/
    /*准备写inode_table中的第0项，即根目录所在的inode*/
    memset(buf,0,buf_size); //先清空缓冲区buf
    struct inode* i = (struct inode*)buf;
    i->i_size = sb.dir_entry_size * 2;  //.和..
    i->i_no = 0;    //根目录占inode数组中第0个inode
    i->i_sectors[0] = sb.data_start_lba;    //由于上面的memset,i_sectors数组的其他袁术都初始化为0
    ide_write(hd,sb.inode_table_lba,buf,sb.inode_table_sects);
    
    /*************** 5.将根目录写入sb.data_start_lba ************************/
    /*写入根目录的两个目录项.和..*/
    memset(buf,0,buf_size);
    struct dir_entry* p_de = (struct dir_entry*)buf;

    /*初始化当前目录".""*/
    memcpy(p_de->filename,".",1);
    p_de->i_no = 0;
    p_de->f_type = FT_DIRECTORY;
    p_de++;
    
    /*初始化当前目录父目录".."*/
    memcpy(p_de->filename,"..",2);
    p_de->i_no = 0; //根目录的父目录依然是根目录自己
    p_de->f_type = FT_DIRECTORY;

    /*sb.data_start_lba已经分配给了根目录，里面是根目录的目录项*/
    ide_write(hd,sb.data_start_lba,buf,1);
    printk(" root_dir_lba:0x%x\n",sb.data_start_lba);
    printk("%s format done\n",part->name);
    sys_free(buf);
}

/*将最上层路径名称解析出来*/
static char* pathr_pase(char* pathname, char* name_store){
    if(pathname[0]=='/'){   //根目录不需要单独解析
        /*路径中出现1个或多个连续的字符'/'，将这些‘/'跳过，如“///a/b”*/
        while(*(++pathname) == '/');
    }

    /*开始一般的路径解析*/
    while(*pathname!='/'&&*pathname!=0){
        *name_store++ = *pathname++;
    }

    if(pathname[0]==0){//若路径字符串为空，则返回NULL
        return NULL;
    }
    return pathname;
}

/*返回路径深度，比如/a/b/c，深度为3*/
int32_t pathr_depth_cnt(char* pathname){
    ASSERT(pathname!=NULL);
    char* p = pathname;
    char name[MAX_FILE_NAME_LEN];   //用于path_parse的参数做路径解析
    uint32_t depth = 0;

    /*解析路径，从中拆分出各级名称*/
    p = pathr_pase(p,name);
    while(name[0]){
        depth++;
        memset(name,0,MAX_FILE_NAME_LEN);
        if(p){  //如果p不等于NULL，继续分析路径
            p = pathr_pase(p,name);
        }
    }
    return depth;
}

/*搜索文件pathname，若找到则返回其inode号，否则就返回-1*/
static int search_file(const char* pathname,struct path_search_record* searched_record){
    /*如果待查找的是根目录，为避免下面无用的查找，直接返回已知根目录信息*/
    if(!strcmp(pathname,"/")||!strcmp(pathname,"/.")||!strcmp(pathname,"/..")){
        searched_record->parent_dir = &root_dir;
        searched_record->file_type = FT_DIRECTORY;
        searched_record->searched_path[0] = 0;  //搜索路径为空
        return 0;
    }

    uint32_t path_len = strlen(pathname);
    /*保证pathname至少是这样的路径/x，且小于最大长度*/
    ASSERT(pathname[0] == '/' && path_len>1 && path_len<MAX_PATH_LEN);
    char* sub_path = (char*)pathname;
    struct dir* parent_dir = &root_dir;
    struct dir_entry dir_e;

    /*记录路径解析出来的各级名称，如路径"/a/b/c",数组name每次的值分别是"a","b","c"*/
    char name[MAX_FILE_NAME_LEN] = {0};
    
    searched_record->parent_dir = parent_dir;
    searched_record->file_type = FT_UNKNOWN;
    uint32_t parent_inode_no = 0;//父目录的inode号

    sub_path = pathr_pase(sub_path,name);
    while(name[0]){//若第一个字符就是结束符，结束循环
        /*记录查找过的路径，但不能超过searched_path的长度512*/
        ASSERT(strlen(searched_record->searched_path)<512);

        /*记录已经存在的父目录*/
        strcat(searched_record->searched_path,"/");
        strcat(searched_record->searched_path,name);

        /*在所给的目录中查找文件*/
        if(search_dir_entry(cur_part,parent_dir,name,&dir_e)){
            memset(name,0,MAX_FILE_NAME_LEN);
            /*若sub_path不等于NULL，也就是未结束时继续拆分路径*/
            if(sub_path){
                sub_path = pathr_pase(sub_path,name);
            }

            if(FT_DIRECTORY == dir_e.f_type){//如果被打开的是目录
                parent_inode_no = parent_dir->inode->i_no;
                dir_close(parent_dir);
                parent_dir = dir_open(cur_part,dir_e.i_no);//更新父目录
                searched_record->parent_dir = parent_dir;
                continue;
            }else if(FT_REGULAR == dir_e.f_type){   //若是普通文件
                searched_record->file_type = FT_REGULAR;
                return dir_e.i_no;
            }
        }else{  //若找不到，则返回-1
            //找不到目录项时 ，要留着 parent_dir 不要关闭，若是创建新文件的话需要在 parent_dir 中创建
            return -1;
        }
    }
    /*执行到此，必然是遍历了完整路径,并且查找的文件或目录只有同名目录存在 */
    dir_close(searched_record->parent_dir);
    /*保存被查找到目录的直接父目录*/
    //parent_inode_no的作用：
    /**
     * “／a/b/c ”， c 是目录，不是普通文件，
     * 此时searched_record->parent_dir是路径pathname中的最后一级目录c，并不是倒数第二级的父目录b，
     * 我们在任何时候都应该使searched_record->parent_dir中记录的都应该是目录b
     * 因此我们需要把searched_record->parent_dir重新更新为父目录b, parent_inode_no就记录了父目录
    */
    searched_record->parent_dir = dir_open(cur_part,parent_inode_no);
    searched_record->file_type = FT_DIRECTORY;
    return dir_e.i_no;
}

/*打开编号为inode_no的inode对应的文件，若成功则返回文件描述符，否则返回-1*/
int32_t file_open(uint32_t inode_no, uint8_t flag){
    int fd_idx = get_free_slot_in_global();
    if(fd_idx == -1){
        printk("exceed max open files\n");
        return -1;
    }
    file_table[fd_idx].fd_inode = inode_open(cur_part,inode_no);
    file_table[fd_idx].fd_pos = 0;  //每次打开文件，要将fd_pos还原为0，即让文件内的指针指向开头
    file_table[fd_idx].fd_flag = flag;
    bool* write_deny = &file_table[fd_idx].fd_inode->write_deny;
    
    if(flag & O_WRONLY || flag & O_RDWR){
        //只要是关于写文件，判断是否有其他进程正在写此文件，要保持互斥
        //若是读文件，不需要考虑write_deny；
        /*以下进入临界区前先关闭中断*/
        enum intr_status old_status = intr_disable();
        if(!(*write_deny)){ //若当前没有其他进程写该文件，将其占用
            *write_deny = true; //置位true,避免多个进程同时写这个文件
            intr_set_status(old_status);
        }else{  //直接返回失败
            intr_set_status(old_status);
            printk("file can`t be write now, try again later\n");
            return -1;
        }
    }   //若是读文件或创建文件，不用例会write_deny，保持默认
    return pcb_fd_install(fd_idx);
}

/*关闭文件*/
int32_t file_close(struct file* file){
    if(file==NULL){
        return -1;
    }

    file->fd_inode->write_deny=false;
    inode_close(file->fd_inode);
    file->fd_inode=NULL;    //使得文件可用
    return 0;
}

/*打开或创建文件成功后，返回文件描述符，否则返回-1*/
int32_t sys_open(const char* pathname, uint8_t flags){
    /*对目录要用dir_open，这里只有open文件*/
    if(pathname[strlen(pathname)-1]=='/'){
        printk("can`t open a diretory %s\n",pathname);
        return -1;
    }
    ASSERT(flags <= 7);
    int32_t fd = -1;    //默认为找不到

    struct path_search_record searched_record;
    memset(&searched_record,0,sizeof(struct path_search_record));
    /*记录目录深度，帮助判断中间某个目录不存在的情况*/
    uint32_t pathname_depth = pathr_depth_cnt((char*)pathname);
    /*先检查文件是否存在*/
    int inode_no = search_file(pathname,&searched_record);
    bool found = inode_no != -1 ? true:false;
    if(searched_record.file_type==FT_DIRECTORY){
        printk("can`t open a direcotry with open(), use opendir() to instead\n");
        dir_close(searched_record.parent_dir);
        return -1;
    }
    uint32_t path_search_depth = pathr_depth_cnt(searched_record.searched_path);
    /*先判断是否把pathname的各层目录都访问到了，即是否在某个中间目录就失败*/
    if(pathname_depth != path_search_depth){
        //说明没有访问到全部的路径，某个中间目录是不存在的
        printk("can`t access %s:Not a directory, subpath %s is`t exist\n",pathname, searched_record.searched_path);
        dir_close(searched_record.parent_dir);
        return -1;
    }
    /*若是在最后一个路径上没有找到，并且不是要创建文件，直接返回-1*/
    if(!found&&!(flags&O_CREAT)){
        printk("in path %s,file %s is`t exist\n",searched_record.searched_path,(strrchr(searched_record.searched_path,'/')+1));
        dir_close(searched_record.parent_dir);
        return -1;
    }else if(found && flags&O_CREAT){    //若是要创建的文件已经存在了
        printk("%s has already exist!\n",pathname);
        dir_close(searched_record.parent_dir);
        return -1;
    }
    switch (flags&O_CREAT){
        case O_CREAT:
            printk("creating file\n");
            fd = file_create(searched_record.parent_dir,(strrchr(pathname,'/')+1),flags);
            dir_close(searched_record.parent_dir);
            break;
        //其余为打开文件
        default:
            fd = file_open(inode_no,flags);
    }
    
    /*此fd是指任务pcb->fd_table数组中的元素下标，并不是指全局file_table中的下标*/
    
    return fd; 
}

/*将文件描述符转化为文件表的下标*/
static uint32_t fd_local2global(uint32_t local_fd){
    struct task_struct* cur = running_thread();
    int32_t global_fd = cur->fd_table[local_fd];
    ASSERT(global_fd>=0&&global_fd<MAX_FILE_OPEN);
    return (uint32_t)global_fd;
}

/*关闭文件描述符fd指向的文件，成功返回0，失败返回-1*/
int32_t sys_close(int32_t fd){
    int32_t ret = -1;   //返回值默认为-1，即失败
    if(fd>2){
        uint32_t _fd = fd_local2global(fd);
        ret = file_close(&file_table[_fd]);
        running_thread()->fd_table[fd] = -1;//使得文件描述符位可用
    }
    return ret;
}

/*将buf中连续count个字节写入文件描述符fd,成功则返回写入的字节数，失败返回-1*/
int32_t sys_write(int32_t fd, const void* buf, uint32_t count){
    if(fd<0){
        printk("sys_write:fd error\n");
        return -1;
    }

    if(fd==stdout_no){
        char tmp_buf[1024] = {0};
        memcpy(tmp_buf,buf,count);
        console_put_str(tmp_buf);
        return count;
    }

    uint32_t _fd = fd_local2global(fd);
    struct file* wr_file = &file_table[_fd];
    if(wr_file->fd_flag & O_WRONLY || wr_file->fd_flag & O_RDWR){
        uint32_t bytes_written = file_write(wr_file,buf,count);
        return bytes_written;
    }else{
        console_put_str("sys_write: not allowed to write file without flag O_RDWR or O_WRONLY\n");
        return -1;
    }
}

/*从文件描述符 fd 指向的文件中读取 count 个字节到 buf,若成功则返回读出的字节数，到文件尾则返回-1*/
int32_t sys_read(int32_t fd, void* buf, uint32_t count){
    if(fd<0){
        printk("sys_read:fd error\n");
        return -1;
    }
    ASSERT(buf!=NULL);
    uint32_t _fd = fd_local2global(fd);
    return file_read(&file_table[_fd],buf,count);
}

/*在磁盘上搜索文件系统，若没有则格式化分区创建文件系统*/
void filesys_init(void){
    uint8_t channel_no = 0, dev_no, part_idx = 0;
    /*sb_buf用来存储从硬盘上读入的超级块*/
    struct super_block* sb_buf = (struct super_block*)sys_malloc(SECTOR_SIZE);
    
    if(sb_buf == NULL){
        PANIC("alloc memory failed!");
    }
    printk("searching  filesystem.......\n");
    while(channel_no < channel_cnt){
        dev_no = 0;
        while(dev_no<2){
            if(dev_no == 0){    //跨过裸盘hd60M.img
                dev_no++;
                continue;
            }
            struct disk* hd = &channels[channel_no].devices[dev_no];
            struct partition* part = hd->prim_parts;
            while(part_idx < 12){   //4个主分区+8个逻辑分区
                if(part_idx == 4){  //开始处理逻辑分区
                    part = hd->logic_parts;
                }

                /**
                 * channels数组是全局变量，默认值为0，disk属于其嵌套结构，
                 * partition又为disk的嵌套结构，因此partition中的成员默认也为0
                 * 若partition未初始化，则partition中的成员扔为0
                 * 下面处理存在的分区
                */
               if(part->sec_cnt!=0){    //如果分区存在
                    memset(sb_buf,0,SECTOR_SIZE);

                    /*独处分区的超级块，更具魔数是否正确判断是否存在文件系统*/
                    ide_read(hd,part->start_lba+1,sb_buf,1);

                    /*只支持自己的文件系统，若磁盘上已经有文件系统就不在格式化了*/
                    if(sb_buf->magic == 0x19590318){
                        printk("%s has filesystem\n",part->name);
                    }else{  //其他文件系统不支持，一律按无文件系统处理
                        printk("formatting %s`s partition %s........\n",hd->name,part->name);
                        partition_format(part);
                    }
                }
                part_idx++;
                part++; //下一分区
            }
            dev_no++;//下一磁盘
        }
        channel_no++;//下一通道
    }
    sys_free(sb_buf);
    /*确定默认操作的分区*/
    char default_part[8] = "sdb1";
    /*挂载分区*/
    list_traversal(&partition_list,mount_partition,(int)default_part);

    /*将当前分区的根目录打开*/
    open_root_dir(cur_part);
    
    /*初始化文件表*/
    uint32_t fd_idx=0;
    while(fd_idx<MAX_FILE_OPEN){
        file_table[fd_idx++].fd_inode = NULL;
    }
}