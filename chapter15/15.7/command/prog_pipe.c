/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-24 11:02:54
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-24 11:13:16
 * @FilePath: /OS/chapter15/15.7/command/prog_pipe.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stdio.h"
#include "syscall.h"
#include "stdint.h"
#include "string.h"
int main(int argc, char** argv){
    int32_t fd[2] = {-1};
    pipe(fd);
    int32_t pid = fork();
    if(pid){//父进程
        close(fd[0]);   //关闭输入
        write(fd[1], "Hi, my son, I love you!", 24);
        printf("\nI`m father, my pid is %d\n",getpid());
        return 8;
    }else{
        close(fd[1]);   //关闭输出
        char buf[32] = {0};
        read(fd[0],buf,24);
        printf("\nI`m child, my pid is %d\n",getpid());
        printf("I`m child, my father said to me: \"%s\"\n",buf);
        return 9;
    }
}