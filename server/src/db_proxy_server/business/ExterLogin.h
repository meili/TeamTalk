/*================================================================
*     Copyright (c) 2015�� lanhu. All rights reserved.
*   
*   �ļ����ƣ�ExterAuth.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2015��03��09��
*   ��    ����
*
#pragma once
================================================================*/
#ifndef __EXTERLOGIN_H__
#define __EXTERLOGIN_H__

#include "LoginStrategy.h"

class CExterLoginStrategy:public CLoginStrategy
{
public:
    virtual bool doLogin(const std::string& strName, const std::string& strPass, IM::BaseDefine::UserInfo& user);
};
#endif /*defined(__EXTERLOGIN_H__) */
