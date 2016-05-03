/******************************************************************************* 
 *  @file      UIIMEdit.cpp 2014\8\19 13:20:13 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "UIIMEdit.h"
#include "Modules/IMiscModule.h"
#include "Modules/IP2PCmdModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IEmotionModule.h"
#include "GifSmiley/GifSmiley.h"
#include "cxImage/cxImage/ximage.h"
#include "utility/utilStrCodingAPI.h"

CString MixedMsg::AddPicTeg2Pic(IN CString picPath)
{
	CString strAddedMSG;
	strAddedMSG += CS_SPLIT_CODE_START;
	strAddedMSG += picPath;
	strAddedMSG += CS_SPLIT_CODE_END;
	return strAddedMSG;
}

MixedMsg::MixedMsg()
:m_nSetNetWorkPathSuccTime(0)
{

}

BOOL MixedMsg::SetNetWorkPicPath(IN CString strLocalPicPath, IN CString strNetPicPath)
{
	for (auto& picData : m_picDataVec)
	{
		if (picData.strLocalPicPath == strLocalPicPath)
		{
			picData.strNetPicPath = strNetPicPath;
			m_nSetNetWorkPathSuccTime++;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL MixedMsg::SucceedToGetAllNetWorkPic()
{
	return m_nSetNetWorkPathSuccTime == m_picDataVec.size();
}

CString MixedMsg::MakeMixedLocalMSG()
{
	CString msg = m_strTextData;
	int nPosAdd = 0;
	for (auto picData : m_picDataVec)
	{
		CString strPic = AddPicTeg2Pic(picData.strLocalPicPath);
		msg.Insert(nPosAdd + picData.nPos, strPic.GetBuffer());
		strPic.ReleaseBuffer();
		nPosAdd += strPic.GetLength();
	}
	return msg;
}

CString MixedMsg::MakeMixedNetWorkMSG()
{
	CString msg = m_strTextData;
	int nPosAdd = 0;
	for (auto picData : m_picDataVec)
	{
		CString strPic = AddPicTeg2Pic(picData.strNetPicPath);
		msg.Insert(nPosAdd + picData.nPos, strPic.GetBuffer());
		strPic.ReleaseBuffer();
		msg.Delete(nPosAdd + picData.nPos + strPic.GetLength());
		nPosAdd += strPic.GetLength();
		--nPosAdd;
	}
	return msg;
}

BOOL MixedMsg::IsPureTextMsg()
{
	return m_picDataVec.empty();
}

void MixedMsg::ReplaceReturnKey(void)
{
    if (m_strTextData.IsEmpty())
    {
        return;
    }

    std::vector<std::wstring> _vecSpliter;
    _vecSpliter.push_back(L"\r\n");
    _vecSpliter.push_back(L"\r");
    _vecSpliter.push_back(L"\n");

    std::vector<std::wstring> _vecList;
    util::splitString(m_strTextData.GetBuffer(), _vecSpliter, _vecList);

    m_strTextData = _T("");
    for (unsigned int i = 0; i < _vecList.size(); i++)
    {
        m_strTextData += _vecList[i].c_str();
        m_strTextData += _T("\r\n");
    }
}

/******************************************************************************/

// -----------------------------------------------------------------------------
//  UIIMEdit: Public, Constructor

UIIMEdit::UIIMEdit()
{
}

// -----------------------------------------------------------------------------
//  UIIMEdit: Public, Destructor

UIIMEdit::~UIIMEdit()
{
}

LPVOID UIIMEdit::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, _T("UIIMEdit")) == 0) return static_cast<UIIMEdit*>(this);
	return __super::GetInterface(pstrName);
}

LPCTSTR UIIMEdit::GetClass() const
{
	return _T("UIIMEdit");
}

void UIIMEdit::_ImEditPaste()
{
	BOOL bHadPic = FALSE;
	if (::OpenClipboard(::GetDesktopWindow()))
	{
		if (IsClipboardFormatAvailable(CF_BITMAP))
		{
			HBITMAP hBitmap = (HBITMAP)::GetClipboardData(CF_BITMAP);
			BITMAP bitmap;
			GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bitmap);
			SIZE bitmapSize = { bitmap.bmWidth, bitmap.bmHeight};
			if (hBitmap)
			{
				_SaveFile(hBitmap, m_strImagePath);
				InsertImage(m_strImagePath.GetBuffer(),bitmapSize,FALSE);
				bHadPic = TRUE;
			}
		}
		CloseClipboard();
	}
}

