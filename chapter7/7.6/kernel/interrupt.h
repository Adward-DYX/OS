/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-01 15:42:21
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-01 15:44:16
 * @FilePath: /OS/chapter7/7.6/kernel/interrupt.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __INTERRUPT_H
#define __INTERRUPT_H
#include "stdint.h"
typedef void* intr_handler;
void idt_init(void);

#endif // !__INTERRUPT_H
