/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-24 10:28:00
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-24 13:06:41
 * @FilePath: /OS/chapter15/15.7/shell/pipe.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __SHELL_PIPE_H
#define __SHELL_PIPE_H

#include "stdint.h"
#include "global.h"

#define PIPE_FLAG    0xFFF

bool is_pipe(uint32_t local_fd);
void sys_fd_redirect(uint32_t old_local_fd,uint32_t new_local_fd);
int32_t sys_pipe(int32_t pipefd[2]);
uint32_t pipe_read(int32_t fd, void* buf, uint32_t count);
uint32_t pipe_write(int32_t fd, const void* buf, uint32_t count);
#endif // !__SHELL_PIPE_H
