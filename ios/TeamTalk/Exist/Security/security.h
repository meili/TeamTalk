/*================================================================
*   Copyright (C) 2015 All rights reserved.
*   
*   文件名称：security.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月29日
*   描    述：
*
#pragma once
================================================================*/

#ifndef __SECURITY_H__
#define __SECURITY_H__


#ifdef _WIN32
typedef char			int8_t;
typedef short			int16_t;
typedef int				int32_t;
typedef	long long		int64_t;
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned int	uint32_t;
typedef	unsigned long long	uint64_t;
typedef int				socklen_t;
#else
#include <stdint.h>
#endif
typedef unsigned char	uchar_t;

#ifdef WIN32
#define DLL_MODIFIER __declspec(dllexport)
#else
#define DLL_MODIFIER
#endif


#ifdef __cplusplus
extern "C" {
#endif
    
#ifdef __ANDROID__
    jbyteArray Java_com_mogujie_tt_Security_EncryptMsg(JNIEnv* env, jobject obj, jstring jstr);
    jbyteArray Java_com_mogujie_tt_Security_DecryptMsg(JNIEnv* env, jobject obj, jstring jstr);
    jbyteArray Java_com_mogujie_tt_Security_EncryptPass(JNIEnv* env, jobject obj, jstring jstr);

#else
    /**
     *  对消息加密
     *
     *  @param pInData  待加密的消息内容指针
     *  @param nInLen   待加密消息内容长度
     *  @param pOutData 加密后的文本
     *  @param nOutLen  加密后的文本长度
     *
     *  @return 返回 0-成功; 其他-失败
     */
    DLL_MODIFIER int EncryptMsg(const char* pInData, uint32_t nInLen, char** pOutData, uint32_t& nOutLen);
    
    /**
     *  对消息解密
     *
     *  @param pInData  待解密的消息内容指针
     *  @param nInLen   待解密消息内容长度
     *  @param pOutData 解密后的文本
     *  @param nOutLen  解密后的文本长度
     *
     *  @return 返回 0-成功; 其他-失败
     */
    DLL_MODIFIER int DecryptMsg(const char* pInData, uint32_t nInLen, char** pOutData, uint32_t& nOutLen);
    
    /**
     *  对密码进行加密
     *
     *  @param pInData  待解密的消息内容指针
     *  @param nInLen   待解密消息内容长度
     *  @param pOutData 解密后的文本
     *  @param nOutLen  解密后的文本长度
     *  @param pKey     32位密钥
     *
     *  @return 返回 0-成功; 其他-失败
     */
    DLL_MODIFIER int EncryptPass(const char* pInData, uint32_t nInLen, char** pOutData, uint32_t& nOutLen);
    /**
     *  释放资源
     *
     *  @param pOutData 需要释放的资源
     */
    DLL_MODIFIER void Free(char* pOutData);
    
#endif
    
#ifdef __cplusplus
}
#endif

#endif
