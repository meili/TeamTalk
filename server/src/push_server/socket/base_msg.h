#ifndef _BASE_MSG_H
#define _BASE_MSG_H

#include "socket_io.h"
#include "../type/base_type.h"
#include "../bufferloop/buffer_loop.hpp"

class SOCKET_IO_DECLARE_CLASS CBaseMsg
{
public:
	CBaseMsg(void)
	{	m_nHeadLength = 0;
		m_nBodyLength = 0;
        m_nTailLength = 0;
	}
	virtual ~CBaseMsg(void) {}

	uint32_t GetHeadLength() const { return m_nHeadLength; }
	uint32_t GetBodyLength() const { return m_nBodyLength; }
    uint32_t GetTailLength() const { return m_nTailLength; }
    uint32_t GetDataLength() const { return m_nHeadLength + m_nBodyLength + m_nTailLength; }
    
    //记录当前buffer中的数据长度
    uint32_t GetDataBufferLength() { return m_databuffer.GetWriteOffset(); }
    
    const char* Header() const { return m_databuffer.GetBuffer(); }
    const char* Body() const { return (m_databuffer.GetBuffer() + GetHeadLength()); }
    const char* Tail() const { return (m_databuffer.GetBuffer() + GetHeadLength() + GetBodyLength());}
    const char* Data() const { return m_databuffer.GetBuffer(); }
    
    void Append(const char* szData, uint32_t nDataSize){ m_databuffer.Write(szData, nDataSize); }
    void Remove(uint32_t nDataSize) { m_databuffer.Read(NULL, nDataSize); }
    void Clear() { m_databuffer.Read(NULL, m_databuffer.GetWriteOffset()); }
protected:
	void __SetHeadLength(uint32_t nHeadLength ) { m_nHeadLength = nHeadLength; }
	void __SetBodyLength(uint32_t nBodyLength) { m_nBodyLength = nBodyLength; }
    void __SetTailLength(uint32_t nTailLength) { m_nTailLength = nTailLength; }
	uint32_t m_nHeadLength;
	uint32_t m_nBodyLength;
    uint32_t m_nTailLength;
    CSimpleBuffer m_databuffer;
};

#endif