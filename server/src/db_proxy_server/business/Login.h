/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   �ļ����ƣ�Login.h
 *   �� �� �ߣ�Zhang Yuanhao
 *   ��    �䣺bluefoxah@gmail.com
 *   �������ڣ�2014��12��15��
 *   ��    ����
 *
 ================================================================*/

#ifndef LOGIN_H_
#define LOGIN_H_

#include "ImPduBase.h"

namespace DB_PROXY {

void doLogin(CImPdu* pPdu, uint32_t conn_uuid);

};


#endif /* LOGIN_H_ */
