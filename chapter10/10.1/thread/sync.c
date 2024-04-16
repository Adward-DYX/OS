/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-15 10:43:15
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-15 11:11:47
 * @FilePath: /OS/chapter10/10.1/thread/sync.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "sync.h"
#include "interrupt.h"
#include "debug.h"
//初始化锁
void sema_init(struct semaphore* psema, uint8_t value){
    psema->value = value;       //为信号量赋值
    list_init(&psema->waiters);     //初始化信号量的等待队列
}

//初始化锁plock
void lock_init(struct lock* plock){
    plock->holder = NULL;
    plock->holder_repeat_nr = 0;
    sema_init(&plock->semaphore,1); //二元锁，信号量初始为1
}

//信号量down操作
void sema_down(struct semaphore* psema){
    /*关中断保证原子操作*/
    enum intr_status old_status = intr_disable();
    //别用if他只做一次判断，当阻塞后，被唤醒就开始后面的操作了，不会再次判断，如果线程数多会出错
    while(psema->value==0){ //若 value 为 0 ，表示已经被别人持有
        ASSERT(!elem_find(&psema->waiters,&running_thread()->general_tag));
        /*前线程不应该已在信号量的 waiters 队列中*/
        if(elem_find(&psema->waiters,&running_thread()->general_tag))
            PANIC("sema_down: thread blocked has been in waiters_list\n");
        /*若信号量的值等于 0 ，贝lj 当前线程把自己加入该锁的等待队列，然后阻塞自己*/
        list_append(&psema->waiters,&running_thread()->general_tag);
        thread_block(TASK_BLOCKED); //阻塞线程，直到唤醒
    }
    /*若 value 为 1 或被唤醒后，会执行下面的代码，也就是获得了锁*/
    psema->value--;
    ASSERT(psema->value==0);
    /*恢复之前的中断状态*/
    intr_set_status(old_status);

}

//信号量up操作
void sema_up(struct semaphore* psema){
    /*关中断，保证原子操作*/
    enum intr_status old_status = intr_disable();
    ASSERT(psema->value==0);
    if(!list_empty(&psema->waiters)){
        struct task_struct* thread_blocked = elem2entry(struct task_struct,general_tag, list_pop(&psema->waiters));
        thread_unblock(thread_blocked);
    }
    psema->value++;
    ASSERT(psema->value==1);
    /*恢复之前的中断*/
    intr_set_status(old_status);
}

/*获取锁*/
void lock_acquire(struct lock* plock){
    /*排除曾经自己已经持有锁但还未将其释放的情况*/
    if(plock->holder!=running_thread()){
        sema_down(&plock->semaphore);   //对信号量P操作，原子操作
        plock->holder = running_thread();
        ASSERT(plock->holder_repeat_nr==0);
        plock->holder_repeat_nr=1;
    }else{
        plock->holder_repeat_nr++;
    }
}

/*释放锁plock*/
void lock_release(struct lock* plock){
    ASSERT(plock->holder==running_thread());
    if(plock->holder_repeat_nr>1){
        plock->holder_repeat_nr--;
        return ;
    }
    ASSERT(plock->holder_repeat_nr==1);
    plock->holder = NULL;   //把锁的持有者置空放在 v 操作之前
    plock->holder_repeat_nr = 0;
    sema_up(&plock->semaphore); //信号量的 V 操作，也是原子操作
}