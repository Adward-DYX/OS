/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-03-27 10:09:38
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-03-27 10:12:59
 * @FilePath: /OS/chapter6/6.3.4/reg4.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <stdio.h>
void main()
{
    int in_a = 0x12345678, in_b = 0;
    asm("movw %1,%0;":"=m"(in_b):"a"(in_a));
    printf("word in_b is 0x%x\n", in_b);
    in_b = 0;

    asm("movb %1,%0;":"=m"(in_b):"a"(in_a));
    printf("low byte in_b is 0x%x\n", in_b);

    in_b = 0;
    asm("movb %h1,%0;":"=m"(in_b):"a"(in_a));
    printf("high byte in_b is 0x%x\n", in_b);
}
