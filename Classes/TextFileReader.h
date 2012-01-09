#pragma once

#ifndef _TEXT_FILE_READER_H_
#define _TEXT_FILE_READER_H_

#include <string>
#include <list>
using namespace std;

enum EnumTextEncode
{
    eTextEncodeAnsi = 0, 
    eTextEncodeUnicode, 
    eTextEncodeUnicodeBigEndian, 
    eTextEncodeUTF8
};

enum EnumEndOfLine
{
    eEndOfLineWindows = 0,   // \r\n
    eEndOfLineUnix,     // \n
    eEndOfLineMac,          // \r
};

enum EnumGuessEncode
{
    eGuessEncodeAnsi,
    eGuessEncodeUTF8,
    eGuessEncodeOEM,
};

enum EnumCodePage
{
    eCodePageEnglish = 0,
    eCodePageOEM,
    eCodePageUnicode
};

struct SFileFormatType
{
    EnumEndOfLine eEndOfLine;
    EnumTextEncode eTextEncode;
    EnumCodePage eCodePage;
    int iBOMFlagCount;                   // BOM标识的长度，字节单位       
};

class CTextFileReader
{
public:
    CTextFileReader(void);
    ~CTextFileReader(void);
    bool LoadFile(const string& strFileName);
    bool GetLine(string& strLine);
    bool Close();
    SFileFormatType GetFileFormatType();

private:

    unsigned char*  FindString(const unsigned char* const pStart, int iMaxLen, const unsigned char* const pStrWillBeFind, int iStrWillBeFindLen);
    EnumEndOfLine   GuessEndOfLine(unsigned char* pText, int iLen, EnumTextEncode eTextEncode);
    EnumGuessEncode GuessTextEncode(unsigned char* pText, int iLen);
    SFileFormatType GuessFileFormatType(const string& strFileName);
    bool GetLineInternal(unsigned char* pStart, int iLen, string& strLine, int& iLenUsed);

    bool m_bLoaded;
    FILE* m_pFile;
    SFileFormatType m_sFileFormatType;
    list<string> m_lstReadedLineBuffer;
    __int64_t m_iLastSeekPos;
    __int64_t m_iFileLength;
    unsigned char* m_pNextLineFlag;
    int m_iNextLineFlagLen;    
};

#endif
