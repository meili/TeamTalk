/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   �ļ����ƣ�AudioModel.cpp
 *   �� �� �ߣ�Zhang Yuanhao
 *   ��    �䣺bluefoxah@gmail.com
 *   �������ڣ�2014��12��15��
 *   ��    ����
 *
 ================================================================*/

#include "../DBPool.h"
#include "../HttpClient.h"
#include "AudioModel.h"

using namespace std;

//AudioModel
CAudioModel* CAudioModel::m_pInstance = NULL;

/**
 *  ���캯��
 */
CAudioModel::CAudioModel()
{

}

/**
 *  ��������
 */
CAudioModel::~CAudioModel()
{

}

/**
 *  ����
 *
 *  @return ������ָ��
 */
CAudioModel* CAudioModel::getInstance()
{
	if (!m_pInstance) {
		m_pInstance = new CAudioModel();
	}

	return m_pInstance;
}

/**
 *  ��ֻ�����洢��url��ַ
 *
 *  @param strFileSite �ϴ���url
 */
void CAudioModel::setUrl(string& strFileSite)
{
    m_strFileSite = strFileSite;
    if(m_strFileSite[m_strFileSite.length()] != '/')
    {
        m_strFileSite += "/";
    }
}

/**
 *  ��ȡ������Ϣ
 *
 *  @param nAudioId ������Id
 *  @param cMsg     ������Ϣ������
 *
 *  @return bool �ɹ�����true��ʧ�ܷ���false
 */
bool CAudioModel::readAudios(list<IM::BaseDefine::MsgInfo>& lsMsg)
{
    if(lsMsg.empty())
    {
        return true;
    }
    bool bRet = false;
    CDBManager* pDBManger = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManger->GetDBConn("teamtalk_slave");
    if (pDBConn)
    {
        for (auto it=lsMsg.begin(); it!=lsMsg.end(); )
        {
            IM::BaseDefine::MsgType nType = it->msg_type();
            if((IM::BaseDefine::MSG_TYPE_GROUP_AUDIO ==  nType) || (IM::BaseDefine::MSG_TYPE_SINGLE_AUDIO == nType))
            {
                string strSql = "select * from IMAudio where id=" + it->msg_data();
                CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
                if (pResultSet)
                {
                    while (pResultSet->Next()) {
                        uint32_t nCostTime = pResultSet->GetInt("duration");
                        uint32_t nSize = pResultSet->GetInt("size");
                        string strPath = pResultSet->GetString("path");
                        readAudioContent(nCostTime, nSize, strPath, *it);
                    }
                    ++it;
                    delete pResultSet;
                }
                else
                {
                    log("no result for sql:%s", strSql.c_str());
                    it = lsMsg.erase(it);
                }
            }
            else
            {
                ++it;
            }
        }
        pDBManger->RelDBConn(pDBConn);
        bRet = true;
    }
    else
    {
        log("no connection for teamtalk_slave");
    }
    return bRet;
}

/**
 *  �洢������Ϣ
 *
 *  @param nFromId     ������Id
 *  @param nToId       ������Id
 *  @param nCreateTime ����ʱ��
 *  @param pAudioData  ָ��������Ϣ��ָ��
 *  @param nAudioLen   ������Ϣ�ĳ���
 *
 *  @return �ɹ���������id��ʧ�ܷ���-1
 */
int CAudioModel::saveAudioInfo(uint32_t nFromId, uint32_t nToId, uint32_t nCreateTime, const char* pAudioData, uint32_t nAudioLen)
{
	// parse audio data
	uint32_t nCostTime = CByteStream::ReadUint32((uchar_t*)pAudioData);
	uchar_t* pRealData = (uchar_t*)pAudioData + 4;
	uint32_t nRealLen = nAudioLen - 4;
    int nAudioId = -1;
    
	CHttpClient httpClient;
	string strPath = httpClient.UploadByteFile(m_strFileSite, pRealData, nRealLen);
	if (!strPath.empty())
    {
        CDBManager* pDBManager = CDBManager::getInstance();
        CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_master");
        if (pDBConn)
        {
            uint32_t nStartPos = 0;
            string strSql = "insert into IMAudio(`fromId`, `toId`, `path`, `size`, `duration`, `created`) "\
            "values(?, ?, ?, ?, ?, ?)";
            replace_mark(strSql, nFromId, nStartPos);
            replace_mark(strSql, nToId, nStartPos);
            replace_mark(strSql, strPath, nStartPos);
            replace_mark(strSql, nRealLen, nStartPos);
            replace_mark(strSql, nCostTime, nStartPos);
            replace_mark(strSql, nCreateTime, nStartPos);
            if (pDBConn->ExecuteUpdate(strSql.c_str()))
            {
                nAudioId = pDBConn->GetInsertId();
                log("audioId=%d", nAudioId);
            } else
            {
                log("sql failed: %s", strSql.c_str());
            }
            pDBManager->RelDBConn(pDBConn);
        }
        else
        {
            log("no db connection for teamtalk_master");
        }
	}
    else
    {
        log("upload file failed");
    }
	return nAudioId;
}

/**
 *  ��ȡ�����ľ�������
 *
 *  @param nCostTime ����ʱ��
 *  @param nSize     ������С
 *  @param strPath   �����洢��url
 *  @param cMsg      msg�ṹ��
 *
 *  @return �ɹ�����true��ʧ�ܷ���false
 */
bool CAudioModel::readAudioContent(uint32_t nCostTime, uint32_t nSize, const string& strPath, IM::BaseDefine::MsgInfo& cMsg)
{
	if (strPath.empty() || nCostTime == 0 || nSize == 0) {
		return false;
	}

	// �����ڴ棬д����Ƶʱ��
    AudioMsgInfo cAudioMsg;
    uchar_t* pData = new uchar_t [4 + nSize];
	cAudioMsg.data = pData;
	CByteStream::WriteUint32(cAudioMsg.data, nCostTime);
    cAudioMsg.data_len = 4;
    cAudioMsg.fileSize = nSize;

	// ��ȡ��Ƶ���ݣ�д�����������ڴ�
    CHttpClient httpClient;
    if(!httpClient.DownloadByteFile(strPath, &cAudioMsg))
    {
        delete [] pData;
        return false;
    }

    log("download_path=%s, data_len=%d", strPath.c_str(), cAudioMsg.data_len);
    cMsg.set_msg_data((const char*)cAudioMsg.data, cAudioMsg.data_len);
    
    delete [] pData;
    return true;
}
