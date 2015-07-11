/*================================================================
*     Copyright (c) 2014年 lanhu. All rights reserved.
*   
*   文件名称：FileActioin.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年12月31日
*   描    述：
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
