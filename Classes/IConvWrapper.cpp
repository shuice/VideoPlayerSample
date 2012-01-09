/*
 *  IConvWrapper.cpp
 *  LanPlayer
 *
 *  Create by xiaoyi liao shuice on 11-5-10.
 *  Copyright 2011 viki. All rights reserved.
 *
 */

#include "IConvWrapper.h"

#include <iconv.h>
#include <iostream>

#define OUTLEN 255

using namespace std;

// 代码转换操作类
CIConvWrapper::CIConvWrapper(const char *to_charset, const char *from_charset)
{
	cd = iconv_open(to_charset,from_charset);
}
	
	// 析构
CIConvWrapper::~CIConvWrapper()
{
	iconv_close(cd);
}
	
	// 转换输出
int CIConvWrapper::Convert(char *inbuf,int inlen,char *outbuf,int outlen)
{
	char **pin = &inbuf;
	char **pout = &outbuf;
	memset(outbuf,0,outlen);
	return iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
}
