
// 文件名 SplitString.h
// 用途: 分割字符串类
// 调用例子：
// vector<string> vectStr1;
// vector<CString> vectStr2;
// list<wstring> listStr3;
// SplitString("hello world", " ", vectStr1);
// SplitString(_T("hello world"), _T(" "), vectStr2);
// SplitString(L("hello world"), L(" "), listStr3);
#ifndef _SPLIT_STRING_H_
#define _SPLIT_STRING_H_

#include <string.h>
template<class BaseType>
class SplitAssist;

template<>
class SplitAssist<char>
{
public:
	static long GetLength(const char* pszStr)
	{
		return strlen(pszStr);
	}
	static const char* FindString(const char * pszSource, const char * pszFlags)
	{
		return strstr(pszSource, pszFlags);
	}
};
template<>
class SplitAssist<wchar_t>
{
public:
	static long GetLength(const wchar_t* pwszStr)
	{
		return wcslen(pwszStr);
	}
	static const wchar_t* FindString(const wchar_t* pwszSource, const wchar_t* pwszFlags)
	{
		return wcsstr(pwszSource, pwszFlags);
	}
};

template<class BaseType, class StringContent>
void SplitString(const BaseType* pszSource, const BaseType* pszFlags, vector<StringContent>& vectReturn)
{
	vectReturn.clear();
	if ((pszSource == NULL) || (pszFlags == NULL))
	{
		return;
	}
	size_t iSourceLen = SplitAssist<BaseType>::GetLength(pszSource);
	size_t iFlagLen = SplitAssist<BaseType>::GetLength(pszFlags);
	if (iFlagLen == 0)
	{
		StringContent str(pszSource, iSourceLen);
		vectReturn.push_back(str);
		return;
	}
	const BaseType* pPosStart = pszSource;

	for (const BaseType* pPosFind = SplitAssist<BaseType>::FindString(pPosStart, pszFlags); 
		pPosFind != NULL;
		pPosFind = SplitAssist<BaseType>::FindString(pPosStart, pszFlags))
	{
		int iLenValid = pPosFind - pPosStart;
		if (iLenValid >= 0)
		{
			StringContent str(pPosStart, iLenValid);
			vectReturn.push_back(str);
		}
		pPosStart = pPosFind + iFlagLen;
	}
	int iLenValid = iSourceLen - (pPosStart - pszSource);
	if (iLenValid >= 0)
	{
		StringContent str(pPosStart, iLenValid);
		vectReturn.push_back(str);
	}
}

#endif
