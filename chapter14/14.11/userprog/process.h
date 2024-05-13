/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-22 11:26:11
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-23 10:32:11
 * @FilePath: /OS/chapter11/11.3/userprog/process.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __USERPROG_PROCESS_H
#define __USERPROG_PROCESS_H


#include "stdint.h"
#include "string.h"
#include "global.h"
#include "thread.h"

#define default_prio 31
#define USER_STACK3_VADDR (0xc0000000 - 0x1000)
#define USER_VADDR_START    0x80480000

void start_process(void* filename_);
void page_dir_activate(struct task_struct* p_thread);
void process_activate(struct task_struct* p_thread);
uint32_t* create_page_dir(void);
void create_user_vaddr_bitmap(struct task_struct* user_prog);
void process_execute(void* filename, char* name);
#endif // !__USERPROG_PROCESS_H
