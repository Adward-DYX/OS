%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR


;构建gdt及其内部的描述符
GDT_BASE:
    dd  0x00000000
    dd  0x00000000

CODE_DESC:
    dd  0x0000FFFF
    dd  DESC_CODE_HIGH4

DATA_STACK_DESC:
    dd  0x0000FFFF
    dd  DESC_DATA_HIGH4

VIDEO_DESC:
    dd  0x80000007
    dd  DESC_VIDEO_HIGH4

    GDT_SIZE    equ $-GDT_BASE
    GDT_LIMIT   equ GDT_SIZE-1
    times 60 dq 0 ;预留60个描述符的空位置
    ;构建段选择子
    SELECTOR_CODE   equ (0x0001<<3)+TI_GDT+RPL0
    SELECTOR_DATA	equ	(0x0002<<3)+TI_GDT+RPL0
    SELECTOR_VIDEO	equ	(0x0003<<3)+TI_GDT+RPL0

;total_mem_bytes 用于保存内存容量，以字节为单位，此位置比较好记
;当前偏移 loader.bin 文件头 Ox200 字节
;loader. bin 的加载地址是 Ox900
;故 total_mem_bytes 内存中的地址是 OxbOO
;将来在内核中咱们会引用此地址
    total_mem_bytes dd 0

;以下是 gdt 的指针，前 2 字节是 gdt界限，后 4 字节是 gdt 起始地址
gdt_ptr dw  GDT_LIMIT
    dd  GDT_BASE

;人工对齐:total_mem_bytes+gdt_ptr6+ards_buf244+adrs_nr2,共256字节，使得loader_start当前偏移 loader.bin文件头为0x300字节
    ards_buf times 244 db 0  ;创建一个名为 ards_buf 的数组或者缓冲区，它包含了 244 个字节的空间，并且所有的字节都被初始化为 0。
    ards_nr dw 0        ;创建一个名为 ards_nr 的字变量，并将其初始化为 0,用于记录ARDS结构体数量

loader_start:
;int 15h eax=0000E820h,edx=534D4150h ('SMAP')获取内存布局
    xor ebx,ebx ;第一次调用时，ebx值要为0
    mov ebx,0x534D4150  ;edx只赋值一次，循环体中不会改变
    mov di,ards_buf ;ards结构缓冲区
.e820_mem_get_loop:
    mov eax,0x0000e820  ;执行int 0x15，eax值变为0x534D4150,所以每次执行的是要要更新
    mov ecx,20      ;ARDS地址范围描述符结构大小为20字节
    int 0x15
    jc  .e820_failed_so_try_e801    ;若cf位为1则有错误发生，尝试0xe801子功能
    add di,cx   ;使di增加20字节指向缓冲区中新的ARDS结构
    inc word [ards_nr]  ;记录ARDS数量
    cmp ebx,0   ;若ebx为0且cf不为1，这说明ards全部返回，当前以为最后一个
    jnz .e820_mem_get_loop

;在所有的ards结构中，找出(base_add_low+length_low)的最大值，即内存容量
    mov cx,[ards_nr]    ;遍历所有的ARDS结构体，循环次数是ARDS的数量
    mov ebx,ards_buf
    xor edx,edx ;edx为最大的内存容量这里先清零
.find_max_mem_area: ;无需判断tyep是否为1，最大的内存块一定是可以用的
    mov eax,[ebx]   ;base_dd_low
    add eax,[ebx+8]   ;length_low
    add ebx,20  ;指向下一个缓存区
    cmp edx,eax;冒泡排序，找出最大值，edx寄存器始终存放的是最大值
    jge .next_ards  ;如果前面的比较结果表明源操作数大于或等于目的操作数，则跳转到标记为 .next_ards 的位置执行相应的指令
    mov edx,eax;edx为总内存大小
.next_ards:
    loop .find_max_mem_area
    jmp .mem_get_ok

