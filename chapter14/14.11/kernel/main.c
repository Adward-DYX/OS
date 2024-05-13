/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-12 17:58:41
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-13 10:33:07
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
   printf("/dir1/subdir1 create %s!\n",sys_mkdir("/dir1/subdir1")==0 ? "done":"fail");
   printf("/dir1 create %s!\n",sys_mkdir("/dir1")==0 ? "done":"fail");
   printf("now, /dir1/subdir1 create %s!\n",sys_mkdir("/dir1/subdir1")==0 ? "done":"fail");
   int fd = sys_open("/dir1/subdir1/file3",O_CREAT|O_RDWR);
   if(fd != -1){
      printf("/dir1/subdir1/file3 create done!\n");
      sys_write(fd,"Catch me if you can!\n",21);
      sys_lseek(fd,0,SEEK_SET);
      char buf[32]= {0};
      sys_read(fd,buf,21);
      printf("/dir1/subdir1/file3 says:%s\n",buf);
      sys_close(fd);
   }
   //printf("/file1 delete %s!\n", sys_unlink("/file1") == 0 ? "done" : "fail");
   //sys_unlink("/file2");
   while(1);
   return 0;
}