/*
 *  Common.cpp
 *  Player
 *
 *  Create by xiaoyi liao  on 11-2-17.
 *  Copyright 2011 . All rights reserved.
 *
 */
#include   <sys/socket.h> 
#include   <netinet/in.h> 
#include   <arpa/inet.h>
#include <ifaddrs.h>
#include "Common1.h"
#include "Common.h"
#include "SplitString.h"
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <map>
#include <set>
using namespace std;



bool GetFolderInfo(const string& strFolderName, SFolder& sFolder, string& strError)
{
	sFolder.m_vectSFolderItem.clear();
	
	sFolder.m_strName = strFolderName;
	DIR* pDir = opendir (strFolderName.c_str());
	if (pDir == NULL) 
	{
		strError = "Failed to open dir";
		return false;
	}
	struct stat statbuf;
	dirent* pDirent = NULL;
	char szFullPath[1024] = {0};
	while ((pDirent = readdir (pDir)) != NULL)
	{
		if (*pDirent->d_name == '.')
		{
			continue;
		}
		snprintf(szFullPath, sizeof(szFullPath), "%s/%s", strFolderName.c_str(), pDirent->d_name);
		if (stat(szFullPath, &statbuf) == -1)
		{
			continue;
		}
		
		SFolderItem sFolderItem;
		sFolderItem.m_strName = pDirent->d_name;
		sFolderItem.m_bFolder = S_ISDIR(statbuf.st_mode);
		sFolderItem.m_iFileSize = statbuf.st_size;
		sFolderItem.m_iFileTime = statbuf.st_mtime;
		sFolder.m_vectSFolderItem.push_back(sFolderItem);
	}
	closedir(pDir);
	return true;
}

// ext include '.'
bool IsExtMatch(const string& strFileName, const string& strExt)
{
	int iExtLength = strExt.size();
	int iFileNameLen = strFileName.size();
	if ((iExtLength == 0) || (iFileNameLen <= iExtLength))
	{
		return false;
	}
	
	const char* pszMayBeExt = strFileName.c_str() + iFileNameLen - iExtLength;
	if (strcasecmp(pszMayBeExt, strExt.c_str()) == 0)
	{
		return true;
	}
	return false;
}

int GetMostSuitableSubtitle(const string& strFileName, const vector<SFileItem>& vectFileItem)
{
	// file name may contain more than one dot
	char szFileName[1024] = {0};
	char* pszDotPos = strrchr(strFileName.c_str(), '.');
	if (pszDotPos == NULL)
	{
		strncpy(szFileName, strFileName.c_str(), strFileName.size());
	}
	else
	{
		memcpy(szFileName, strFileName.c_str(), pszDotPos - strFileName.c_str());
	}
	int iFileTitleLen = strlen(szFileName);
	int iCount = vectFileItem.size();
	for (int iIndex = 0; iIndex < iCount; iIndex ++)
	{
		const SFileItem& sFileItem = vectFileItem[iIndex];
		if (sFileItem.m_strName.size() <= iFileTitleLen)
		{
			continue;
		}
		
		if (strncasecmp(sFileItem.m_strName.c_str(), szFileName, iFileTitleLen) != 0)
		{
			continue;
		}
		if (sFileItem.m_strName[iFileTitleLen] == '.')
		{
			return iIndex;
		}
	}

	return 0;
}



const vector<SCodePageInfo>& GetCodePageInfo()
{
	static vector<SCodePageInfo> vectSCodePageInfo;
	if (vectSCodePageInfo.size() == 0)
	{
		vectSCodePageInfo.push_back(SCodePageInfo("en", 1252, "英文", "English"));
		vectSCodePageInfo.push_back(SCodePageInfo("zh-Hans", 936, "简体中文", "简体中文"));
		vectSCodePageInfo.push_back(SCodePageInfo("ja", 932, "日语", "日本語"));
		vectSCodePageInfo.push_back(SCodePageInfo("fr", 1252, "法语", "Français"));
		vectSCodePageInfo.push_back(SCodePageInfo("de", 1252, "德语", "Deutsch"));
		
		vectSCodePageInfo.push_back(SCodePageInfo("nl", 1252, "荷兰", "Nederlands"));
		vectSCodePageInfo.push_back(SCodePageInfo("it", 1252, "意大利", "Italiano"));
		vectSCodePageInfo.push_back(SCodePageInfo("es", 1252, "西班牙", "Español"));
		vectSCodePageInfo.push_back(SCodePageInfo("pt", 1252, "葡萄牙", "Português"));
		vectSCodePageInfo.push_back(SCodePageInfo("pt-PT", 1252, "葡萄牙语（葡萄牙)", "Português (Portugal)"));
		
		vectSCodePageInfo.push_back(SCodePageInfo("da", 1252, "丹麦", "Dansk"));
		vectSCodePageInfo.push_back(SCodePageInfo("fi", 1252, "芬兰", "Suomi"));
		vectSCodePageInfo.push_back(SCodePageInfo("nb", 1252, "挪威语", "Norsk (bokmål)"));
		vectSCodePageInfo.push_back(SCodePageInfo("sv", 1252, "瑞典语", "Svenska"));
		vectSCodePageInfo.push_back(SCodePageInfo("ko", 949, "朝韩", "한국어"));
		
		vectSCodePageInfo.push_back(SCodePageInfo("zh-Hant", 950, "繁体中文", "繁體中文"));
		vectSCodePageInfo.push_back(SCodePageInfo("ru", 1251, "俄罗斯", "Русский"));
		vectSCodePageInfo.push_back(SCodePageInfo("pl", 1250, "波兰", "Polski"));
		vectSCodePageInfo.push_back(SCodePageInfo("tr", 1254, "土耳其", "Türkçe"));
		vectSCodePageInfo.push_back(SCodePageInfo("uk", 1252, "乌克兰", "Українська"));
		
		vectSCodePageInfo.push_back(SCodePageInfo("ar", 1252, "阿拉伯", "العربية"));
		vectSCodePageInfo.push_back(SCodePageInfo("hr", 10082, "克罗地亚", "Hrvatski"));
		vectSCodePageInfo.push_back(SCodePageInfo("cs", 1252, "捷克", "České"));
		vectSCodePageInfo.push_back(SCodePageInfo("el", 1253, "希腊语", "Ελληνικά"));
		vectSCodePageInfo.push_back(SCodePageInfo("he", 1255, "希伯来语", "עברית"));
		
		vectSCodePageInfo.push_back(SCodePageInfo("ro", 1250, "罗马尼亚", "Română"));
		vectSCodePageInfo.push_back(SCodePageInfo("sk", 1250, "斯洛伐克", "Slovenčina"));
		vectSCodePageInfo.push_back(SCodePageInfo("th", 874, "泰国", "ภาษาไทย"));
		vectSCodePageInfo.push_back(SCodePageInfo("id", 1252, "印度尼西亚", "Bahasa Indonesia "));
		vectSCodePageInfo.push_back(SCodePageInfo("ms", 1252, "马来西亚语", "Bahasa Melayu"));
		
		vectSCodePageInfo.push_back(SCodePageInfo("en-GB", 1252, "英语(英国)", "British English"));
		vectSCodePageInfo.push_back(SCodePageInfo("ca", 1252, "加泰罗尼亚语", "català"));
		vectSCodePageInfo.push_back(SCodePageInfo("hu", 1250, "匈牙利", "Magyar"));
		vectSCodePageInfo.push_back(SCodePageInfo("vi", 1258, "越南语", "Tiêng Việt"));
	}
	return vectSCodePageInfo;
}

