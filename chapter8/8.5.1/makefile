BUILD_DIR = ./build
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc-4.4
LD = ld
LIB = -I ./lib/ -I ./lib/kernel/ -I ./lib/user/ -I ./kernel/ -I ./device/
ASFLAGS = -f elf -g
I386 = -m elf_i386
ASBINLIB = -I ./boot/include/
#BOCHS_GDB = ../../bochs-gdb/
CFLAGS = -Wall $(LIB) -c -fno-builtin -m32 -W -Wstrict-prototypes -fno-stack-protector -g
LDFLAGS = -Ttext $(ENTRY_POINT) -e main -Map $(BUILD_DIR).map $(I386)
OBJS = $(BUILD_DIR)/mian.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/timer.o $(BUILD_DIR)/print.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/debug.o $(BUILD_DIR)/string.o $(BUILD_DIR)/bitmap.o $(BUILD_DIR)/memory.o


##############     MBR代码编译     ############### 
$(BUILD_DIR)/mbr.bin: ./boot/mbr.s
	$(AS) $(ASBINLIB) $< -o $@

##############     bootloader代码编译     ###############
$(BUILD_DIR)/loader.bin: ./boot/loader.s 
	$(AS) $(ASBINLIB) $< -o $@

################### c代码编辑 #########################
$(BUILD_DIR)/mian.o: ./kernel/main.c ./lib/kernel/print.h ./lib/stdint.h ./kernel/init.h ./kernel/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: ./kernel/init.c ./kernel/init.h ./kernel/interrupt.h ./lib/kernel/print.h ./lib/stdint.h ./device/timer.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: ./kernel/interrupt.c ./kernel/interrupt.h ./kernel/global.h ./lib/kernel/print.h ./lib/kernel/io.h ./lib/stdint.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: ./device/timer.c ./device/timer.h ./lib/kernel/print.h ./lib/kernel/io.h ./lib/stdint.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: ./kernel/debug.c ./kernel/debug.h ./lib/kernel/print.h ./lib/stdint.h ./kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/string.o: ./lib/string.c ./kernel/debug.h ./lib/kernel/print.h ./lib/stdint.h ./kernel/interrupt.h ./lib/string.h ./kernel/global.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/bitmap.o: ./lib/kernel/bitmap.c ./lib/kernel/bitmap.h ./kernel/debug.h ./lib/kernel/print.h ./lib/stdint.h ./kernel/interrupt.h ./lib/string.h ./kernel/global.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: ./kernel/memory.c ./kernel/memory.h ./lib/kernel/bitmap.h ./lib/string.h ./lib/kernel/print.h ./lib/stdint.h ./kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@
################### 汇编代码编辑 #########################
$(BUILD_DIR)/kernel.o: ./kernel/kernel.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/print.o: ./lib/kernel/print.s
	$(AS) $(ASFLAGS) $< -o $@

################### 链接所有目标文件 #########################
$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

.PHONY : mk_dir hd hd_gdb clean all gdb_symbol gdb_run

mk_dir:
	if [ ! -d $(BUILD_DIR) ];then mkdir $(BUILD_DIR);fi

hd:
	dd if=$(BUILD_DIR)/mbr.bin of=../../bochs/hd60M.img bs=512 count=1  conv=notrunc
	dd if=$(BUILD_DIR)/loader.bin of=../../bochs/hd60M.img bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin \
           of=../../bochs/hd60M.img \
           bs=512 count=200 seek=9 conv=notrunc

hd_gdb:
	dd if=$(BUILD_DIR)/mbr.bin of=../../bochs-gdb/hd60M.img bs=512 count=1  conv=notrunc
	dd if=$(BUILD_DIR)/loader.bin of=../../bochs-gdb/hd60M.img bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin \
           of=../../bochs-gdb/hd60M.img \
           bs=512 count=200 seek=9 conv=notrunc
clean:
	cd $(BUILD_DIR) && rm -f ./*

build: $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/mbr.bin $(BUILD_DIR)/loader.bin

#生成可以用于GDB调试的东东
gdb_symbol:
	objcopy --only-keep-debug $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/kernel.sym

gdb_run: mk_dir build hd_gdb gdb_symbol
	echo '' | $(BOCHS_GDB) -f bochsrc_gdb.bochs & \
	gdb -ex "target remote:1234" -ex "symbol-file $(shell pwd)/build/kernel.sym"
all: mk_dir build hd 