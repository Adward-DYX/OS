/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-22 10:33:08
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-22 11:39:07
 * @FilePath: /OS/chapter15/15.5/userprog/exec.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __USERPROG_EXEC_H
#define __USERPROG_EXEC_H
#include "stdint.h"
int32_t sys_execv(const char* path, const char*  argv[]);
#endif
