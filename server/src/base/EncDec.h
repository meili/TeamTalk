/*================================================================
 *     Copyright (c) 2015�� lanhu. All rights reserved.
 *
 *   �ļ����ƣ�EncDec.h
 *   �� �� �ߣ�Zhang Yuanhao
 *   ��    �䣺bluefoxah@gmail.com
 *   �������ڣ�2015��01��27��
 *   ��    ����AES
 *
 #pragma once
 ================================================================*/
#ifndef __ENCDEC_H__
#define __ENCDEC_H__

#include <iostream>
#include <openssl/aes.h>
#include <openssl/md5.h>
#include "ostype.h"

using namespace std;
class CAes
{
public:
    CAes(const string& strKey);
    
    int Encrypt(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen);
    int Decrypt(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen);
    void Free(char* pData);
    
private:
    AES_KEY m_cEncKey;
    AES_KEY m_cDecKey;
    
};

class CMd5
{
public:
    static void MD5_Calculate (const char* pContent, unsigned int nLen,char* md5);
};

#endif /*defined(__ENCDEC_H__) */