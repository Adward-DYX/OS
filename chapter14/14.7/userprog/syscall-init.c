/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-28 13:54:30
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-28 14:12:35
 * @FilePath: /OS/chapter12/12.4.6/userprog/syscall-init.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "syscall-init.h"
#include "syscall.h"
#include "stdint.h"
#include "print.h"
#include "thread.h"
#include "console.h"
#include "string.h"
#include "memory.h"

#define syscall_nr 32 
typedef void* syscall;
syscall syscall_table[syscall_nr];

/* 返回当前任务的pid */
uint32_t sys_getpid(void) {
   return running_thread()->pid;
}

/* 打印字符串str(未实现文件系统前的版本) */
uint32_t sys_write(char* str) {
   console_put_str(str);
   return strlen(str);
}

/* 初始化系统调用 */
void syscall_init(void) {
   put_str("syscall_init start\n");
   syscall_table[SYS_GETPID] = sys_getpid;
   syscall_table[SYS_WRITE] = sys_write;
   syscall_table[SYS_MALLOC] = sys_malloc;
   syscall_table[SYS_FREE] = sys_free;
   put_str("syscall_init done\n");
}
