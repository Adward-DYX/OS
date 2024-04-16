/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-15 11:26:36
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-15 12:29:28
 * @FilePath: /OS/chapter10/10.2/device/console.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __DEVICE_CONSOLE_H_
#define __DEVICE_CONSOLE_H_
#include "stdint.h"
#include "global.h"

void console_init(void);
void console_acquire(void);
void console_release(void);
void console_put_str(char* str);
void console_put_char(uint8_t char_asci);
void console_put_int(uint32_t num);

#endif // !__DEVICE_CONSOLE_H_
