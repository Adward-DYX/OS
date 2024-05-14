/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-12 17:58:41
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-14 10:22:37
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
   printf("/dir1 content before delete /dir1/subdir1:\n");
    struct dir *dir = sys_opendir("/dir1/");
    char *type = NULL;
    struct dir_entry *dir_e = NULL;
    while ((dir_e = sys_readdir(dir)))
    {
        if (dir_e->f_type == FT_REGULAR)
        {
            type = "regular";
        }
        else
        {
            type = "directory";
        }
        printf("      %s   %s\n", type, dir_e->filename);
    }
    printf("try to delete nonempty directory /dir1/subdir1\n");
    if (sys_rmdir("/dir1/subdir1") == -1)
    {
        printf("sys_rmdir: /dir1/subdir1 delete fail!\n");
    }

    printf("try to delete /dir1/subdir1/file3\n");
    if (sys_rmdir("/dir1/subdir1/file3") == -1)
    {
        printf("sys_rmdir: /dir1/subdir1/file3 delete fail!\n");
    }
    if (sys_unlink("/dir1/subdir1/file3") == 0)
    {
        printf("sys_unlink: /dir1/subdir1/file3 delete done\n");
    }

    printf("try to delete directory /dir1/subdir1 again\n");
    if (sys_rmdir("/dir1/subdir1") == 0)
    {
        printf("/dir1/subdir1 delete done!\n");
    }

    printf("/dir1 content after delete /dir1/subdir1:\n");
    sys_rewinddir(dir);
    while ((dir_e = sys_readdir(dir)))
    {
        if (dir_e->f_type == FT_REGULAR)
        {
            type = "regular";
        }
        else
        {
            type = "directory";
        }
        printf("      %s   %s\n", type, dir_e->filename);
    }
   /************** 测试代码 *****************/
   while(1);
   return 0;
}