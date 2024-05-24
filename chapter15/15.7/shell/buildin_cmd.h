/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-21 13:21:16
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-24 13:54:57
 * @FilePath: /OS/chapter15/15.4/shell/buildin_cmd.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __SHELL_BUILDIN_CMD_H
#define __SHELL_BUILDIN_CMD_H

#include "stdint.h"
#include "global.h"

void buildin_ls(uint32_t argc, char** argv);
char* buildin_cd(uint32_t argc, char** argv);
int32_t buildin_mkdir(uint32_t argc, char** argv);
int32_t buildin_rmdir(uint32_t argc, char** argv);
int32_t buildin_rm(uint32_t argc, char** argv);
void make_clear_abs_path(char* path, char* wash_buf);
void buildin_pwd(uint32_t argc, char** argv);
void buildin_ps(uint32_t argc, char** argv);
void buildin_clear(uint32_t argc, char** argv);
void buildin_help(uint32_t argc, char** argv);
#endif // !__SHELL_BUILDIN_CMD_H



