/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-21 09:19:25
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-21 09:29:28
 * @FilePath: /OS/chapter15/15.2/lib/user/syscall.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H
#include "stdint.h"
enum SYSCALL_NR {
   SYS_GETPID,
   SYS_WRITE,
   SYS_MALLOC,
   SYS_FREE,
   SYS_FORK,
   SYS_READ
};
uint32_t getpid(void);
uint32_t write(int32_t fd, const void* buf, uint32_t count);
void* malloc(uint32_t size);
void free(void* ptr);
int16_t fork(void);
#endif

