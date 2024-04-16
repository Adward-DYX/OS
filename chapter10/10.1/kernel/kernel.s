[bits 32]
%define ERROR_CODE nop  ;若在相关的异常中CPU已经自动压入了
                        ;错误码，为了保存栈中格式统一，这里不做操作
%define ZERO push 0     ;若在相关的异常中CPU没有压入错误码
                        ;为了统一栈中的格式，就手动压入一个0
extern put_str  ;声明外部函数
extern idt_table

section .data
global inter_entry_table
inter_entry_table:

%macro VECTOR 2
section .text
intr%1entry:        ;每个中断处理函数都要压入中断向量号
                    ;所以一个中断类型一个中断处理程序
                    ;自己知道自己的中断向量号是多少
    %2
    ;以下是保持上下问环境
    push ds
    push es
    push fs
    push gs
    pushad        ;PUSHAD指令压入32位寄存器，其入栈顺序是：EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI最先入栈

    ;如果是从片上进入中断,除了往片上发送EOI，还要往主片上发送EOI
    mov al, 0x20    ;中断结束命令EOI
    out 0xa0, al    ;向从片发送
    out 0x20, al    ;向主片发送
    push %1       ;不管idt_table中的目标程序是否需要参数，都一律压入中断向量，调试时更加方便
    call [idt_table+%1*4];  调用idt_table中的C版本中断处理函数
    jmp intr_exit

section .data
    dd intr%1entry  ;存储各个中断入口程序的地址
                    ;形成intr_entry_table数组
%endmacro

section .text
global intr_exit
intr_exit:
    ;以下是恢复上下文环境
    add esp, 4     ;跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4     ;跳过error_code
    iretd

VECTOR 0x00,ZERO    
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO
VECTOR 0x08,ZERO
VECTOR 0x09,ZERO
VECTOR 0x0A,ZERO
VECTOR 0x0B,ZERO
VECTOR 0x0C,ZERO
VECTOR 0x0D,ZERO
VECTOR 0x0E,ZERO
VECTOR 0x0F,ZERO
VECTOR 0x10,ZERO
VECTOR 0x11,ZERO
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO
VECTOR 0x18,ZERO
VECTOR 0x19,ZERO
VECTOR 0x1A,ZERO
VECTOR 0x1B,ZERO
VECTOR 0x1C,ZERO
VECTOR 0x1D,ZERO
VECTOR 0x1E,ERROR_CODE
VECTOR 0x1F,ZERO
VECTOR 0x20,ZERO