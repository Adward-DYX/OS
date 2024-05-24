/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-21 13:41:41
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-21 13:42:05
 * @FilePath: /OS/chapter15/15.4/lib/user/assert.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "assert.h"
#include "stdio.h"
void user_spin(char* filename, int line, const char* func, const char* condition) {
   printf("\n\n\n\nfilename %s\nline %d\nfunction %s\ncondition %s\n", filename, line, func, condition);
   while(1);
}
