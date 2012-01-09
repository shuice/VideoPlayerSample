#include "TextFileReader.h"

static const unsigned char g_strEndOfLineAnsiWindows[] = {'\r', '\n'};
static const unsigned char g_strEndOfLineAnsiUnix[] = {'\n'};
static const unsigned char g_strEndOfLineAnsiMac[] = {'\r'};

static const unsigned char g_strEndOfLineUnicodeWindows[] = {'\r', '\0', '\n', '\0'};
static const unsigned char g_strEndOfLineUnicodeUnix[] = {'\n', '\0'};
static const unsigned char g_strEndOfLineUnicodeMac[] = {'\r', '\0'};

static const unsigned char g_strEndOfLineUnicodeBigEndianWindows[] = {'\0', '\r', '\0', '\n'};
static const unsigned char g_strEndOfLineUnicodeBigEndianUnix[] = {'\0', '\n'};
static const unsigned char g_strEndOfLineUnicodeBigEndianMac[] = {'\0', '\r'};

static const unsigned char g_strEndOfLineUTF8Windows[] = {'\r', '\n'};
static const unsigned char g_strEndOfLineUTF8Unix[] = {'\n'};
static const unsigned char g_strEndOfLineUTF8Mac[] = {'\r'};

CTextFileReader::CTextFileReader(void)
{    
    m_bLoaded = false;
    memset(&m_sFileFormatType, 0, sizeof(SFileFormatType));
    m_iLastSeekPos = 0;
    m_iFileLength = 0;
    m_pNextLineFlag = NULL;
    m_iNextLineFlagLen = 0;    
}

CTextFileReader::~CTextFileReader(void)
{
    Close();
}

bool CTextFileReader::LoadFile(const string& strFileName)
{
    if (m_bLoaded)
    {
        return false;
    }
	
	m_pFile = fopen(strFileName.c_str(), "rb");
	if (m_pFile == NULL)
	{
		return false;
	}
	fseek(m_pFile, 0, SEEK_END);
	m_iFileLength = ftell(m_pFile);
    fseek(m_pFile, 0, SEEK_SET);
	
    m_sFileFormatType = GuessFileFormatType(strFileName);
    if (m_sFileFormatType.eTextEncode == eTextEncodeUnicode)
    {   
        if (m_sFileFormatType.eEndOfLine == eEndOfLineWindows)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineUnicodeWindows[0]);
            m_iNextLineFlagLen = 4;
        }
        else if (m_sFileFormatType.eEndOfLine == eEndOfLineMac)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineUnicodeMac[0]);
            m_iNextLineFlagLen = 2;
        }
        else if (m_sFileFormatType.eEndOfLine == eEndOfLineUnix)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineUnicodeUnix[0]);
            m_iNextLineFlagLen = 2;
        }
        else
        {
            // 不应该运行到这里
        }
    }
    else if (m_sFileFormatType.eTextEncode == eTextEncodeUnicodeBigEndian)
    {   
        if (m_sFileFormatType.eEndOfLine == eEndOfLineWindows)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineUnicodeBigEndianWindows[0]);
            m_iNextLineFlagLen = 4;
        }
        else if (m_sFileFormatType.eEndOfLine == eEndOfLineMac)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineUnicodeBigEndianMac[0]);
            m_iNextLineFlagLen = 2;
        }
        else if (m_sFileFormatType.eEndOfLine == eEndOfLineUnix)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineUnicodeBigEndianUnix[0]);
            m_iNextLineFlagLen = 2;
        }
        else
        {
            // 不应该运行到这里
        }            
    }        
    else if (m_sFileFormatType.eTextEncode == eTextEncodeUTF8)
    {   
        if (m_sFileFormatType.eEndOfLine == eEndOfLineWindows)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineAnsiWindows[0]);
            m_iNextLineFlagLen = 2;
        }
        else if (m_sFileFormatType.eEndOfLine == eEndOfLineMac)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineAnsiMac[0]);
            m_iNextLineFlagLen = 1;
        }
        else if (m_sFileFormatType.eEndOfLine == eEndOfLineUnix)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineAnsiUnix[0]);
            m_iNextLineFlagLen = 1;
        }
        else
        {
            // 不应该运行到这里
        }           
    }
    else
    {   

        if (m_sFileFormatType.eEndOfLine == eEndOfLineWindows)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineAnsiWindows[0]);
            m_iNextLineFlagLen = 2;
        }
        else if (m_sFileFormatType.eEndOfLine == eEndOfLineMac)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineAnsiMac[0]);
            m_iNextLineFlagLen = 1;
        }
        else if (m_sFileFormatType.eEndOfLine == eEndOfLineUnix)
        {
            m_pNextLineFlag = const_cast<unsigned char*>(&g_strEndOfLineAnsiUnix[0]);
            m_iNextLineFlagLen = 1;
        }
        else
        {
            // 不应该运行到这里
        }           
    }
    m_iLastSeekPos = m_sFileFormatType.iBOMFlagCount;
    return true;
}


