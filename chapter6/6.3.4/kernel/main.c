/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-03-26 10:04:44
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-03-26 11:17:29
 * @FilePath: /OS/chapter6/6.3.4/kernel/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "print.h"
void main(void){
    put_char('k');
    put_char('e');
    put_char('r');
    put_char('n');
    put_char('e');
    put_char('l');
    put_char('\n');
    put_char('1');
    put_char('2');
    put_char('3');
    put_char('\n');
    put_str("I am kernel\n");
    put_int(0);
    put_char('\n');
    put_int(9);
    put_char('\n');
    put_int(0x00021a3f);
    put_char('\n');
    put_int(0x12345678);
    put_char('\n');
    put_int(0x00000000);
    put_char('\n');
    while(1);
}