const SCodePageInfo& GetCodePageByIndex(int iIndex)
{
	const vector<SCodePageInfo>& vectSCodePageInfo = GetCodePageInfo();
	if (iIndex < vectSCodePageInfo.size())
	{
		return vectSCodePageInfo[iIndex];
	}
	return vectSCodePageInfo[0];
}

const SCodePageInfo& GetCodePagByName(const string& strName)
{
	const vector<SCodePageInfo>& vectSCodePageInfo = GetCodePageInfo();
	int iCodePageInfoCount = vectSCodePageInfo.size();
	for (int iCodePageInfoIndex = 0; iCodePageInfoIndex < iCodePageInfoCount; iCodePageInfoIndex ++)
	{
		const SCodePageInfo& sCodePageInfo = vectSCodePageInfo[iCodePageInfoIndex];
		if (sCodePageInfo.strName == strName)
		{
			return sCodePageInfo;
		}
	}
	return vectSCodePageInfo[0];
}
							 
const SCodePageInfo& GetCodePageByValue(int iCodePage)
{
	const vector<SCodePageInfo>& vectSCodePageInfo = GetCodePageInfo();
	int iCodePageInfoCount = vectSCodePageInfo.size();
	for (int iCodePageInfoIndex = 0; iCodePageInfoIndex < iCodePageInfoCount; iCodePageInfoIndex ++)
	{
		const SCodePageInfo& sCodePageInfo = vectSCodePageInfo[iCodePageInfoIndex];
		if (sCodePageInfo.iValue == iCodePage)
		{
			return sCodePageInfo;
		}
	}
	return vectSCodePageInfo[0];
}

const SCodePageInfo& GetDefaultCodePage()
{
	const vector<SCodePageInfo>& vectSCodePageInfo = GetCodePageInfo();
	
	NSUserDefaults* defs = [NSUserDefaults standardUserDefaults];
	NSArray* languages = [defs objectForKey:@"AppleLanguages"];
	NSString* preferredLang = [languages objectAtIndex:0];
	string strName = [preferredLang UTF8String];
	
	int iCodePageInfoCount = vectSCodePageInfo.size();
	for (int iCodePageInfoIndex = 0; iCodePageInfoIndex < iCodePageInfoCount; iCodePageInfoIndex ++)
	{
		const SCodePageInfo& sCodePageInfo = vectSCodePageInfo[iCodePageInfoIndex];
		if (sCodePageInfo.strName == strName)
		{
			return sCodePageInfo;
		}
	}
	return vectSCodePageInfo[0];
}
void GetFilesInFolder(const string& strFolderName, vector<string>& vectFolderItems)
{
	vectFolderItems.clear();
	DIR* pDir = opendir (strFolderName.c_str());
	if (pDir == NULL) 
	{
		return;
	}
	
	dirent* pDirent = NULL;
	while ((pDirent = readdir (pDir)) != NULL)
	{
		if (strcmp (pDirent->d_name, ".") == 0 || strcmp (pDirent->d_name, "..") == 0)
		{
			continue;
		}
		if (!(pDirent->d_type == DT_DIR))
		{
			vectFolderItems.push_back(pDirent->d_name);
		}
	}
	closedir(pDir);
	return;
}

#include <errno.h>
extern "C" 
int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	// need check
	if ((alignment & (alignment -1)))
	{		
		return EINVAL;
	}
	size = size + (alignment - 1) / alignment * alignment;
	*memptr = malloc(size);
	if (*memptr == NULL)
	{
		return ENOMEM;
	}
	return 0;
}



SThumbBitmapHeader& GetThunbBitmapHeader()
{
	static SThumbBitmapHeader sThumbBitmapHeader;
	static bool bInited = false;
	if (!bInited)
	{
		memset(&sThumbBitmapHeader, 0, sizeof(sThumbBitmapHeader));
		int colorTablesize = 0;
		int lineByte = (THUMB_WIDTH * 24 /8+3) / 4*4;
		
		BITMAPFILEHEADER& fileHead = sThumbBitmapHeader.m_bitmapFileHeader;
		fileHead.bfType= 0x4d42;
		fileHead.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER) 
		+ colorTablesize + lineByte *THUMB_HEIGHT;
		fileHead.bfReserved1 = 0;
		fileHead.bfReserved2 = 0;
		fileHead.bfOffBits = 54 +colorTablesize;
		BITMAPINFOHEADER& head = sThumbBitmapHeader.m_bitmapInfoHeader;
		head.biBitCount = 24;
		head.biClrImportant = 0;
		head.biClrUsed = 0;
		head.biCompression = 0;
		head.biHeight = THUMB_HEIGHT;
		head.biPlanes =1;
		head.biSize = 40;
		head.biSizeImage = lineByte * THUMB_HEIGHT;
		head.biWidth = THUMB_WIDTH;
		head.biXPelsPerMeter = 0;
		head.biYPelsPerMeter = 0;
	}
	return sThumbBitmapHeader;
}
extern bool saveBmp(const char* bmpName,unsigned char *imgBuf,int width,int height,int biBitCount);
pthread_mutex_t g_mutexAvCocecOpen;
void InitGetMediaInfo()
{
	pthread_mutex_init(&g_mutexAvCocecOpen, NULL);
}

