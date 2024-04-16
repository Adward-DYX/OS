/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-11 09:19:18
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-15 10:03:47
 * @FilePath: /OS/chapter9/9.2/thread/thread.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE:
 */
#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "debug.h"
#include "interrupt.h"
#include "print.h"
#include "memory.h"
#include "list.h"

#define PG_SIZE 4096

struct task_struct* main_thread;    //主线程PCB
struct list thread_read_list;     //就绪队列
struct list thread_all_list;     //所有任务队列
static struct list_elem* thread_tag;    //用于保存队列中的线程结点

extern void switch_to(struct task_struct* cur, struct task_struct* next);

/*获取当前PCB*/
struct task_struct* running_thread(){
    uint32_t esp;
    asm ("mov %%esp, %0" : "=g" (esp));
    /*取esp整数部分，即pcb起始地址*/
    return (struct task_struct*)(esp & 0xfffff000);
}


/*由kernel_thread去执行function(func_arg)*/
static void kernel_thread(thread_func* function, void* func_arg){
    /*执行 function 前要开中断，避免后面的时钟中断被屏蔽，而无法调度其他线程*/
    intr_enable();
    function(func_arg);
}

/*初始化线程栈thread_stack,将待执行的函数和参数放到thread_stack中相应的位置*/
void thread_create(struct task_struct* pthread, thread_func function, void* func_arg){
    /*先预留中断使用栈的空间，可见thread.h中定义的结构*/
    pthread->self_kstack -= sizeof(struct intr_stack);

    /*再流出线程栈空间，可见thread.h中定义*/
    pthread->self_kstack -= sizeof(struct thread_stack);
    struct thread_stack* kthread_stack = (struct thread_stack*) pthread->self_kstack;
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi = kthread_stack->edi = 0;
}

/*初始化线程基本信息*/
void init_thread(struct task_struct* pthread, char* name, int prio){
    memset(pthread,0,sizeof(*pthread));
    strcpy(pthread->name,name);

    if(pthread == main_thread)
        /*由于把 main 函数也封装成一个线程，并且它一直是运行的，故将其直接设为 TASK_RUNNING*/
        pthread->status = TASK_RUNNING;
    else   
        pthread->status = TASK_READY;

    /*self_kstack是线程自己在内核态下使用的栈顶地址*/
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread+PG_SIZE);
    pthread->priority = prio;
    pthread->ticks = prio;
    pthread->elapsed_ticks = 0;
    pthread->pgdir = NULL;
    pthread->stack_magic = 0x19870916;  //自己定义的魔数
}

/*创建一优先级为prio的线程，线程名为name,线程所执行的函数是function(func_arg)*/
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg){
    /*pcb都位于内核空间，包括用户进程的pcb也是在内核空间*/
    struct task_struct* thread = get_kernel_pages(1);
    
    init_thread(thread,name,prio);
    thread_create(thread,function,func_arg);

    /*确保之前不在队列中*/
    ASSERT(!elem_find(&thread_read_list,&thread->general_tag));
    /*加入就绪线程队列*/
    list_append(&thread_read_list,&thread->general_tag);

    /*确保之前不在队列中*/
    ASSERT(!elem_find(&thread_all_list,&thread->all_list_tag));
    /*全部加入就绪线程队列*/
    list_append(&thread_all_list,&thread->all_list_tag);
    
    return thread;
}

/*将 kernel 中的 main 函数完善为主线*/
static void make_main_thread(void){
    /** 因为 main 线程早已运行，
     * 咱们在 loader.s 中进入内核时的 mov esp,Oxc009f000
     * 就是为其预留 pcb. 的，因此 pcb 地址为 Oxc009e000，
     * 不需要通过 get_kernel_pages 另分配一页＊
    */
   main_thread = running_thread();
   init_thread(main_thread,"main",31);

   /*main 函数是当前线程，当前线程不在 thread_ready_list 中，所以只将其加在 thread_all_list 中*/
   ASSERT(!elem_find(&thread_all_list,&main_thread->all_list_tag));
   list_append(&thread_all_list,&main_thread->all_list_tag);
}

/*当前线程将自己阻塞，标志其状态为 stat.*/
void thread_block(enum task_status stat){
    /*stat取值为 TASK_RUNNING、TASK_WAITING、TASK_HANGING也就是只有这三种状态才不会被调度＊*/
    ASSERT(((stat == TASK_RUNNING)||(stat == TASK_WAITING)||(stat == TASK_HANGING)));
    enum intr_status old_status = intr_disable();
    struct task_struct* cur_thread = running_thread();
    cur_thread->status = stat; //／置其状态为 stat
    schedule();         //将当前线程换下处理器
    /*待当前线程被解除阻塞后才继续运行下面的intr_set_status*/
    intr_set_status(old_status);
}

/*将线程 pthread 解除阻塞*/
void thread_unblock(struct task_struct* pthread){
    enum intr_status old_status = intr_disable();
    ASSERT(((pthread->status == TASK_RUNNING)||(pthread->status == TASK_WAITING)||(pthread->status == TASK_HANGING)));
    if(pthread->status!=TASK_READY){
        ASSERT(!elem_find(&thread_read_list,&pthread->general_tag));
        if(elem_find(&thread_read_list,&pthread->general_tag)){
            PANIC("thread_unblock: block thread in ready_list\n");
        }
        list_push(&thread_read_list,&pthread->general_tag);//放到队列的最前面，使其尽快得到调度
        pthread->status=TASK_READY;
    }
    intr_set_status(old_status);
}

/*实现调度器*/
void schedule(){
    ASSERT(intr_get_status()==INTR_OFF);
    struct task_struct* cur = running_thread();
    if(cur->status == TASK_RUNNING){
        //若此线程只是 cpu 时间片到了，将其加入到就绪队列尾
        ASSERT(!elem_find(&thread_read_list,&cur->general_tag));
        list_append(&thread_read_list,&cur->general_tag);
        cur->ticks = cur->priority;
        //重新将当前线程的ticks再重置为priority
        cur->status = TASK_READY;
    }else{
        /*此线程需要某事件发生后才能继续上 cpu 运行，不需要将其加入队列，因为当前线程不在就绪队列中*/
    }
    ASSERT(!list_empty(&thread_read_list));
    thread_tag = NULL;  //thread_tag清空
    /*将 thread_ready_list 队列中的第一个就绪线程弹出，准备将其调度上 cpu*/
    thread_tag = list_pop(&thread_read_list);
    //宏elem2en町的作用是将指针 elem_ptr 转换成 struct_type 类型的指针，其原理是用elem_ptr的地址减去 elem_ptr在结构体 struct_type 中的偏移量，此地址差便是结构体 struct_type 的起始地址，最后再将此地址差转换为 struct_type 指针类型
    struct task_struct* next = elem2entry(struct task_struct, general_tag, thread_tag);
    next->status = TASK_RUNNING;
    switch_to(cur,next);
}

/*初始化线程环境*/
void thread_init(){
    put_str("thread_init start\n");
    list_init(&thread_read_list);
    list_init(&thread_all_list);
    /*将当前main函数创建为线程*/
    make_main_thread();
    put_str("thread_init end\n");
}