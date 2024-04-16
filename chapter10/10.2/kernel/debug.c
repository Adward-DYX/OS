/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-08 09:38:54
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-08 10:26:38
 * @FilePath: /OS/chapter8/8.2/kernel/debug.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "debug.h"
#include "print.h"
#include "interrupt.h"

void panic_spin(char* filename, int line, const char* func, const char* condition){
    intr_disable(); //因为有时候会单独调用panic_spin，所以在这里先关闭中断
    put_str("\n\n\n!!!!!!!!!!!error!!!!!!!!!!!!!!!!\n");
    put_str("filename:");put_str(filename);put_str("\n");
    put_str("line:0x");put_int(line);put_str("\n");
    put_str("function:");put_str((char*)func);put_str("\n");
    put_str("condition:");put_str((char*)condition);put_str("\n");
    while(1);
}