/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：MessageCounter.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
 *
 ================================================================*/

#ifndef MESSAGECOUNTER_H_
#define MESSAGECOUNTER_H_

#include "ImPduBase.h"
namespace DB_PROXY {

    void getUnreadMsgCounter(CImPdu* pPdu, uint32_t conn_uuid);
    void clearUnreadMsgCounter(CImPdu* pPdu, uint32_t conn_uuid);
    
    void setDevicesToken(CImPdu* pPdu, uint32_t conn_uuid);
    void getDevicesToken(CImPdu* pPdu, uint32_t conn_uuid);
};


#endif /* MESSAGECOUNTER_H_ */
