TI_GDT equ 0
RPL0 equ 0
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

[bits 32]
section .data
put_int_buffer dq 0 ;定义8字节缓冲区用于数字到字符串的转换
section .text
;------------------------------------------------------------------
;put_str 通过put_char来打印以0字符结尾的字符串
;------------------------------------------------------------------
global put_str
put_str:
;由于本函数只使用到了ebx和ecx寄存器
;编译器将该宇符串作为参数时，传递的是宇符串所在的内存起始地址，也就是说压入到校中的是存储该字符串的内存首地址。
    push ebx
    push ecx
    xor ecx, ecx
    mov ebx, [esp+12]
.goon:
    mov cl, [ebx]   ;从战中获取到的参数是字符串内存地址，我们需要对该地址进行内存寻址才能找到字符的 ASCII 码
    cmp cl, 0
    jz .str_over
    push ecx    ;将参数传给put_char 因为ecx寄存器存储了参数所以要压入栈,压入了32位操作数
    call put_char
    add esp, 4  ;回收参数所占用的空间 这个就是上面压入栈的空间
    inc ebx    ;使得ebx指向下一个字符
    jmp .goon
.str_over:
    pop ecx
    pop ebx
    ret


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
    ;mov esi, 0xb80a0;第一行行首
    mov esi, 0xc00b80a0;第一行行首
    ;mov edi, 0xb8000;di0行行首
    mov edi, 0xc00b8000;di0行行首
    rep movsd   ;rep movs word ptr es:[edi], word ptr ds:[esi] 简写为: rep movsw

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


;-------------------------将小端字节序的数字变成对应的ASCII后，倒置------------------------
;输入：栈中参数为待打印的数字
;输出：在屏幕上打印十六进制数字，并不会打印前缀0x
;如：打印十进制15时，只会打印f，不会打印0xf
;------------------------------------------------------------------------------------------

global put_int
put_int:
    pushad 
    mov ebp, esp
    mov eax, [ebp+4*9]
    mov edx, eax
    mov edi, 7          ;指定在put_int_buffer中初始的偏移
    mov ecx, 8          ;32位数字中，十六进制的位数是八个
    mov ebx, put_int_buffer

;将 32 位数字按照十六进制的形式从低位到高位逐个处理。一共处理8个十六进制数字
.16based_4bits: ;每4位二进制是十六进制数字的1位
;遍历每一个十六进制数字
    and edx, 0x0000000F ;解析十六进制数字的每一位,and与操作后,edx只有低4位
    cmp edx, 9      ;数字0-9和a-f需要分别处理成对应的字符
    jg .is_A2F
    add edx, '0'    ;ASCII码是8位大小 ，add求和操作后，edx低8位有效
    jmp .store
.is_A2F:
    sub edx, 10     ;A-F减去10所得到的差，再加上字符A的ASCII码，便是A-F对应的ASCII码
    add edx, 'A'
;将每一位数字转换成对应的字符后，按照类似“大端”的顺序存储到缓冲区 put_int_buffer
;高位字符放在低地址，低位字符要放在高地址，这样和大端字节序类似，只不过咱们这里是字符序
.store:
;此时dl中应该是数字对应的字符的 ASCII 码
    mov [ebx+edi],dl
    dec edi
    shr eax, 4
    mov edx, eax
    loop .16based_4bits

;现在 put_int_buffer 中已全是字符，打印之前；把高位连续的字符去掉，比如把字符000123变成123
.ready_to_print:
    inc edi     ;此时edi退减为－1(Oxffffffff ），加1使其为0。
.skip_prefix_0:
    cmp edi, 8  ;若已经比较第九个字符l
            ;表示待打印的字符串为全0。
    je .full0
;找出连续的0字符， edi作为非0的最高位字符的偏移
.go_on_skip:
    mov cl, [put_int_buffer+edi]
    inc edi
    cmp cl, '0'
    je .skip_prefix_0   ;继续判断下一位字符是否为字符0。
    dec edi ;edi在上面的inc操作中指向了下一个字符,若当前字符不为’0’，要使edi减l恢复指向当前字符
    jmp .put_each_num
.full0:
    mov cl, '0'     ;输入的数字全为0时，则只打印0
.put_each_num:
    push ecx
    call put_char
    add esp, 4
    inc edi 
    mov cl, [put_int_buffer+edi]
    cmp edi, 8
    jl .put_each_num
    popad
    ret

global set_cursor:
set_cursor:
    pushad
    mov bx, [esp+4*9]
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
    popad
    ret