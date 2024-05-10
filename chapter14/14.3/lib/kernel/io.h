/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-01 11:50:11
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-01 16:06:16
 * @FilePath: /OS/chapter7/7.6/lib/kernel/io.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/***********************机器模式********************
 * b 一输出寄存器 QImode 名称，即寄存器中的最低 8 位：［a-d]l
 * w 一输出寄存器 HImode 名称，即寄存器中 个字节的部分，如［a-d]x
 * 
 * HImode
 *    "Half-Integer"模式，表示一个两字节的整数
 * QImode
 *    “Quarter—Integer”模式，表示一个字节的整数
 * ***************************************************/

#ifndef __LIB_IO_H
#define __LIB_IO_H
#include "stdint.h"

/*向端口port写入一个字节*/
static inline void outb(uint16_t port,uint8_t data){
    /*
     * 对端口制定N表示0-255，d表示用dx存储端口号，
     * %b0表示对应al，%wl表示对于的dx
    */
   asm volatile ("out %b0,%w1": : "a" (data), "Nd" (port));
}

/*将addr处起始的word_cnt个字写入端口port*/
static inline void outsw(uint16_t port,const void* addr,uint32_t word_cnt){
    /*********
     * +表示此限制即做输入，又做输出
     * outsw是吧ds:esi处的16位的内容写入port端口，我们在设置短描述符时,这里 outsw 将从 ds:esi 处读取一个 16 位字，并将其写入到 port 端口。
     * 已经将ds,es,ss断电选择子都设置为想通的值了，此时不用担心数据错乱
    */
   asm volatile ("cld; \
            rep outsw":"+S"(addr),"+c"(word_cnt):"d"(port));
}

/*从端口port读入的一个字节返回*/
static inline uint8_t inb(uint16_t port){
    uint8_t data;
    //asm volatile ("inb %w1, %b0":"=a"(data),"Nd"(port));
    asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

/*将从端口port读入的word_cnt个字节写入addr*/
static inline void insw(uint16_t port, void* addr, uint32_t word_cnt){
    /*****
     * insw是将从端口port处读入的16位内容写入es:edi指向的内存，
     * 我们在设置段描述符，已经将ds,es,ss段的选择子都设置为想通的值了，
     * 此时不用担心数据错乱。
    */

   asm volatile ("cld; \
            rep insw":"+D"(addr),"+c"(word_cnt):"d"(port):"memory");
}
#endif // !__LIB_IO_H
