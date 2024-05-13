/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-12 17:58:41
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-13 12:20:53
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
   struct dir* p_dir = sys_opendir("/dir1/subdir1");
   if(p_dir){
      printf("/dir1/subdir1 open done!\ncontent:\n");
      char* type = NULL;
      struct dir_entry* dir_e = NULL;
      while((dir_e=sys_readdir(p_dir))){
         if(dir_e->f_type == FT_REGULAR){
            type = "regular";
         }else{
            type = "directory";
         }
         printf(" %s %s\n",type,dir_e->filename);
      }
      if(sys_closedir(p_dir)==0){
         printf("/dir1/subdir1 close done!\n");
      }else{
         printf("/dir1/subdir1 close fail!\n");
      }
   }else{
      printf("/dir1/subdir1 open fail!\n");
   }
   /************** 测试代码 *****************/
   while(1);
   return 0;
}