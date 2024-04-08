gcc-4.4 -c -m32 -o main.o main.c && ld main.o -Ttext 0xc0001500 -e main -o kernel.bin -m elf_i386 && dd if=kernel.bin of=../../../bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc
###
 # @Author: Adward-DYX 1654783946@qq.com
 # @Date: 2024-03-20 10:55:36
 # @LastEditors: Adward-DYX 1654783946@qq.com
 # @LastEditTime: 2024-03-25 09:59:09
 # @FilePath: /OS/chapter5/5.3/kernel/ok.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 
#-m elf_i386 是输出为32位