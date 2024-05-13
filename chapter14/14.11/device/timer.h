/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-07 16:06:33
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-06 09:47:55
 * @FilePath: /OS/chapter7/7.8/device/timer.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __DEVICE_TIME_H
#define __DEVICE_TIME_H
#include "stdint.h"
void mtime_sleep(uint32_t m_seconds);
void timer_init(void);
#endif
