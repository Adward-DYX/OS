/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-21 09:19:25
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-21 09:27:09
 * @FilePath: /OS/chapter15/15.2/device/keyboard.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __DEVICE_KEYBOARD_H
#define __DEVICE_KEYBOARD_H

extern struct ioqueue kbd_buf;	   // 定义键盘缓冲区
void keyboard_init(void); 
#endif
