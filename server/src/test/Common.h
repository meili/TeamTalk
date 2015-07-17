/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：Common.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月20日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __COMMON_H__
#define __COMMON_H__
#include "ImPduBase.h"

#define PROMPT		"im-client> "
#define PROMPTION fprintf(stderr, "%s", PROMPT);

typedef void (*packet_callback_t)(CImPdu* pPdu);

#endif /*defined(__COMMON_H__) */
