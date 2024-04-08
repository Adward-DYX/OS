HD60M_PATH=/home/adward/OS/bochs-gdb/hd60M.img	#hd60M的路径改成支持GDB调试的bochs环境内的磁盘

ASFLAGS= -f elf -g
CFLAGS= -Wall $(LIB) -c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes -m32 -fno-stack-protector -g

.PHONY:mk_dir hd clean build all boot gdb_symbol	#定义了7个伪目标


#生成可以被GDB理解的符号表，用于GDB调试
gdb_symbol:
	objcopy --only-keep-debug $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/kernel.sym

#gdb_run: all
#	echo '' | $(BOCHS_GDB) -f bochsrc_gdb.bochs & \
#		gdb -ex "target remote:1234" -ex "symbol-file $(shell pwd)/build/kernel.sym"

all:mk_dir boot build hd gdb_symbol
#make all 就是依次执行mk_dir build hd gdb_symbol

