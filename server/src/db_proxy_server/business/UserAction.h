/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   �ļ����ƣ�UserAction.h
 *   �� �� �ߣ�Zhang Yuanhao
 *   ��    �䣺bluefoxah@gmail.com
 *   �������ڣ�2014��12��15��
 *   ��    ����
 *
 ================================================================*/

#ifndef __USER_ACTION_H__
#define __USER_ACTION_H__

#include "ImPduBase.h"

namespace DB_PROXY {

    void getUserInfo(CImPdu* pPdu, uint32_t conn_uuid);
    void getChangedUser(CImPdu* pPdu, uint32_t conn_uuid);
};


#endif /* __USER_ACTION_H__ */
