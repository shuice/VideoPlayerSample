#pragma once
// lxy 2009-12-9 15:35:46 读取srt字幕
// windows 支持的codepage列表
// http://msdn.microsoft.com/en-us/goglobal/bb964653.aspx
// http://msdn.microsoft.com/en-us/library/dd317756(VS.85).aspx
// 文本文件的前几个字符含义
//  0xFF, 0xFE Unicode
//  0xFE, 0xFF Unicode big endian
//  0xEF, 0xBB, 0xBF UTF-8
//  没有的可能是ANSI，可能是当前的OEM，可能是UTF-8(no BOM)
// 应该分成两个类来写的，一个类提供ReadLine到string,另外一个解析SRT结构

// 现在可支持 ANSI, UNICODE, UNICODE-BigEndian, UTF-8, UTF-8(No BOM)
// 行结束符号支持Windows("\r\n") Unix("\n") Mac("\r")
/*
如何使用此类
1. 定义一个变量
CSrtSubTitleReader srtReader;
2. 加载文件
srtReader.LoadFile(_T("C:\\1.srt"));
3. 得到字幕内容，返回Unicode编码
        方法1
            先得到总个数，再得到每一个
            GetCount() GetIndex()
        方法2
            通过时间
            GetString(3000, string);

4: 可以设置延时，可以设置代码页，在加载文件之后，程序会分析文件，可以得到默认代码页。

*/
#ifndef _SRT_SUB_TITLE_READER_H_
#define _SRT_SUB_TITLE_READER_H_

#include <string>
#include <vector>
#include <set>
using namespace std;

struct SSubTitleFormat
{
    enum {eInvalidColor = 0xFF000000};
    enum {eInvalidFontSize = 0};
    bool bItalic;       // 斜体
    bool bUnderLine;    // 下划线
    bool bBold;          // 粗体
    int  iColor;    // 颜色,为eInvalidColor的时候无效
    int  iFontSize; // 字体大小，为eInvalidFontSize时候无效
};

struct SSrtSubTitleItemDecoded
{
    int iIndex;
    int iStartTime;
    int iEndTime;     
    wstring strSubTitleDecoded; // 一定是UNICODE的,带末尾的\0
    SSubTitleFormat sSubTitleFormat;
};


struct SSrtSubTitleItemReaded
{
    int iIndex;
    int iStartTime;
    int iEndTime;     
    string strSubTitleReaded; //从文件里读出来的，没有确定格式

    int iCodePage;   // 当前解码数据的编码方式，如果和设置的不一样，需要重新解码,不带末尾的\0
    wstring strSubTitleDecoded;         // 这里保存一份解码后的数据，免得每次都解码,一定是UNICODE的,带末尾的\0    
    bool operator < (const SSrtSubTitleItemReaded& sSrtSubTitleItemReaded) const
    {
        if (iStartTime < sSrtSubTitleItemReaded.iStartTime)
        {
            return true;
        }        
        return false;
    }
};

struct SCodePage
{
	string strType;
	int iCodePage;
	string strCodePage;
	string strDesc;
	
	SCodePage()
	{
		iCodePage = 0;
	}
	SCodePage(const string& strType1, int iCodePage1, 
			  const string& strCodePage1, const string& strDesc1)
	{
		strType = strType1;
		iCodePage = iCodePage1;
		strCodePage = strCodePage1;
		strDesc = strDesc1;
	}
	bool operator < (const SCodePage& sCodePage) const
	{
		return strDesc < sCodePage.strDesc;
	}
};

enum EnumTextEncode{eTextEncodeAnsi = 0, 
                                eTextEncodeUnicode, 
                                eTextEncodeUnicodeBigEndian, 
                                eTextEncodeUTF8
                                };

enum EnumEndOfLine{ eEndOfLineWindows = 0,   // \r\n
                        eEndOfLineUnix,     // \n
                        eEndOfLineMac,          // \r
                                };

enum EnumGuessEncode{eGuessEncodeAnsi,
                    eGuessEncodeUTF8,
                    eGuessEncodeOEM,
                    };

enum EnumCodePage{eCodePageEnglish = 0,
                    eCodePageOEM,
                    eCodePageUnicode};


struct SFileFormatType
{
    EnumEndOfLine eEndofLine;
    EnumTextEncode eTextEncode;
    EnumCodePage eCodePage;
    int iBOMFlagCount;                   // BOM标识的长度，字节单位       
};

typedef  int (*FunStringToInt)(const unsigned char* const pStart, const unsigned char* const pEnd);
typedef  bool (*FunStringToTime)(const unsigned char* const pStart, const unsigned char* const pEnd, int* pTimeStart, int* pTimeEnd);

#define WESTERN_EUROPEAN_WINDOWS_CODE_PAGE 1252
class CSrtSubTitleReader
{
public:

    CSrtSubTitleReader();
    // 如果文本是Unicode，采用Unicode处理
    // 如果文本不是Unicode
    // 如果eCharacterCode为eCharaterCodeDefault，采用计算机当前的字符集处理
    // 如果eCharacterCode不为eCharaterCodeDefault，采用eCharacterCode字符集处理
    // 如果文件是有效的srt文件，返回true，否则返回false
    bool LoadFile(const string& strSrtFileName);

