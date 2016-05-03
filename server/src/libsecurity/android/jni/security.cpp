/*================================================================
 *   Copyright (C) 2015 All rights reserved.
 *
 *   文件名称：security.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2015年01月29日
 *   描    述：
 *
 #include "security.h"
 ================================================================*/
#ifdef __ANDROID__
#include <jni.h>
#include <android/log.h>
#define LOGV(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, "native-activity", __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-activity", __VA_ARGS__))
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "aes.h"
#include "aes_locl.h"
#include "base64.h"
#include "security.h"
#include "md5.h"

uint32_t ReadUint32(uchar_t *buf)
{
    uint32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    return data;
}

void WriteUint32(uchar_t *buf, uint32_t data)
{
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
}


#ifdef __cplusplus
extern "C" {
#endif
    
#ifdef __ANDROID__

    jbyteArray Java_com_mogujie_tt_Security_EncryptMsg(JNIEnv* env, jobject obj, jstring jstr)
    {
        const char *pInData = env->GetStringUTFChars(jstr, NULL);		//待加密内容,转换格式
        uint32_t nInLen = strlen(pInData);
        
        uint32_t nRemain = nInLen % 16;
        uint32_t nBlocks = (nInLen + 15) / 16;
        
        if (nRemain > 12 || nRemain == 0) {
            nBlocks += 1;
        }
        uint32_t nEncryptLen = nBlocks * 16;
        
        unsigned char* pData = (unsigned char*) calloc(nEncryptLen, 1);
        memcpy(pData, pInData, nInLen);
        env->ReleaseStringUTFChars(jstr,pInData);
        
        unsigned char* pEncData = (unsigned char*) malloc(nEncryptLen);
        
        WriteUint32((pData + nEncryptLen - 4), nInLen);
        AES_KEY aesKey;
        const char *key = "12345678901234567890123456789012";
        AES_set_encrypt_key((const unsigned char*)key, 256, &aesKey);
        for (uint32_t i = 0; i < nBlocks; i++) {
            AES_encrypt(pData + i * 16, pEncData + i * 16, &aesKey);
        }
        
        free(pData);
        string strEnc((char*)pEncData, nEncryptLen);
        free(pEncData);
        string strDec = base64_encode(strEnc);
        
        jbyteArray carr = env->NewByteArray(strDec.length());
        env->SetByteArrayRegion(carr,0,strDec.length(),(jbyte*)strDec.c_str());
        return carr;
    }
    
    /**
     * 解密
     */
    jbyteArray Java_com_mogujie_tt_Security_DecryptMsg(JNIEnv* env, jobject obj, jstring jstr)
    {
        const char *pInData = env->GetStringUTFChars(jstr, NULL);   //获取待揭秘内容,转换格式
        uint32_t nInLen = strlen(pInData);
        string strInData(pInData, nInLen);
        env->ReleaseStringUTFChars(jstr,pInData);
        std::string strResult = base64_decode(strInData);
        uint32_t nLen = (uint32_t)strResult.length();
        if(nLen == 0)
        {
            jbyteArray carr = env->NewByteArray(0);
            return carr;
        }
        
        const unsigned char* pData = (const unsigned char*) strResult.c_str();
        
        if (nLen % 16 != 0) {
            jbyteArray carr = env->NewByteArray(0);
            return carr;
        }
        // 先申请nLen 个长度，解密完成后的长度应该小于该长度
        char* pTmp = (char*)malloc(nLen + 1);
        
        uint32_t nBlocks = nLen / 16;
        AES_KEY aesKey;
        
        const char *key = "12345678901234567890123456789012";
        AES_set_decrypt_key((const unsigned char*) key, 256, &aesKey);           //设置AES解密密钥
        for (uint32_t i = 0; i < nBlocks; i++) {
            AES_decrypt(pData + i * 16, (unsigned char*)pTmp + i * 16, &aesKey);
        }
        uchar_t* pStart = (uchar_t*)pTmp+nLen-4;
        uint32_t nOutLen = ReadUint32(pStart);
        
        if(nOutLen > nLen)
        {
            free(pTmp);
            jbyteArray carr = env->NewByteArray(0);
            return carr;
        }
        pTmp[nOutLen] = 0;
        jbyteArray carr = env->NewByteArray(nOutLen);
        env->SetByteArrayRegion(carr,0,nOutLen,(jbyte*)pTmp);
        free(pTmp);
        return carr;
    }
    
    jbyteArray Java_com_mogujie_tt_Security_EncryptPass(JNIEnv* env, jobject obj, jstring jstr)
    {
        const char *pInData = env->GetStringUTFChars(jstr, NULL);		//待加密内容,转换格式
        uint32_t nInLen = strlen(pInData);
        if(pInData == NULL || nInLen <=0)
        {
            env->ReleaseStringUTFChars(jstr,pInData);
            jbyteArray carr = env->NewByteArray(0);
            return carr;
        }
        char *pTmp = (char*)malloc(33);
        if(pTmp == NULL)
        {
            env->ReleaseStringUTFChars(jstr,pInData);
            jbyteArray carr = env->NewByteArray(0);
            return carr;
        }
        MD5_Calculate(pInData, nInLen, pTmp);
        pTmp[32] = 0;
        env->ReleaseStringUTFChars(jstr,pInData);
        
        jbyteArray carr = env->NewByteArray(32);
        env->SetByteArrayRegion(carr,0,32,(jbyte*)pTmp);
        free(pTmp);
        return carr;
    }
    
#else
    int EncryptMsg(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen)
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

        WriteUint32((pData + nEncryptLen - 4), nInLen);
        AES_KEY aesKey;
        
        const char *key = "12345678901234567890123456789012";
        AES_set_encrypt_key((const unsigned char*)key, 256, &aesKey);
        for (uint32_t i = 0; i < nBlocks; i++) {
            AES_encrypt(pData + i * 16, pEncData + i * 16, &aesKey);
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
    
    int DecryptMsg(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen)
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
        AES_KEY aesKey;
        
        const char *key = "12345678901234567890123456789012";
        AES_set_decrypt_key((const unsigned char*) key, 256, &aesKey);           //设置AES解密密钥
        for (uint32_t i = 0; i < nBlocks; i++) {
            AES_decrypt(pData + i * 16, (unsigned char*)pTmp + i * 16, &aesKey);
        }

        uchar_t* pStart = (uchar_t*)pTmp+nLen-4;
        nOutLen = ReadUint32(pStart);
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

    int EncryptPass(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen)
    {
        if(pInData == NULL|| nInLen <=0 )
        {
            return -1;
        }
        char *pTmp = (char*)malloc(33);
        MD5_Calculate(pInData, nInLen, pTmp);
        pTmp[32] = 0;
        *ppOutData = pTmp;
        nOutLen = 32;
        return 0;
    }
    
    void Free(char* pOutData)
    {
        if(pOutData)
        {
            free(pOutData);
            pOutData = NULL;
        }
    }
    
    
#endif
    
#ifdef __cplusplus
}
#endif
