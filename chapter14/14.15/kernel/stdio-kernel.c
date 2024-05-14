/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-28 17:33:08
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-06 12:59:59
 * @FilePath: /OS/chapter13/13.2/kernel/stdio-kernel.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stdio-kernel.h"
#include "console.h"
#include "stdint.h"
#include "global.h"
#include "stdio.h"

#define va_start(args, first_fix) args = (va_list)&first_fix
#define va_end(args) args = NULL


/*供内核使用的格式化输出函数*/
void printk(const char* format, ...){
    va_list args;
    va_start(args, format);
    char buf[1024] = {0};
    vsprintf(buf,format,args);
    va_end(args);
    console_put_str(buf);
}