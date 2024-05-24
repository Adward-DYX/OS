/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-01 15:36:32
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-15 14:01:29
 * @FilePath: /OS/chapter7/7.6/kernel/init.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "init.h"

void init_all(){
    put_str("init_all\n");
    idt_init();	     // 初始化中断
    mem_init();	     // 初始化内存管理系统
    thread_init();    // 初始化线程相关结构
    timer_init();     // 初始化PIT
    console_init();   // 控制台初始化最好放在开中断之前
    keyboard_init();  // 键盘初始化
    tss_init();       // tss初始化
    syscall_init();   // 初始化系统调用
    intr_enable();    // 后面的ide_init需要打开中断
    ide_init();	     // 初始化硬盘
    filesys_init();   // 初始化文件系统
}