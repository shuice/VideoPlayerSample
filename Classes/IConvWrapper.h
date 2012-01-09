/*
 *  IConvWrapper.h
 *  LanPlayer
 *
 *  Create by xiaoyi liao shuice on 11-5-10.
 *  Copyright 2011 viki. All rights reserved.
 *
 */

#include <iconv.h>
#include <iostream>

#define OUTLEN 255

using namespace std;

// 代码转换操作类
class CIConvWrapper
{
public:
	// 构造
	CIConvWrapper(const char *to_charset, const char *from_charset);
	// 析构
	~CIConvWrapper();
	
	// 转换输出
	int Convert(char *inbuf,int inlen,char *outbuf,int outlen);
private:
	iconv_t cd;
};
