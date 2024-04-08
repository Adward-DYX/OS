/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-03-26 09:25:10
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-03-26 10:28:44
 * @FilePath: /OS/chapter6/6.3/lib/kernel/print.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H
#include "stdint.h"
void put_char(uint8_t char_asci);
void put_str(char* message);
#endif // !__LIB_KERNEL_PRINT_H
