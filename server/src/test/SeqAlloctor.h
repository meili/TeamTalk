/*================================================================
*     Copyright (c) 2015�� lanhu. All rights reserved.
*   
*   �ļ����ƣ�SeqAlloctor.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2015��01��20��
*   ��    ����
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