void UnInitGetMediaInfo()
{
	pthread_mutex_destroy(&g_mutexAvCocecOpen);
}

void GetMediaInfo(const string& strFileName, SMediaInfoLocal& sMediaInfoStream)
{
	AVFormatContext* pAvFormatContext = avformat_alloc_context();
	if (pAvFormatContext == NULL)
	{
		return;
	}
	int video_stream = -1;
	int audio_stream = -1;
	
	AVFormatParameters avFormatParameters = {0};	
	avFormatParameters.prealloced_context = 1;
	avFormatParameters.width = 640;
	avFormatParameters.height= 480;
	avFormatParameters.time_base.num = 1;
	avFormatParameters.time_base.den = 25;
	avFormatParameters.pix_fmt = PIX_FMT_NONE;
	
	char szFileName[520] = {0};
	
#ifdef _WIN32
	wchar_t wszFileName[260] = {0};
	MultiByteToWideChar(CP_UTF8, 0, strFileName.c_str(), strlen(strFileName.c_str()), wszFileName, sizeof(wszFileName)/sizeof(wchar_t));
	WideCharToMultiByte(CP_ACP, 0, wszFileName, -1, szFileName, sizeof(szFileName), NULL, NULL);
#else
	strcpy(szFileName, strFileName.c_str());
#endif
	AVInputFormat* iformat = NULL;
	int iRet = av_open_input_file(&pAvFormatContext, szFileName, 
								  iformat, 0, &avFormatParameters);
	if (iRet < 0) 
	{
		av_free(pAvFormatContext);
		return;
	}
	
	pAvFormatContext->flags |= AVFMT_FLAG_GENPTS;
	
	iRet = av_find_stream_info(pAvFormatContext);
	if (iRet < 0)
	{
		av_close_input_file(pAvFormatContext);
		return;
	}
	if(pAvFormatContext->pb)
	{
		pAvFormatContext->pb->eof_reached= 0; //FIXME hack, ffplay maybe should not use url_feof() to test for the end
	}
	
	for (int iStreamIndex = 0; iStreamIndex < pAvFormatContext->nb_streams; iStreamIndex++)
	{
		pAvFormatContext->streams[iStreamIndex]->discard = AVDISCARD_ALL;
		// try find suitable stream
		AVCodecContext *pAvCodecContext = pAvFormatContext->streams[iStreamIndex]->codec;
		switch(pAvCodecContext->codec_type)
		{
			case CODEC_TYPE_AUDIO:
				if (audio_stream == -1)
				{
					audio_stream = iStreamIndex;
				}
				break;
			case CODEC_TYPE_VIDEO:
				if (video_stream == -1)
				{
					video_stream = iStreamIndex;
				}
				break;
			default:
				break;
		}
		
	}
	
	// try open stream
	if ((audio_stream < 0) || (video_stream < 0)) 
	{
		av_close_input_file(pAvFormatContext);
		return;
	}
	
	sMediaInfoStream.m_bHasVideo = (video_stream != -1); 
	sMediaInfoStream.m_bHasAudio = (audio_stream != -1);
	sMediaInfoStream.m_iDuration =  pAvFormatContext->duration / AV_TIME_BASE;
	if (video_stream != -1)
	{
		AVStream* pAvStream = pAvFormatContext->streams[video_stream];
		sMediaInfoStream.m_iWidth = pAvStream->codec->width;
		sMediaInfoStream.m_iHeight = pAvStream->codec->height;
		sMediaInfoStream.m_iVideoCodec = pAvStream->codec->codec_id;
		sMediaInfoStream.m_iVideoBitrate = pAvStream->codec->bit_rate;
		sMediaInfoStream.m_iFramerate = pAvStream->codec->time_base.den / pAvStream->codec->time_base.num;
	}
	if (audio_stream != -1)
	{
		AVStream* pAvStream = pAvFormatContext->streams[audio_stream];
		sMediaInfoStream.m_iSamplerate = pAvStream->codec->sample_rate;
		sMediaInfoStream.m_iChannels = pAvStream->codec->channels;
		sMediaInfoStream.m_iAudioBitrate = pAvStream->codec->bit_rate;
		sMediaInfoStream.m_iAudioCodec = pAvStream->codec->codec_id;
	}
	
	av_close_input_file(pAvFormatContext);
	return;
}

void GetFileSize(__int64_t iFileSize, char* pszSize, int iLen)
{
	const static __int64_t iGB = 1024 * 1024 * 1024;
	const static __int64_t iMB = 1024 * 1024;
	const static __int64_t iKB = 1024;
	
	if (pszSize == NULL)
	{
		return;
	}
	*pszSize = '\0';
	
	if (iLen < 10)
	{
		return;
	}
	
	const char* pszUint = NULL;
	double dValue = 0.0;
	if (iFileSize >= iGB)
	{
		pszUint = "GB";
		dValue = iFileSize * 1.0 / iGB;
	}
	else if (iFileSize >= iMB)
	{
		pszUint = "MB";
		dValue = iFileSize * 1.0 / iMB;
	}
	else if (iFileSize >= iKB)
	{
		pszUint = "KB";
		dValue = iFileSize * 1.0 / iKB;
	}
	else
	{
		pszUint = "B";
		dValue = iFileSize;
	}
	sprintf(pszSize, "%0.2f%s", dValue, pszUint);
}


void GetFileDate(__int64_t iFileData, char* pszDate, int iLen)
{
	time_t timerLocal = iFileData;
	tm* tm1 = localtime(&timerLocal);
	snprintf(pszDate, iLen, "%04d-%02d-%02d", 
								tm1->tm_year + 1900, 
								tm1->tm_mon + 1,
								tm1->tm_mday);
}

SApplicationSetting g_applicationSetting;

