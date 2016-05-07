
/******************************************************************************* 
 *  @file      ScreenCaptureModule_Impl.cpp 2015\5\5 15:51:49 $
 *  @author    ÄÏËÉ<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "ScreenCaptureModule_Impl.h"
#include "ScreenCapture/ScreenCapture.h"
#include "utility/utilCommonAPI.h"
#include "utility/utilStrCodingAPI.h"
#include "../../3rdParty/src/cxImage/cxImage/ximage.h"

/******************************************************************************/
namespace module
{
	IScreenCaptureModule* getScreenCaptureModule()
	{
		static ScreenCaptureModule_Impl module;
		return &module;
	}
}

BOOL ScreenCaptureModule_Impl::initCapture(__in HWND hWnd)
{
	return ScreenCapture::getInstance()->initCapture(hWnd);
}

void ScreenCaptureModule_Impl::onScreenCaptureFinish(__in std::wstring resultPicPath)
{
    //1. ¼ÓÔØµ½¼ôÇÐ°å
    /*CxImage img;
    img.Load(resultPicPath.c_str(), CXIMAGE_SUPPORT_PNG);
    HBITMAP hBitmap = img.MakeBitmap();
    if (!hBitmap)
    {
    return;
    }
    OpenClipboard(AfxGetMainWnd()->GetSafeHwnd());
    EmptyClipboard();
    SetClipboardData(CF_BITMAP, hBitmap);
    CloseClipboard();

    std::string strCapturePath = util::ws2s(resultPicPath);
    asynNotifyObserver(module::MODULE_SCREEN_CAPTURE_PREFIX + "id", strCapturePath);*/
    HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, resultPicPath.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (!hBitmap)
    {
        return;
    }
    if (!OpenClipboard(NULL))
    {
        return;
    }

    do
    {
        if (!EmptyClipboard())
        {
            break;
        }
        SetClipboardData(CF_BITMAP, hBitmap);

        std::string strCapturePath = util::ws2s(resultPicPath);
        asynNotifyObserver(module::MODULE_SCREEN_CAPTURE_PREFIX + "id", strCapturePath);
    } while (FALSE);

    CloseClipboard();
    DeleteObject(hBitmap);
}

BOOL ScreenCaptureModule_Impl::startCapture(__in std::wstring strSavePath, __in BOOL bMinimizeWindow)
{
	return ScreenCapture::getInstance()->startCapture(strSavePath, (ScreenCaptureCallback *)this, bMinimizeWindow);
}

module::ScreenCaptureHotkeyId ScreenCaptureModule_Impl::shouldHandle(__in LPARAM lParam)
{
	return (module::ScreenCaptureHotkeyId)ScreenCapture::getInstance()->shouldHandle(lParam);
}

void ScreenCaptureModule_Impl::cancelCapture()
{
    ScreenCapture::getInstance()->cancelCapture();
}

/**************************************************************/