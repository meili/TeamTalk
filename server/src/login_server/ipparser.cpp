/*================================================================
*   Copyright (C) 2014 All rights reserved.
*   
*   �ļ����ƣ�ipparser.cpp
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2014��08��26��
*   ��    ����
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
