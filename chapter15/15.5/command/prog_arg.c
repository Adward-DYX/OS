/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-22 12:46:30
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-23 10:23:27
 * @FilePath: /OS/chapter15/15.5/command/prog_no_arg.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stdio.h"
#include "syscall.h"
#include "string.h"

int main(int argc, char** argv){
    //printf("prog_no_arg from disk\n");
    int arg_idx = 0;
    while(arg_idx < argc){
        printf("argc[%d] is %s\n", arg_idx, argv[arg_idx]);
        arg_idx++;
    }
    int pid = fork();
    if(pid){
        int delay = 900000;
        while(delay--);
        printf("\n  I`m father prog, my pid:%d, I will show process list\n",getpid());
        ps();
    }else{
        char abs_path[512] = {0};
        printf("\n  I`m child prog, m pid:%d, I will exec %s right now\n", getpid(),argv[1]);
        if(argv[1][0]!='/'){
            getcwd(abs_path, 512);
            strcat(abs_path, "/");
            strcat(abs_path, argv[1]);
            execv(abs_path, argv);
        }else{
            execv(argv[1],argv);
        }
    }
    while(1);
    return 0;
}