SFileFormatType CTextFileReader::GuessFileFormatType(const string& strFileName)
{
    SFileFormatType sFileFormatType;
    sFileFormatType.eCodePage = eCodePageEnglish;
    sFileFormatType.eEndOfLine = eEndOfLineWindows;
    sFileFormatType.eTextEncode = eTextEncodeAnsi;
    sFileFormatType.iBOMFlagCount = 0;

	FILE* pFile = fopen(strFileName.c_str(), "rb");
	if (pFile == NULL)
	{
		return sFileFormatType;
	}
	fseek(pFile, 0, SEEK_END);
	const int iLength = ftell(pFile);
	if (iLength < 3)
	{
		fclose(pFile);
		return sFileFormatType;
	}

	fseek(pFile, 0, SEEK_SET);
	unsigned char strFileEncodeTypeFlags[3] = {0}; //最多取3个能判断得出文件格式
	fread(strFileEncodeTypeFlags, 1, 3, pFile);   

	if (strFileEncodeTypeFlags[0] == 0xFF && strFileEncodeTypeFlags[1] == 0xFE)
	{
		sFileFormatType.eTextEncode = eTextEncodeUnicode;
		sFileFormatType.iBOMFlagCount = 2;
		sFileFormatType.eCodePage = eCodePageUnicode;
	}
	else if (strFileEncodeTypeFlags[0] == 0xFE && strFileEncodeTypeFlags[1] == 0xFF)
	{            
		sFileFormatType.eTextEncode  = eTextEncodeUnicodeBigEndian;
		sFileFormatType.iBOMFlagCount = 2;         
		sFileFormatType.eCodePage = eCodePageUnicode;
	}        
	else if (strFileEncodeTypeFlags[0] == 0xEF && strFileEncodeTypeFlags[1] == 0xBB && strFileEncodeTypeFlags[2] == 0xBF)
	{
		sFileFormatType.eTextEncode  = eTextEncodeUTF8;
		sFileFormatType.iBOMFlagCount = 3; 
		sFileFormatType.eCodePage = eCodePageUnicode;
	}
	else
	{
#define GURESS_FILE_FORMAT_READ_LENGTH 1000
		unsigned char sReaded[GURESS_FILE_FORMAT_READ_LENGTH] = {0};
		int iLenRead = min(GURESS_FILE_FORMAT_READ_LENGTH, iLength);
		fseek(pFile, 0, SEEK_SET);
		iLenRead = fread(sReaded, 1, iLenRead, pFile);          
		EnumGuessEncode eGuessEncode = GuessTextEncode(sReaded, iLenRead);
		sFileFormatType.iBOMFlagCount = 0;
		if (eGuessEncode == eGuessEncodeUTF8)
		{
			sFileFormatType.eTextEncode  = eTextEncodeUTF8;
			sFileFormatType.eCodePage = eCodePageUnicode;
		}
		else if (eGuessEncode == eGuessEncodeOEM)
		{
			sFileFormatType.eTextEncode  = eTextEncodeAnsi;
			sFileFormatType.eCodePage = eCodePageOEM;
		}  
		else if (eGuessEncode == eGuessEncodeAnsi)
		{
			sFileFormatType.eTextEncode  = eTextEncodeAnsi;
			sFileFormatType.eCodePage = eCodePageEnglish;
		}
		else
		{
			// 不应该运行到这里
		}
	}
	fseek(pFile, sFileFormatType.iBOMFlagCount, SEEK_SET);
#define GURESS_ENDOFLINE_READ_LENGTH 50
	unsigned char sReaded[GURESS_ENDOFLINE_READ_LENGTH] = {0};        
	int iLenRead = min(GURESS_ENDOFLINE_READ_LENGTH, iLength);
	iLenRead = fread(sReaded, 1, iLenRead, pFile);
	fclose(pFile);
	sFileFormatType.eEndOfLine = GuessEndOfLine(sReaded, iLenRead, sFileFormatType.eTextEncode);         

    return sFileFormatType;
}



