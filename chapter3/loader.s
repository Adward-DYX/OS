%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR
;输出背景色绿色，前景色红色，并且跳动的字符"1 MBR"
	mov byte [gs:0x00],'2'
	mov byte [gs:0x01],0xA4
	
	mov byte [gs:0x02],' '
	mov byte [gs:0x03],0xA4
	
	mov byte [gs:0x04],'L'
	mov byte [gs:0x05],0xA4
	
	mov byte [gs:0x06],'O'
	mov byte [gs:0x07],0xA4
	
	mov byte [gs:0x08],'A'
	mov byte [gs:0x09],0xA4
	
	mov byte [gs:0x0A],'D'
	mov byte [gs:0x0B],0xA4
	
	mov byte [gs:0x0C],'E'
	mov byte [gs:0x0D],0xA4
	
	mov byte [gs:0x0E],'R'
	mov byte [gs:0x0F],0xA4
	
	jmp $

