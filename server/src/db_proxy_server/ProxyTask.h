/*================================================================
*   Copyright (C) 2014 All rights reserved.
*   
*   文件名称：ProxyTask.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年09月16日
*   描    述：
*
#pragma once
================================================================*/

#ifndef __PROXY_TASK_H__
#define __PROXY_TASK_H__
#include "Task.h"
#include "util.h"
#include "ImPduBase.h"

typedef void (*pdu_handler_t)(CImPdu* pPdu, uint32_t conn_uuid);

class CProxyTask:public CTask
{
public:
    CProxyTask(uint32_t conn_uuid, pdu_handler_t pdu_handler, CImPdu* pPdu);
	virtual ~CProxyTask();
    
	virtual void run();
private:
	uint32_t 		m_conn_uuid;
	pdu_handler_t	m_pdu_handler;
	CImPdu* 		m_pPdu;
};
#endif

