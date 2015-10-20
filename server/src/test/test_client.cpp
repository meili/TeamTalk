/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：test_client.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月30日
 *   描    述：
 *
 ================================================================*/

#include <vector>
#include <iostream>
#include "ClientConn.h"
#include "netlib.h"
#include "TokenValidator.h"
#include "Thread.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Buddy.pb.h"
#include "playsound.h"
#include "Common.h"
#include "Client.h"
#include "security.h"
using namespace std;

#define MAX_LINE_LEN	1024
string g_login_domain = "http://127.0.0.1:8080/msg_server";
string g_cmd_string[10];
int g_cmd_num;
CClient* g_pClient = NULL;
void split_cmd(char* buf)
{
	int len = strlen(buf);
	string element;

	g_cmd_num = 0;
	for (int i = 0; i < len; i++) {
		if (buf[i] == ' ' || buf[i] == '\t') {
			if (!element.empty()) {
				g_cmd_string[g_cmd_num++] = element;
				element.clear();
			}
		} else {
			element += buf[i];
		}
	}

	// put the last one
	if (!element.empty()) {
		g_cmd_string[g_cmd_num++] = element;
	}
}

void print_help()
{
	printf("Usage:\n");
    printf("login user_name user_pass\n");
	printf("getRecentSession\n");
	printf("getUnreadMsgCnt\n");
    printf("getChangedUser\n");
	printf("getUserInfo id1 id2 id3\n");    
	printf("sendmsg nToid nMsgType\n");
	printf("getDepartMentInfo\n");
	printf("getGroupListInfo\n");
	printf("close\n");
	printf("quit\n");
}

void do_getRecentSession()
{
   g_pClient->getRecentSession();
}

void do_getUnreadMsgCnt()
{
   g_pClient->getUnreadMsgCnt();
}
void do_getChangedUser()
{
   g_pClient->getChangedUser();
}

void do_sendmsg(int nToid , int nMsgType , const string & msg)
{
	char *enOut = 0;
	uint32_t enLen = 0;
	int ret = EncryptMsg(msg.c_str() , msg.length() ,&enOut , enLen);
	string out(enOut , enLen);
	Free(enOut);
	g_pClient->sendMsg(nToid ,(IM::BaseDefine::MsgType) nMsgType , out);
}

void doLogin(const string& strName, const string& strPass)
{
    try
    {
        g_pClient = new CClient(strName, strPass,g_login_domain);
    }
    catch(...)
    {
        printf("get error while alloc memory\n");
        PROMPTION;
        return;
    }
    g_pClient->connect();
}
void exec_cmd()
{
	if (g_cmd_num == 0) {
		return;
	}
    
    if(g_cmd_string[0] == "login")
    {
        if(g_cmd_num == 3)
        {	
            char *enPass = 0;
            uint32_t enLen = 0;
            
            EncryptPass(g_cmd_string[2].c_str() , g_cmd_string[2].length(),&enPass , enLen , 0);
            string pass(enPass , enLen);
            Free(enPass);
            doLogin(g_cmd_string[1], pass);
        }
        else
        {
            print_help();
        }
    }
    else if(strcmp(g_cmd_string[0].c_str() , "sendmsg") == 0){
	do_sendmsg(atoi(g_cmd_string[1].c_str()) , atoi(g_cmd_string[2].c_str()) , g_cmd_string[3]);	
    }
    else if (strcmp(g_cmd_string[0].c_str(), "close") == 0) {
        g_pClient->close();
    }
    else if (strcmp(g_cmd_string[0].c_str(), "quit") == 0) {
		exit(0);

    }
    else if(strcmp(g_cmd_string[0].c_str() , "getChangedUser") == 0){
		do_getChangedUser();

	}
    else if(strcmp(g_cmd_string[0].c_str() , "getUnreadMsgCnt")== 0){
    	do_getUnreadMsgCnt();

	}
   else if(strcmp(g_cmd_string[0].c_str() , "getRecentSession")== 0){
        do_getRecentSession(); 

   }
   else if(strcmp(g_cmd_string[0].c_str() , "getUserInfo") == 0){
        
	list<uint32_t> lst;
    for( int i = 1 ; i < g_cmd_num  ; ++i)
	{
		lst.push_back(atoi(g_cmd_string[i].c_str()));
	}
    g_pClient->getUserInfo(lst);

}
    else if(strcmp(g_cmd_string[0].c_str(), "list") == 0)
    {
    
        CMapNick2User_t mapUser = g_pClient->getNick2UserMap();
		printf("%d users:\n" , mapUser.size());
        auto it = mapUser.begin();
        for(;it!=mapUser.end();++it)
        {
           printf("id:%d user:%s status:%d\n" , it->second->user_id() , it->second->user_real_name().c_str() , it->second->status());
        }
    }
	else if(strcmp(g_cmd_string[0].c_str(), "getDepartMentInfo") == 0)
	{
		g_pClient->getDepartMentInfo();
	}
	else if(strcmp(g_cmd_string[0].c_str(), "getGroupListInfo") == 0)
	{
		g_pClient->getGroupList();
	}
    else {
		print_help();
	}
}


class CmdThread : public CThread
{
public:
	void OnThreadRun()
	{
		while (true)
		{
			fprintf(stderr, "%s", PROMPT);	// print to error will not buffer the printed message

			if (fgets(m_buf, MAX_LINE_LEN - 1, stdin) == NULL)
			{
				fprintf(stderr, "fgets failed: %d\n", errno);
				continue;
			}

			m_buf[strlen(m_buf) - 1] = '\0';	// remove newline character

			split_cmd(m_buf);

			exec_cmd();
		}
	}
private:
	char	m_buf[MAX_LINE_LEN];
};

CmdThread g_cmd_thread;

int main(int argc, char* argv[])
{
//    play("message.wav");
	g_cmd_thread.StartThread();

	signal(SIGPIPE, SIG_IGN);

	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;
    
	netlib_eventloop();

	return 0;
}