BOOL UIIMEdit::_SaveFile(IN HBITMAP hbitmap, OUT CString& strFilePath)
{
	SYSTEMTIME st = { 0 };
	GetLocalTime(&st);
	CString strTime;
	strTime.Format(_T("%d-%02d-%02d-%02d%02d%02d-%03d"), st.wYear,
		st.wMonth,
		st.wDay,
		st.wHour,
		st.wMinute,
		st.wSecond,
		st.wMilliseconds);
	util::createAllDirectories(module::getMiscModule()->getUserTempDir());
	CString csDstFileName = module::getMiscModule()->getUserTempDir() + strTime;
	CString csExtName = _T(".jpg");
	CxImage cximage;
	cximage.CreateFromHBITMAP(hbitmap);
	if (cximage.Save(csDstFileName + csExtName, CXIMAGE_FORMAT_JPG))
	{
		strFilePath = csDstFileName + csExtName;
		return TRUE;
	}
	return FALSE;
}

HBITMAP UIIMEdit::_LoadAnImage(IN CString filePath)
{
	HANDLE hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //从指定的路径szImagePath中读取文件句柄
	DWORD dwFileSize = GetFileSize(hFile, NULL); //获得图片文件的大小，用来分配全局内存
	HGLOBAL hImageMemory = GlobalAlloc(GMEM_MOVEABLE, dwFileSize); //给图片分配全局内存
	void *pImageMemory = GlobalLock(hImageMemory); //锁定内存
	DWORD dwReadedSize; //保存实际读取的文件大小
	ReadFile(hFile, pImageMemory, dwFileSize, &dwReadedSize, NULL); //读取图片到全局内存当中
	GlobalUnlock(hImageMemory); //解锁内存
	CloseHandle(hFile); //关闭文件句柄

	HRESULT hr = NULL;
	IStream *pIStream = NULL;//创建一个IStream接口指针，用来保存图片流
	IPicture *pIPicture = NULL;//创建一个IPicture接口指针，表示图片对象

	hr = CreateStreamOnHGlobal(hImageMemory, false, &pIStream); //用全局内存初使化IStream接口指针
	ASSERT(SUCCEEDED(hr));

	hr = OleLoadPicture(pIStream, 0, false, IID_IPicture, (LPVOID*)&(pIPicture));//用OleLoadPicture获得IPicture接口指针
	ASSERT(SUCCEEDED(hr));

	HBITMAP hB = NULL;
	pIPicture->get_Handle((unsigned int*)&hB);
	// Copy the image. Necessary, because upon p's release,
	// the handle is destroyed.
	HBITMAP hBB = (HBITMAP)CopyImage(hB, IMAGE_BITMAP, 0, 0,
		LR_COPYRETURNORG);

	GlobalFree(hImageMemory); //释放全局内存
	pIStream->Release(); //释放pIStream
	pIPicture->Release(); //释放pIPictur
	return hBB;
}

int UIIMEdit::GetObjectPos()
{
	bool findObject = false;

	IRichEditOle *pRichEditOle = m_pRichEditOle;
	if (NULL == pRichEditOle)
	{
		return 0;
	}

	int nCount = pRichEditOle->GetObjectCount();
	for (int i = nCount - 1; i >= 0; i--)
	{
		REOBJECT reobj = { 0 };
		reobj.cbStruct = sizeof(REOBJECT);
		pRichEditOle->GetObject(i, &reobj, REO_GETOBJ_POLEOBJ);
		reobj.poleobj->Release();
	}
	return 0;
}