// 00000000 - 0000007F 0xxxxxxx 
// 00000080 - 000007FF 110xxxxx 10xxxxxx 
// 00000800 - 0000FFFF 1110xxxx 10xxxxxx 10xxxxxx 
// 00010000 - 001FFFFF 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 
// 00200000 - 03FFFFFF 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 
// 04000000 - 7FFFFFFF 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
// 如果都是小于0x80，返回eGuessEncodeAnsi
// 如果发现有一个不是小于0x80
//              如果到字符串的结尾都发现符合UTF-8编码，返回eGuessEncodeUTF8
//              如果到字符串的结尾发现有不符合UTF-8编码的，返回eGuessEncodeOEM
EnumGuessEncode CTextFileReader::GuessTextEncode(unsigned char* pText, int iLen)
{
    //判断是不是ANSI
    int iIndex = 0;
    bool bAllAnsi = true;
    for (iIndex = 0; iIndex < iLen; iIndex ++)
    {
        if (pText[iIndex] >= 0x80)
        {
            bAllAnsi = false;
            break;
        }
    }
    if (bAllAnsi)
    {
        return eGuessEncodeAnsi;
    }

    // 判断是不是UTF-8
    int iScaned = 0;

    for (iIndex = 0; iIndex < iLen; iIndex+=iScaned)
    {
        int iLeft = iLen - iIndex;
        unsigned char* pStart = pText + iIndex;
        if (pStart[0] < 0x80)
        {
            iScaned = 1;
            continue;   // 可能是UTF-8里面的第一种情形
        }
        else
        {
            int iUTF8CodeLen = 0;
            unsigned char iFistChar = pStart[0];

            if (iFistChar < 0xC0)
            {
                return eGuessEncodeOEM;
            }
            else if (iFistChar < 0xE0)
            {
                iUTF8CodeLen = 2;
            }
            else if (iFistChar < 0xF0)
            {
                iUTF8CodeLen = 3;
            }
            else if (iFistChar < 0xF8)
            {
                iUTF8CodeLen = 4;
            }
            else if (iFistChar < 0xFC)
            {
                iUTF8CodeLen = 5;
            }
            else if (iFistChar < 0xFE)
            {
                iUTF8CodeLen = 6;
            }
            else
            {
                return eGuessEncodeOEM;
            }

            int iNeedToScanCount = min (iLeft, iUTF8CodeLen);

            for (int iCharIndex = 1; iCharIndex < iNeedToScanCount; iCharIndex ++)
            {
                if ((pStart[iCharIndex] & 0xC0) != 0x80)
                {
                    return eGuessEncodeOEM;
                }
            }
            iScaned = iUTF8CodeLen;
            continue;
        }
    }
    return eGuessEncodeUTF8;

}

EnumEndOfLine CTextFileReader::GuessEndOfLine(unsigned char* pText, int iLen, EnumTextEncode eTextEncode)
{
    EnumEndOfLine eEndOfLine = eEndOfLineWindows;
    unsigned char* pFind = NULL;
    switch (eTextEncode)
    {
    case eTextEncodeUnicode:
        if ((pFind = FindString(pText, iLen, &g_strEndOfLineUnicodeWindows[0], 4)) != NULL)
        {
            eEndOfLine = eEndOfLineWindows;
        }
        else if ((pFind = FindString(pText, iLen, &g_strEndOfLineUnicodeUnix[0], 2)) != NULL)
        {
            eEndOfLine = eEndOfLineUnix;
        }
        else if ((pFind = FindString(pText, iLen, &g_strEndOfLineUnicodeMac[0], 2)) != NULL)
        {
            eEndOfLine = eEndOfLineMac;
        }
        else
        {
            // 不应该运行到这里
        }
        break;

    case eTextEncodeUnicodeBigEndian:
        if ((pFind = FindString(pText, iLen, &g_strEndOfLineUnicodeBigEndianWindows[0], 4)) != NULL)
        {
            eEndOfLine = eEndOfLineWindows;
        }
        else if ((pFind = FindString(pText, iLen, &g_strEndOfLineUnicodeBigEndianUnix[0], 2)) != NULL)
        {
            eEndOfLine = eEndOfLineUnix;
        }
        else if ((pFind = FindString(pText, iLen, &g_strEndOfLineUnicodeBigEndianMac[0], 2)) != NULL)
        {
            eEndOfLine = eEndOfLineMac;
        }
        else
        {
            // 不应该运行到这里
        }
        break;

    case eTextEncodeUTF8:
    case eTextEncodeAnsi:
    default:
        if ((pFind = FindString(pText, iLen, &g_strEndOfLineAnsiWindows[0], 2)) != NULL)
        {
            eEndOfLine = eEndOfLineWindows;
        }
        else if ((pFind = FindString(pText, iLen, &g_strEndOfLineAnsiUnix[0], 1)) != NULL)
        {
            eEndOfLine = eEndOfLineUnix;
        }
        else if ((pFind = FindString(pText, iLen, &g_strEndOfLineAnsiMac[0], 1)) != NULL)
        {
            eEndOfLine = eEndOfLineMac;
        }
        else
        {
            // 不应该运行到这里
        }
        break;

    }
    return eEndOfLine;
}

