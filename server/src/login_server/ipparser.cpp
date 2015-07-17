/*================================================================
*   Copyright (C) 2014 All rights reserved.
*   
*   文件名称：ipparser.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年08月26日
*   描    述：
*
#include "ipparser.h"
================================================================*/
#include "ipparser.h"

IpParser::IpParser()
{
}

IpParser::~IpParser()
{
    
}

bool IpParser::isTelcome(const char *pIp)
{
    if(!pIp)
    {
        return false;
    }
    CStrExplode strExp((char*)pIp,'.');
    if(strExp.GetItemCnt() != 4)
    {
        return false;
    }
    return true;
}
