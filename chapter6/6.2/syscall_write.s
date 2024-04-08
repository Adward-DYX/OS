section .data
str_c_lib: db "c library says: hello world!",0xa 
str_c_lib_len equ $-str_c_lib

str_syscall: db "syscall says: hello world!",0xa
str_syscall_len equ $-str_syscall

section .text
global _start
_start:
    ;方式1
    push str_c_lib_len  ;按照c调用约定压入参数
    push str_c_lib
    push 1

    call simu_write ;
    add esp, 12

    ;方式2
    mov eax, 4  ;第4号子功能是write系统调用
    mov ebx, 1
    mov ecx, str_syscall
    mov edx, str_syscall_len
    int 0x80   ;发起中断，通知 Linux 完成请求的功能

    ;退出程序
    mov eax, 1  ;第 1 号子功能是 exit
    int 0x80

;下面自定义的 simu_write 用来模拟 c 库中系统调用函数 write ,这里模拟原理
simu_write:
    push ebp
    mov ebp, esp
    mov eax, 4
    mov ebx, [ebp+8]
    mov ecx, [ebp+12]
    mov edx, [ebp+16]
    int 0x80
    pop ebp
    ret