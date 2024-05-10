/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-07 10:35:58
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-07 15:41:12
 * @FilePath: /OS/chapter14/14.2/fs/dir.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _FS_DIR_H
#define _FS_DIR_H
#include "stdint.h"
#include "inode.h"
#include "fs.h"
#include "global.h"

#define MAX_FILE_NAME_LEN 16    //最大文件名长度

/*目录结构*/
struct dir{
    struct inode* inode;
    uint32_t dir_pos;   //记录在目录内的偏移
    uint8_t dirr_buf[512];  //目录的数据缓存
};

/*目录项结构*/
struct dir_entry{
    char filename[MAX_FILE_NAME_LEN];   //普通文件或目录名称
    uint32_t i_no;  //普通文件或目录对应的inode编号
    enum file_types f_type; //文件类型
};

#endif // !_FS_DIR_H
