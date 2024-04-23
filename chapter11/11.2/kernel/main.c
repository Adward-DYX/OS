/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-03-26 10:04:44
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-17 14:30:53
 * @FilePath: /OS/chapter6/6.3.4/kernel/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "print.h"
#include "init.h"
#include "ioqueue.h"
#include "keyboard.h"

void k_thread_a(void* arg);
void k_thread_b(void* arg);
int main(void){
    put_str("I am kernel\n");
    init_all(); //初始化
   
    thread_start("k_thread_a", 31, k_thread_a, " A_");
    thread_start("k_thread_b", 31, k_thread_b, " B_");
    intr_enable();
    //asm volatile("sti");    //为了演示中断处理，在此临时开中断
    while(1){
        //while(i--);
        //i=999999;
        //console_put_str("main ");
        //if(!ioq_empty(&kbd_buf));
    }
    return 0;
}

void k_thread_a(void* arg){
    while(1){
        enum intr_status old_status = intr_disable();
        if(!ioq_empty(&kbd_buf)){
            console_put_str(arg);
            char byte = ioq_getchar(&kbd_buf);
            console_put_char(byte);
        }
        intr_set_status(old_status);
    }
}
void k_thread_b(void* arg){
    while(1){
        enum intr_status old_status = intr_disable();
        if(!ioq_empty(&kbd_buf)){
            console_put_str(arg);
            char byte = ioq_getchar(&kbd_buf);
            console_put_char(byte);
        }
        intr_set_status(old_status);
    }
}