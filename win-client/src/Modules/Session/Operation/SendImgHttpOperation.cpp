/******************************************************************************* 
 *  @file      SendImgOperation.cpp 2014\8\8 9:39:46 $
 *  @author    ´ó·ð<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "SendImgHttpOperation.h"
#include "http/httpclient/http.h"
#include "utility/utilStrCodingAPI.h"
#include "cxImage/cxImage/ximage.h"
#include "Modules/ISysConfigModule.h"
#include "json/reader.h"
#include <Shlwapi.h>
/******************************************************************************/

// -----------------------------------------------------------------------------
//  SendImgOperation: Public, Constructor

SendImgHttpOperation::SendImgHttpOperation(SendImgParam& sendImgParam, module::IOperationDelegate callback)
:IHttpOperation(callback)
,m_sendImgParam(sendImgParam)
{

}

// -----------------------------------------------------------------------------
//  SendImgOperation: Public, Destructor

SendImgHttpOperation::~SendImgHttpOperation()
{

}

void SendImgHttpOperation::processOpertion()
{
	if (m_sendImgParam.csFilePath.IsEmpty())
	{
		LOG__(ERR, _T("image path is empty!"));
		return;
	}
	module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
	std::string& fileSysAddr = util::cStringToString(pCfg->fileSysAddr);
	if (fileSysAddr.empty())
	{
		LOG__(ERR, _T("fileSysAddr is empty,m_downUrl: %s"),util::stringToCString(fileSysAddr));
		return;
	}

	std::string url = fileSysAddr + "upload/";
	Http::HttpResponse	response;
	Http::HttpClient	client;
	Http::HttpRequest	request("post", url);

	CString strExt = ::PathFindExtension(m_sendImgParam.csFilePath);
	SendImgParam* pPamram = new SendImgParam();
	pPamram->m_result = SendImgParam::SENDIMG_OK;
	pPamram->csFilePath = m_sendImgParam.csFilePath;

	std::string sExtName = util::cStringToString(strExt);
	std::string sContentType = "image/" + sExtName;
	UInt32 width = 0, height = 0;
	_getImgSize(width,height);
	CString csSize;
	csSize.Format(_T("_%dx%d%s"), width, height, strExt);
	std::string fileName = util::cStringToString(PathFindFileName(m_sendImgParam.csFilePath) + csSize);
	Http::HttpFileStream file(std::wstring(m_sendImgParam.csFilePath), fileName, sContentType);
	request.addFile("image", &file);

	if (isCanceled())
		return;

	client.execute(&request, &response);
	std::string header = response.getHeader();
	std::string body = response.getBody();
	std::string pathUrl;
	if (200 == response.getHttpCode() && _parseResponse(body, pathUrl))
	{
		if (fileSysAddr.at(fileSysAddr.length() - 1) != '/')
		{
			fileSysAddr += std::string("/");
		}
		pPamram->m_pathUrl = fileSysAddr + pathUrl;
	}
	else
	{
		CString csHeader = util::stringToCString(header);
		LOG__(ERR,_T("SendImgHttpOperation failed %s"), csHeader);
		pPamram->m_result = SendImgParam::SENDIMG_ERROR_UP;
	}

	if (!isCanceled())
	{
		asyncCallback(std::shared_ptr<void>(pPamram));
	}
	client.killSelf();
}

void SendImgHttpOperation::release()
{
	delete this;
}

BOOL SendImgHttpOperation::_parseResponse(IN const std::string& body, OUT std::string& pathUrl)
{
	Json::Reader reader;
	Json::Value root;
	try
	{
		if (!reader.parse(body, root) || root.isNull())
		{
			LOG__(ERR, _T("parse json failed! json data:%s"),util::stringToCString(body));
			return FALSE;
		}

		UInt32 errCode = root.get("error_code", 0).asUInt();
		if (errCode != 0)
		{
			std::string errMsg = root.get("error_msg", "").asString();
			CString csErrMsg = util::stringToCString(errMsg);
			LOG__(ERR, _T("sendImg errmsg:%s"), csErrMsg);
			return FALSE;
		}
		pathUrl = root.get("path", "").asString();
		if (pathUrl.empty())
			return FALSE;
		return TRUE;
	}
	catch (...)
	{
		LOG__(ERR, _T("parse json failed! json data:%s"), util::stringToCString(body));
		return FALSE;
	}
}

void SendImgHttpOperation::_getImgSize(UInt32& width, UInt32& height)
{
	if (!PathFileExists(m_sendImgParam.csFilePath))
	{
		LOG__(ERR, _T("file not exist"));
		return;
	}
	CxImage cximage;
	bool bSucc = cximage.Load(m_sendImgParam.csFilePath);
	if (bSucc)
	{
		width = cximage.GetWidth();
		height = cximage.GetHeight();
	}
}

/******************************************************************************/