    // 设置字符集
    void SetCodePage(int iCodePage);

    // 设置时间偏移，ms单位，0表示不偏移，1000表示字幕提前1秒出现
    void SetOffset(int iOffset);

    // 返回有几条字幕记录
    int GetCount();

    // 返回第几条字幕记录
    void GetIndex(int iIndex, SSrtSubTitleItemDecoded& sSrtSubTitleDecoded);

    // 得到某一个时间的字符
    // iTime表明了某一个时间,minisecond
    // strSubTitleItem返回了在iTime时间上的字符串,pSubTitleFormat返回当前时间的格式
    void GetString(int iTime, wstring& strSubTitleItem, SSubTitleFormat* pSubTitleFormat = NULL);
    
    //下面的不常用
    // 是不是英文字幕
    bool IsEnglishSubTitle();

    // srt文件是用Unicode存的吗，是eTextFileEncodeTypeAnsi返回false, 其余返回true
    bool IsUnicodeFileType();

    // 返回srt文件的末尾时间，毫秒单位
    int GetDuration();    // 返回srt文件的里面的时间最大值，影片长度,毫秒单位
 
	// 返回所有支持的代码页
	static const set<SCodePage>& GetCodePage();
private:
   // 在长度为iMaxLen的pStart里面寻找第一个长度为iStrWillBeFindLen的pStrWillBeFind，找到返回指向找到处的指针
    // 没有找到，返回NULL
    unsigned char* FindString(const unsigned char* const pStart, int iMaxLen, const unsigned char* const pStrWillBeFind, int iStrWillBeFindLen);

    // 从pStart开始处读取一个SSrtSubTitleItemReaded结构体存入到sSrtSubTitleItemReaded里面，iLenUsed返回使用了多少数据了
    // 如果能成功找到一个数据，返回true，否则返回false，如果返回false，sSrtSubTitleItemReaded和iLenUsed数据无效
    bool AnalySrtSubTitleItem(unsigned char* const pStart, int iMaxLen, SSrtSubTitleItemReaded& sSrtSubTitleItemReaded, int& iLenUsed, bool& bValid);

    // 字符串的编码转换，总是转成Unicode编码的
    void ConvertStringToDecoded(SSrtSubTitleItemReaded& sSrtSubTitleItemReaded);

    // 两个结构体之间的转换，纯拷贝
    void CopyReadedToDecoded(SSrtSubTitleItemReaded& sSrtSubTitleItemReaded, SSrtSubTitleItemDecoded& sSrtSubTitleItemDecoded);

    // 文本转int的字符串，假设文本不会超过int的界限
    static int StringToIntUnicode(const unsigned char* const pStart, const unsigned char* const pEnd);
    static int StringToIntUnicodeBigEndin(const unsigned char* const pStart, const unsigned char* const pEnd);
    static int StringToIntUTF8(const unsigned char* const pStart, const unsigned char* const pEnd);
    static int StringToIntAnsi(const unsigned char* const pStart, const unsigned char* const pEnd);

    static bool StringToTimeUnicode(const unsigned char* const pStart, const unsigned char* const pEnd, int* pTimeStart, int* pTimeEnd);
    static bool StringToTimeUnicodeBigEndin(const unsigned char* const pStart, const unsigned char* const pEnd, int* pTimeStart, int* pTimeEnd);
    static bool StringToTimeUTF8(const unsigned char* const pStart, const unsigned char* const pEnd, int* pTimeStart, int* pTimeEnd);
    static bool StringToTimeAnsi(const unsigned char* const pStart, const unsigned char* const pEnd, int* pTimeStart, int* pTimeEnd);
    void Reset();
	// 如果找不到了返回 WESTERN_EUROPEAN_WINDOWS_CODE_PAGE
	string GetCodePage(int iCodePage);

    // 根据buffer里面的字符串编码判断文本是UTF-8还是ANSI还是OEM
    SFileFormatType GuessFileFormatType(const string& strFileName);
    EnumGuessEncode GuessTextEncode(unsigned char* pText, int iLen);
    EnumEndOfLine GuessEndOfLine(unsigned char* pText, int iLen, EnumTextEncode eTextEncode);

	static void InitCodePage();
	
    int m_iCodePage;
    vector<SSrtSubTitleItemReaded> m_vctSrtSubTitleItemReaded;
    bool m_bLoaded;
    EnumTextEncode m_eTextEncode;

    const unsigned char* m_pNextLineFlag;   // 换行标识是哪一种，根据文件编码而定
    int m_iNextLineFlagLen;                 // 换行表示的长度，有可能是4，有可能是2
    FunStringToInt m_pFunStringToInt;
    FunStringToTime m_pFunStringToTime;
    
	string strCodePage;	// m_iCodePage对应缓存
    int m_iLastGetStringCache;   // GetString缓存
    int m_iOffset;
	
	static set<SCodePage> m_setSCodePage;
};

#endif
