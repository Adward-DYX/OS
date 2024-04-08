###
 # @Author: Adward-DYX 1654783946@qq.com
 # @Date: 2024-03-26 10:14:19
 # @LastEditors: Adward-DYX 1654783946@qq.com
 # @LastEditTime: 2024-03-26 10:32:37
 # @FilePath: /OS/chapter6/6.3.3/kernel/ok.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 
gcc-4.4 -I ../lib/kernel -c -m32 -o main.o main.c 
ld ./main.o ../lib/kernel/print.o -Ttext 0xc0001500 -e main -o kernel.bin -m elf_i386 
dd if=kernel.bin of=../../../bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc