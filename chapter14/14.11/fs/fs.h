/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-07 11:06:32
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-13 10:19:52
 * @FilePath: /OS/chapter14/14.2/fs/fs.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _FS_FS_H
#define _FS_FS_H
#include "stdint.h"
#include "global.h"

#define MAX_FILES_PER_PART 4096 //每个分区所支持最大创建的文件数
#define BITS_PER_SECTOR 4096    //每扇区的位数
#define SECTOR_SIZE 512 //扇区字节大小
#define BLOCK_SIZE SECTOR_SIZE  //块字节大小
#define MAX_PATH_LEN 512    //路径最大长度

/*文件类型*/
enum file_types{
    FT_UNKNOWN, //不支持的文件类型
    FT_REGULAR, //普通文件
    FT_DIRECTORY    //目录
};

/*打开文件的选项,这里是按“位”来定义各标识的*/
enum oflags{
    O_RDONLY,   //只读000
    O_WRONLY,   //只写001
    O_RDWR, //读写010
    O_CREAT = 4 //创建100
};

/*文件读写为止偏移量*/
enum whence{
    SEEK_SET = 1,
    SEEK_CUR,
    SEEK_END
};

/*用来记录查找文件过程中已找到的上级路径，也就是查找文件过程中”走过的地方··*/
struct path_search_record{
    char searched_path[MAX_PATH_LEN];   //查找过程中的父目录
    struct dir* parent_dir; //文件或目录所在的直接父目录
    enum file_types file_type;  //／找到的是普通文件，还是目录，找不到将为未知类型（ FT_UNKNOWN)
};

extern struct partition* cur_part;
void filesys_init(void);
int32_t pathr_depth_cnt(char* pathname);
int32_t sys_open(const char* pathname, uint8_t flags);
int32_t sys_close(int32_t fd);
int32_t sys_write(int32_t fd, const void* buf, uint32_t count);
int32_t sys_read(int32_t fd, void* buf, uint32_t count);
int32_t sys_lseek(int32_t fd, int32_t offset, uint8_t whence);
int32_t sys_mkdir(const char* pathname);
#endif // !_FS_FS_H
