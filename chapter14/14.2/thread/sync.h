/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-15 10:27:16
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-15 11:06:10
 * @FilePath: /OS/chapter10/10.1/thread/sync.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H
#include "list.h"
#include "stdint.h"
#include "thread.h"

/*信号量结构*/
struct semaphore{
    uint8_t value;
    struct list waiters;
};

/*锁结构*/
struct lock{
    struct task_struct* holder; //锁的持有者
    struct semaphore semaphore; //用二元信号量实现锁
    uint32_t holder_repeat_nr;  //锁的持有者重复申请锁的次数
};

void sema_init(struct semaphore* psema, uint8_t value);
void lock_init(struct lock* plock);
void sema_down(struct semaphore* psema);
void sema_up(struct semaphore* psema);
void lock_acquire(struct lock* plock);
void lock_release(struct lock* plock);

#endif // !__THREAD_SYNC_H
