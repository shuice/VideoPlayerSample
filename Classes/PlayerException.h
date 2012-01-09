/*
 *  PlayerException.h
 *  FFmpegTest
 *
 *  Create by xiaoyi liao  on 11-2-25.
 *  Copyright 2011 . All rights reserved.
 *
 */
#ifndef _PLAYER_EXCEPTION_H_
#define _PLAYER_EXCEPTION_H_

#include <string>
using namespace std;


class CPlayerException
{
public:
	CPlayerException(const char* pszFormat, ...);
	string m_strDescription;
};

#endif