void UIIMEdit::InsertImage(BSTR bstrFileName,SIZE size,BOOL isGif)
{
	LPSTORAGE lpStorage = NULL;
	LPOLEOBJECT	lpObject = NULL;
	LPLOCKBYTES lpLockBytes = NULL;
	LPOLECLIENTSITE lpClientSite = NULL;
	GifSmiley::IGifSmileyCtrl* lpAnimator = nullptr;
	HRESULT hr = ::CoCreateInstance(GifSmiley::CLSID_CGifSmileyCtrl, NULL, CLSCTX_INPROC, GifSmiley::IID_IGifSmileyCtrl, (LPVOID*)&lpAnimator);
	if (NULL == lpAnimator || FAILED(hr))
	{
		LOG__(ERR, _T("InsertImage CoCreateInstance failed"));
		goto End;
	}

	COLORREF backColor = (COLORREF)(::GetSysColor(COLOR_WINDOW));
	HWND hwnd = (HWND)((long)m_pManager->GetPaintWindow());
	IRichEditOle *pRichEditOle = m_pRichEditOle;
	if (NULL == pRichEditOle)
		goto End;
	BSTR path = NULL;
	//Create lockbytes
	hr = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
	if (FAILED(hr))
	{
		LOG__(ERR, _T("InsertImage CreateILockBytesOnHGlobal failed"));
		goto End;
	}
	//use lockbytes to create storage
	SCODE sc = ::StgCreateDocfileOnILockBytes(lpLockBytes, STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, &lpStorage);
	if (sc != S_OK)
	{
		LOG__(ERR, _T("InsertImage StgCreateDocfileOnILockBytes failed"));
		goto End;
	}
	// retrieve OLE interface for richedit   and  Get site
	pRichEditOle->GetClientSite(&lpClientSite);
	try
	{
		//COM operation need BSTR, so get a BSTR
		path = bstrFileName;
		//Load the image
		if (isGif)
			lpAnimator->LoadFromFile(path);
		else
		{
			UInt32 height = (size.cy < GetHeight()) ? size.cy : GetHeight();
			UInt32 width = (size.cx < GetWidth() / 2) ? size.cx : GetWidth() / 2;
			lpAnimator->LoadFromFileSized(path, width, height);
		}
		//Set back color
		OLE_COLOR oleBackColor = (OLE_COLOR)backColor;
		lpAnimator->put_BackColor(oleBackColor);
		//get the IOleObject
		hr = lpAnimator->QueryInterface(IID_IOleObject, (void**)&lpObject);
		if (FAILED(hr))
		{
			LOG__(ERR, _T("InsertImage lpAnimator QueryInterface failed"));
			goto End;
		}
		//Set it to be inserted
		OleSetContainedObject(lpObject, TRUE);
		//to insert into richedit, you need a struct of REOBJECT
		REOBJECT reobject;
		ZeroMemory(&reobject, sizeof(REOBJECT));
		reobject.cbStruct = sizeof(REOBJECT);
		CLSID clsid;
		hr = lpObject->GetUserClassID(&clsid);
		//set clsid
		reobject.clsid = clsid;
		//can be selected
		reobject.cp = REO_CP_SELECTION;
		//content, but not static
		reobject.dvaspect = DVASPECT_CONTENT;
		//goes in the same line of text line
		reobject.dwFlags = REO_BELOWBASELINE;
		//reobject.dwUser = (DWORD)myObject;
		//the very object
		reobject.poleobj = lpObject;
		//client site contain the object
		reobject.polesite = lpClientSite;
		//the storage 
		reobject.pstg = lpStorage;
		SIZEL sizel = { 0 };
		reobject.sizel = sizel;
		LPOLECLIENTSITE lpObjectClientSite = NULL;
		hr = lpObject->GetClientSite(&lpObjectClientSite);
		if (FAILED(hr) || lpObjectClientSite == NULL)
			lpObject->SetClientSite(lpClientSite);
		pRichEditOle->InsertObject(&reobject);
		//redraw the window to show animation
		::RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
	}
	catch (...)
	{
		LOG__(ERR, _T("InsertImage unknown exeption"));
	}

End:
	if (lpClientSite)
	{
		lpClientSite->Release();
		lpClientSite = nullptr;
	}
	if (lpObject)
	{
		lpObject->Release();
		lpObject = nullptr;
	}
	if (lpLockBytes)
	{
		lpLockBytes->Release();
		lpLockBytes = nullptr;
	}
	if (lpStorage)
	{
		lpStorage->Release();
		lpStorage = nullptr;
	}
	if (lpAnimator)
	{
		lpAnimator->Release();
		lpAnimator = nullptr;
	}
}

