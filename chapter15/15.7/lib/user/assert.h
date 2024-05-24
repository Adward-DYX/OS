/*
 * @Author: Adward-DYX 1654783946@qq.com
 * @Date: 2024-05-21 13:41:46
 * @LastEditors: Adward-DYX 1654783946@qq.com
 * @LastEditTime: 2024-05-21 13:41:58
 * @FilePath: /OS/chapter15/15.4/lib/user/assert.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __LIB_USER_ASSERT_H
#define __LIB_USER_ASSERT_H

#define NULL ((void*)0)

void user_spin(char* filename, int line, const char* func, const char* condition);
#define panic(...) user_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef NDEBUG
   #define assert(CONDITION) ((void)0)
#else
   #define assert(CONDITION)  \
      if (!(CONDITION)) {     \
	 panic(#CONDITION);   \
      }    

#endif/*NDEBUG*/

#endif/*__LIB_USER_ASSERT_H*/
