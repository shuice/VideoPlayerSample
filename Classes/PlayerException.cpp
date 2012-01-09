/*
 *  PlayerException.cpp
 *  FFmpegTest
 *
 *  Create by xiaoyi liao  on 11-2-25.
 *  Copyright 2011 . All rights reserved.
 *
 */

#include "PlayerException.h"

CPlayerException::CPlayerException(const char* pszFormat, ...)
{
	va_list vaList;
	va_start (vaList, pszFormat);
	char szDescription[1000] = {0};
	vsprintf(szDescription, pszFormat, vaList);
	va_end(vaList);
	m_strDescription = szDescription;
}