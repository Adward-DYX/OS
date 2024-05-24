/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-23 15:11:22
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-23 16:44:34
 * @FilePath: /OS/chapter15/15.6/userprog/wait_exit.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __USERPROG_WAITEXIT_H
#define __USERPROG_WAITEXIT_H
#include "thread.h"
pid_t sys_wait(int32_t* status);
void sys_exit(int32_t status);
#endif 
