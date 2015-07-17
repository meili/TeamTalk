/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   �ļ����ƣ�MessageCounter.h
 *   �� �� �ߣ�Zhang Yuanhao
 *   ��    �䣺bluefoxah@gmail.com
 *   �������ڣ�2014��12��15��
 *   ��    ����
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