;-----------int 15h ax=E801H获取内存大小，最大支持4G------------
;返回后，ax cx值一样，以KB为单位 ，bx dx值一样以64KB为单位
;ax cx寄存器中为低16MB，bx dx为16MB—4GB
.e820_failed_so_try_e801:
    mov ax,0xe801
    int 0x15
    jc  .e820_failed_so_try_88  ;若当前失效则去使用0x88方法

;先计算出低15MB，寄存器中存放的是内存数量，将其转化为以byte为单位
    mov cx,0x400;   0x400为1024,将ax中的内存容量换成以byte为单位的
    mul cx  ;16位乘法，高16为在的dx中低16在ax中
    shl edx,16;左移16位
    and eax,0x0000FFFF
    or edx,eax
    add edx,0x100000    ;ax只是15MB，故要加1MB
    mov esi,edx     ;先把低15MB的内存加入esi寄存器备用

;再计算16MB以上数据
    xor eax,eax
    mov ax,bx
    mov ecx,0x10000     ;0x10000十进制为64KB,即为64*1024
    mul ecx ;32位乘法，默认的被乘数是eax，积为64位，高32在edx，低32在eax

    add esi,eax ;由于次方法只能测出4GB的内存没所以32位就够了
    ;edx肯定为0,只加eax便可
    mov edx,esi ;eds为总内大小
    jmp .mem_get_ok

;--------------------------int 15 ah=0x88获取内存，只能获取64MB内存-----------------------
.e820_failed_so_try_88:
    ;int 15后，ax存放的是以KB为单位的内存容量
    mov ah,0x88
    int 0x15
    jc .error_hlt
    and eax,0x0000FFFF
    ;16位乘法，被乘数是ax，积为32位，高16在dx中，低16在ax中
    mov cx,0x400    ;0x400为1024,将ax中的内存容量换成以byte为单位的
    mul cx
    shl edx,16  ;把dx移到高16位
    or edx,eax  ;把积的低16为组合到edx，为32位的积
    add edx,0x100000   ;此中断只会显示 1MB 之上的内存，不包括这1MB,  咱们在使用的时候记得加上1MB 


.mem_get_ok:
    mov [total_mem_bytes],edx

;-----------------   准备进入保护模式   -------------------
;1 打开A20
;2 加载gdt
;3 将cr0的pe位置1

   ;-----------------  打开A20  ----------------
   in al,0x92
   or al,0000_0010B
   out 0x92,al

   ;-----------------  加载GDT  ----------------
   lgdt [gdt_ptr]

   ;-----------------  cr0第0位置1  ----------------
   mov eax, cr0
   or eax, 0x00000001
   mov cr0, eax

   jmp dword SELECTOR_CODE:p_mode_start	     ; 刷新流水线，避免分支预测的影响,这种cpu优化策略，最怕jmp跳转，
					     ; 这将导致之前做的预测失效，从而起到了刷新的作用。

.error_hlt:		      ;出错则挂起
    hlt

[bits 32]
p_mode_start:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp,LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    mov byte [gs:160], 'P'

;-----------------------------------------------------加载内核------------------------------------------
    mov eax, KERNEL_START_SECTOR    ;kernel.bin所在区域
    mov ebx, KERNEL_BIN_BASE_ADDR   ;从磁盘中读初后，写入到ebx指定的地址
    mov ecx, 200           ;读入扇区的数量

    call rd_disk_m_32


;创建页 目录及页表并初始化页内存位图
    call setup_page

;要将描述符表地址及偏移量写入内存 gdt_ptr ，一会儿用新地址重新加载
    sgdt [gdt_ptr]  ; 存储到原来 gdt 所有的位置

    mov ebx, [gdt_ptr+2]    ;将 gdt 描述符中视频段描述符中的段基址＋OxcOOOOOOO
    or dword [ebx+0x18+4], 0xc0000000   ;视频段是第3个段描述符，每个描述符是8字节，故Ox18,段描述符的高4字节的最高位是段基扯的第31 ～ 24位
    add dword [gdt_ptr+2], 0xc0000000   ;将 gdt 的基址加上 OxcOOOOOOO 使其成为内核所在的高地址
    add esp, 0xc0000000     ;将栈指针同样映射到内核地址

