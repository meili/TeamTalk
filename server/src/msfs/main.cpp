/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   @       File              :  main.cpp
 *   @       Author      :  Zhang Yuanhao
 *   @       Email         :  bluefoxah@gmail.com
 *   @       Date           :  2014Äê7ÔÂ29ÈÕ
 *   @       Version     :   1.0
 *   @  Description	:
 *
 ================================================================*/

#include <iostream>
#include <signal.h>
#include "netlib.h"
#include "ConfigFileReader.h"
#include "HttpConn.h"
#include "FileManager.h"
#include "ThreadPool.h"

using namespace std;
using namespace msfs;


FileManager* FileManager::m_instance = NULL;
FileManager* g_fileManager = NULL;
CConfigFileReader config_file("msfs.conf");
CThreadPool g_PostThreadPool;
CThreadPool g_GetThreadPool;

void closeall(int fd)
{
    int fdlimit = sysconf(_SC_OPEN_MAX);

    while (fd < fdlimit)
        close(fd++);
}

int daemon(int nochdir, int noclose, int asroot)
{
    switch (fork())
    {
        case 0:  break;
        case -1: return -1;
        default: _exit(0);          /* exit the original process */
    }

    if (setsid() < 0)               /* shoudn't fail */
        return -1;

    if ( !asroot && (setuid(1) < 0) )              /* shoudn't fail */
        return -1;

    /* dyke out this switch if you want to acquire a control tty in */
    /* the future -- not normally advisable for daemons */

    switch (fork())
    {
        case 0:  break;
        case -1: return -1;
        default: _exit(0);
    }

    if (!nochdir)
        chdir("/");

    if (!noclose)
    {
        closeall(0);
        dup(0); dup(0);
    }

    return 0;
}

// for client connect in
void http_callback(void* callback_data, uint8_t msg, uint32_t handle,
        void* pParam)
{
    if (msg == NETLIB_MSG_CONNECT)
    {
        CHttpConn* pConn = new CHttpConn();
//        CHttpTask* pTask = new CHttpTask(handle, pConn);
//        g_ThreadPool.AddTask(pTask);
        pConn->OnConnect(handle);
    } else
    {
        log("!!!error msg: %d", msg);
    }
}

void doQuitJob()
{
	char fileCntBuf[20] = {0};
	snprintf(fileCntBuf, 20, "%llu", g_fileManager->getFileCntCurr());
    	config_file.SetConfigValue("FileCnt", fileCntBuf);
	FileManager::destroyInstance();
netlib_destroy();
    log("I'm ready quit...");
}
void Stop(int signo)
{
    log("receive signal:%d", signo);
    switch(signo)
    {
    case SIGINT:
    case SIGTERM:
    case SIGQUIT:
        doQuitJob();
        _exit(0);
        break;
    default:
        cout<< "unknown signal"<<endl;
        _exit(0);
    }
}
int main(int argc, char* argv[])
{
    for(int i=0; i < argc; ++i)
       {
           if(strncmp(argv[i], "-d", 2) == 0)
           {
               if(daemon(1, 0, 1) < 0)
               {
                   cout<<"daemon error"<<endl;
                   return -1;
               }
               break;
           }
       }
    log("MsgServer max files can open: %d", getdtablesize());


    char* listen_ip = config_file.GetConfigName("ListenIP");
    char* str_listen_port = config_file.GetConfigName("ListenPort");
    char* base_dir = config_file.GetConfigName("BaseDir");
    char* str_file_cnt = config_file.GetConfigName("FileCnt");
    char* str_files_per_dir = config_file.GetConfigName("FilesPerDir");
    char* str_post_thread_count = config_file.GetConfigName("PostThreadCount");
    char* str_get_thread_count = config_file.GetConfigName("GetThreadCount");

    if (!listen_ip || !str_listen_port || !base_dir || !str_file_cnt || !str_files_per_dir || !str_post_thread_count || !str_get_thread_count)
    {
        log("config file miss, exit...");
        return -1;
    }
    
    log("%s,%s",listen_ip, str_listen_port);
    uint16_t listen_port = atoi(str_listen_port);
    long long int  fileCnt = atoll(str_file_cnt);
    int filesPerDir = atoi(str_files_per_dir);
    int nPostThreadCount = atoi(str_post_thread_count);
    int nGetThreadCount = atoi(str_get_thread_count);
    if(nPostThreadCount <= 0 || nGetThreadCount <= 0)
    {
        log("thread count is invalied");
        return -1;
    }
    g_PostThreadPool.Init(nPostThreadCount);
    g_GetThreadPool.Init(nGetThreadCount);

    g_fileManager = FileManager::getInstance(listen_ip, base_dir, fileCnt, filesPerDir);
	int ret = g_fileManager->initDir();
	if (ret) {
		printf("The BaseDir is set incorrectly :%s\n",base_dir);
		return ret;
    }
	ret = netlib_init();
    if (ret == NETLIB_ERROR)
        return ret;

    CStrExplode listen_ip_list(listen_ip, ';');
    for (uint32_t i = 0; i < listen_ip_list.GetItemCnt(); i++)
    {
        ret = netlib_listen(listen_ip_list.GetItem(i), listen_port,
                http_callback, NULL);
        if (ret == NETLIB_ERROR)
            return ret;
    }

    signal(SIGINT, Stop);
    signal (SIGTERM, Stop);
    signal (SIGQUIT, Stop);
    signal(SIGPIPE, SIG_IGN);
    signal (SIGHUP, SIG_IGN);

    printf("server start listen on: %s:%d\n", listen_ip, listen_port);
    init_http_conn();
    printf("now enter the event loop...\n");

    netlib_eventloop();
    return 0;
}


