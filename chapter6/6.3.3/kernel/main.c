/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-03-26 10:14:19
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-03-26 10:29:11
 * @FilePath: /OS/chapter6/6.3.3/kernel/main.c
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
    while(1);
}