section call_test vstart=0x900
call far [addr]
jmp $
addr dw far_proc,0
far_proc:
	mov ax,0x1234
	retf
