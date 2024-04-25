/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-24 09:32:31
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-24 13:18:10
 * @FilePath: /OS/chapter12/12.3/kernel/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "process.h"
#include "syscall-init.h"
#include "syscall.h"

void k_thread_a(void*);
void k_thread_b(void*);
void u_prog_a(void);
void u_prog_b(void);
int prog_a_pid = 0, prog_b_pid = 0;

int main(void) {
   put_str("I am kernel\n");
   init_all();

   process_execute(u_prog_a, "user_prog_a");
   process_execute(u_prog_b, "user_prog_b");

   intr_enable();
   console_put_str(" main_pid:0x");
   console_put_int(sys_getpid());
   console_put_char('\n');
   thread_start("k_thread_a", 31, k_thread_a, "argA ");
   thread_start("k_thread_b", 31, k_thread_b, "argB ");
   while(1);
   return 0;
}

/* 在线程中运行的函数 */
void k_thread_a(void* arg) {     
   //char* para = arg;
   console_put_str(" thread_a_pid:0x");
   console_put_int(sys_getpid());
   console_put_char('\n');
   console_put_str(" prog_a_pid:0x");
   console_put_int(prog_a_pid);
   console_put_char('\n');
   while(1);
}

/* 在线程中运行的函数 */
void k_thread_b(void* arg) {     
   //char* para = arg;
   console_put_str(" thread_b_pid:0x");
   console_put_int(sys_getpid());
   console_put_char('\n');
   console_put_str(" prog_b_pid:0x");
   console_put_int(prog_b_pid);
   console_put_char('\n');
   while(1);
}

/* 测试用户进程 */
void u_prog_a(void) {
   prog_a_pid = getpid();
   while(1);
}

/* 测试用户进程 */
void u_prog_b(void) {
   prog_b_pid = getpid();
   while(1);
}
