/*================================================================
 *     Copyright (c) 2014年 lanhu. All rights reserved.
 *
 *   文件名称：HandlerMap.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月02日
 *   描    述：
 *
 ================================================================*/

#ifndef HANDLERMAP_H_
#define HANDLERMAP_H_


#include "../base/util.h"
#include "ProxyTask.h"

typedef map<uint32_t, pdu_handler_t> HandlerMap_t;

class CHandlerMap {
public:
	virtual ~CHandlerMap();

	static CHandlerMap* getInstance();

	void Init();
	pdu_handler_t GetHandler(uint32_t pdu_type);
private:
	CHandlerMap();

private:
	static  CHandlerMap* s_handler_instance;

	HandlerMap_t 	m_handler_map;
};

#endif /* HANDLERMAP_H_ */
