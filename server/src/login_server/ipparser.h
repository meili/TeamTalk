/*================================================================
*   Copyright (C) 2014 All rights reserved.
*   
*   �ļ����ƣ�ipparser.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2014��08��26��
*   ��    ����
*
#pragma once
================================================================*/
#ifndef _IPPARSER_H_
#define _IPPARSER_H_

#include "util.h"

class IpParser
{
    public:
        IpParser();
        virtual ~IpParser();
    bool isTelcome(const char* ip);
};

#endif

