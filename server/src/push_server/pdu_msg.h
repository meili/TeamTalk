//
//  pdu_msg.h
//  my_push_server
//
//  Created by luoning on 14-11-12.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#ifndef __my_push_server__pdu_msg__
#define __my_push_server__pdu_msg__

#include <stdio.h>
#include "socket/base_msg.h"
#include "type/base_type.h"
#include "push_define.h"
#include <list>
#include <string>
#include "google/protobuf/message_lite.h"

using namespace std;


#define HEAD_LENGTH     16

typedef struct tag_head_t
{
    uint32_t 	length;		// the whole pdu length
    uint16_t 	version;	// pdu version number
    uint16_t	flag;		// not used
    uint16_t	service_id;	//
    uint16_t	command_id;	//
    uint16_t	error;		//
    uint16_t	seq_no;	//
}ST_HEAD, *LPST_HEAD;

class CPduMsg : public CBaseMsg
{
public:
    CPduMsg();
    virtual ~CPduMsg();
    
    uint32_t GetPduLength() { return m_sthead.length; }
    uint16_t GetVersion() { return m_sthead.version; }
    uint16_t GetServiceID() { return m_sthead.service_id; }
    uint16_t GetCommandID() { return m_sthead.command_id; }
    uint16_t GetError() { return m_sthead.error; }
    uint16_t GetSeqNum() { return m_sthead.seq_no; }
    
    void SetServiceID(uint16_t service_id) { m_sthead.service_id = service_id; }
    void SetCommandID(uint16_t command_id) { m_sthead.command_id = command_id; }
    void SetVersion(uint16_t version) { m_sthead.version = version; }
    void SetSeqNum(uint16_t seq_no) { m_sthead.seq_no = seq_no; }
    
    BOOL ReadHead();
    void WriteHead();
    
    BOOL CheckMsgAvailable();
    
    BOOL ParseFromArray(const char* buf, uint32_t len);
    BOOL SerializeToArray(google::protobuf::MessageLite* msg);
    
protected:
    ST_HEAD m_sthead;
};


#endif /* defined(__my_push_server__pdu_msg__) */
