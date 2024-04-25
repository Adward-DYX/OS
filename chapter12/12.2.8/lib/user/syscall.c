/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-24 14:05:39
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-25 09:57:43
 * @FilePath: /OS/chapter12/12.2.8/lib/user/syscall.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "syscall.h"

/* 无参数的系统调用 */
#define _syscall0(NUMBER) ({				       \
   int retval;					               \
   asm volatile (					       \
   "pushl %[number]; int $0x80; addl $4, %%esp"						       \
   : "=a" (retval)					       \
   : [number] "i" (NUMBER)					       \
   : "memory"						       \
   );							       \
   retval;						       \
})

/* 一个参数的系统调用 */
#define _syscall1(NUMBER, ARG1) ({			       \
   int retval;					               \
   asm volatile (					       \
   "int $0x80"						       \
   : "=a" (retval)					       \
   : "a" (NUMBER), "b" (ARG1)				       \
   : "memory"						       \
   );							       \
   retval;						       \
})

/* 两个参数的系统调用 */
#define _syscall2(NUMBER, ARG1, ARG2) ({		       \
   int retval;						       \
   asm volatile (					       \
   "pushl %[arg2]; pushl %[arg1]; pushl %[number]; int $0x80"   \
   "addl $12, %%esp"    \
   : "=a" (retval)					       \
   : [number] "i" (NUMBER), [arg1] "g" (ARG1), [arg2] "g" (ARG2)		       \
   : "memory"						       \
   );							       \
   retval;						       \
})

/* 三个参数的系统调用 */
#define _syscall3(NUMBER, ARG1, ARG2, ARG3) ({		       \
   int retval;						       \
   asm volatile (					       \
      "pushl %[arg3]; pushl %[arg2]; pushl %[arg1];"   \
      "pushl %[number]; int $0x80; addl $16, %%esp"	    \
      : "=a" (retval)					       \
      : [number] "i" (NUMBER), [arg1] "g" (ARG1), [arg2] "g" (ARG2),  [arg3] "g" (ARG3)      \
      : "memory"					       \
   );							       \
   retval;						       \
})

/* 返回当前任务pid */
uint32_t getpid() {
   return _syscall0(SYS_GETPID);
}

