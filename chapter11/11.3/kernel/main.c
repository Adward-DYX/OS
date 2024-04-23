/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-03-26 10:04:44
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-23 11:30:21
 * @FilePath: /OS/chapter6/6.3.4/kernel/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "print.h"
#include "init.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "process.h"

void k_thread_a(void* arg);
void k_thread_b(void* arg);
void u_prog_a(void);
void u_prog_b(void);
int test_var_a = 0, test_var_b = 0;
int main(void){
    put_str("I am kernel\n");
    init_all(); //初始化
   
    thread_start("k_thread_a", 31, k_thread_a, " A_");
    thread_start("k_thread_b", 31, k_thread_b, " B_");
    process_execute(u_prog_a,"user_prog_a");
    process_execute(u_prog_b,"user_prog_b");
    intr_enable();
    while(1){
    }
    return 0;
}

void k_thread_a(void* arg){
    //char* para = arg;
    int i=999999;
    while(1){
        while(i--);
        i=999999;
        console_put_str(" v_a:0x");
        console_put_int(test_var_a);
    }
}
void k_thread_b(void* arg){
    //char* para = arg;
    int i=999999;
    while(1){
        while(i--);
        i=999999;
        console_put_str(" v_b:0x");
        console_put_int(test_var_b);
    }
}

void u_prog_a(void){
    while(1){
        
        test_var_a++;
    }
}
void u_prog_b(void){
    while(1){ 
        test_var_b++;
    }
}