/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：GroupAction.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
 *
 ================================================================*/

#ifndef GROUPACTION_H_
#define GROUPACTION_H_

#include "ImPduBase.h"

namespace DB_PROXY {

    void createGroup(CImPdu* pPdu, uint32_t conn_uuid);
    
    void getNormalGroupList(CImPdu* pPdu, uint32_t conn_uuid);
    
    void getGroupInfo(CImPdu* pPdu, uint32_t conn_uuid);
    
    void modifyMember(CImPdu* pPdu, uint32_t conn_uuid);
    
    void setGroupPush(CImPdu* pPdu, uint32_t conn_uuid);
    
    void getGroupPush(CImPdu* pPdu, uint32_t conn_uuid);

};



#endif /* GROUPACTION_H_ */
