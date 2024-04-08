#ifndef __KERNEL_GLOBAL_H
#define __KERNEL_GLOBAL_H
#include "stdint.h"

#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1

#define SELECTOR_K_CODE ((1<<3)+(TI_GDT<<2)+RPL0)
#define SELECTOR_K_DATA ((2<<3)+(TI_GDT<<2)+RPL0)
#define SELECTOR_K_STACK SELECTOR_K_DATA
#define SELECTOR_K_GS ((3<<3)+(TI_GDT<<2)+RPL0)

/*-----------------------IDT描述符属性------------------*/
#define IDT_DESC_P 1    //有效位
#define IDT_DESC_DPL0 0   //描述符特权级别
#define IDT_DESC_DPL3   3
#define IDT_DESC_32_TYPE    0xE //32的门
#define IDT_DESC_16_TYPE    0x0 //16位的门
#define IDT_DESC_ATTR_DPL0 ((IDT_DESC_P<<7)+(IDT_DESC_DPL0<<5)+IDT_DESC_32_TYPE)
#define IDT_DESC_ATTR_DPL3 ((IDT_DESC_P<<7)+(IDT_DESC_DPL3<<5)+IDT_DESC_32_TYPE)
#endif // !__KERNEL_GLOBAL_H