HRESULT UIIMEdit::GetNewStorage(LPSTORAGE* ppStg)
{

	if (!ppStg)
		return E_INVALIDARG;

	*ppStg = NULL;

	//
	// We need to create a new storage for an object to occupy.  We're going
	// to do this the easy way and just create a storage on an HGLOBAL and let
	// OLE do the management.  When it comes to saving things we'll just let
	// the RichEdit control do the work.  Keep in mind this is not efficient, 
	// but this program is just for demonstration.
	//

	LPLOCKBYTES pLockBytes;
	HRESULT hr = CreateILockBytesOnHGlobal(NULL, TRUE, &pLockBytes);
	if (FAILED(hr))
		return hr;

	hr = StgCreateDocfileOnILockBytes(pLockBytes,
		STGM_SHARE_EXCLUSIVE | STGM_CREATE |
		STGM_READWRITE,
		0,
		ppStg);
	pLockBytes->Release();
	return (hr);
}


void UIIMEdit::GetObjectInfo(IRichEditOle *pIRichEditOle)
{
	long count = pIRichEditOle->GetObjectCount();
	if (count)
	{
		REOBJECT reobj = { 0 };
		reobj.cbStruct = sizeof(REOBJECT);
		pIRichEditOle->GetObject(0, &reobj, REO_GETOBJ_POLEOBJ);
		GifSmiley::IGifSmileyCtrl* lpAnimator = 0;
		HRESULT hr = reobj.poleobj->QueryInterface(GifSmiley::IID_IGifSmileyCtrl, (void**)&lpAnimator);
		if (SUCCEEDED(hr) && lpAnimator)
		{
			BSTR*  fileName = nullptr;
			hr = lpAnimator->FileName(fileName);
		}
		reobj.poleobj->Release();
	}
}

void UIIMEdit::ReleaseAllGif()
{
	SetText(_T(""));
	IRichEditOle *pRichEditOle = m_pRichEditOle;
	PTR_VOID(pRichEditOle);

	for (int i = 0; i < pRichEditOle->GetObjectCount(); i++)
	{
		REOBJECT reobj = { 0 };
		reobj.cbStruct = sizeof(REOBJECT);
		pRichEditOle->GetObject(i, &reobj, REO_GETOBJ_POLEOBJ);
		GifSmiley::IGifSmileyCtrl* lpAnimator = 0;
		HRESULT hr = reobj.poleobj->QueryInterface(GifSmiley::IID_IGifSmileyCtrl, (void**)&lpAnimator);
		if (SUCCEEDED(hr) && lpAnimator)
		{
			lpAnimator->FreeImage();
			lpAnimator->Release();
		}
		reobj.poleobj->Release();
	}
}

BOOL UIIMEdit::GetPicPosAndPathbyOrder(IN UInt32 nOrder, OUT UInt32& nPos, OUT CString& path)
{
	IRichEditOle* pRichEditOle = m_pRichEditOle;
	PTR_FALSE(pRichEditOle);

	REOBJECT reobj = { 0 };
	reobj.cbStruct = sizeof(REOBJECT);
	HRESULT hr = pRichEditOle->GetObject(nOrder, &reobj, REO_GETOBJ_POLEOBJ);
	if (SUCCEEDED(hr) && reobj.poleobj)
	{
		GifSmiley::IGifSmileyCtrl* lpAnimator = 0;
		hr = reobj.poleobj->QueryInterface(GifSmiley::IID_IGifSmileyCtrl, (void**)&lpAnimator);
		if (SUCCEEDED(hr) && lpAnimator)
		{
			CComBSTR  fileName;
			hr = lpAnimator->FileName(&fileName);
			if (SUCCEEDED(hr) && 0 != fileName.Length())
			{
				nPos = reobj.cp;
				path = CString(fileName.m_str);
			}
			lpAnimator->FreeImage();
			lpAnimator->Release();
		}
		reobj.poleobj->Release();
		return TRUE;
	}
	return FALSE;
}

