/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-14 15:41:45
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-15 09:52:19
 * @FilePath: /OS/chapter15/15.1/userprog/fork.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __USERPROG_FORK_H
#define __USERPROG_FORK_H
#include "stdint.h"
#include "thread.h"
#include "global.h"

pid_t sys_fork(void);
#endif // !__USERPROG_FORK_H
