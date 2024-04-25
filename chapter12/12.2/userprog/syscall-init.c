/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-24 11:08:58
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-24 13:26:46
 * @FilePath: /OS/chapter12/12.3/userprog/syscall-init.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "syscall-init.h"
#include "syscall.h"
#include "stdint.h"
#include "print.h"
#include "thread.h"

#define syscall_nr 32 
typedef void* syscall;
syscall syscall_table[syscall_nr];

/* 返回当前任务的pid */
uint32_t sys_getpid(void) {
   return running_thread()->pid;
}

/* 初始化系统调用 */
void syscall_init(void) {
   put_str("syscall_init start\n");
   syscall_table[SYS_GETPID] = sys_getpid;
   put_str("syscall_init done\n");
}
