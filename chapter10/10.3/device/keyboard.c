/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-16 09:45:38
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-16 09:57:20
 * @FilePath: /OS/chapter10/10.3/device/keyboard.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"

#define KBD_BUF_PORT 0x60   //键盘buffer寄存器端口号为0x60

/*键盘中断处理程序*/
static void intr_keyboard_handler(void){
    //put_char('k');
    /*必须要读取输出缓冲寄存器，否则8042不再继续响应键盘中断*/
    uint8_t scancode = inb(KBD_BUF_PORT);
    put_int(scancode);
    return;
}

/*键盘初始化*/
void keyboard_init(){
    put_str("keyboard init start\n");
    register_handler(0x21,intr_keyboard_handler);
    put_str("keyboard init done\n");
}