/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-21 09:46:52
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-21 10:24:52
 * @FilePath: /OS/chapter15/15.4/lib/kernel/print.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H
#include "stdint.h"
void put_char(uint8_t char_asci);
void put_str(char* message);
void put_int(uint32_t num);	 // 以16进制打印
void set_cursor(uint32_t cursor_pos);
void cls_screen(void);
#endif

