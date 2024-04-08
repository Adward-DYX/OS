call near near_proc
jmp $
aaddr dd 4
near_proc:
 mov ax, 0x1234
 mov bx, ax
 ret
