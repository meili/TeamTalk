/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：FriendShip.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
 *
 ================================================================*/

#ifndef FRIEND_SHIP_H_
#define FRIEND_SHIP_H_

#include "ImPduBase.h"

namespace DB_PROXY {

    void getRecentSession(CImPdu* pPdu, uint32_t conn_uuid);
    
    void deleteRecentSession(CImPdu* pPdu, uint32_t conn_uuid);

};


#endif /* FRIEND_SHIP_H_ */
