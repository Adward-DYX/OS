/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-24 09:10:23
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-24 09:21:43
 * @FilePath: /OS/chapter15/15.6/command/cat.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "syscall.h"
#include "stdio.h"
#include "string.h"
#include "fs.h"

int main(int argc,char** argv){
    if(argc > 2 || argc == 1){
        printf("cat: only support 1 argumen. \neg: cat filename\n");
        exit(-2);
    }
    int buf_size = 1024;
    char abs_path[512] = {0};
    void* buf = malloc(buf_size);
    if(buf==NULL){
        printf("cat:malloc memory failed\n");
        return -1;
    }
    if(argv[1][0] != '/'){
        getcwd(abs_path,512);
        strcat(abs_path, "/");
        strcat(abs_path,argv[1]);
    }else{
        strcpy(abs_path,argv[1]);
    }
    int fd = open(abs_path,O_RDONLY);
    if(fd == -1){
        printf("cat: open: open %s failed\n", argv[1]);
        return -1;
    }
    int read_bytes = 0;
    while(1){
        read_bytes = read(fd, buf, buf_size);
        if(read_bytes == -1){
            break;
        }
        write(1,buf,read_bytes);
    }
    free(buf);
    close(fd);
    return 66;
}