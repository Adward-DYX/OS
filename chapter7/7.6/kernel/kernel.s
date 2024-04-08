[bits 32]
%define ERROR_CODE nop  ;若在相关的异常中CPU已经自动压入了
                        ;错误码，为了保存栈中格式统一，这里不做操作
%define ZERO push 0     ;若在相关的异常中CPU没有压入错误码
                        ;为了统一栈中的格式，就手动压入一个0
extern put_str  ;声明外部 函数
section .data
intr_str db "interrupt occur!", 0xa ,0
global inter_entry_table
inter_entry_table:

%macro VECTOR 2
section .text
intr%1entry:        ;每个中断处理函数都要压入中断向量号
                    ;所以一个中断类型一个中断处理程序
                    ;自己知道自己的中断向量号是多少
    %2
    push intr_str

    call put_str
    add esp ,4     ;跳过参数

    ;若果是从片上进入的中断，除了往片上发生EOI外，还要往住主片上发送EOI
    mov al, 0x20    ;中断结束命令EOI
    out 0xa0, al    ;向从片发送
    out 0x20, al    ;向主片发送

    add esp,  4     ;跨越error_code
    iret            ;从中断返回，32位下等同指令iretd

section .data
    dd intr%1entry  ;存储各个中断入口程序的地址
                    ;形成intr_entry_table数组
%endmacro

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