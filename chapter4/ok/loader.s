%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
	jmp loader_start

;构建gdt及其内部的描述符
GDT_BASE: 
	dd	0x00000000
	dd	0x00000000

CODE_DESC: 
	dd	0x0000FFFF
	dd	DESC_CODE_HIGH4
	   
DATA_STACK_DESC: 
	dd	0x0000FFFF
	dd	DESC_DATA_HIGH4
		 
VIDEO_DESC: 
	dd	0x80000007		;limit=(0xbffff-0xb8000)/4k=0x7
	dd	DESC_VIDEO_HIGH4	;此时dp1=0
	    
	GDT_SIZE equ $-GDT_BASE
	GDT_LIMIT equ GDT_SIZE-1
	times 60 dq 0	;预留60个描述符的空位置
	SELECTOR_CODE	equ	(0x0001<<3)+TI_GDT+RPL0	;相当于（ CODE_DESC - GDT_BASE) / 8 + TI_GDT + RPLO
	SELECTOR_DATA	equ	(0x0002<<3)+TI_GDT+RPL0
	SELECTOR_VIDEO	equ	(0x0003<<3)+TI_GDT+RPL0

;以下是 gdt 的指针，前 2 字节是 gdt界限，后 4 字节是 gdt 起始地址

gdt_ptr dw	GDT_LIMIT
	dd	GDT_BASE
loadermsg	db	'2 loader in real.'

loader_start:
	mov sp, LOADER_BASE_ADDR
	mov bp, loadermsg ;ES:BP ＝ 字符串地址
	mov cx, 17 ;ex ＝字符事长度
	mov ax, 0x1301 ;AH = 13, AL = Olh
	mov bx, 0x001f ;页号为 0 (BH = 0 ）蓝底粉红字（ BL = lfh)
	mov dx, 0x1800
	int 0x10 ;lOh 号中断

;一一一一一一一一一一 准备进入保护模式 一一一一一一一一一一一
;1.打开A20
;加载gdt
;将cr0的pe置1

;------------------------打开A20--------------------
	in al, 0x92
	or al, 0000_0010B
	out 0x92,al

;------------------------加载GDT--------------------
	lgdt 	[gdt_ptr]

;------------------------cr0的0位为1-----------------
	mov eax,cr0
	or eax,0x00000001
	mov cr0,eax

	jmp dword SELECTOR_CODE:p_mode_start	;刷新流水线

[bits 32]
p_mode_start:
	mov ax,	SELECTOR_DATA
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov esp, LOADER_STACK_TOP
	mov ax, SELECTOR_VIDEO
	mov gs, ax
	
	mov byte [gs:160], 'P'
	
	jmp $

