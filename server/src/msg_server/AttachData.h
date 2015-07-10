/*
 * AttachData.h
 *
 *  Created on: 2014年4月16日
 *      Author: ziteng
 */

#ifndef ATTACHDATA_H_
#define ATTACHDATA_H_

#include "util.h"

enum {
	ATTACH_TYPE_HANDLE	= 1,
    ATTACH_TYPE_PDU = 2,
    ATTACH_TYPE_HANDLE_AND_PDU = 3,
    ATTACH_TYPE_PDU_FOR_PUSH = 4,
    ATTACH_TYPE_HANDLE_AND_PDU_FOR_FILE = 5,
};

class CDbAttachData
{
public:
	CDbAttachData(uint32_t type, uint32_t handle, uint32_t service_type = 0);				// 序列化
	CDbAttachData(uchar_t* attach_data, uint32_t attach_len);	// 反序列化
	virtual ~CDbAttachData() {}

	uchar_t* GetBuffer() {return m_buf.GetBuffer(); }
	uint32_t GetLength() { return m_buf.GetWriteOffset(); }
	uint32_t GetType() { return m_type; }
	uint32_t GetHandle() { return m_handle; }
    uint32_t GetServiceType() { return m_service_type; }
private:
	CSimpleBuffer	m_buf;
	uint32_t 		m_type;
	uint32_t		m_handle;
    uint32_t        m_service_type;
};

class CPduAttachData
{
public:
	CPduAttachData(uint32_t type, uint32_t handle, uint32_t pduLength, uchar_t* pdu, uint32_t service_type = 0);				// 序列化
	CPduAttachData(uchar_t* attach_data, uint32_t attach_len);	// 反序列化
	virtual ~CPduAttachData() {}
    
	uchar_t* GetBuffer() {return m_buf.GetBuffer(); }
	uint32_t GetLength() { return m_buf.GetWriteOffset(); }
	uint32_t GetType() { return m_type; }
	uint32_t GetHandle() { return m_handle; }
    uint32_t GetServiceType() { return m_service_type; }
    uint32_t GetPduLength() { return m_pduLength; }
    uchar_t* GetPdu() { return m_pdu; }
private:
	CSimpleBuffer	m_buf;
	uint32_t 		m_type;
	uint32_t		m_handle;
    uint32_t        m_service_type;
    uint32_t        m_pduLength;
    uchar_t*        m_pdu;
};

#endif /* ATTACHDATA_H_ */
