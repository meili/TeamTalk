/*================================================================
*     Copyright (c) 2015�� lanhu. All rights reserved.
*   
*   �ļ����ƣ�InternalAuth.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2015��03��09��
*   ��    �����ڲ����ݿ���֤����
*
#pragma once
================================================================*/
#ifndef __INTERLOGIN_H__
#define __INTERLOGIN_H__

#include "LoginStrategy.h"

class CInterLoginStrategy :public CLoginStrategy
{
public:
    virtual bool doLogin(const std::string& strName, const std::string& strPass, IM::BaseDefine::UserInfo& user);
};

#endif /*defined(__INTERLOGIN_H__) */
