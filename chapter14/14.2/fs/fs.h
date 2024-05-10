/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-07 11:06:32
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-07 15:29:03
 * @FilePath: /OS/chapter14/14.2/fs/fs.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _FS_FS_H
#define _FS_FS_H
#include "stdint.h"

#define MAX_FILES_PER_PART 4096 //每个分区所支持最大创建的文件数
#define BITS_PER_SECTOR 4096    //每扇区的位数
#define SECTOR_SIZE 512 //扇区字节大小
#define BLOCK_SIZE SECTOR_SIZE  //块字节大小

/*文件类型*/
enum file_types{
    FT_UNKNOWN, //不支持的文件类型
    FT_REGULAR, //普通文件
    FT_DIRECTORY    //目录
};

void filesys_init(void);
#endif // !_FS_FS_H
