/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   �ļ����ƣ�MessageContent.h
 *   �� �� �ߣ�Zhang Yuanhao
 *   ��    �䣺bluefoxah@gmail.com
 *   �������ڣ�2014��12��15��
 *   ��    ����
 *
 ================================================================*/

#ifndef MESSAGECOUTENT_H_
#define MESSAGECOUTENT_H_

#include "ImPduBase.h"

namespace DB_PROXY {

    void getMessage(CImPdu* pPdu, uint32_t conn_uuid);

    void sendMessage(CImPdu* pPdu, uint32_t conn_uuid);
    
    void getMessageById(CImPdu* pPdu, uint32_t conn_uuid);
    
    void getLatestMsgId(CImPdu* pPdu, uint32_t conn_uuid);
};

#endif /* MESSAGECOUTENT_H_ */
