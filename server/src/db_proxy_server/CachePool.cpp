/*
 * CachePool.cpp
 *
 *  Created on: 2014年7月24日
 *      Author: ziteng
 *  Modify By ZhangYuanhao
 *  2015-01-13
 *  Add some redis command
 * Modify By ZhangYuanhao
 * 2015-01-14
 * Add comment for function
 *
 */

#include "CachePool.h"
#include "ConfigFileReader.h"

#define MIN_CACHE_CONN_CNT	2

CacheManager* CacheManager::s_cache_manager = NULL;

CacheConn::CacheConn(CachePool* pCachePool)
{
	m_pCachePool = pCachePool;
	m_pContext = NULL;
	m_last_connect_time = 0;
}

CacheConn::~CacheConn()
{
	if (m_pContext) {
		redisFree(m_pContext);
		m_pContext = NULL;
	}
}

/*
 * redis初始化连接和重连操作，类似mysql_ping()
 */
int CacheConn::Init()
{
	if (m_pContext) {
		return 0;
	}

	// 4s 尝试重连一次
	uint64_t cur_time = (uint64_t)time(NULL);
	if (cur_time < m_last_connect_time + 4) {
		return 1;
	}

	m_last_connect_time = cur_time;

	// 200ms超时
	struct timeval timeout = {0, 200000};
	m_pContext = redisConnectWithTimeout(m_pCachePool->GetServerIP(), m_pCachePool->GetServerPort(), timeout);
	if (!m_pContext || m_pContext->err) {
		if (m_pContext) {
			log("redisConnect failed: %s", m_pContext->errstr);
			redisFree(m_pContext);
			m_pContext = NULL;
		} else {
			log("redisConnect failed");
		}

		return 1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SELECT %d", m_pCachePool->GetDBNum());
	if (reply && (reply->type == REDIS_REPLY_STATUS) && (strncmp(reply->str, "OK", 2) == 0)) {
		freeReplyObject(reply);
		return 0;
	} else {
		log("select cache db failed");
		return 2;
	}
}


const char* CacheConn::GetPoolName()
{
	return m_pCachePool->GetPoolName();
}


string CacheConn::get(string key)
{
	string value;

	if (Init()) {
		return value;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "GET %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return value;
	}

	if (reply->type == REDIS_REPLY_STRING) {
		value.append(reply->str, reply->len);
	}

	freeReplyObject(reply);
	return value;
}

string CacheConn::setex(string key, int timeout, string value)
{
	string ret_value;

	if (Init()) {
		return ret_value;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SETEX %s %d %s", key.c_str(), timeout, value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	ret_value.append(reply->str, reply->len);
	freeReplyObject(reply);
	return ret_value;
}

string CacheConn::set(string key, string &value)
{
    string ret_value;
    
    if (Init()) {
        return ret_value;
    }
    
    redisReply* reply = (redisReply *)redisCommand(m_pContext, "SET %s %s", key.c_str(), value.c_str());
    if (!reply) {
        log("redisCommand failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        m_pContext = NULL;
        return ret_value;
    }
    
    ret_value.append(reply->str, reply->len);
    freeReplyObject(reply);
    return ret_value;
}

bool CacheConn::mget(const vector<string>& keys, map<string, string>& ret_value)
{
    if(Init())
    {
        return false;
    }
    if(keys.empty())
    {
        return false;
    }
    
    string strKey;
    bool bFirst = true;
    for (vector<string>::const_iterator it=keys.begin(); it!=keys.end(); ++it) {
        if(bFirst)
        {
            bFirst = false;
            strKey = *it;
        }
        else
        {
            strKey += " " + *it;
        }
    }
    
    if(strKey.empty())
    {
        return false;
    }
    strKey = "MGET " + strKey;
    redisReply* reply = (redisReply*) redisCommand(m_pContext, strKey.c_str());
    if (!reply) {
        log("redisCommand failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        m_pContext = NULL;
        return false;
    }
    if(reply->type == REDIS_REPLY_ARRAY)
    {
        for(size_t i=0; i<reply->elements; ++i)
        {
            redisReply* child_reply = reply->element[i];
            if (child_reply->type == REDIS_REPLY_STRING) {
                ret_value[keys[i]] = child_reply->str;
            }
        }
    }
    freeReplyObject(reply);
    return true;
}

bool CacheConn::isExists(string &key)
{
    if (Init()) {
        return false;
    }
    
    redisReply* reply = (redisReply*) redisCommand(m_pContext, "EXISTS %s", key.c_str());
    if(!reply)
    {
        log("redisCommand failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        return false;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    if(0 == ret_value)
    {
        return false;
    }
    else
    {
        return true;
    }
}
long CacheConn::hdel(string key, string field)
{
	if (Init()) {
		return 0;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HDEL %s %s", key.c_str(), field.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return 0;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

string CacheConn::hget(string key, string field)
{
	string ret_value;
	if (Init()) {
		return ret_value;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HGET %s %s", key.c_str(), field.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	if (reply->type == REDIS_REPLY_STRING) {
		ret_value.append(reply->str, reply->len);
	}

	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::hgetAll(string key, map<string, string>& ret_value)
{
	if (Init()) {
		return false;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HGETALL %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if ( (reply->type == REDIS_REPLY_ARRAY) && (reply->elements % 2 == 0) ) {
		for (size_t i = 0; i < reply->elements; i += 2) {
			redisReply* field_reply = reply->element[i];
			redisReply* value_reply = reply->element[i + 1];

			string field(field_reply->str, field_reply->len);
			string value(value_reply->str, value_reply->len);
			ret_value.insert(make_pair(field, value));
		}
	}

	freeReplyObject(reply);
	return true;
}

long CacheConn::hset(string key, string field, string value)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::hincrBy(string key, string field, long value)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HINCRBY %s %s %ld", key.c_str(), field.c_str(), value);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::incrBy(string key, long value)
{
    if(Init())
    {
        return -1;
    }
    
    redisReply* reply = (redisReply*)redisCommand(m_pContext, "INCRBY %s %ld", key.c_str(), value);
    if(!reply)
    {
        log("redis Command failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

string CacheConn::hmset(string key, map<string, string>& hash)
{
	string ret_value;

	if (Init()) {
		return ret_value;
	}

	int argc = hash.size() * 2 + 2;
	const char** argv = new const char* [argc];
	if (!argv) {
		return ret_value;
	}

	argv[0] = "HMSET";
	argv[1] = key.c_str();
	int i = 2;
	for (map<string, string>::iterator it = hash.begin(); it != hash.end(); it++) {
		argv[i++] = it->first.c_str();
		argv[i++] = it->second.c_str();
	}

	redisReply* reply = (redisReply *)redisCommandArgv(m_pContext, argc, argv, NULL);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		delete [] argv;

		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	ret_value.append(reply->str, reply->len);

	delete [] argv;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::hmget(string key, list<string>& fields, list<string>& ret_value)
{
	if (Init()) {
		return false;
	}

	int argc = fields.size() + 2;
	const char** argv = new const char* [argc];
	if (!argv) {
		return false;
	}

	argv[0] = "HMGET";
	argv[1] = key.c_str();
	int i = 2;
	for (list<string>::iterator it = fields.begin(); it != fields.end(); it++) {
		argv[i++] = it->c_str();
	}

	redisReply* reply = (redisReply *)redisCommandArgv(m_pContext, argc, (const char**)argv, NULL);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		delete [] argv;

		redisFree(m_pContext);
		m_pContext = NULL;

		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; i++) {
			redisReply* value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	delete [] argv;
	freeReplyObject(reply);
	return true;
}

long CacheConn::incr(string key)
{
    if(Init())
    {
        return -1;
    }
    
    redisReply* reply = (redisReply*)redisCommand(m_pContext, "INCR %s", key.c_str());
    if(!reply)
    {
        log("redis Command failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

long CacheConn::decr(string key)
{
    if(Init())
    {
        return -1;
    }
    
    redisReply* reply = (redisReply*)redisCommand(m_pContext, "DECR %s", key.c_str());
    if(!reply)
    {
        log("redis Command failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

long CacheConn::lpush(string key, string value)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "LPUSH %s %s", key.c_str(), value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::rpush(string key, string value)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "RPUSH %s %s", key.c_str(), value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::llen(string key)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "LLEN %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::lrange(string key, long start, long end, list<string>& ret_value)
{
	if (Init()) {
		return false;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "LRANGE %s %d %d", key.c_str(), start, end);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; i++) {
			redisReply* value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	freeReplyObject(reply);
	return true;
}

///////////////
CachePool::CachePool(const char* pool_name, const char* server_ip, int server_port, int db_num, int max_conn_cnt)
{
	m_pool_name = pool_name;
	m_server_ip = server_ip;
	m_server_port = server_port;
	m_db_num = db_num;
	m_max_conn_cnt = max_conn_cnt;
	m_cur_conn_cnt = MIN_CACHE_CONN_CNT;
}

CachePool::~CachePool()
{
	m_free_notify.Lock();
	for (list<CacheConn*>::iterator it = m_free_list.begin(); it != m_free_list.end(); it++) {
		CacheConn* pConn = *it;
		delete pConn;
	}

	m_free_list.clear();
	m_cur_conn_cnt = 0;
	m_free_notify.Unlock();
}

int CachePool::Init()
{
	for (int i = 0; i < m_cur_conn_cnt; i++) {
		CacheConn* pConn = new CacheConn(this);
		if (pConn->Init()) {
			delete pConn;
			return 1;
		}

		m_free_list.push_back(pConn);
	}

	log("cache pool: %s, list size: %lu", m_pool_name.c_str(), m_free_list.size());
	return 0;
}

CacheConn* CachePool::GetCacheConn()
{
	m_free_notify.Lock();

	while (m_free_list.empty()) {
		if (m_cur_conn_cnt >= m_max_conn_cnt) {
			m_free_notify.Wait();
		} else {
			CacheConn* pCacheConn = new CacheConn(this);
			int ret = pCacheConn->Init();
			if (ret) {
				log("Init CacheConn failed");
				delete pCacheConn;
				m_free_notify.Unlock();
				return NULL;
			} else {
				m_free_list.push_back(pCacheConn);
				m_cur_conn_cnt++;
				log("new cache connection: %s, conn_cnt: %d", m_pool_name.c_str(), m_cur_conn_cnt);
			}
		}
	}

	CacheConn* pConn = m_free_list.front();
	m_free_list.pop_front();

	m_free_notify.Unlock();

	return pConn;
}

void CachePool::RelCacheConn(CacheConn* pCacheConn)
{
	m_free_notify.Lock();

	list<CacheConn*>::iterator it = m_free_list.begin();
	for (; it != m_free_list.end(); it++) {
		if (*it == pCacheConn) {
			break;
		}
	}

	if (it == m_free_list.end()) {
		m_free_list.push_back(pCacheConn);
	}

	m_free_notify.Signal();
	m_free_notify.Unlock();
}

///////////
CacheManager::CacheManager()
{

}

CacheManager::~CacheManager()
{

}

CacheManager* CacheManager::getInstance()
{
	if (!s_cache_manager) {
		s_cache_manager = new CacheManager();
		if (s_cache_manager->Init()) {
			delete s_cache_manager;
			s_cache_manager = NULL;
		}
	}

	return s_cache_manager;
}

int CacheManager::Init()
{
	CConfigFileReader config_file("dbproxyserver.conf");

	char* cache_instances = config_file.GetConfigName("CacheInstances");
	if (!cache_instances) {
		log("not configure CacheIntance");
		return 1;
	}

	char host[64];
	char port[64];
	char db[64];
    char maxconncnt[64];
	CStrExplode instances_name(cache_instances, ',');
	for (uint32_t i = 0; i < instances_name.GetItemCnt(); i++) {
		char* pool_name = instances_name.GetItem(i);
		//printf("%s", pool_name);
		snprintf(host, 64, "%s_host", pool_name);
		snprintf(port, 64, "%s_port", pool_name);
		snprintf(db, 64, "%s_db", pool_name);
        snprintf(maxconncnt, 64, "%s_maxconncnt", pool_name);

		char* cache_host = config_file.GetConfigName(host);
		char* str_cache_port = config_file.GetConfigName(port);
		char* str_cache_db = config_file.GetConfigName(db);
        char* str_max_conn_cnt = config_file.GetConfigName(maxconncnt);
		if (!cache_host || !str_cache_port || !str_cache_db || !str_max_conn_cnt) {
			log("not configure cache instance: %s", pool_name);
			return 2;
		}

		CachePool* pCachePool = new CachePool(pool_name, cache_host, atoi(str_cache_port),
				atoi(str_cache_db), atoi(str_max_conn_cnt));
		if (pCachePool->Init()) {
			log("Init cache pool failed");
			return 3;
		}

		m_cache_pool_map.insert(make_pair(pool_name, pCachePool));
	}

	return 0;
}

CacheConn* CacheManager::GetCacheConn(const char* pool_name)
{
	map<string, CachePool*>::iterator it = m_cache_pool_map.find(pool_name);
	if (it != m_cache_pool_map.end()) {
		return it->second->GetCacheConn();
	} else {
		return NULL;
	}
}

void CacheManager::RelCacheConn(CacheConn* pCacheConn)
{
	if (!pCacheConn) {
		return;
	}

	map<string, CachePool*>::iterator it = m_cache_pool_map.find(pCacheConn->GetPoolName());
	if (it != m_cache_pool_map.end()) {
		return it->second->RelCacheConn(pCacheConn);
	}
}
