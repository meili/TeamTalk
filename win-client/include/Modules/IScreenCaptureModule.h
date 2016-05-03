/*******************************************************************************
 *  @file      IScreenCaptureModule.h 2014\7\17 19:38:12 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief   
 ******************************************************************************/
//{5CF3E642-5F2A-4841-B497-A7C015E6E1D4}

#ifndef ISCREEN_CAPTURE_5CF3E642_5F2A_4841_B497_A7C015E6E1D4_H__
#define ISCREEN_CAPTURE_5CF3E642_5F2A_4841_B497_A7C015E6E1D4_H__

#include "GlobalDefine.h"
#include "Modules/IModuleInterface.h"
#include "Modules/ModuleBase.h"
#include "Modules/ModuleDll.h"
#include <string>
/******************************************************************************/
NAMESPACE_BEGIN(module)
const std::string MODULE_SCREEN_CAPTURE_PREFIX = "screen_capture";

#define WM_SNAPSHOT_SCREEN_FILE_PATH WM_USER + 0x999
class IScreenCaptureCallback
{
public:
	virtual void onScreenCaptureFinish(__in std::wstring resultPicPath) = 0;
};

enum ScreenCaptureHotkeyId
{
    SC_HK_MIN = 0,
    SC_HK_START_CAPTURE,
    SC_HK_ESCAPE,

    SC_HK_MAX
};
/**
 * The class <code>IScreenCaptureModule</code> 
 *
 */
class MODULE_API IScreenCaptureModule:public module::ModuleBase, public IScreenCaptureCallback
{
public:
	virtual BOOL initCapture(__in HWND hWnd) = 0;
	virtual BOOL startCapture(__in std::wstring strSavePath, __in BOOL bMinimizeWindow = FALSE) = 0;
    virtual void cancelCapture() = 0;
    virtual ScreenCaptureHotkeyId shouldHandle(__in LPARAM lParam) = 0;
};

MODULE_API IScreenCaptureModule* getScreenCaptureModule();

NAMESPACE_END(module)
/******************************************************************************/
#endif// ISCREEN_CAPTURE_5CF3E642_5F2A_4841_B497_A7C015E6E1D4_H__
