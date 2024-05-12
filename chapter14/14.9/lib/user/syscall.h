/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-24 11:09:17
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-11 13:47:12
 * @FilePath: /OS/chapter12/12.3/lib/user/syscall.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H
#include "stdint.h"
enum SYSCALL_NR {
   SYS_GETPID,
   SYS_WRITE,
   SYS_MALLOC,
   SYS_FREE
};
uint32_t getpid(void);
uint32_t write(int32_t fd, const void* buf, uint32_t count);
void* malloc(uint32_t size);
void free(void* ptr);
#endif

