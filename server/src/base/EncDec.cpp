/*================================================================
 *     Copyright (c) 2015年 lanhu. All rights reserved.
 *
 *   文件名称：EncDec.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2015年01月27日
 *   描    述：
 *
 ================================================================*/

#include <string.h>
#include <stdio.h>
#include "EncDec.h"
#include "UtilPdu.h"
#include "Base64.h"

CAes::CAes(const string& strKey)
{
    AES_set_encrypt_key((const unsigned char*)strKey.c_str(), 256, &m_cEncKey);
    AES_set_decrypt_key((const unsigned char*)strKey.c_str(), 256, &m_cDecKey);
}

int CAes::Encrypt(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen)
{
    if(pInData == NULL|| nInLen <=0 )
    {
        return -1;
    }
    uint32_t nRemain = nInLen % 16;
    uint32_t nBlocks = (nInLen + 15) / 16;
    
    if (nRemain > 12 || nRemain == 0) {
        nBlocks += 1;
    }
    uint32_t nEncryptLen = nBlocks * 16;
    
    unsigned char* pData = (unsigned char*) calloc(nEncryptLen, 1);
    memcpy(pData, pInData, nInLen);
    unsigned char* pEncData = (unsigned char*) malloc(nEncryptLen);
    
    CByteStream::WriteUint32((pData + nEncryptLen - 4), nInLen);
    for (uint32_t i = 0; i < nBlocks; i++) {
        AES_encrypt(pData + i * 16, pEncData + i * 16, &m_cEncKey);
    }
    
    free(pData);
    string strEnc((char*)pEncData, nEncryptLen);
    free(pEncData);
    string strDec = base64_encode(strEnc);
    nOutLen = (uint32_t)strDec.length();
    
    char* pTmp = (char*) malloc(nOutLen + 1);
    memcpy(pTmp, strDec.c_str(), nOutLen);
    pTmp[nOutLen] = 0;
    *ppOutData = pTmp;
    return 0;
}

int CAes::Decrypt(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen)
{
    if(pInData == NULL|| nInLen <=0 )
    {
        return -1;
    }
    string strInData(pInData, nInLen);
    std::string strResult = base64_decode(strInData);
    uint32_t nLen = (uint32_t)strResult.length();
    if(nLen == 0)
    {
        return -2;
    }
    
    const unsigned char* pData = (const unsigned char*) strResult.c_str();
    
    if (nLen % 16 != 0) {
        return -3;
    }
    // 先申请nLen 个长度，解密完成后的长度应该小于该长度
    char* pTmp = (char*)malloc(nLen + 1);
    
    uint32_t nBlocks = nLen / 16;
    for (uint32_t i = 0; i < nBlocks; i++) {
        AES_decrypt(pData + i * 16, (unsigned char*)pTmp + i * 16, &m_cDecKey);
    }
    
    uchar_t* pStart = (uchar_t*)pTmp+nLen-4;
    nOutLen = CByteStream::ReadUint32(pStart);
    //        printf("%u\n", nOutLen);
    if(nOutLen > nLen)
    {
        free(pTmp);
        return -4;
    }
    pTmp[nOutLen] = 0;
    *ppOutData = pTmp;
    return 0;
}

void CAes::Free(char* pOutData)
{
    if(pOutData)
    {
        free(pOutData);
        pOutData = NULL;
    }
}

void CMd5::MD5_Calculate(const char *pContent, unsigned int nLen, char *md5)
{
    uchar_t d[16];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, pContent, nLen);
    MD5_Final(d, &ctx);
    for (int i=0; i<16; ++i) {
        snprintf(md5+(i*2), 32, "%02x", d[i]);
    }
    md5[32] = 0;
    return;
}