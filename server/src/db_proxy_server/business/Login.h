/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：Login.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
 *
 ================================================================*/

#ifndef LOGIN_H_
#define LOGIN_H_

#include "ImPduBase.h"

namespace DB_PROXY {

void doLogin(CImPdu* pPdu, uint32_t conn_uuid);

};


#endif /* LOGIN_H_ */
