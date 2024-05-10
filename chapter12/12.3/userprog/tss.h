/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-19 10:19:44
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-23 10:28:51
 * @FilePath: /OS/chapter11/11.2/userprog/tss.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __USERPROG_TSS_H
#define __USERPROG_TSS_H
#include "stdint.h"
#include "global.h"
#include "string.h"
#include "thread.h"

void tss_init(void);
void update_tss_esp(struct task_struct* pthread);

#endif // !__USERPROG_TSS_H
