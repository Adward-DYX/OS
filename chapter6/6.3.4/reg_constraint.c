/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-03-27 09:41:04
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-03-27 09:45:55
 * @FilePath: /OS/chapter6/6.3.4/base_asm.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include<stdio.h>

void main(){
    int in_a=1,in_b=2,out_sum;
    asm("addl %%ebx, %%eax":"=a"(out_sum):"a"(in_a),"b"(in_b));
    printf("sum is %d\n",out_sum);
}