/******************************************************************************* 
 *  @file      DownloadImgHttpOperation.cpp 2014\8\14 10:19:07 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "DownloadAvatarHttpOperation.h"
#include "http/httpclient/http.h"
#include "Modules/IMiscModule.h"
#include "Modules/IDatabaseModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IUserListModule.h"
#include "utility/utilStrCodingAPI.h"
#include "cxImage/cxImage/ximage.h"
/******************************************************************************/

// -----------------------------------------------------------------------------
//  DownloadImgHttpOperation: Public, Constructor

DownloadAvatarHttpOperation::DownloadAvatarHttpOperation(std::string sId, std::string& downUrl, BOOL bGrayScale
	,const std::string format, module::IOperationDelegate callback)
:IHttpOperation(callback)
,m_downUrl(downUrl)
,m_sId(sId)
,m_bGrayScale(bGrayScale)
,m_format(format)
{

}

// -----------------------------------------------------------------------------
//  DownloadImgHttpOperation: Public, Destructor

DownloadAvatarHttpOperation::~DownloadAvatarHttpOperation()
{

}

void DownloadAvatarHttpOperation::processOpertion()
{
	CString extName = ::PathFindExtension(util::stringToCString(m_downUrl));
	UInt32 hashcode = util::hash_BKDR((m_downUrl + m_format).c_str());
	std::string downFormat = util::cStringToString(extName);
	Http::HttpResponse	response;
	Http::HttpClient	client;
	Http::HttpRequest	request("GET", m_downUrl);

	if (isCanceled())
		return;

	//下载资源
	CString csFileName = util::int32ToCString(hashcode) + extName;
	CString csLocalPath = module::getMiscModule()->getDownloadDir() + csFileName;
	std::wstring cs = csLocalPath;
	request.saveToFile(cs);
	if (!client.execute(&request, &response))
	{
		CString csTemp = util::stringToCString(m_downUrl, CP_UTF8);
		LOG__(ERR, _T("DownloadImgHttpOperation failed %s"), csTemp);
		client.killSelf();
		return;
	}
	client.killSelf();

	if (PathFileExists(csLocalPath))
	{
		DownloadImgParam* pParam = new DownloadImgParam;
		pParam->m_sId = m_sId;
		pParam->m_result = DownloadImgParam::DOWNLOADIMG_OK;

		//存入ImImage表
		std::string localPath = util::cStringToString(csFileName);
		module::ImImageEntity imgTemp;
		module::ImImageEntity imgEntity = { hashcode, localPath, m_downUrl };
		module::getDatabaseModule()->sqlInsertImImageEntity(imgEntity);

		//会头像做灰度处理，并且保存到本地
		if (m_bGrayScale)
		{
			CxImage cximage;
			bool bSucc = cximage.Load(csLocalPath);
			if (bSucc)
			{
				if (cximage.GrayScale())
				{
					CString csGrayPath = module::getMiscModule()->getDownloadDir()
						+ PREFIX_GRAY_AVATAR + csFileName;
					cximage.Save(csGrayPath, CXIMAGE_SUPPORT_JPG);
				}
			}
		}

		if (!isCanceled())
		{
			//回调
			pParam->m_imgEntity = imgEntity;
			asyncCallback(std::shared_ptr<void>(pParam));
		}
	}
}

void DownloadAvatarHttpOperation::release()
{
	delete this;
}

/******************************************************************************/