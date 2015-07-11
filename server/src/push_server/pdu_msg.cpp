//
//  pdu_msg.cpp
//  my_push_server
//
//  Created by luoning on 14-11-12.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#include "pdu_msg.h"
#include "byte_stream.h"

#define CHECK_PACKET_PARSE if (is.GetPos() != (len - GetHeadLength()))\
                    {\
                        bRet = FALSE;\
                    }\

CPduMsg::CPduMsg()
{
    memset(&m_sthead, 0, sizeof(ST_HEAD));
    __SetHeadLength(HEAD_LENGTH);
    m_sthead.version = PDU_VERSION;
}

CPduMsg::~CPduMsg()
{}

BOOL CPduMsg::CheckMsgAvailable()
{
    BOOL bRet = FALSE;
    if (m_databuffer.GetWriteOffset() > GetHeadLength())
    {
        CByteStream is((uchar_t*)m_databuffer.GetBuffer(), sizeof(ST_HEAD));
        is >> m_sthead.length;
        is >> m_sthead.version;
        is >> m_sthead.flag;
        is >> m_sthead.service_id;
        is >> m_sthead.command_id;
        is >> m_sthead.error;
        is >> m_sthead.seq_no;
        if (m_sthead.length <= m_databuffer.GetWriteOffset())
        {
            bRet = TRUE;
        }
    }
    return bRet;
}

BOOL CPduMsg::ReadHead()
{
    BOOL bRet = FALSE;
    if (m_databuffer.GetWriteOffset() >= GetHeadLength()) {
        CByteStream is((uchar_t*)m_databuffer.GetBuffer(), sizeof(ST_HEAD));
        is >> m_sthead.length;
        is >> m_sthead.version;
        is >> m_sthead.flag;
        is >> m_sthead.service_id;
        is >> m_sthead.command_id;
        is >> m_sthead.error;
        is >> m_sthead.seq_no;
        __SetBodyLength(m_sthead.length - GetHeadLength());
        __SetTailLength(0);
        bRet = TRUE;
    }
    return bRet;
}

void CPduMsg::WriteHead()
{
    CByteStream os(&m_databuffer, GetHeadLength());
    os << GetDataLength();
    os << m_sthead.version;
    os << m_sthead.flag;
    os << m_sthead.service_id;
    os << m_sthead.command_id;
    os << m_sthead.error;
    os << m_sthead.seq_no;
}

BOOL CPduMsg::ParseFromArray(const char *buf, uint32_t len)
{
    BOOL bRet = FALSE;
    if (m_databuffer.GetWriteOffset() != 0)
    {
        return bRet;
    }
    Append(buf, len);
    if (CheckMsgAvailable())
    {
        ReadHead();
        bRet = TRUE;
    }
    return bRet;
}

BOOL CPduMsg::SerializeToArray(google::protobuf::MessageLite* msg)
{
    BOOL bRet = FALSE;
    if (m_databuffer.GetWriteOffset() != 0)
    {
        return bRet;
    }
    
    uint32_t msg_size = msg->ByteSize();
    uchar_t* szData = new uchar_t[msg_size];
    if (!msg->SerializeToArray(szData, msg_size))
    {
        PUSH_SERVER_WARN("pb msg miss required fields.");
    }
    else
    {
        m_sthead.length = msg_size + GetHeadLength();
        __SetTailLength(0);
        __SetBodyLength(msg_size);
        WriteHead();
        Append((const char*)szData, msg_size);
        bRet = TRUE;
    }
    
    delete []szData;
    return bRet;
}