;把页目录地址赋给 cr3
    mov eax, PAGE_DIR_TABLE_POS
    mov cr3, eax

;打开 crO 的 pg 位（第 31 位）
    mov eax, cr0
    or eax, 0x80000000
    mov cr0,eax

;在开启分页后，用 gdt 新的地址重新加载
    lgdt [gdt_ptr]
    mov byte [gs:160], 'V'  ;视频段段基址已经被更新，用字符 v 表示 virtual addr

    jmp SELECTOR_CODE:enter_kernel
    

enter_kernel:
    mov byte [gs:320], 'k'     ;视频段段基址已经被更新
    mov byte [gs:322], 'e'     ;视频段段基址已经被更新
    mov byte [gs:324], 'r'     ;视频段段基址已经被更新
    mov byte [gs:326], 'n'     ;视频段段基址已经被更新
    mov byte [gs:328], 'e'     ;视频段段基址已经被更新
    mov byte [gs:330], 'l'     ;视频段段基址已经被更新

    mov byte [gs:480], 'w'     ;视频段段基址已经被更新
    mov byte [gs:482], 'h'     ;视频段段基址已经被更新
    mov byte [gs:484], 'i'     ;视频段段基址已经被更新
    mov byte [gs:486], 'l'     ;视频段段基址已经被更新
    mov byte [gs:488], 'e'     ;视频段段基址已经被更新
    mov byte [gs:490], '('     ;视频段段基址已经被更新
    mov byte [gs:492], '1'     ;视频段段基址已经被更新
    mov byte [gs:494], ')'     ;视频段段基址已经被更新
    mov byte [gs:496], ';'     ;视频段段基址已经被更新
    call kernel_init
    mov esp, 0xc009f000
    jmp KERNEL_ENTRY_POINT

;------------------------------------------------
;功能：读取硬盘的n个扇区
rd_disk_m_32:
;--------------------------------------------------
					;eax=LBA扇区号
					;ebx=将数据写入的内存地质
					;ecx=读入的扇区数
	mov esi,eax		;备份eax
	mov di,cx		;备份cx
;读写硬盘
;第一布：设置要读取的扇区数
	mov dx,0x1f2
	mov al,cl 
	out dx,al	;读取的扇区数
	
	mov eax,esi	;恢复ax
	
;第二步：将LBA地址存入0x1f3~0x1f6
	
	;LBA地址7-0位写入端口0x1f3
	mov dx,0x1f3
	out dx,al	
	
	;LBA地址15-8位写入端口0x1f4
	mov cl,8
	shr eax,cl
	mov dx,0x1f4
	out dx,al
	
	;LBA地址23-16位写入端口0x1f5
	shr eax,cl
	mov dx,0x1f5
	out dx,al
	
	shr eax,cl
	and al,0x0f	;lba第24-27位	
	or al,0xe0	;设置7-4位为1110,表示lba模式
	mov dx,0x1f6
	out dx,al
	
;第3步：向0x1f7端口哦写入读命令,0x20
	mov dx,0x1f7
	mov al,0x20
	out dx,al

;第4步：检测硬盘的状态
.not_ready:
   	;同一端口，写时表示写入命令字，读时表示读入硬盘状态
    nop
   	in al,dx
   	and al,0x88	;第 3 位为 1 表示硬盘控制器已准备好数据传输
			;第 7 位为 1 表示硬盘忙	
	cmp al,0x08
	jnz .not_ready	;若未准备号，继续等待

