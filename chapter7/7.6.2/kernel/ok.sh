gcc-4.4  -I ../lib/kernel/ -I ../lib/ -I ../kernel/ -c -fno-builtin -m32 -o ../build/main.o main.c
nasm -f elf -o ../build/print.o ../lib/kernel/print.s
nasm -f elf -o ../build/kernel.o ../kernel/kernel.s
gcc-4.4  -I ../lib/kernel/ -I ../lib/ -I ../kernel/ -c -fno-builtin -m32 -o ../build/interrupt.o interrupt.c
gcc-4.4  -I ../lib/kernel/ -I ../lib/ -I ../kernel/ -c -fno-builtin -m32 -o ../build/init.o init.c
ld  ../build/main.o ../build/init.o ../build/interrupt.o ../build/print.o ../build/kernel.o -Ttext 0xc0001500 -e main -o ../build/kernel.bin -m elf_i386
dd if=../build/kernel.bin of=../../../bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc
###
 # @Author: Adward-DYX 1654783946@qq.com
 # @Date: 2024-03-20 10:55:36
 # @LastEditors: Adward-DYX 1654783946@qq.com
 # @LastEditTime: 2024-04-01 16:22:19
 # @FilePath: /OS/chapter5/5.3/kernel/ok.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 
#-m elf_i386 是输出为32位