LRESULT UIIMEdit::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	if (uMsg == WM_KEYDOWN)
	{
		if ('V' == wParam && ::GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			_ImEditPaste();
		}
		else if (VK_RETURN == wParam)//回车
		{
			module::TTConfig* pTTConfig = module::getSysConfigModule()->getSystemConfig();
			BOOL bWantCtrlEnter = (pTTConfig->sysBaseFlag & module::BASE_FLAG_SENDIMG_BY_CTRLENTER);
			SetWantReturn(!bWantCtrlEnter);
		}
	}
	else if (uMsg == WM_KILLFOCUS)
	{
		//removed by kuaidao 2015-03-05,这段话会导致焦点之间切换有问题
		////fix bug,当嵌入Windows控件时，如果别的windows控件（如嵌入的IE，它是该窗口的一个子窗口）得到Focus的时候，自己的焦点并没有去掉，造成下次不能再输入
		//https://code.google.com/p/duilib/issues/detail?id=102
		//if (m_bFocused && m_pManager
		//	&& m_pManager->GetFocus() == this)
		//{
		//	OutputDebugString(_T("-------WM_KILLFOCUS\n"));
		//	m_pManager->SetFocus(NULL);
		//	m_bFocused = FALSE;
		//}
		
		if (m_pTwh)
		{
			m_pTwh->GetTextServices()->OnTxUIDeactivate();
			m_pTwh->GetTextServices()->TxSendMessage(WM_KILLFOCUS, 0, 0, 0);
			m_pTwh->TxShowCaret(FALSE);
		}
	}
	else if (uMsg == WM_SETFOCUS)
	{
		if (m_pTwh)
		{
			m_pTwh->GetTextServices()->OnTxUIActivate();
			m_pTwh->GetTextServices()->TxSendMessage(WM_SETFOCUS, 0, 0, 0);
			m_pTwh->TxShowCaret(TRUE);
		}
	}
	return __super::MessageHandler(uMsg, wParam, lParam, bHandled);
}

bool UIIMEdit::OnTxViewChanged()
{
	m_pManager->SendNotify(this, UIIMEdit_MSGTYPE_TEXTCHANGED);
	return true;
}

BOOL UIIMEdit::GetContent(OUT MixedMsg& mixMsg)
{
	mixMsg.m_strTextData = GetTextRange(0, GetTextLength());
	if (mixMsg.m_strTextData.IsEmpty())
		return FALSE;
    mixMsg.ReplaceReturnKey();

	IRichEditOle *pRichEditOle = m_pRichEditOle;
	if (NULL == pRichEditOle)
	{
		return FALSE;
	}
	UInt32 nImageCount = pRichEditOle->GetObjectCount();
	if (nImageCount == 0)//纯文字
	{
		CString strContent = mixMsg.m_strTextData;
		strContent.Trim();
		if (strContent.IsEmpty())
		{
			LOG__(DEBG, _T("After trimed,is empty msg"));//日志干扰
			return FALSE;
		}
	}
	else//图文混排
	{
		CString strEmotionFilesDir = module::getMiscModule()->getEmotionFilesDir();
		int nPosAdd = 0;
		for (UInt32 i = 0; i < nImageCount; i++)
		{
			ST_picData picData;
			if (GetPicPosAndPathbyOrder(i, picData.nPos, picData.strLocalPicPath))
			{
				TCHAR fullPath[MAX_PATH] = { 0 };
				LPTSTR* pStart = nullptr;
				if (!GetFullPathName(picData.strLocalPicPath, MAX_PATH, fullPath, pStart))
				{
					continue;
				}
				CString strFullPath = fullPath;
				int nPos = strFullPath.Find(strEmotionFilesDir);
				if (-1 != nPos)
				{
					//是表情，不用上传图片
					int nLen = picData.strLocalPicPath.GetLength();
					CString fileName = strFullPath.Mid(strEmotionFilesDir.GetLength(), nLen - strEmotionFilesDir.GetLength() + 1);
					CString fileID;
					if (!module::getEmotionModule()->getEmotionIDByName(fileName, fileID))
					{
						return FALSE;
					}
					mixMsg.m_strTextData.Insert(nPosAdd + picData.nPos, fileID);
					mixMsg.m_strTextData.Delete(nPosAdd + picData.nPos + fileID.GetLength(), 1);
					nPosAdd += picData.nPos + fileID.GetLength();
				}
				else
				{
					picData.nPos += nPosAdd;
					mixMsg.m_picDataVec.push_back(picData);
				}
			}
		}
	}
	SetText(_T(""));
	return TRUE;
}

void UIIMEdit::DoInit()
{
	__super::DoInit();
	GetRichEditOle();
}




/******************************************************************************/