;第 5 步：从 OxlfO 端口读数据
	mov ax,di	;di里是扇区数
	mov dx,256
	mul dx
	mov cx,ax
;di 为要读取的扇区数，一个扇区有 512 字节，每次读入一个字,共需 di*512/2 次，所以 di*256	
	mov dx,0x1f0
.go_on_read:
    in ax,dx    ;16位操作数前面有66
    mov [ebx],ax
    add ebx,2
              ; 由于在实模式下偏移地址为16位,所以用bx只会访问到0~FFFFh的偏移。
			  ; loader的栈指针为0x900,bx为指向的数据输出缓冲区,且为16位，
			  ; 超过0xffff后,bx部分会从0开始,所以当要读取的扇区数过大,待写入的地址超过bx的范围时，
			  ; 从硬盘上读出的数据会把0x0000~0xffff的覆盖，
			  ; 造成栈被破坏,所以ret返回时,返回地址被破坏了,已经不是之前正确的地址,
			  ; 故程序出会错,不知道会跑到哪里去。
			  ; 所以改为ebx代替bx指向缓冲区,这样生成的机器码前面会有0x66和0x67来反转。
			  ; 0X66用于反转默认的操作数大小! 0X67用于反转默认的寻址方式.
			  ; cpu处于16位模式时,会理所当然的认为操作数和寻址都是16位,处于32位模式时,
			  ; 也会认为要执行的指令是32位.
			  ; 当我们在其中任意模式下用了另外模式的寻址方式或操作数大小(姑且认为16位模式用16位字节操作数，
			  ; 32位模式下用32字节的操作数)时,编译器会在指令前帮我们加上0x66或0x67，
			  ; 临时改变当前cpu模式到另外的模式下.
			  ; 假设当前运行在16位模式,遇到0X66时,操作数大小变为32位.
			  ; 假设当前运行在32位模式,遇到0X66时,操作数大小变为16位.
			  ; 假设当前运行在16位模式,遇到0X67时,寻址方式变为32位寻址
			  ; 假设当前运行在32位模式,遇到0X67时,寻址方式变为16位寻址.
    loop .go_on_read
    ret 

;-------------创建页目录及页表-------------------
setup_page:
;先把页目录占用大空间逐字节清0
    mov ecx, 4096
    mov esi, 0
.clear_page_dir:
    mov byte [PAGE_DIR_TABLE_POS+esi],0
    inc esi
    loop .clear_page_dir

;开创建页目录项PDE
.create_pde:
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x1000 ;此时eax为第一个页表的位置以及属性
    mov ebx, eax  ;此处为ebx赋值，是为了.create_pte做准备，ebx为基址

; 下面将页目录项 0 和 OxcOO 都存为第一个页表的地址，每个页表表示 4MB 内存
; 这样 Oxc03ffff 以下的地址和 Ox003fffff 以下的地址都指向相同的页表
; 这是为将地址映射为内核地址做准备
    or eax, PG_US_U | PG_RW_W | PG_P    ;页目录项的属性 RW 和 p 位为 1, us 为 1 ，表示用户属性，所有特权级别都可以访问
    mov [PAGE_DIR_TABLE_POS+0x0], eax    ;第一个目录项
                        ;在页目录表中的第 1 个 目录项写入第一个页表的位量（ 0x101000 ）及属性（7)
    mov [PAGE_DIR_TABLE_POS+0xc00], eax  ;一个页表项占用4字节,OxcOO 表示第768个页表占用的目录项， OxcOO以上的目录项用于内核空间
                        ;也就是页表的 OxcOOOOOOO ～ Oxffffffff共计lG属于内核,OxO-Oxbfffffff共计3G属于用户进程
    sub eax, 0x1000
    mov [PAGE_DIR_TABLE_POS+4092],eax   ;使最后一个目录项指向页目录表自己的地址

