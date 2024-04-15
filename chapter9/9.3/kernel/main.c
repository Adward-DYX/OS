/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-03-26 10:04:44
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-14 15:21:16
 * @FilePath: /OS/chapter6/6.3.4/kernel/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "print.h"
#include "init.h"
#include "thread.h"

void k_thread_a(void* arg);
int main(void){
    put_str("I am kernel\n");
    init_all();
    mem_init();
    asm volatile("cli" : : : "memory");//关闭中断
    thread_start("k_thread_a",31,k_thread_a,"arg A");
    //asm volatile("sti");    //为了演示中断处理，在此临时开中断
    while(1);
    return 0;
}

void k_thread_a(void* arg){
    intr_disable();
    char* para = arg;
    while(1){
        put_str(para);
    }
}
