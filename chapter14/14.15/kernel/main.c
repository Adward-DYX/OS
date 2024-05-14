/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-12 17:58:41
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-14 13:15:44
 * @FilePath: /OS/chapter14/14.9 copy/kernel/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "print.h"
#include "init.h"
#include "fs.h"
#include "stdio.h"
#include "string.h"
#include "dir.h"

int main(void) {
   put_str("I am kernel\n");
   init_all();
   /************** 测试代码 *****************/
   struct stat obj_stat;
   sys_stat("/",&obj_stat);
   printf("/`s info \n i_no:%d\n size:%d\n filetype:%s\n",obj_stat.st_ino,obj_stat.st_size,obj_stat.st_filetype == 2 ? "directory":"regular");
   sys_stat("/dir1",&obj_stat);
   printf("/`s info \n i_no:%d\n size:%d\n filetype:%s\n",obj_stat.st_ino,obj_stat.st_size,obj_stat.st_filetype == 2 ? "directory":"regular");
   /************** 测试代码 *****************/
   while(1);
   return 0;
}