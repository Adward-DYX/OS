/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-03-27 10:31:32
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-03-27 10:41:03
 * @FilePath: /OS/chapter6/6.3.4/reg7.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <stdio.h>

void main()
{
    int ret_cnt = 0, test = 0;
    char* fmt ="hello,world\n"; 	//共 12 个字符
    asm(" push %1;	\
        call printf;	\
        add $4, %%esp;	\
        movl $6, %2"
        :"=&a"(ret_cnt)	\
        :"m"(fmt),"r"(test) 
       );
    printf("the number of bytes written is %d\n", ret_cnt);
}
//call printf;返回后屏幕会打印 hello, world 换行。在此， eax寄存器中值是 printf 的返回值，应该为 12 。
//add $4, %%esp; 回收参数所占的战空间 
//出错原因是%2被gee分配为寄存器 eax 了