void InitApplicationSetting()
{
	g_applicationSetting.m_iVolume = 100;
}
/*
example of appconf.xml:
<?xml version="1.0" encoding="UTF-8"?>
<AppConf version="1">
	<volume value="100" />
	<SmbServer>
		<SmbServerItem 
				Name="Smb1" 
				IP="192.168.0.100" 
				User="Administrator"
				Password=""
				Workgroup="" />
		<SmbServerItem 
				Name="Smb2" 
				IP="192.168.0.101" 
				User="Administrator"
				Password=""
				Workgroup="" />
		 <SmbServerItemAutoSearch 
				 Name="Smb2" 
				 IP="192.168.0.101" 
				 User="Administrator"
				 Password=""
				 Workgroup="" />
		 <SmbServerItemAutoSearch 
				 Name="Smb2" 
				 IP="192.168.0.101" 
				 User="Administrator"
				 Password=""
				 Workgroup="" />
	</SmbServer>
	<StreamServer>
		<StreamServerItem IP="192.168.0.101" />
		<StreamServerItem IP="192.168.0.102" />
	</StreamServer
</AppConf>
 
*/


bool operator < (const SFolderItem& sFolderItem1, const SFolderItem& sFolderItem2)
{
	if (sFolderItem1.m_bFolder != sFolderItem2.m_bFolder)
	{
		return sFolderItem1.m_bFolder;
	}
	bool bRet = (strcasecmp(sFolderItem1.m_strName.c_str(), sFolderItem2.m_strName.c_str()) < 0);
	AddLog("bRet = %d, 1=(%s), 2= (%s)", bRet, sFolderItem1.m_strName.c_str(), sFolderItem2.m_strName.c_str());
	return bRet;
}

bool IsValidIpAddress(const string& strIpAddress)
{
	vector<string> vectString;
	SplitString(strIpAddress.c_str(), ".", vectString);
	if (vectString.size() != 4)
	{
		return false;
	}
	for (int i = 0; i < 4; i++)
	{
		const string& str = vectString[i];
		int iLen = str.size();
		if ((iLen == 0) || (iLen > 3))
		{
			return false;
		}
		for (int iIndex = 0; iIndex < iLen; iIndex ++)
		{
			char c = str[iIndex];
			if (!isdigit(c))
			{
				return false;
			}
		}
		if (atoi(str.c_str()) > 255)
		{
			return false;
		}
	}
	return true;
}

