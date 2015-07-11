/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：MessageContent.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
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
