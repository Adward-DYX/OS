####  此脚本应该在command目录下执行
###
 # @Author: Adward-DYX 1654783946@qq.com
 # @Date: 2024-05-22 12:48:01
 # @LastEditors: Adward-DYX 1654783946@qq.com
 # @LastEditTime: 2024-05-23 11:00:56
 # @FilePath: /OS/chapter15/15.5/command/compile_no.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 

if [[ ! -d "../lib" || ! -d "../build" ]];then
   echo "dependent dir don\`t exist!"
   cwd=$(pwd)
   cwd=${cwd##*/}
   cwd=${cwd%/}
   if [[ $cwd != "command" ]];then
      echo -e "you\`d better in command dir\n"
   fi 
   exit
fi
CC="gcc-4.4"
BIN="prog_no_arg"
CFLAGS="-Wall -c -fno-builtin -W -Wstrict-prototypes \
      -Wmissing-prototypes -Wsystem-headers -m32 -fno-stack-protector"
LIB="../lib/"
OBJS="../build/string.o ../build/syscall.o \
      ../build/stdio.o ../build/assert.o"
DD_IN=$BIN
DD_OUT="/home/adward/OS/bochs/hd60M.img" 

$CC $CFLAGS -I $LIB -o $BIN".o" $BIN".c"
ld -e main $BIN".o" $OBJS -o $BIN -m elf_i386
SEC_CNT=$(ls -l $BIN|awk '{printf("%d", ($5+511)/512)}')

if [[ -f $BIN ]];then
   dd if=./$DD_IN of=$DD_OUT bs=512 \
   count=$SEC_CNT seek=300 conv=notrunc
fi

##########   以上核心就是下面这三条命令   ##########
#gcc -Wall -c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes \
#   -Wsystem-headers -I ../lib -o prog_no_arg.o prog_no_arg.c
#ld -e main prog_no_arg.o ../build/string.o ../build/syscall.o\
#   ../build/stdio.o ../build/assert.o -o prog_no_arg
#dd if=prog_no_arg of=/home/adward/OS/bochs/hd60M.img \
#   bs=512 count=10 seek=300 conv=notrunc

