/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-01 15:36:32
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-07 16:00:16
 * @FilePath: /OS/chapter7/7.6/kernel/init.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"

void init_all(){
    put_str("init_all\n");
    idt_init(); //初始化中断
    timer_init();   //初始化PIT8253
}