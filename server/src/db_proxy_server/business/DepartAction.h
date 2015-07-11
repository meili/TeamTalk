/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：DepartAction.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年03月13日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __DEPARTACTION_H__
#define __DEPARTACTION_H__
#include "ImPduBase.h"

namespace DB_PROXY {
    
    void getChgedDepart(CImPdu* pPdu, uint32_t conn_uuid);
};


#endif /*defined(__DEPARTACTION_H__) */
