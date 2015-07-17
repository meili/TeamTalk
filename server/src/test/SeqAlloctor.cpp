/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：SeqAlloctor.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月20日
*   描    述：
*
================================================================*/
#include <stdlib.h>
#include "SeqAlloctor.h"

CSeqAlloctor* CSeqAlloctor::m_pInstance = NULL;

CSeqAlloctor::CSeqAlloctor()
{
    
}

CSeqAlloctor::~CSeqAlloctor()
{
    
}

CSeqAlloctor* CSeqAlloctor::getInstance()
{
    if(!m_pInstance)
    {
        m_pInstance = new CSeqAlloctor();
    }
    return m_pInstance;
}

uint32_t CSeqAlloctor::getSeq(uint32_t nType)
{
    auto it=m_hmAlloctor.find(nType);
    uint32_t nSeqNo = 0;
    if(it != m_hmAlloctor.end())
    {
        it->second++;
        nSeqNo = it->second;
    }
    else
    {
        srand(time(NULL));
        nSeqNo = rand() + 1;
        m_hmAlloctor[nType] = nSeqNo;
    }
    return nSeqNo;
}