/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：SeqAlloctor.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月20日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __SEQALLOCTOR_H__
#define __SEQALLOCTOR_H__

#include "ostype.h"

typedef enum
{
    ALLOCTOR_PACKET = 1,
} ALLOCTOR_TYPE;

class CSeqAlloctor
{
public:
    static CSeqAlloctor* getInstance();
    uint32_t getSeq(uint32_t nType);
private:
    CSeqAlloctor();
    virtual ~CSeqAlloctor();
private:
    static CSeqAlloctor* m_pInstance;
    hash_map<uint32_t, uint32_t> m_hmAlloctor;
};

#endif /*defined(__SEQALLOCTOR_H__) */
