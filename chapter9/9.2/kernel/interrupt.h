/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-01 15:42:21
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-08 10:22:39
 * @FilePath: /OS/chapter7/7.6/kernel/interrupt.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __INTERRUPT_H
#define __INTERRUPT_H
#include "stdint.h"
typedef void* intr_handler;
void idt_init(void);

/*定义中断的两个状态*/
enum intr_status{
    INTR_OFF,
    INTR_ON
};

enum intr_status intr_enable(void);
enum intr_status intr_disable(void);
enum intr_status intr_set_status(enum intr_status status);
enum intr_status intr_get_status(void);
#endif // !__INTERRUPT_H
