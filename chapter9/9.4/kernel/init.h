/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-01 15:40:44
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-14 14:49:27
 * @FilePath: /OS/chapter7/7.6/kernel/init.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __INIT_H
#define __INIT_H
#include "print.h"
#include "interrupt.h"
#include "timer.h"

void init_all(void);
#endif // DEBUG