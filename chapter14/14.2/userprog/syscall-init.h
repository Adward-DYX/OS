/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-25 10:09:18
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-25 15:24:53
 * @FilePath: /OS/chapter12/12.3/userprog/syscall-init.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __USERPROG_SYSCALLINIT_H
#define __USERPROG_SYSCALLINIT_H
#include "stdint.h"
void syscall_init(void);
uint32_t sys_getpid(void);
uint32_t sys_write(char* str);
#endif
