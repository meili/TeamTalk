/*================================================================
*   Copyright (C) 2014 All rights reserved.
*   
*   文件名称：ipparser.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年08月26日
*   描    述：
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

