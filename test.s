SECTION MBR vstart=0x7c00
	mov ax,0x0000	;设置栈指应该是程序一开始就应该做的事情,这个值是参照1m内存空间布局图选择的，以后会刻意避开
	mov ss,ax
	mov ax,0x7c00
	mov sp,ax	
 
	mov ax,0x0600
	mov bx,0x0700	;BH是设置缺省属性，属性是指背景色，前景色，是否闪烁等，例如07H表示黑底白字，70H表示灰底黑字等等。
	mov cx,0x0000
	mov dx,0x184f	;这个看书p61，同时看其中关于页的知识
	int 0x10
	
	mov ax,0x0300	
	mov bx,0x0000	
	int 0x10
	
	mov ax,0x0000
	mov es,ax
	mov ax,message
	mov bp,ax
	mov ax,0x1301
	mov bx,0x0007	;设置字体属性，02是黑底绿字，07是黑底白字
	mov cx,0x000c
	int 0x10
	
	jmp $
	message db "Hello World!"
	times 510-($-$$) db 0
	db 0x55,0xaa
