
/*******************************************************************************
 *  @file      Common.h 2015\5\5 15:56:19 $
 *  @author    南松<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef COMMON_AB85173C_FF84_48AF_A453_99E670F74E73_H__
#define COMMON_AB85173C_FF84_48AF_A453_99E670F74E73_H__

#include <windows.h>
#include <string>
/******************************************************************************/

/**
 * The class <code>Common</code> 
 *
 */

template <typename T>
class Singleton
{
public:
	static T* Instance()
	{
		static T obj;
		return &obj;
	}
};


#define SCREEN_CAPTURE_MSG_WND_CLASS L"SCREEN_CAPTURE_MSG_WND_CLASS"
#define SCREEN_CAPTURE_MSG_WND_NAME L"SCREEN_CAPTURE_MSG_WND_NAME"

#define WM_SNAPSHOT_MOUSE_WINDOW_CHANGE WM_USER + 0x888
#define WM_SNAPSHOT_ADJUST_TOOL_BAR_POSITION WM_USER + 0x887
#define WM_SNAPSHOT_TEST_SELECT_IDX WM_USER + 0x886
#define WM_SNAPSHOT_TEST_SELECT_RECT WM_USER + 0x885

#define WM_SNAPSHOT_DRAW_RECT WM_USER + 0x884
#define WM_SNAPSHOT_DRAW_POLY_REGION WM_USER + 0x883
#define WM_SNAPSHOT_ADD_RECT_ITEM WM_USER + 0x882
#define WM_SNAPSHOT_ADD_POLY_REGION_ITEM WM_USER + 0x881

#define WM_SNAPSHOT_MODE_CHANGE WM_USER + 0x880
#define WM_SNAPSHOT_FINISH_CAPTURE WM_USER + 0x879
#define WM_SNAPSHOT_CANCEL_CPATURE WM_USER + 0x878


namespace ScreenCommon
{
	inline LPWORD makeWORDAlign(LPWORD lpIn)
	{
		//地址2字节对齐
		ULONG ul;
		ul = (ULONG)lpIn;
		ul++;
		ul >>= 1;
		ul <<= 1;

		return (LPWORD)ul;
	}

	inline BOOL isRectEmpty(const RECT &rc)
	{
		if ((rc.right <= rc.left)
			||(rc.bottom <= rc.top))
		{
			return TRUE;
		}

		return FALSE;
	}

    inline BOOL isRectEqual(const RECT *rc1, const RECT *rc2)
    {
        if (rc1->left == rc2->left
            && rc1->top == rc2->top
            && rc1->right == rc2->right
            && rc1->bottom == rc2->bottom)
        {
            return TRUE;
        }
        return FALSE;
    }

	inline BOOL isLButtonDown()
	{
		SHORT iKeyState = GetAsyncKeyState(VK_LBUTTON);
		return iKeyState & 0x8000;
	}

    inline void generateMsgWindowName(__inout wchar_t *szWndName, __in int maxLen)
    {
        wnsprintf(szWndName, maxLen, L"%s_%d", SCREEN_CAPTURE_MSG_WND_NAME, GetCurrentProcessId());
    }

    inline HWND findMsgWindow()
    {
        wchar_t szWndName[MAX_PATH] = {0};
        generateMsgWindowName(szWndName, MAX_PATH);

        return FindWindow(szWndName, szWndName);
    }

    inline void postNotifyMessage(__in int uMsg, __in WPARAM wParam, __in LPARAM lParam)
    {
        HWND hMsgWnd = findMsgWindow();
        PostMessage(hMsgWnd, uMsg, wParam, lParam);
    }

    inline LRESULT sendNotifyMessage(__in int uMsg, __in WPARAM wParam, __in LPARAM lParam)
    {
        HWND hMsgWnd = findMsgWindow();
        return SendMessage(hMsgWnd, uMsg, wParam, lParam);
    }
}

/******************************************************************************/
#endif// COMMON_AB85173C-FF84-48AF-A453-99E670F74E73_H__