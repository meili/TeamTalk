/*================================================================
*     Copyright (c) 2015�� lanhu. All rights reserved.
*   
*   �ļ����ƣ�Common.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2015��01��20��
*   ��    ����
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
