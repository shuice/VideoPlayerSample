/*
 *  Common.h
 *  Player
 *
 *  Create by xiaoyi liao  on 11-2-17.
 *  Copyright 2011 . All rights reserved.
 *
 */
#ifndef _COMMON_H_1_
#define _COMMON_H_1_
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;

#define THUMB_WIDTH 64
#define THUMB_HEIGHT 64

typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long LONG;
#pragma pack(1)
typedef struct tagBITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

struct SThumbBitmapHeader 
{
	BITMAPFILEHEADER m_bitmapFileHeader;
	BITMAPINFOHEADER m_bitmapInfoHeader;
};

#pragma pack()

struct SMediaInfoStream
{
	SMediaInfoStream()
	{
		memset(this, 0, sizeof(SMediaInfoStream));
	}
	bool m_bHasVideo;
	int m_iWidth;
	int m_iHeight;
	int m_iVideoCodec;
	int m_iVideoBitrate;
	int m_iFramerate;
	int m_iWidthEncoded;
	int m_iHeightEncoded;
	bool m_bHasAudio;
	int m_iSamplerate;	// hz
	int m_iChannels;
	int m_iAudioCodec;
	int m_iAudioBitrate;
	unsigned long m_iDuration;
};

typedef SMediaInfoStream SMediaInfoLocal;

struct SFileItem
{
	SFileItem()
	{
		m_bFolder = false;
		m_iFileSize = 0;
		m_iFileTime = 0;
	}
	string m_strName;
	bool m_bFolder;
	__int64_t m_iFileSize;
	__int64_t m_iFileTime;
	SMediaInfoStream m_sMediaInfoStream;
};

typedef SFileItem SFolderItem;

// a folder for controller showing
struct SFolder
{
	string m_strName; // current folder name
	vector<SFolderItem> m_vectSFolderItem;
};

bool operator < (const SFolderItem& sFolderItem1, const SFolderItem& sFolderItem2);

struct SSmbLanServer
{
	string m_strName;
	string m_strIpAddress;
	string m_strUser;
	string m_strPassword;
	string m_strWorkgroup;
};

struct SStreamServer
{
	string m_strIpAddress;
};

struct SApplicationSetting
{
	int m_iVolume; //0-100
	vector<SSmbLanServer> m_vectSmbLanServer;
	vector<SSmbLanServer> m_vectSmbLanServerAutoSearch;
	vector<SStreamServer> m_vectStreamServer;
};

struct SCodePageInfo
{
	string strName;
	int iValue;
	string strDesc;
	string strDesc2;
	SCodePageInfo()
	{
	}
	SCodePageInfo(string strName1, int iValue1, string strDesc1, string strDesc21)
	{
		strName = strName1;
		iValue = iValue1;
		strDesc = strDesc1;
		strDesc2 = strDesc21;
	}
	bool operator < (const SCodePageInfo& sCodePageInfo) const
	{
		return strName < sCodePageInfo.strName;
	}
};

SThumbBitmapHeader& GetThunbBitmapHeader();
void InitApplicationSetting();

string GetCodecIDDesc(int iCodecID);
void InitGetMediaInfo();
void UnInitGetMediaInfo();
void GetMediaInfo(const string& strFileName, SMediaInfoLocal& sMediaInfo);
bool GetFolderInfo(const string& strFolderName, SFolder& sFolder, string& strError);
void GetFilesInFolder(const string& strFolderName, vector<string>& vectSubtitles);
void GetFileSize(__int64_t iFileSize, char* pszSize, int iLen);
void GetFileDate(__int64_t iFileData, char* pszDate, int iLen);
void ShowAlartMessage(string strMessage);
bool IsValidIpAddress(const string& strIpAddress);
bool IsExtMatch(const string& strFileName, const string& strExt);
int GetMostSuitableSubtitle(const string& strFileName, const vector<SFileItem>& vectFileItem);
string GetLanTitle(const string& strName);
const SCodePageInfo& GetDefaultCodePage();
const SCodePageInfo& GetCodePageByIndex(int iIndex);
const SCodePageInfo& GetCodePagByName(const string& strName);
const SCodePageInfo& GetCodePageByValue(int iCodePage);
const vector<SCodePageInfo>& GetCodePageInfo();

#endif