/*******************************************************************************
 *  @file      utilStrCoding.h 2014\7\15 20:22:14 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef UTILSTRCODING_EBFB2FF8_92ED_408B_9A6B_A59579E9942D_H__
#define UTILSTRCODING_EBFB2FF8_92ED_408B_9A6B_A59579E9942D_H__

#include "GlobalDefine.h"
#include "utility/utilityDll.h"
#include <string>
/******************************************************************************/

NAMESPACE_BEGIN(util)

UTILITY_API CString		int32ToCString(Int32 n);
UTILITY_API std::string uint32ToString(UInt32 n);
UTILITY_API Int32		stringToInt32(const std::string& src);
UTILITY_API Int32		cstringToInt32(LPCTSTR buff);
UTILITY_API CString		utf8ToCString(const char* src);
UTILITY_API std::string	cStringToUtf8(const CString& src);
UTILITY_API std::string	cStringToString(const CString& src, UINT codepage = CP_UTF8);
UTILITY_API CString		stringToCString(const std::string& src, UINT codepage = CP_UTF8);

UTILITY_API BOOL		isIncludeChinese(const std::string& str);//是否包含中文
UTILITY_API CString		HZ2FirstPY(IN  std::string szHZ);//汉字转成拼音首字母
UTILITY_API CString		HZ2AllPY(IN  CString szHZ);//汉字转成全拼

UTILITY_API const std::string ws2s(const std::wstring& src);
UTILITY_API const std::wstring s2ws(const std::string& src);

UTILITY_API Int32       splitString(__in std::wstring src, __in std::vector<std::wstring> _vecSpliter,
    __out std::vector<std::wstring> &_splitList);

NAMESPACE_END(util)

/******************************************************************************/
#endif// UTILSTRCODING_EBFB2FF8_92ED_408B_9A6B_A59579E9942D_H__