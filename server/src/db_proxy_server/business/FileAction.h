/*================================================================
*     Copyright (c) 2014�� lanhu. All rights reserved.
*   
*   �ļ����ƣ�FileActioin.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2014��12��31��
*   ��    ����
*
#pragma once
================================================================*/
#ifndef __FILEACTION_H__
#define __FILEACTION_H__

#include "ImPduBase.h"

namespace DB_PROXY {
    void hasOfflineFile(CImPdu* pPdu, uint32_t conn_uuid);
    void addOfflineFile(CImPdu* pPdu, uint32_t conn_uuid);
    void delOfflineFile(CImPdu* pPdu, uint32_t conn_uuid);
};

#endif /*defined(__FILEACTION_H__) */
