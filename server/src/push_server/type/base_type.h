//
//  base_type.h
//  my-code
//
//  Created by shiwei on 14-7-14.
//  Copyright (c) 2014Äê shiwei. All rights reserved.
//

#ifndef BASE_TYPE_H
#define BASE_TYPE_H

#ifdef _WIN32
#else
#include <sys/types.h>
#include <stdint.h>		// define int8_t ...
#include <errno.h>

#endif

#ifdef _WIN32

#if (_MSC_VER >= 1800)
//vs2013+
#include <stdint.h>
#else
typedef char			int8_t;
typedef short			int16_t;
typedef int				int32_t;
typedef	long long		int64_t;
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned int	uint32_t;
typedef	unsigned long long	uint64_t;
#endif

typedef int				socklen_t;
#else
typedef unsigned char	uchar_t;
typedef int BOOL;
const int TRUE = 1;
const int FALSE = 0;

#ifndef NULL
#define NULL 0
#endif 
#endif

#endif
