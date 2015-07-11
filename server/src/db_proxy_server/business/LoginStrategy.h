/*================================================================
*     Copyright (c) 2015�� lanhu. All rights reserved.
*   
*   �ļ����ƣ�LoginStrategy.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2015��03��09��
*   ��    ����
*
#pragma once
================================================================*/
#ifndef __LOGINSTRATEGY_H__
#define __LOGINSTRATEGY_H__

#include <iostream>

#include "IM.BaseDefine.pb.h"

class CLoginStrategy
{
public:
    virtual bool doLogin(const std::string& strName, const std::string& strPass, IM::BaseDefine::UserInfo& user) = 0;
};

#endif /*defined(__LOGINSTRATEGY_H__) */
