/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-07 10:35:58
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-14 09:51:36
 * @FilePath: /OS/chapter14/14.2/fs/dir.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _FS_DIR_H
#define _FS_DIR_H
#include "stdint.h"
#include "inode.h"
#include "fs.h"
#include "ide.h"
#include "global.h"

#define MAX_FILE_NAME_LEN 16    //最大文件名长度

/*目录结构*/
struct dir{
    struct inode* inode;
    uint32_t dir_pos;   //记录在目录内的偏移,dir_pos是目录的“游标”，作用同文件结构中的fd_pos一样，用于记录下一个读写对象的地址，dir_pos用于指向目录中某个目录项的地址。 dir_pos在执行sys_opendir时就被置为0了
    uint8_t dirr_buf[512];  //目录的数据缓存
};

/*目录项结构*/
struct dir_entry{
    char filename[MAX_FILE_NAME_LEN];   //普通文件或目录名称
    uint32_t i_no;  //普通文件或目录对应的inode编号
    enum file_types f_type; //文件类型
};

extern struct dir root_dir;
void open_root_dir(struct partition* part);
struct dir* dir_open(struct partition* part, uint32_t inode_no);
bool search_dir_entry(struct partition* part, struct dir* pdir, const char* name, struct dir_entry* dir_e);
void dir_close(struct dir* dir);
void create_dir_entry(char* filename, uint32_t inode_no, uint8_t file_type, struct dir_entry* p_de);
bool sync_dir_entry(struct dir* parent_dir, struct dir_entry* p_de, void* io_buf);
bool delete_dir_entry(struct partition* part, struct dir* pdir, uint32_t inode_no, void* io_buf);
struct dir_entry* dir_read(struct dir* dir);
bool dir_is_empty(struct dir* dir);
int32_t dir_remove(struct dir* parent_dir, struct dir* child_dir);
#endif // !_FS_DIR_H