;下面创建页表项PTE
    mov ecx, 256     ;1M低端内存/每一页大小4k=256
    mov esi, 0   
    xor edx, edx        ;将edx置为0，现在edx指向0地址   
    mov edx, PG_US_U | PG_RW_W | PG_P    ;属性为7
.create_pte:    
    mov [ebx+esi*4], edx    ;此时的 ebx 已经在上茵通过eax赋值为Ox101000 ， 也就是第一个页表的地址
    add edx, 4096
    inc esi
    loop .create_pte
;创建内核其他页表的PDE
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x2000        ;此时 eax 为第二个页表的位置
    or eax, PG_US_U | PG_RW_W | PG_P;
    mov ebx, PAGE_DIR_TABLE_POS
    mov ecx, 254        ;范围为第769-1022的所有目录项数量
    mov esi, 769
.create_kernel_pde:
    mov [ebx+esi*4], eax
    inc esi
    add eax,0x1000
    loop .create_kernel_pde
    ret

kernel_init:
    xor eax, eax
    xor ebx, ebx    ;ebx记录程序头表地址
    xor ecx, ecx    ;cx记录程序头表中的program header数量
    xor edx, edx    ;dx记录program  header尺寸，即e_phentsize

    mov dx, [KERNEL_BIN_BASE_ADDR+42]   ;偏移文件42字节处的属性是e_phentsize，表示program header大小
    mov ebx, [KERNEL_BIN_BASE_ADDR+28]  ;偏移文件28字节处是e_poff表示第一个program header在文件中的偏移量
    add ebx, KERNEL_BIN_BASE_ADDR     ;这就是第一个program header的地址
    mov cx, [KERNEL_BIN_BASE_ADDR+44]  ;偏移文件44字节处是e_phnum表示第一个program header在文件中的数量，就是段个数

.each_segment:
    cmp byte [ebx+0], PT_NULL   ;若p_type等于PT_NULL，说明program header未使用
    je .PTNULL

    ;为函数memcpy压参数，参数是从右往左依次压入，函数原型类似于 memcpy(dst,src,size)
    push dword [ebx+16] ;program header中偏移16字节的地方是p_filesz,压入函数memcoy的第三个参数size
    mov eax, [ebx+4]    ;距程序头偏移量为4字节的位置是p_offest
    add eax, KERNEL_BIN_BASE_ADDR   ;加上kernel.bin被加载到的物理地址,eax为该段的物理地址
    push eax    ;压入函数memcpy的第二个参数：源地址
    push dword [ebx+8]  ;压入函数memcoy的第三个参数：目的地址,偏移程序头 8 字节的位置是 p_vaddr ，这就是目的地址
    call mem_cpy      ;调用mem_cpy完成段复制
    add esp, 12 ;清理栈中压入的三个参数
.PTNULL:
    add ebx, edx    ;edx为program header大小，即e_phentsize,在此ebx指向下一个program header
    loop .each_segment
    ret

;--------------------逐字的拷贝 mem_cpy(dst,src,size)-----------------------------
;输入：栈中的三个参数(dst,src,size)
;输出：无
;----------------------------------------------------------------------------------
mem_cpy:
    cld ;将方向标志位（Direction Flag）清零，确保字符串操作指令的方向是从源到目的地
    push ebp    ;保存当前函数的基址指针（Base Pointer，BP），为了后面恢复现场
    mov ebp, esp
    push ecx    ;rep指令通道了ecx，但ecx对于外层段的循环还有用，故先备份入栈
    mov edi, [ebp+8]    ;dst
    mov esi, [ebp+12]   ;src
    mov ecx, [ebp+16]   ;size
    rep movsb   ;逐字拷贝 ,重复执行movsb指令，将ECX寄存器指定数量的字节从DS:ESI（源地址）拷贝到ES:EDI（目的地址）。每执行一次movsb，ESI和EDI分别会根据方向标志的设置自动增减

    ;恢复环境
    pop ecx
    pop ebp
    ret