unsigned char* CTextFileReader::FindString(const unsigned char* const pStart, int iMaxLen, const unsigned char* const pStrWillBeFind, int iStrWillBeFindLen)
{
    unsigned char* pFindStart = (unsigned char*)pStart;
    unsigned char* pFindEnd = (unsigned char*)(pStart + iMaxLen - iStrWillBeFindLen);
    for (pFindStart = (unsigned char*)pStart; pFindStart <= pFindEnd; pFindStart ++)
    {        
        bool bDifferent = false;
        for (int i = 0; i < iStrWillBeFindLen; i ++)
        {
            if (pFindStart[i] != pStrWillBeFind[i])
            {
                bDifferent = true;
                break;
            }
        }
        if (!bDifferent)
        {
            // 找到了
            return pFindStart;
        }
    }
    return NULL;
}


bool CTextFileReader::GetLine(string& strLine)
{    
    if (m_lstReadedLineBuffer.size() > 0)
    {
        strLine = *m_lstReadedLineBuffer.begin();
        m_lstReadedLineBuffer.erase(m_lstReadedLineBuffer.begin());
        return true;
    }
    if (m_iLastSeekPos >= m_iFileLength)
    {
        return false;
    }

#define MAX_BUFFER_LENGTH 4096
	fseek(m_pFile, m_iLastSeekPos, SEEK_SET);
	unsigned char strReaded[MAX_BUFFER_LENGTH] = {0}; 
	int iReaded = fread(strReaded, 1, MAX_BUFFER_LENGTH, m_pFile);
	if (iReaded <= 0)
	{
		return false;
	}
	if (iReaded > MAX_BUFFER_LENGTH - 4)
	{
		iReaded = MAX_BUFFER_LENGTH - 4;
	}
	else
	{
		memcpy(&strReaded[iReaded], m_pNextLineFlag, m_iNextLineFlagLen);
		iReaded += m_iNextLineFlagLen;
	}

	int iAnaliedCount = 0;
	string strLineTmp;
	while (iAnaliedCount <= iReaded)
	{
		int iLenUsed = 0;
		if (GetLineInternal(&strReaded[iAnaliedCount], iReaded - iAnaliedCount, strLineTmp, iLenUsed))
		{
			m_lstReadedLineBuffer.push_back(strLineTmp);
			iAnaliedCount += iLenUsed;
		}
		else
		{
			m_iLastSeekPos += iAnaliedCount;
			break;
		}
	}

	if (m_lstReadedLineBuffer.size() > 0)
	{
		strLine = *m_lstReadedLineBuffer.begin();
		m_lstReadedLineBuffer.erase(m_lstReadedLineBuffer.begin());
		return true;
	}
    return false;
}
bool CTextFileReader::Close()
{
    if (m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
    return true;
}

bool CTextFileReader::GetLineInternal(unsigned char* pStart, int iLen, string& strLine, int& iLenUsed)
{
    unsigned char* pNextLinePos = FindString(pStart, iLen, m_pNextLineFlag, m_iNextLineFlagLen);
    if (pNextLinePos == NULL)
    {
        return false;
    }
    iLenUsed = pNextLinePos - pStart + m_iNextLineFlagLen;
    strLine.resize(pNextLinePos - pStart);
    memcpy((char*)strLine.c_str(), pStart, strLine.size());
    return true;
}

SFileFormatType CTextFileReader::GetFileFormatType()
{
    return m_sFileFormatType;
}