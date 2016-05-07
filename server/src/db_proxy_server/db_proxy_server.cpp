/*
 * db_proxy_server.cpp
 *
 *  Created on: 2014年7月21日
 *      Author: ziteng
 */

#include "netlib.h"
#include "ConfigFileReader.h"
#include "version.h"
#include "ThreadPool.h"
#include "DBPool.h"
#include "CachePool.h"
#include "ProxyConn.h"
#include "HttpClient.h"
#include "EncDec.h"
#include "business/AudioModel.h"
#include "business/MessageModel.h"
#include "business/SessionModel.h"
#include "business/RelationModel.h"
#include "business/UserModel.h"
#include "business/GroupModel.h"
#include "business/GroupMessageModel.h"
#include "business/FileModel.h"
#include "SyncCenter.h"

string strAudioEnc;
// this callback will be replaced by imconn_callback() in OnConnect()
void proxy_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	if (msg == NETLIB_MSG_CONNECT)
	{
		CProxyConn* pConn = new CProxyConn();
		pConn->OnConnect(handle);
	}
	else
	{
		log("!!!error msg: %d", msg);
	}
}

int main(int argc, char* argv[])
{
	if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
		printf("Server Version: DBProxyServer/%s\n", VERSION);
		printf("Server Build: %s %s\n", __DATE__, __TIME__);
		return 0;
	}

	signal(SIGPIPE, SIG_IGN);
	srand(time(NULL));

	CacheManager* pCacheManager = CacheManager::getInstance();
	if (!pCacheManager) {
		log("CacheManager init failed");
		return -1;
	}

	CDBManager* pDBManager = CDBManager::getInstance();
	if (!pDBManager) {
		log("DBManager init failed");
		return -1;
	}
puts("db init success");
	// 主线程初始化单例，不然在工作线程可能会出现多次初始化
	if (!CAudioModel::getInstance()) {
		return -1;
	}
    
    if (!CGroupMessageModel::getInstance()) {
        return -1;
    }
    
    if (!CGroupModel::getInstance()) {
        return -1;
    }
    
    if (!CMessageModel::getInstance()) {
        return -1;
    }

	if (!CSessionModel::getInstance()) {
		return -1;
	}
    
    if(!CRelationModel::getInstance())
    {
        return -1;
    }
    
    if (!CUserModel::getInstance()) {
        return -1;
    }
    
    if (!CFileModel::getInstance()) {
        return -1;
    }


	CConfigFileReader config_file("dbproxyserver.conf");

	char* listen_ip = config_file.GetConfigName("ListenIP");
	char* str_listen_port = config_file.GetConfigName("ListenPort");
	char* str_thread_num = config_file.GetConfigName("ThreadNum");
    char* str_file_site = config_file.GetConfigName("MsfsSite");
    char* str_aes_key = config_file.GetConfigName("aesKey");

	if (!listen_ip || !str_listen_port || !str_thread_num || !str_file_site || !str_aes_key) {
		log("missing ListenIP/ListenPort/ThreadNum/MsfsSite/aesKey, exit...");
		return -1;
	}
    
    if(strlen(str_aes_key) != 32)
    {
        log("aes key is invalied");
        return -2;
    }
    string strAesKey(str_aes_key, 32);
    CAes cAes = CAes(strAesKey);
    string strAudio = "[语音]";
    char* pAudioEnc;
    uint32_t nOutLen;
    if(cAes.Encrypt(strAudio.c_str(), strAudio.length(), &pAudioEnc, nOutLen) == 0)
    {
        strAudioEnc.clear();
        strAudioEnc.append(pAudioEnc, nOutLen);
        cAes.Free(pAudioEnc);
    }

	uint16_t listen_port = atoi(str_listen_port);
	uint32_t thread_num = atoi(str_thread_num);
    
    string strFileSite(str_file_site);
    CAudioModel::getInstance()->setUrl(strFileSite);

	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;
    
    /// yunfan add 2014.9.28
    // for 603 push
    curl_global_init(CURL_GLOBAL_ALL);
    /// yunfan add end

	init_proxy_conn(thread_num);
    CSyncCenter::getInstance()->init();
    CSyncCenter::getInstance()->startSync();

	CStrExplode listen_ip_list(listen_ip, ';');
	for (uint32_t i = 0; i < listen_ip_list.GetItemCnt(); i++) {
		ret = netlib_listen(listen_ip_list.GetItem(i), listen_port, proxy_serv_callback, NULL);
		if (ret == NETLIB_ERROR)
			return ret;
	}

	printf("server start listen on: %s:%d\n", listen_ip,  listen_port);
	printf("now enter the event loop...\n");
    writePid();
	netlib_eventloop(10);

	return 0;
}


