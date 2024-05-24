/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-12 17:58:41
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-15 11:14:24
 * @FilePath: /OS/chapter14/14.9 copy/kernel/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "print.h"
#include "init.h"
#include "fs.h"
#include "stdio.h"
#include "string.h"
#include "dir.h"
#include "syscall-init.h"

void init(void);

int main(void) {
   put_str("I am kernel\n");
   init_all();
   /************** 测试代码 *****************/
   //init();
   /************** 测试代码 *****************/
   while(1);
   return 0;
}

void init(void){
   uint32_t ret_pid = fork();
   if(ret_pid){
      printf("i am father, my pid is %d, child pid is %d\n",getpid(),ret_pid);
   }else{
      printf("i am child, my pid is %d, ret pid is %d\n",getpid(),ret_pid);
   }
}