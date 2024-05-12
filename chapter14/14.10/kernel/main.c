/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-12 17:58:41
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-12 18:08:29
 * @FilePath: /OS/chapter14/14.9 copy/kernel/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "print.h"
#include "init.h"
#include "fs.h"
#include "stdio.h"
#include "string.h"

int main(void) {
   put_str("I am kernel\n");
   init_all();
   uint32_t fd1 = sys_open("/file1",O_RDWR);
   sys_write(fd1,"Adward_file1\n",14);
   uint32_t fd2 = sys_open("/file2",O_RDWR);
   sys_write(fd2,"Adward_file2\n",14);
   //printf("/file1 delete %s!\n", sys_unlink("/file1") == 0 ? "done" : "fail");
   //sys_unlink("/file2");
   while(1);
   return 0;
}