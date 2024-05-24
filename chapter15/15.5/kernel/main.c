/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-22 10:30:20
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-23 11:00:05
 * @FilePath: /OS/chapter15/15.5/kernel/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "print.h"
#include "init.h"
#include "fork.h"
#include "stdio.h"
#include "syscall.h"
#include "assert.h"
#include "shell.h"
#include "console.h"
#include "ide.h"
#include "stdio-kernel.h"

void init(void);

int main(void)
{
    put_str("I am kernel\n");
    init_all();

    uint32_t file_size = 20928;      //这个变量请自行修改成自己的prog_no_arg大小
    uint32_t sec_cnt = DIV_ROUND_UP(file_size, 512);
    struct disk *sda = &channels[0].devices[0];
    void *prog_buf = sys_malloc(file_size);
    ide_read(sda, 300, prog_buf, sec_cnt);
    int32_t fd = sys_open("/prog_no_arg", O_CREAT | O_RDWR);
    if (fd != -1)
    {
        if (sys_write(fd, prog_buf, file_size) == -1)
        {
            printk("file write error!\n");
            while (1)
                ;
        }
    }

    cls_screen();
    console_put_str("[rabbit@localhost /]$ ");
    while (1)
        ;
    return 0;
}

/* init进程 */
void init(void)
{
    uint32_t ret_pid = fork();
    if (ret_pid)
    { // 父进程
        while (1)
            ;
    }
    else
    { // 子进程
        my_shell();
    }
    panic("init: should not be here");
}
