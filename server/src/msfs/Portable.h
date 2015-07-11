/*
* 数据类型与一些可移植相关定义
*
*/

#ifndef _PORTABLE_H_
#define _PORTABLE_H_

typedef signed char 		s8;
typedef unsigned char 		u8;
typedef signed short 		s16;
typedef unsigned short 		u16;
typedef signed int 		s32;
typedef unsigned int 		u32;
typedef long long 		s64;
typedef unsigned long long 	u64;
typedef unsigned int 		uint;
typedef unsigned char 		byte;

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif


#endif
