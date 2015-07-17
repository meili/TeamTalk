/*================================================================
*   Copyright (C) 2015 All rights reserved.
*   
*   �ļ����ƣ�security.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2015��01��29��
*   ��    ����
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
    jstring Java_com_mogujie_im_security_EncryptMsg(JNIEnv* env, jobject obj, jstring jstr);
    jstring Java_com_mogujie_im_security_DecryptMsg(JNIEnv* env, jobject obj, jstring jstr);
    jstring Java_com_mogujie_im_security_EncryptPass(JNIEnv* env, jobject obj, jstring jstr, jstring jkey);

#else
    /**
     *  ����Ϣ����
     *
     *  @param pInData  �����ܵ���Ϣ����ָ��
     *  @param nInLen   ��������Ϣ���ݳ���
     *  @param pOutData ���ܺ���ı�
     *  @param nOutLen  ���ܺ���ı�����
     *
     *  @return ���� 0-�ɹ�; ����-ʧ��
     */
    DLL_MODIFIER int EncryptMsg(const char* pInData, uint32_t nInLen, char** pOutData, uint32_t& nOutLen);
    
    /**
     *  ����Ϣ����
     *
     *  @param pInData  �����ܵ���Ϣ����ָ��
     *  @param nInLen   ��������Ϣ���ݳ���
     *  @param pOutData ���ܺ���ı�
     *  @param nOutLen  ���ܺ���ı�����
     *
     *  @return ���� 0-�ɹ�; ����-ʧ��
     */
    DLL_MODIFIER int DecryptMsg(const char* pInData, uint32_t nInLen, char** pOutData, uint32_t& nOutLen);
    
    /**
     *  ��������м���
     *
     *  @param pInData  �����ܵ���Ϣ����ָ��
     *  @param nInLen   ��������Ϣ���ݳ���
     *  @param pOutData ���ܺ���ı�
     *  @param nOutLen  ���ܺ���ı�����
     *  @param pKey     32λ��Կ
     *
     *  @return ���� 0-�ɹ�; ����-ʧ��
     */
    DLL_MODIFIER int EncryptPass(const char* pInData, uint32_t nInLen, char** pOutData, uint32_t& nOutLen, const char* pKey);
    
    DLL_MODIFIER int DecryptPass(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen, const char* pKey);
    /**
     *  �ͷ���Դ
     *
     *  @param pOutData ��Ҫ�ͷŵ���Դ
     */
    DLL_MODIFIER void Free(char* pOutData);
    
#endif
    
#ifdef __cplusplus
}
#endif

#endif
