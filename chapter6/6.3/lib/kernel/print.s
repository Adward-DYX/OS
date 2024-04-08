TI_GDT equ 0
RPL0 equ 0
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

[bits 32]
section .text
;put_char功能描述：把栈中的一个字符写入光标所在处
global put_char
put_char:
    pushad  ;备份 32 位寄存器环境
        ;需要保证 gs 中为正确的视频段选择子
        ;为保险起见，l 每次打印时都为 gs 赋值
    mov ax, SELECTOR_VIDEO  ;不能直接把立即数送入段寄存器
    mov gs, ax

;---------------------获取光标当前位置-----------------------------
;先获取高8位
    mov dx, 0x03d4  ;索引寄存器
    mov al, 0x0e    ;用于提供光标位置的高八位
    out dx, al
    mov dx, 0x03d5  ;通过读写数据端口 Ox3d5来获得或设置光标位置
    in al, dx       ;得到了光标位置的高8位 
    ;为什么不直接是 in ah, dx，因为如果源操作是8位寄存器，目的操作数必须是al,如果源操作数是16位寄存器，目的操作数必须是ax。
    mov ah, al

;在获取低8位
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in al, dx

;将光标存入 bx
    mov bx, ax
;下面这行是在栈中获取待打印的字符
    mov ecx, [esp+36]   ;pushad 压入 4 x 8 = 32 字节，这是那8个寄存器
                ;加上主调函数 4 字节的返回地址，故 esp+36 字节
    cmp cl, 0xd     ;CR回车符是0xd,LF换行符是0x0a
    jz .is_carriage_return
    cmp cl, 0xa
    jz .is_line_feed

    cmp cl, 0x8     ;BS(backspace）退格键的asc码是8
    jz .is_backspace
    jmp .put_other

.is_backspace:
;----------------------------backspace 的一点说明-----------------------
;当为backspace时，本质上只要将光标移向前一个显存位置即可．后面再输入的字符自然会覆盖此处的字符
;但有可能在键入 backspace 后并不再键入新的字符，这时光标已经向前移动到待删除的字符位置，但字符还在原处
;这就显得好怪异，所以此处添加了空格或空字符。
    dec bx
    shl bx, 1   ;光标左移1位等于乘2
            ;表示光标对应显存中的偏移字节，到显存上了
    mov byte [gs:bx], 0x20  ;将待删除的字节补为 0 或空格皆可 ,因为字符占两个字节这相当于是显存地址的低字节为ACS
    inc bx
    mov byte [gs:bx], 0x07  ;这是高字节为属性
    shr bx, 1   ;除2取整，回到光标位置
    jmp .set_cursor ;设置光标流程

.put_other:
    shl bx, 1
    mov [gs:bx], cl
    inc bx
    mov byte [gs:bx], 0x07
    shr bx, 1
    inc bx  ;下一个光标位置
    cmp bx, 2000
    jl .set_cursor  ;如果光标值小于2000，表示未写到显存的最后位置，则去设置新的光标值，如果超出了2000则换行处理

.is_line_feed:  ;换行符
.is_carriage_return:    ;回车符
    xor dx, dx   ;dx 是被除数的高 16 位，清 0
    mov ax, bx   ;ax 是被除数的低 16 位
    mov si, 80   ;由于是效仿 Linux ，Linux 中＼n 表示下一行的行首，所以本系统中把＼n和＼r都处理为Linux中\n的意思
    div si     ;也就是下一行的行首,标值减去除80的余数便是取整
;商将会存储在累加器 AX 中。
;余数将会存储在 DX 寄存器中。
    sub bx, dx  ;以上4行处理\r的代码

.is_carriage_return_end:    ;回车符 CR 处理结束
    add bx, 80
    cmp bx, 2000
.is_line_feed_end:      ;若是 LF (\n），将光标移＋80便可
    jl .set_cursor
    
;屏幕行范围是 0-24 ，滚屏的原理是将屏幕的第 1-24 行搬运到第 0-23 行,再将第 24 行用空格填充
.roll_screen:   ;若超出屏幕大小，开始滚屏
    cld
    mov ecx, 960 ;2000-80=1920字符要搬运，共190*2=3840字节 一次搬运4字节 就是960次
    mov esi, 0xb80a0;第一行行首
    mov edi, 0xb8000;di0行行首
    rep movsd

;将最后一行填充为空白
    mov ebx, 3840   ;最后行首字符的第个字节偏移＝1920 * 2 
    mov ecx, 80    ;一行是 80 字符（ 160 字节），每次清空 1 字符( 2 字节），一行需要移动 80 次

.cls:
    mov word [gs:ebx], 0x0720 ; Ox0720是黑底白字的空格键
    add ebx, 2
    loop .cls
    mov bx, 1920    ;将光标值重量为 1920 ，最后一行的首字符

.set_cursor:
;将光标设置为bx值
 ; ; ; ; ; ; ; 1 先设置高 8 位
    mov dx, 0x03d4  ;索引寄存器
    mov al, 0x0e   ;用于提供光标位置的高 8 位
    out dx, al
    mov dx, 0x03d5  ;通过读写数据端口 Ox3出来获得或设置光标位置
    mov al, bh
    out dx, al

;;;;;;; 2 再设置低 8 位 
    mov dx, 0x03d4  ;索引寄存器
    mov al, 0x0f   ;用于提供光标位置的低 8 位
    out dx, al
    mov dx, 0x03d5  ;通过读写数据端口 Ox3出来获得或设置光标位置
    mov al, bl
    out dx, al

.put_char_done:
    popad   ;恢复压入栈的寄存器
    ret