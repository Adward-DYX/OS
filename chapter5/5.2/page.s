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
    add eax,0x10000
    loop .create_kernel_pde
    ret