typedef map<int, string> MapCodecID2String;
string GetCodecIDDesc(int iCodecID)
{
	static MapCodecID2String mapCodecID2String;
	if (mapCodecID2String.size() == 0)
	{
		mapCodecID2String[CODEC_ID_MPEG1VIDEO] = "CODEC_ID_MPEG1VIDEO";
		mapCodecID2String[CODEC_ID_MPEG2VIDEO] = "CODEC_ID_MPEG2VIDEO";
		mapCodecID2String[CODEC_ID_MPEG2VIDEO_XVMC] = "CODEC_ID_MPEG2VIDEO_XVMC";
		mapCodecID2String[CODEC_ID_H261] = "CODEC_ID_H261";
		mapCodecID2String[CODEC_ID_H263] = "CODEC_ID_H263";
		mapCodecID2String[CODEC_ID_RV10] = "CODEC_ID_RV10";
		mapCodecID2String[CODEC_ID_RV20] = "CODEC_ID_RV20";
		mapCodecID2String[CODEC_ID_MJPEG] = "CODEC_ID_MJPEG";
		mapCodecID2String[CODEC_ID_MJPEGB] = "CODEC_ID_MJPEGB";
		mapCodecID2String[CODEC_ID_LJPEG] = "CODEC_ID_LJPEG";
		mapCodecID2String[CODEC_ID_SP5X] = "CODEC_ID_SP5X";
		mapCodecID2String[CODEC_ID_JPEGLS] = "CODEC_ID_JPEGLS";
		mapCodecID2String[CODEC_ID_MPEG4] = "CODEC_ID_MPEG4";
		mapCodecID2String[CODEC_ID_RAWVIDEO] = "CODEC_ID_RAWVIDEO";
		mapCodecID2String[CODEC_ID_MSMPEG4V1] = "CODEC_ID_MSMPEG4V1";
		mapCodecID2String[CODEC_ID_MSMPEG4V2] = "CODEC_ID_MSMPEG4V2";
		mapCodecID2String[CODEC_ID_MSMPEG4V3] = "CODEC_ID_MSMPEG4V3";
		mapCodecID2String[CODEC_ID_WMV1] = "CODEC_ID_WMV1";
		mapCodecID2String[CODEC_ID_WMV2] = "CODEC_ID_WMV2";
		mapCodecID2String[CODEC_ID_H263P] = "CODEC_ID_H263P";
		mapCodecID2String[CODEC_ID_H263I] = "CODEC_ID_H263I";
		mapCodecID2String[CODEC_ID_FLV1] = "CODEC_ID_FLV1";
		mapCodecID2String[CODEC_ID_SVQ1] = "CODEC_ID_SVQ1";
		mapCodecID2String[CODEC_ID_SVQ3] = "CODEC_ID_SVQ3";
		mapCodecID2String[CODEC_ID_DVVIDEO] = "CODEC_ID_DVVIDEO";
		mapCodecID2String[CODEC_ID_HUFFYUV] = "CODEC_ID_HUFFYUV";
		mapCodecID2String[CODEC_ID_CYUV] = "CODEC_ID_CYUV";
		mapCodecID2String[CODEC_ID_H264] = "CODEC_ID_H264";
		mapCodecID2String[CODEC_ID_INDEO3] = "CODEC_ID_INDEO3";
		mapCodecID2String[CODEC_ID_VP3] = "CODEC_ID_VP3";
		mapCodecID2String[CODEC_ID_THEORA] = "CODEC_ID_THEORA";
		mapCodecID2String[CODEC_ID_ASV1] = "CODEC_ID_ASV1";
		mapCodecID2String[CODEC_ID_ASV2] = "CODEC_ID_ASV2";
		mapCodecID2String[CODEC_ID_FFV1] = "CODEC_ID_FFV1";
		mapCodecID2String[CODEC_ID_4XM] = "CODEC_ID_4XM";
		mapCodecID2String[CODEC_ID_VCR1] = "CODEC_ID_VCR1";
		mapCodecID2String[CODEC_ID_CLJR] = "CODEC_ID_CLJR";
		mapCodecID2String[CODEC_ID_MDEC] = "CODEC_ID_MDEC";
		mapCodecID2String[CODEC_ID_ROQ] = "CODEC_ID_ROQ";
		mapCodecID2String[CODEC_ID_INTERPLAY_VIDEO] = "CODEC_ID_INTERPLAY_VIDEO";
		mapCodecID2String[CODEC_ID_XAN_WC3] = "CODEC_ID_XAN_WC3";
		mapCodecID2String[CODEC_ID_XAN_WC4] = "CODEC_ID_XAN_WC4";
		mapCodecID2String[CODEC_ID_RPZA] = "CODEC_ID_RPZA";
		mapCodecID2String[CODEC_ID_CINEPAK] = "CODEC_ID_CINEPAK";
		mapCodecID2String[CODEC_ID_WS_VQA] = "CODEC_ID_WS_VQA";
		mapCodecID2String[CODEC_ID_MSRLE] = "CODEC_ID_MSRLE";
		mapCodecID2String[CODEC_ID_MSVIDEO1] = "CODEC_ID_MSVIDEO1";
		mapCodecID2String[CODEC_ID_IDCIN] = "CODEC_ID_IDCIN";
		mapCodecID2String[CODEC_ID_8BPS] = "CODEC_ID_8BPS";
		mapCodecID2String[CODEC_ID_SMC] = "CODEC_ID_SMC";
		mapCodecID2String[CODEC_ID_FLIC] = "CODEC_ID_FLIC";
		mapCodecID2String[CODEC_ID_TRUEMOTION1] = "CODEC_ID_TRUEMOTION1";
		mapCodecID2String[CODEC_ID_VMDVIDEO] = "CODEC_ID_VMDVIDEO";
		mapCodecID2String[CODEC_ID_MSZH] = "CODEC_ID_MSZH";
		mapCodecID2String[CODEC_ID_ZLIB] = "CODEC_ID_ZLIB";
		mapCodecID2String[CODEC_ID_QTRLE] = "CODEC_ID_QTRLE";
		mapCodecID2String[CODEC_ID_SNOW] = "CODEC_ID_SNOW";
		mapCodecID2String[CODEC_ID_TSCC] = "CODEC_ID_TSCC";
		mapCodecID2String[CODEC_ID_ULTI] = "CODEC_ID_ULTI";
		mapCodecID2String[CODEC_ID_QDRAW] = "CODEC_ID_QDRAW";
		mapCodecID2String[CODEC_ID_VIXL] = "CODEC_ID_VIXL";
		mapCodecID2String[CODEC_ID_QPEG] = "CODEC_ID_QPEG";
		mapCodecID2String[CODEC_ID_XVID] = "CODEC_ID_XVID";
		mapCodecID2String[CODEC_ID_PNG] = "CODEC_ID_PNG";
		mapCodecID2String[CODEC_ID_PPM] = "CODEC_ID_PPM";
		mapCodecID2String[CODEC_ID_PBM] = "CODEC_ID_PBM";
		mapCodecID2String[CODEC_ID_PGM] = "CODEC_ID_PGM";
		mapCodecID2String[CODEC_ID_PGMYUV] = "CODEC_ID_PGMYUV";
		mapCodecID2String[CODEC_ID_PAM] = "CODEC_ID_PAM";
		mapCodecID2String[CODEC_ID_FFVHUFF] = "CODEC_ID_FFVHUFF";
		mapCodecID2String[CODEC_ID_RV30] = "CODEC_ID_RV30";
		mapCodecID2String[CODEC_ID_RV40] = "CODEC_ID_RV40";
		mapCodecID2String[CODEC_ID_VC1] = "CODEC_ID_VC1";
		mapCodecID2String[CODEC_ID_WMV3] = "CODEC_ID_WMV3";
		mapCodecID2String[CODEC_ID_LOCO] = "CODEC_ID_LOCO";
		mapCodecID2String[CODEC_ID_WNV1] = "CODEC_ID_WNV1";
		mapCodecID2String[CODEC_ID_AASC] = "CODEC_ID_AASC";
		mapCodecID2String[CODEC_ID_INDEO2] = "CODEC_ID_INDEO2";
		mapCodecID2String[CODEC_ID_FRAPS] = "CODEC_ID_FRAPS";
		mapCodecID2String[CODEC_ID_TRUEMOTION2] = "CODEC_ID_TRUEMOTION2";
		mapCodecID2String[CODEC_ID_BMP] = "CODEC_ID_BMP";
		mapCodecID2String[CODEC_ID_CSCD] = "CODEC_ID_CSCD";
		mapCodecID2String[CODEC_ID_MMVIDEO] = "CODEC_ID_MMVIDEO";
		mapCodecID2String[CODEC_ID_ZMBV] = "CODEC_ID_ZMBV";
		mapCodecID2String[CODEC_ID_AVS] = "CODEC_ID_AVS";
		mapCodecID2String[CODEC_ID_SMACKVIDEO] = "CODEC_ID_SMACKVIDEO";
		mapCodecID2String[CODEC_ID_NUV] = "CODEC_ID_NUV";
		mapCodecID2String[CODEC_ID_KMVC] = "CODEC_ID_KMVC";
		mapCodecID2String[CODEC_ID_FLASHSV] = "CODEC_ID_FLASHSV";
		mapCodecID2String[CODEC_ID_CAVS] = "CODEC_ID_CAVS";
		mapCodecID2String[CODEC_ID_JPEG2000] = "CODEC_ID_JPEG2000";
		mapCodecID2String[CODEC_ID_VMNC] = "CODEC_ID_VMNC";
		mapCodecID2String[CODEC_ID_VP5] = "CODEC_ID_VP5";
		mapCodecID2String[CODEC_ID_VP6] = "CODEC_ID_VP6";
		mapCodecID2String[CODEC_ID_VP6F] = "CODEC_ID_VP6F";
		mapCodecID2String[CODEC_ID_TARGA] = "CODEC_ID_TARGA";
		mapCodecID2String[CODEC_ID_DSICINVIDEO] = "CODEC_ID_DSICINVIDEO";
		mapCodecID2String[CODEC_ID_TIERTEXSEQVIDEO] = "CODEC_ID_TIERTEXSEQVIDEO";
		mapCodecID2String[CODEC_ID_TIFF] = "CODEC_ID_TIFF";
		mapCodecID2String[CODEC_ID_GIF] = "CODEC_ID_GIF";
		mapCodecID2String[CODEC_ID_FFH264] = "CODEC_ID_FFH264";
		mapCodecID2String[CODEC_ID_DXA] = "CODEC_ID_DXA";
		mapCodecID2String[CODEC_ID_DNXHD] = "CODEC_ID_DNXHD";
		mapCodecID2String[CODEC_ID_THP] = "CODEC_ID_THP";
		mapCodecID2String[CODEC_ID_SGI] = "CODEC_ID_SGI";
		mapCodecID2String[CODEC_ID_C93] = "CODEC_ID_C93";
		mapCodecID2String[CODEC_ID_BETHSOFTVID] = "CODEC_ID_BETHSOFTVID";
		mapCodecID2String[CODEC_ID_PTX] = "CODEC_ID_PTX";
		mapCodecID2String[CODEC_ID_TXD] = "CODEC_ID_TXD";
		mapCodecID2String[CODEC_ID_VP6A] = "CODEC_ID_VP6A";
		mapCodecID2String[CODEC_ID_AMV] = "CODEC_ID_AMV";
		mapCodecID2String[CODEC_ID_VB] = "CODEC_ID_VB";
		mapCodecID2String[CODEC_ID_PCX] = "CODEC_ID_PCX";
		mapCodecID2String[CODEC_ID_SUNRAST] = "CODEC_ID_SUNRAST";
		mapCodecID2String[CODEC_ID_INDEO4] = "CODEC_ID_INDEO4";
		mapCodecID2String[CODEC_ID_INDEO5] = "CODEC_ID_INDEO5";
		mapCodecID2String[CODEC_ID_MIMIC] = "CODEC_ID_MIMIC";
		mapCodecID2String[CODEC_ID_RL2] = "CODEC_ID_RL2";
		mapCodecID2String[CODEC_ID_8SVX_EXP] = "CODEC_ID_8SVX_EXP";
		mapCodecID2String[CODEC_ID_8SVX_FIB] = "CODEC_ID_8SVX_FIB";
		mapCodecID2String[CODEC_ID_ESCAPE124] = "CODEC_ID_ESCAPE124";
		mapCodecID2String[CODEC_ID_DIRAC] = "CODEC_ID_DIRAC";
		mapCodecID2String[CODEC_ID_BFI] = "CODEC_ID_BFI";
		mapCodecID2String[CODEC_ID_CMV] = "CODEC_ID_CMV";
		mapCodecID2String[CODEC_ID_MOTIONPIXELS] = "CODEC_ID_MOTIONPIXELS";
		mapCodecID2String[CODEC_ID_TGV] = "CODEC_ID_TGV";
		mapCodecID2String[CODEC_ID_TGQ] = "CODEC_ID_TGQ";
		mapCodecID2String[CODEC_ID_TQI] = "CODEC_ID_TQI";
		mapCodecID2String[CODEC_ID_AURA] = "CODEC_ID_AURA";
		mapCodecID2String[CODEC_ID_AURA2] = "CODEC_ID_AURA2";
		mapCodecID2String[CODEC_ID_V210X] = "CODEC_ID_V210X";
		mapCodecID2String[CODEC_ID_TMV] = "CODEC_ID_TMV";
		mapCodecID2String[CODEC_ID_V210] = "CODEC_ID_V210";
		mapCodecID2String[CODEC_ID_DPX] = "CODEC_ID_DPX";
		mapCodecID2String[CODEC_ID_MAD] = "CODEC_ID_MAD";
		mapCodecID2String[CODEC_ID_FRWU] = "CODEC_ID_FRWU";
		mapCodecID2String[CODEC_ID_FLASHSV2] = "CODEC_ID_FLASHSV2";
		mapCodecID2String[CODEC_ID_CDGRAPHICS] = "CODEC_ID_CDGRAPHICS";
		mapCodecID2String[CODEC_ID_R210] = "CODEC_ID_R210";
		
		
		mapCodecID2String[CODEC_ID_PCM_S16LE] = "CODEC_ID_PCM_S16LE";
		mapCodecID2String[CODEC_ID_PCM_S16BE] = "CODEC_ID_PCM_S16BE";
		mapCodecID2String[CODEC_ID_PCM_U16LE] = "CODEC_ID_PCM_U16LE";
		mapCodecID2String[CODEC_ID_PCM_U16BE] = "CODEC_ID_PCM_U16BE";
		mapCodecID2String[CODEC_ID_PCM_S8] = "CODEC_ID_PCM_S8";
		mapCodecID2String[CODEC_ID_PCM_U8] = "CODEC_ID_PCM_U8";
		mapCodecID2String[CODEC_ID_PCM_MULAW] = "CODEC_ID_PCM_MULAW";
		mapCodecID2String[CODEC_ID_PCM_ALAW] = "CODEC_ID_PCM_ALAW";
		mapCodecID2String[CODEC_ID_PCM_S32LE] = "CODEC_ID_PCM_S32LE";
		mapCodecID2String[CODEC_ID_PCM_S32BE] = "CODEC_ID_PCM_S32BE";
		mapCodecID2String[CODEC_ID_PCM_U32LE] = "CODEC_ID_PCM_U32LE";
		mapCodecID2String[CODEC_ID_PCM_U32BE] = "CODEC_ID_PCM_U32BE";
		mapCodecID2String[CODEC_ID_PCM_S24LE] = "CODEC_ID_PCM_S24LE";
		mapCodecID2String[CODEC_ID_PCM_S24BE] = "CODEC_ID_PCM_S24BE";
		mapCodecID2String[CODEC_ID_PCM_U24LE] = "CODEC_ID_PCM_U24LE";
		mapCodecID2String[CODEC_ID_PCM_U24BE] = "CODEC_ID_PCM_U24BE";
		mapCodecID2String[CODEC_ID_PCM_S24DAUD] = "CODEC_ID_PCM_S24DAUD";
		mapCodecID2String[CODEC_ID_PCM_ZORK] = "CODEC_ID_PCM_ZORK";
		mapCodecID2String[CODEC_ID_PCM_S16LE_PLANAR] = "CODEC_ID_PCM_S16LE_PLANAR";
		mapCodecID2String[CODEC_ID_PCM_DVD] = "CODEC_ID_PCM_DVD";
		mapCodecID2String[CODEC_ID_PCM_F32BE] = "CODEC_ID_PCM_F32BE";
		mapCodecID2String[CODEC_ID_PCM_F32LE] = "CODEC_ID_PCM_F32LE";
		mapCodecID2String[CODEC_ID_PCM_F64BE] = "CODEC_ID_PCM_F64BE";
		mapCodecID2String[CODEC_ID_PCM_F64LE] = "CODEC_ID_PCM_F64LE";
		mapCodecID2String[CODEC_ID_PCM_BLURAY] = "CODEC_ID_PCM_BLURAY";
		
		
		mapCodecID2String[CODEC_ID_ADPCM_IMA_QT] = "CODEC_ID_ADPCM_IMA_QT";
		mapCodecID2String[CODEC_ID_ADPCM_IMA_WAV] = "CODEC_ID_ADPCM_IMA_WAV";
		mapCodecID2String[CODEC_ID_ADPCM_IMA_DK3] = "CODEC_ID_ADPCM_IMA_DK3";
		mapCodecID2String[CODEC_ID_ADPCM_IMA_DK4] = "CODEC_ID_ADPCM_IMA_DK4";
		mapCodecID2String[CODEC_ID_ADPCM_IMA_WS] = "CODEC_ID_ADPCM_IMA_WS";
		mapCodecID2String[CODEC_ID_ADPCM_IMA_SMJPEG] = "CODEC_ID_ADPCM_IMA_SMJPEG";
		mapCodecID2String[CODEC_ID_ADPCM_MS] = "CODEC_ID_ADPCM_MS";
		mapCodecID2String[CODEC_ID_ADPCM_4XM] = "CODEC_ID_ADPCM_4XM";
		mapCodecID2String[CODEC_ID_ADPCM_XA] = "CODEC_ID_ADPCM_XA";
		mapCodecID2String[CODEC_ID_ADPCM_ADX] = "CODEC_ID_ADPCM_ADX";
		mapCodecID2String[CODEC_ID_ADPCM_EA] = "CODEC_ID_ADPCM_EA";
		mapCodecID2String[CODEC_ID_ADPCM_G726] = "CODEC_ID_ADPCM_G726";
		mapCodecID2String[CODEC_ID_ADPCM_CT] = "CODEC_ID_ADPCM_CT";
		mapCodecID2String[CODEC_ID_ADPCM_SWF] = "CODEC_ID_ADPCM_SWF";
		mapCodecID2String[CODEC_ID_ADPCM_YAMAHA] = "CODEC_ID_ADPCM_YAMAHA";
		mapCodecID2String[CODEC_ID_ADPCM_SBPRO_4] = "CODEC_ID_ADPCM_SBPRO_4";
		mapCodecID2String[CODEC_ID_ADPCM_SBPRO_3] = "CODEC_ID_ADPCM_SBPRO_3";
		mapCodecID2String[CODEC_ID_ADPCM_SBPRO_2] = "CODEC_ID_ADPCM_SBPRO_2";
		mapCodecID2String[CODEC_ID_ADPCM_THP] = "CODEC_ID_ADPCM_THP";
		mapCodecID2String[CODEC_ID_ADPCM_IMA_AMV] = "CODEC_ID_ADPCM_IMA_AMV";
		mapCodecID2String[CODEC_ID_ADPCM_EA_R1] = "CODEC_ID_ADPCM_EA_R1";
		mapCodecID2String[CODEC_ID_ADPCM_EA_R3] = "CODEC_ID_ADPCM_EA_R3";
		mapCodecID2String[CODEC_ID_ADPCM_EA_R2] = "CODEC_ID_ADPCM_EA_R2";
		mapCodecID2String[CODEC_ID_ADPCM_IMA_EA_SEAD] = "CODEC_ID_ADPCM_IMA_EA_SEAD";
		mapCodecID2String[CODEC_ID_ADPCM_IMA_EA_EACS] = "CODEC_ID_ADPCM_IMA_EA_EACS";
		mapCodecID2String[CODEC_ID_ADPCM_EA_XAS] = "CODEC_ID_ADPCM_EA_XAS";
		mapCodecID2String[CODEC_ID_ADPCM_EA_MAXIS_XA] = "CODEC_ID_ADPCM_EA_MAXIS_XA";
		mapCodecID2String[CODEC_ID_ADPCM_IMA_ISS] = "CODEC_ID_ADPCM_IMA_ISS";
		
		
		mapCodecID2String[CODEC_ID_AMR_NB] = "CODEC_ID_AMR_NB";
		mapCodecID2String[CODEC_ID_AMR_WB] = "CODEC_ID_AMR_WB";
		
		
		mapCodecID2String[CODEC_ID_RA_144] = "CODEC_ID_RA_144";
		mapCodecID2String[CODEC_ID_RA_288] = "CODEC_ID_RA_288";
		
		
		mapCodecID2String[CODEC_ID_ROQ_DPCM] = "CODEC_ID_ROQ_DPCM";
		mapCodecID2String[CODEC_ID_INTERPLAY_DPCM] = "CODEC_ID_INTERPLAY_DPCM";
		mapCodecID2String[CODEC_ID_XAN_DPCM] = "CODEC_ID_XAN_DPCM";
		mapCodecID2String[CODEC_ID_SOL_DPCM] = "CODEC_ID_SOL_DPCM";
		
		
		mapCodecID2String[CODEC_ID_MP2] = "CODEC_ID_MP2";
		mapCodecID2String[CODEC_ID_MP3] = "CODEC_ID_MP3";
		mapCodecID2String[CODEC_ID_AAC] = "CODEC_ID_AAC";
		mapCodecID2String[CODEC_ID_AC3] = "CODEC_ID_AC3";
		mapCodecID2String[CODEC_ID_DTS] = "CODEC_ID_DTS";
		mapCodecID2String[CODEC_ID_VORBIS] = "CODEC_ID_VORBIS";
		mapCodecID2String[CODEC_ID_DVAUDIO] = "CODEC_ID_DVAUDIO";
		mapCodecID2String[CODEC_ID_WMAV1] = "CODEC_ID_WMAV1";
		mapCodecID2String[CODEC_ID_WMAV2] = "CODEC_ID_WMAV2";
		mapCodecID2String[CODEC_ID_MACE3] = "CODEC_ID_MACE3";
		mapCodecID2String[CODEC_ID_MACE6] = "CODEC_ID_MACE6";
		mapCodecID2String[CODEC_ID_VMDAUDIO] = "CODEC_ID_VMDAUDIO";
		mapCodecID2String[CODEC_ID_SONIC] = "CODEC_ID_SONIC";
		mapCodecID2String[CODEC_ID_SONIC_LS] = "CODEC_ID_SONIC_LS";
		mapCodecID2String[CODEC_ID_FLAC] = "CODEC_ID_FLAC";
		mapCodecID2String[CODEC_ID_MP3ADU] = "CODEC_ID_MP3ADU";
		mapCodecID2String[CODEC_ID_MP3ON4] = "CODEC_ID_MP3ON4";
		mapCodecID2String[CODEC_ID_SHORTEN] = "CODEC_ID_SHORTEN";
		mapCodecID2String[CODEC_ID_ALAC] = "CODEC_ID_ALAC";
		mapCodecID2String[CODEC_ID_WESTWOOD_SND1] = "CODEC_ID_WESTWOOD_SND1";
		mapCodecID2String[CODEC_ID_GSM] = "CODEC_ID_GSM"; ///< as in Berlin toast format
		mapCodecID2String[CODEC_ID_QDM2] = "CODEC_ID_QDM2";
		mapCodecID2String[CODEC_ID_COOK] = "CODEC_ID_COOK";
		mapCodecID2String[CODEC_ID_TRUESPEECH] = "CODEC_ID_TRUESPEECH";
		mapCodecID2String[CODEC_ID_TTA] = "CODEC_ID_TTA";
		mapCodecID2String[CODEC_ID_SMACKAUDIO] = "CODEC_ID_SMACKAUDIO";
		mapCodecID2String[CODEC_ID_QCELP] = "CODEC_ID_QCELP";
		mapCodecID2String[CODEC_ID_WAVPACK] = "CODEC_ID_WAVPACK";
		mapCodecID2String[CODEC_ID_DSICINAUDIO] = "CODEC_ID_DSICINAUDIO";
		mapCodecID2String[CODEC_ID_IMC] = "CODEC_ID_IMC";
		mapCodecID2String[CODEC_ID_MUSEPACK7] = "CODEC_ID_MUSEPACK7";
		mapCodecID2String[CODEC_ID_MLP] = "CODEC_ID_MLP";
		mapCodecID2String[CODEC_ID_GSM_MS] = "CODEC_ID_GSM_MS";
		mapCodecID2String[CODEC_ID_ATRAC3] = "CODEC_ID_ATRAC3";
		mapCodecID2String[CODEC_ID_VOXWARE] = "CODEC_ID_VOXWARE";
		mapCodecID2String[CODEC_ID_APE] = "CODEC_ID_APE";
		mapCodecID2String[CODEC_ID_NELLYMOSER] = "CODEC_ID_NELLYMOSER";
		mapCodecID2String[CODEC_ID_MUSEPACK8] = "CODEC_ID_MUSEPACK8";
		mapCodecID2String[CODEC_ID_SPEEX] = "CODEC_ID_SPEEX";
		mapCodecID2String[CODEC_ID_WMAVOICE] = "CODEC_ID_WMAVOICE";
		mapCodecID2String[CODEC_ID_WMAPRO] = "CODEC_ID_WMAPRO";
		mapCodecID2String[CODEC_ID_WMALOSSLESS] = "CODEC_ID_WMALOSSLESS";
		mapCodecID2String[CODEC_ID_ATRAC3P] = "CODEC_ID_ATRAC3P";
		mapCodecID2String[CODEC_ID_EAC3] = "CODEC_ID_EAC3";
		mapCodecID2String[CODEC_ID_SIPR] = "CODEC_ID_SIPR";
		mapCodecID2String[CODEC_ID_MP1] = "CODEC_ID_MP1";
		mapCodecID2String[CODEC_ID_TWINVQ] = "CODEC_ID_TWINVQ";
		mapCodecID2String[CODEC_ID_TRUEHD] = "CODEC_ID_TRUEHD";
		mapCodecID2String[CODEC_ID_MP4ALS] = "CODEC_ID_MP4ALS";
		mapCodecID2String[CODEC_ID_ATRAC1] = "CODEC_ID_ATRAC1";
		
		
		mapCodecID2String[CODEC_ID_DVD_SUBTITLE] = "CODEC_ID_DVD_SUBTITLE";
		mapCodecID2String[CODEC_ID_DVB_SUBTITLE] = "CODEC_ID_DVB_SUBTITLE";
		mapCodecID2String[CODEC_ID_TEXT] = "CODEC_ID_TEXT";
		mapCodecID2String[CODEC_ID_XSUB] = "CODEC_ID_XSUB";
		mapCodecID2String[CODEC_ID_SSA] = "CODEC_ID_SSA";
		mapCodecID2String[CODEC_ID_MOV_TEXT] = "CODEC_ID_MOV_TEXT";
		mapCodecID2String[CODEC_ID_HDMV_PGS_SUBTITLE] = "CODEC_ID_HDMV_PGS_SUBTITLE";
		mapCodecID2String[CODEC_ID_DVB_TELETEXT] = "CODEC_ID_DVB_TELETEXT";
	}
	
	MapCodecID2String::const_iterator it = mapCodecID2String.find(iCodecID);
	if (mapCodecID2String.end() == it)
	{
		return "unkown";
	}
	return it->second;
}
