/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-04-08 11:22:49
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-04-09 17:20:19
 * @FilePath: /OS/chapter8/8.3/lib/string.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "string.h"
#include "global.h"
#include "debug.h"

/*将 dst＿起始的 size 个字节置为 value*/
void memset(void* dst_, uint8_t value, uint32_t size){
    ASSERT(dst_!= NULL);
    uint8_t* dst = (uint8_t*) dst_;
    while(size-->0)
        *dst++ = value;
}

/*将 src＿起始的 size 个字节复制到 dst_;*/
void memcpy(void* dst_, const void* src_, uint32_t size){
    ASSERT(dst_!=NULL&&src_!=NULL);
    uint8_t* dst = dst_;
    const uint8_t* src = src_;
    while(size-->0)
        *dst++=*src++;
}

/*续比较以地址a_和地址b_开头的 size 个字节，若相等则返回 O,若 a_大于 b_ ，返回 +1 ，否则返回－1 */
int memcmp(const void* a_, const void* b_, uint32_t size){
    const uint8_t* a = a_;
    const uint8_t* b = b_;
    ASSERT(a_!=NULL||b_!=NULL);
    while(size-->0){
        if(*a!=*b)
            return *a>*b ? 1:-1;
        a++;
        b++;
    }
    return 0;
}

/*字符串从 src＿复制到 dst_*/
char *strcpy(char* dst_, const char* src_){
    ASSERT(dst_!=NULL&&src_!=NULL);
    char* r = dst_;
    while((*dst_++=*src_++));
    return r;
}

/*返回字符串长度*/
uint32_t strlen(const char* str){
    ASSERT(str!=NULL);
    const char* p = str;
    while(*p++);
    return (p-str-1);
}

/*较两个字符串，若 a_中的字符大于 b_中的字符返回 1,相等时返回 0 ，否则返回 -1 . */
int8_t strcmp(const char* a, const char* b){
    ASSERT(a!=NULL&&b!=NULL);
    while(*a!=0&&*a==*b){
        a++;
        b++;
    }
    return *a<*b ? -1 : *a>*b;
}

/*从左到右查找字符串str中首次出现字符ch的地址*/
char* strchr(const char* str, const uint8_t ch){
    ASSERT(str!=NULL);
    while(*str!=0){
        if(*str==ch)
            return (char*)str; //需要强制转化成和返回值类型一样否则编译器会报 const 属性丢失
        str++;
    }
    return NULL;
}

/*从后往前查找字符串 str 中首次出现字符 ch 的地址*/
char* strrchr(const char* str, const uint8_t ch){
    ASSERT(str!=NULL);
    const char* last_char = NULL;
    while(*str!=0){
        if(*str==ch)
            last_char = str;
        str++;
    }
    return (char*)last_char;
}


/*将字符串src_拼接到dst_之后，返回拼接的串地址*/
char* strcat(char* dst_, const char* src_){
    ASSERT(dst_!=NULL&&src_!=NULL);
    char* str = dst_;
    while(*str++);
    --str;
    while((*str++=*src_++)); //当str被赋值为0时也就是表达式不成立，正好添加了字符串结尾的。
    return dst_;
}

/*在字符串str中查找字符ch出现次数*/
uint32_t strchrs(const char* str, uint8_t ch){
    ASSERT(str!=NULL);
    uint32_t ch_cnt = 0;
    const char* p = str;
    while(*p!=0){
        if(*p==ch)
            ch_cnt++;
        p++;
    }
    return ch_cnt;
}