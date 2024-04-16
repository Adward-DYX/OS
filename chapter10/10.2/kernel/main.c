/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-03-26 10:04:44
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-15 12:28:13
 * @FilePath: /OS/chapter6/6.3.4/kernel/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "print.h"
#include "init.h"


void k_thread_a(void* arg);
void k_thread_b(void* arg);
int main(void){
    put_str("I am kernel\n");
    init_all(); //初始化
   
    int i = 999999;
    thread_start("k_thread_a", 31, k_thread_a, "argA ");
    thread_start("k_thread_b", 8, k_thread_b, "argB ");
    intr_enable();
    //asm volatile("sti");    //为了演示中断处理，在此临时开中断
    while(1){
        while(i--);
        i=999999;
        console_put_str("main ");
    }
    return 0;
}

void k_thread_a(void* arg){
    int i = 999999;
    char* para = arg;
    while(1){
        while(i--);
        i=999999;
        console_put_str(para);
    }
}
void k_thread_b(void* arg){
    int i = 999999;
    char* para = arg;
    while(1){
        while(i--);
        i=999999;
        console_put_str(para);
    }
}