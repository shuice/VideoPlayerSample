#include <list>
#include <algorithm>
#include <map>
#include <wchar.h>
#include <assert.h>
using namespace std;
#include "IConvWrapper.h"
#include "SrtSubTitleReader.h"


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

static const string g_strSep1Ansi = " --> ";
static const string g_strSep2Ansi = ":";
static const string g_strSep3Ansi = ",";

static const wstring g_strSep1Unicode = L" --> ";
static const wstring g_strSep2Unicode = L":";
static const wstring g_strSep3Unicode = L",";

/************************************************************************/
/* The format has no header, and no footer. Each subtitle has four parts:

Line 1 is a sequential count of subtitles, starting with 1.

Line 2 is the start timecode, followed by the string " --> ", followed by the end timecode. Timecodes are in the format HH:MM:SS,MIL (hours, minutes, seconds, milliseconds). The end timecode can optionally be followed by display coordinates (example " X1:100 X2:600 Y1:050 Y2:100"). Without coordinates displayed, each line of the subtitle will be centered and the block will appear at the bottom of the screen.

Lines 3 onward are the text of the subtitle. New lines are indicated by new lines (i.e. there's no "\n" code). The only formatting accepted are the following:

<b>text</b>: put text in boldface
<i>text</i>: put text in italics
<u>text</u>: underline text
<font color="#00ff00">text</font>: apply green color formatting to the text (you can use the font tag only to change color)

Tags can be combined (and should be nested properly). Note that the SubRip code appears to prefer whole-line formatting (no underlining just one word in the middle of a line).

Finally, successive subtitles are separated from each other by blank lines.

Here is an example of an SRT file:

1
00:02:26,407 --> 00:02:31,356 X1:100 X2:100 Y1:100 Y2:100
<font color="#00ff00">Detta handlar om min storebrors</font> 
<b><i><u>kriminella beteende och foersvinnade.</u></i></b>

2
00:02:31,567 --> 00:02:37,164 
Vi talar inte laengre om Wade. Det aer 
som om han aldrig hade existerat.                                                                     */
/************************************************************************/
/*
暗洋红color=darkmagenta（#8B008B）	暗红色color=darkred（#8B0000）
褐玫瑰红color=rosybrown（#BC8F8F）	印地安红color=indianred（#CD5C5C）
深红色color=crimson（#DC143C）	红色color=red（#FF0000）
艳粉色color=hotpink（#FF69B4）	深粉红color=deeppink（#FF1493）

橙红色color=orangered（#FF4500）
番茄色color=tomato（#FF6347）	鲜肉色color=salmon（#FA8072）
珊瑚色color=coral（#FF7F50）	浅珊瑚色color=lightcoral（#F08080）
浅肉色color=lightsalmon（#FFA07A）	浅粉色color=lightpink（#FFB6C1）
粉红色color=pink（#FFC0CB）	浅玫瑰色color=mistyrose（#FFE4E1）
重褐色color=saddlebrown（#8B4513）	栗色color=maroon（#800000）
土黄色color=sienna（#A0522D）	赭色color=sienna（#A0522D）
火砖色color=firebrick（#B22222）	褐色color=brown（#A52A2A）
巧克力色color=chocolate（#D2691E）	秘鲁色color=peru（#CD853F）
暗金黄色color=darkgoldenrod（#B8860B）	实木色color=burlywood（#DEB887）
苍麒麟色color=palegoldenrod（#EEE8AA）	茶色color=tan（#D2B48C）
金麒麟色color=goldenrod（#DAA520）	沙褐色color=sandybrown（#F4A460）
暗桔色color=darkorange（#FF8C00）	橙色color=orange（#FFA500）
金黄色color=gold（#FFD700）	黄色color=yellow（#FFFF00）
暗卡其色color=darkkhaki（#BDB76B）	卡其色color=khaki（#F0E68C）
纳瓦白color=navajowhite（#FFDEAD）	桔黄色color=bisque（#FFE4C4）
鹿皮色color=moccasin（#FFE4B5）	桃色color=peachpuff（#FFDAB9）
墨绿色color=darkslategray（#2F4F4F）	暗绿色color=darkgreen（#006400）

森林绿color=forestgreen（#228B22）
深绿褐色color=olivedrab（#6B8E23）	绿色color=green（#008000）
间海蓝color=mediumseagreen（#3CB371）	海绿色color=seagreen（#2E8B57）
浅海蓝color=lightseagreen（#20B2AA）	水鸭色color=teal（#008080）
间绿色color=mediumaquamarine（#66CDAA）	橄榄色color=olive（#808000）
暗海蓝color=darkseagreen（#8FBC8B）	黄绿色color=yellowgreen（#9ACD32）
橙绿色color=limegreen（#32CD32）	浅绿色color=lightgreen（#90EE90）
酸橙色color=lime（#00FF00）	黄绿色color=greenyellow（#ADFF2F）
黄绿色color=chartreuse（#7FFF00）	草绿色color=lawngreen（#7CFC00）
苍木色color=palegreen（#98FB98）	春绿色color=springgreen（#00FF7F）
间春绿color=mediumspringgreen（#00FA9A）	碧绿色color=aquamarine（#7FFFD4）
藏青色color=navy（#000080）
间宝石绿color=mediumturquoise（#48D1CC）	暗青色color=darkcyan（#008B8B）



青色color=cyan（#00FFFF）
暗灰蓝color=darkslateblue（#483D8B）	暗蓝色color=darkblue（#00008B）
中灰蓝color=midnightblue（#191970）	靛蓝色color=indigo（#4B0082）
浅蓝灰color=lightslategray（#778899）	钢蓝色color=steelblue（#4682B4）
石蓝色color=slateblue（#6A5ACD）	军蓝色color=cadetblue（#5F9EA0）
间蓝色color=mediumblue（#0000CD）	宝蓝色color=royalblue（#4169E1）
间暗蓝color=mediumslateblue（#7B68EE）	蓝色color=blue（#0000FF）
菊蓝色color=cornflowerblue（#6495ED）	闪蓝色color=dodgerblue（#1E90FF）
深天蓝color=deepskyblue（#00BFFF）	天蓝色color=skyblue（#87CEEB）
浅天蓝color=lightskyblue（#87CEFA）	浅蓝色color=lightblue（#ADD8E6）
浅钢蓝color=lightsteelblue（#B0C4DE）	粉蓝色color=powderblue（#B0E0E6）
间紫罗兰色color=mediumvioletred（#c71585）	紫色color=purple（#800080）
苍紫罗兰色color=palevioletred（#DB7093）	紫罗兰色color=blueviolet（#8A2BE2）
紫丁香色color=violet（#EE82EE）	暗紫色color=darkviolet（#9400D3）
紫红色color=Fuchsia（#FF00FF）	暗紫色color=darkorchid（#9932CC）
淡紫色color=orchid（#DA70D6）	间紫色color=mediumpurple（#9370DB）
杨李色color=plum（#DDA0DD）	间紫色color=mediumorchid（#BA55D3）
蓟色color=thistle（#D8BFD8）	淡紫色color=lavender（#E6E6FA）
黑色color＝black（#000000）	暗灰色color=dimgray（#696969）
灰色color=gray（#808080）	灰石色color=slategray（#708090）
暗灰色color=darkgray（#A9A9A9）	银白色color=silver（#C0C0C0）
浅灰色color=lightgrey（#D3D3D3）	淡灰色color=gainsboro（#DCDCDC）
*/

map<string, int> g_mapColorNameToColorAnsi;
map<wstring, int> g_mapColorNameToColorUnicode;
string g_strItalicAnsi; // <I>
static string g_strBoldAnsi;    // <B>
static string g_strFontAnsi;    // <font
static string g_strColorAnsi;   // color=
static string g_strUnderLineAnsi;   // <U>
static string g_strBlankAnsi;   // 

static wstring g_strItalicUnicode; // <I>
static wstring g_strBoldUnicode;    // <B>
static wstring g_strFontUnicode;    // <font
static wstring g_strColorUnicode;   // color=
static wstring g_strUnderLineUnicode;   // <U>
static wstring g_strBlankUnicode;   // 


template<typename CharType, typename StringType=basic_string<CharType> >
struct SExpression
{
    StringType strName;
    StringType strValue;
};

template<typename CharType, typename StringType=basic_string<CharType> >
class CSrtSubTitleReaderAssist
{
public:
	// 分析“a=b b = c c= d d= e”这样子的表达式，如果遇到了双等于符号"==",返回已经分离了的
	static void SplitExpression(StringType strExpression, vector< SExpression<CharType> >& vctSExpression)
	{
		int iStatus = 0;    // 0 寻找名字，1寻找等于，2寻找右侧
		SExpression<CharType> sExpression;
		vector<StringType> vctString;
		StringType* pStr = NULL;
		if (sizeof(CharType) == 1)
		{
			pStr = (StringType*)&g_strBlankAnsi;
		}
		else
		{
			pStr = (StringType*)&g_strBlankUnicode;
		}    
		SplitString(strExpression, *pStr, vctString);
		for (int i = 0; i < (int)vctString.size(); i++)
		{
			StringType& str = vctString[i];
			const int iPos = str.find('=');
			if (iStatus == 0)
			{            
				if (iPos == StringType::npos)
				{
					sExpression.strName = str;
					iStatus = 1;
					continue;
				}
				else
				{
					if (iPos == str.size() - 1)
					{
						sExpression.strName = str.substr(0, iPos);
						iStatus = 2;
						continue;
					}
					else
					{
						sExpression.strName = str.substr(0, iPos);
						sExpression.strValue = str.substr(iPos + 1, str.size() - 1 - iPos);
						vctSExpression.push_back(sExpression);
						iStatus = 0;
						continue;
					}
				}
			}
			else if (iStatus == 1)
			{    
				if (iPos == StringType::npos)
				{
					sExpression.strName = str;
					iStatus = 1;
					continue;
				}
				else
				{
					if (iPos != 0)
					{
						if (iPos == str.size() - 1)
						{
							sExpression.strName = str.substr(0, iPos);
							iStatus = 2;
							continue;
						}
						else
						{
							sExpression.strName = str.substr(0, iPos);
							sExpression.strValue = str.substr(iPos + 1, str.size() - 1 - iPos);
							vctSExpression.push_back(sExpression);
							iStatus = 0;
							continue;
						}                    
					}
					else
					{
						if (str.size() > 1)
						{
							sExpression.strValue = str.substr(1, str.size() - 1);
							vctSExpression.push_back(sExpression);
							iStatus = 0;
							continue;
						}
						else
						{
							iStatus = 2;
							continue;
						}
					}
				}
			}
			else if (iStatus == 2)
			{
				sExpression.strValue = str;
				vctSExpression.push_back(sExpression);
				iStatus = 0;
				continue; 
			}
		}
	}
	
	static void GetSrtStylesByTags(StringType& strTag, SSubTitleFormat* pSubTitleFormat)
	{
		if (pSubTitleFormat == NULL)
		{
			return;
		}
		InitTags();
		for_each(strTag.begin(), strTag.end(), PreFixSrtTag);    
		
		StringType* pStrBold = NULL;
		StringType* pStrItalic = NULL;
		StringType* pStrUnderLine = NULL;
		StringType* pStrFont = NULL;
		StringType* pStrColor = NULL;
		StringType* pStrBlank = NULL;
		map<StringType, int>* pMapColorNameToColor = NULL;
		
		if (sizeof(CharType) == 1)
		{
			pStrBold = (StringType*)&g_strBoldAnsi;
			pStrItalic = (StringType*)&g_strItalicAnsi;
			pStrUnderLine = (StringType*)&g_strUnderLineAnsi;
			pStrFont = (StringType*)&g_strFontAnsi;
			pStrColor = (StringType*)&g_strColorAnsi;
			pStrBlank = (StringType*)&g_strBoldAnsi;
			pMapColorNameToColor = (map<StringType, int>*)&g_mapColorNameToColorAnsi;
		}
		else
		{
			pStrBold = (StringType*)&g_strBoldUnicode;
			pStrItalic = (StringType*)&g_strItalicUnicode;
			pStrUnderLine = (StringType*)&g_strUnderLineUnicode;
			pStrFont = (StringType*)&g_strFontUnicode;
			pStrColor = (StringType*)&g_strColorUnicode;
			pStrBlank = (StringType*)&g_strBoldUnicode;
			pMapColorNameToColor = (map<StringType, int>*)&g_mapColorNameToColorUnicode;
		}
		
		if (!pSubTitleFormat->bBold)
		{
			if (strTag.compare(*pStrBold) == 0)
			{
				pSubTitleFormat->bBold = true;
			}
		}
		if (!pSubTitleFormat->bItalic)
		{
			if (strTag.compare(*pStrItalic) == 0)
			{
				pSubTitleFormat->bItalic = true;
			}
		}
		if (!pSubTitleFormat->bUnderLine)
		{
			if (strTag.compare(*pStrUnderLine) == 0)
			{
				pSubTitleFormat->bUnderLine = true;
			}
		}
		if (pSubTitleFormat->iColor == SSubTitleFormat::eInvalidColor)
		{
			if (strTag.compare(0, (*pStrFont).size(), (*pStrFont).c_str()) == 0)
			{
				int iPos = strTag.find(*pStrColor);
				if (iPos != string::npos)
				{
					StringType strLeft = strTag.substr(iPos, strTag.size() - iPos -1);
					vector< SExpression<CharType> > vctSExpression;
					CSrtSubTitleReaderAssist<CharType>::SplitExpression(strLeft, vctSExpression);
					int iExpressionCount = (int)vctSExpression.size();
					
					for (int iExpressionIndex = 0; iExpressionIndex < iExpressionCount; iExpressionIndex ++)
					{				
						if ((*pStrColor).compare(1, vctSExpression[iExpressionIndex].strName.size(), vctSExpression[iExpressionIndex].strName) == 0)                    
						{
							StringType& strColorValue = vctSExpression[iExpressionIndex].strValue;
							//strColorValue 可能是"#FF0000", 可能是"FF0000",可能是FF0000可能是RED
							int iColorStartPos = 0;
							if (strColorValue[iColorStartPos] == '\"' || strColorValue[iColorStartPos] == '\'')
							{
								iColorStartPos ++;
							}
							if (strColorValue[iColorStartPos] == '#')
							{
								iColorStartPos ++;
								// 只可能是数值
								int iCharLeft = strColorValue.size() - iColorStartPos;   
								CharType strValue[7] = {0};
								memcpy(strValue, (char*)(strColorValue.c_str() + iColorStartPos), min(6, iCharLeft) * sizeof(CharType));
								pSubTitleFormat->iColor = StringToColor(strValue);
							}
							else
							{
								// 有可能是数值，也有可能是字符串
								bool bFinded = false;
								if (sizeof(CharType) == sizeof(wchar_t))
								{
									map<wstring, int>::iterator it
										= pMapColorNameToColor->find(strColorValue);
									if (it != pMapColorNameToColor->end())
									{
										pSubTitleFormat->iColor = it->second;
									} 
									bFinded = true;
								}
								else
								{
									assert(0 && "Not supported");                  
								}
								 
								if (!bFinded)
								{
									int iCharLeft = strColorValue.size() - iColorStartPos;    
									CharType strValue[7] = {0};
									memcpy(strValue, (char*)(strColorValue.c_str() + iColorStartPos), min(6, iCharLeft) * sizeof(CharType));
									pSubTitleFormat->iColor = StringToColor(strValue);
								}
								
							}
							
							break;
						}   // end if (vctSExpression[iExpressionIndex].strName == pStrColor)
					}   // end for (int iExpressionIndex = 0; iExpressionIndex < iExpressionCount; iExpressionIndex ++)
				} // end if (iPos != string::npos)
			} // end if (strTag.compare(0, (*pStrFont).size(), (*pStrFont).c_str()) == 0)
		} // end if (pSubTitleFormat->iColor == SSubTitleFormat::eInvalidColor)
	}
	
	// 去掉 '<' 和 '>'和其中包含的内容，如果没有找到匹配，而不删除
	static void GetSrtStyles(StringType& str, SSubTitleFormat* pSubTitleFormat)
	{
		if (pSubTitleFormat != NULL)
		{
			pSubTitleFormat->bItalic = false;
			pSubTitleFormat->bUnderLine = false;
			pSubTitleFormat->bBold = false;
			pSubTitleFormat->iColor = SSubTitleFormat::eInvalidColor;
			pSubTitleFormat->iFontSize = SSubTitleFormat::eInvalidFontSize;
		}
		
		
		CharType iLessSymbol = 60;
		CharType iLargeSymbol = 62;    
		
		int iLengthChecked = 0;
		// 从前往后，找匹配的<>，然后删除它们
		while (true)
		{
			int iSize = str.size();
			int iLessSymbolPos = str.find_first_of(iLessSymbol, iLengthChecked);
			if (iLessSymbolPos == string::npos)
			{
				break;
			}
			int iLargeSymbolPos = str.find_first_of(iLargeSymbol, iLessSymbolPos + 1);
			if (iLargeSymbolPos == string::npos)
			{
				break;
			}
			
			StringType strStyle = str.substr(iLessSymbolPos, iLargeSymbolPos - iLessSymbolPos + 1);
			GetSrtStylesByTags(strStyle, pSubTitleFormat);
			
			// 找到了匹配
			iLengthChecked = iLessSymbolPos;
			if (iLessSymbolPos == 0 && iLargeSymbolPos != iSize - 1)
			{
				str = str.substr(iLargeSymbolPos + 1, iSize - 1 - iLargeSymbolPos);
			}
			else if (iLessSymbolPos != 0 && iLargeSymbolPos == iSize - 1)
			{
				str = str.substr(0, iLessSymbolPos);
			}
			else if (iLessSymbolPos != 0 && iLargeSymbolPos != iSize -1)
			{
				str = str.substr(0, iLessSymbolPos) + str.substr(iLargeSymbolPos + 1, iSize - 1 - iLargeSymbolPos);
			}
			else
			{
				str.clear();
			}
		}  
	}
	
	static int StringToTime(StringType& str, int iLen)
	{
		int iSize = str.size();
		int i = 0;
		for (i = 0; i < iSize; i ++)
		{
			if (str[i] != '0')
			{
				break;
			}
		}
		int iLenLeft = iSize - i;    
		
		StringType strTemp;
		strTemp.resize(iLen + 1);
		memset((char*)strTemp.c_str(), 0, (iLen + 1) * sizeof(CharType));
		memcpy((char*)strTemp.c_str(), (char*)(str.c_str() + i), min(iLenLeft, iLen) * sizeof(CharType));
		if (sizeof(CharType) == 1)
		{
			return atoi((char*)strTemp.c_str());
		}
		else
		{
			char sz[20] = {0};
			CIConvWrapper iConvWrapper("UTF-16LE", "CP1252");
			iConvWrapper.Convert((char*)strTemp.c_str(), strTemp.size()*2, sz, sizeof(sz));
			return atoi(sz);
		}
	}

	static void Trim(StringType& str)
	{
		int iStart = 0;
		int iEnd = 0;
		int iLength = 0;
		int iSize = str.size();
		CharType iBlank = 32;
		CharType iTable = 9;    
		
		
		for (iStart = 0; iStart< iSize; iStart++)
		{
			CharType iValue = str[iStart];		
			if (iValue == iBlank  || iValue == iTable)
			{
				continue;
			}
			break;
		}
		
		for (iEnd = iSize - 1; iEnd > iStart; iEnd --)
		{
			CharType iValue = str[iEnd];		
			if (iValue == iBlank  || iValue == iTable)
			{
				continue;
			}
			break;
		}
		
		iLength = iEnd - iStart + 1;
		if (!(iStart == 0 && iLength == iSize - 1))
		{
			str = str.substr(iStart, iLength);
		}  
	}
	
	static void SplitString(const StringType& strWillBeSplit, const StringType& strFlag, vector<StringType>& vctStrRet)
	{
		vctStrRet.clear();
		if (strFlag.size() == 0)
		{
			vctStrRet.push_back(strWillBeSplit);
			return;
		}
		int iFindPos = string::npos;
		int iStartPos = 0;
		StringType strSub;
		while ((iFindPos = strWillBeSplit.find(strFlag, iStartPos)) != StringType::npos)
		{
			if (iFindPos > iStartPos)
			{
				strSub = strWillBeSplit.substr(iStartPos, iFindPos - iStartPos);
				Trim(strSub);
				vctStrRet.push_back(strSub);
			}
			
			iStartPos = iFindPos + strFlag.size();
		}
		int iSize = strWillBeSplit.size();
		if (iSize > iStartPos)
		{
			strSub = strWillBeSplit.substr(iStartPos, iFindPos - iStartPos);
			Trim(strSub);
			vctStrRet.push_back(strSub);
		}
	}

	static void PreFixSrtTag(CharType& iValue)
	{
		if (iValue >= 'a' && iValue <= 'z')
		{
			iValue -= 32;   // 小写转大写
		}
		else if (iValue == 9)
		{
			iValue = 32;   // 制表符转空格
		}
	}
	
	static long StringToColor(const CharType* pStart)
	{
		CharType* pEnd = NULL;
		long iValue = 0;
		if (sizeof(CharType) == 1)
		{
			iValue = strtol((char*)pStart, (char**)&pEnd, 16);
		}
		else
		{
			iValue =  wcstol((wchar_t*)pStart, (wchar_t**)&pEnd, 16);        
		}
		return iValue & 0x00FFFFFF;
	}
	
	static void InitTags()
	{
		if (g_mapColorNameToColorAnsi.size() > 0)
		{
			return;
		}
		// ANSI Colors
		g_mapColorNameToColorAnsi["ORANGERED"]          = 0x0045FF;
		g_mapColorNameToColorAnsi["HOTPINK"]            = 0xB469FF;
		g_mapColorNameToColorAnsi["DEEPPINK"]           = 0x9314FF;
		g_mapColorNameToColorAnsi["CRIMSON"]            = 0x3C14DC;
		g_mapColorNameToColorAnsi["RED"]                = 0x0000FF;
		g_mapColorNameToColorAnsi["ROSYBROWN"]          = 0x8F8FBC;
		g_mapColorNameToColorAnsi["INDIANRED"]          = 0x5C5CCD;
		g_mapColorNameToColorAnsi["DARKMAGENTA"]        = 0x8B008B;
		g_mapColorNameToColorAnsi["DARKRED"]            = 0x00008B;
		g_mapColorNameToColorAnsi["LIGHTSALMON"]        = 0x7AA0FF;
		
		g_mapColorNameToColorAnsi["LIGHTPINK"]          = 0xC1B6FF;
		g_mapColorNameToColorAnsi["CORAL"]              = 0x507FFF;
		g_mapColorNameToColorAnsi["LIGHTCORAL"]         = 0x8080F0;
		g_mapColorNameToColorAnsi["TOMATO"]             = 0x4763FF;
		g_mapColorNameToColorAnsi["SALMON"]             = 0x7280FA;
		g_mapColorNameToColorAnsi["SIENNA"]             = 0x2D52A0;
		g_mapColorNameToColorAnsi["SADDLEBROWN"]        = 0x13458B;
		g_mapColorNameToColorAnsi["MAROON"]             = 0x000080;
		g_mapColorNameToColorAnsi["PINK"]               = 0xCBC0FF;
		g_mapColorNameToColorAnsi["MISTYROSE"]          = 0xE1E4FF;
		
		g_mapColorNameToColorAnsi["DARKGOLDENROD"]      = 0x0B86B8;
		g_mapColorNameToColorAnsi["BURLYWOOD"]          = 0x87B8DE;
		g_mapColorNameToColorAnsi["CHOCOLATE"]          = 0x1E69D2;
		g_mapColorNameToColorAnsi["PERU"]               = 0x3F85CD;
		g_mapColorNameToColorAnsi["FIREBRICK"]          = 0x2222B2;
		g_mapColorNameToColorAnsi["BROWN"]              = 0x2A2AA5;
		g_mapColorNameToColorAnsi["DARKORANGE"]         = 0x008CFF;
		g_mapColorNameToColorAnsi["ORANGE"]             = 0x00A5FF;
		g_mapColorNameToColorAnsi["GOLDENROD"]          = 0x20A5DA;
		g_mapColorNameToColorAnsi["SANDYBROWN"]         = 0x60A4F4;
		
		g_mapColorNameToColorAnsi["PALEGOLDENROD"]      = 0xAAE8EE;
		g_mapColorNameToColorAnsi["TAN"]                = 0x8CB4D2;
		g_mapColorNameToColorAnsi["FORESTGREEN"]        = 0x228B22;
		g_mapColorNameToColorAnsi["DARKSLATEGRAY"]      = 0x4F4F2F;
		g_mapColorNameToColorAnsi["DARKGREEN"]          = 0x006400;
		g_mapColorNameToColorAnsi["MOCCASIN"]           = 0xB5E4FF;
		g_mapColorNameToColorAnsi["PEACHPUFF"]          = 0xB9DAFF;
		g_mapColorNameToColorAnsi["LIGHTSEAGREEN"]      = 0xAAB220;
		g_mapColorNameToColorAnsi["TEAL"]               = 0x808000;
		
		g_mapColorNameToColorAnsi["MEDIUMSEAGREEN"]     = 0x71B33C;
		g_mapColorNameToColorAnsi["SEAGREEN"]           = 0x578B2E;
		g_mapColorNameToColorAnsi["OLIVEDRAB"]          = 0x238E6B;
		g_mapColorNameToColorAnsi["GREEN"]              = 0x008000;
		g_mapColorNameToColorAnsi["LIMEGREEN"]          = 0x32CD32;
		g_mapColorNameToColorAnsi["LIGHTGREEN"]         = 0x90EE90;
		g_mapColorNameToColorAnsi["DARKSEAGREEN"]       = 0x8BBC8F;
		g_mapColorNameToColorAnsi["YELLOWGREEN"]        = 0x32CD9A;
		g_mapColorNameToColorAnsi["MEDIUMAQUAMARINE"]   = 0xAACD66;
		
		g_mapColorNameToColorAnsi["OLIVE"]              = 0x008080;
		g_mapColorNameToColorAnsi["PALEGREEN"]          = 0x98FB98;
		g_mapColorNameToColorAnsi["SPRINGGREEN"]        = 0x7FFF00;
		g_mapColorNameToColorAnsi["CHARTREUSE"]         = 0x00FF7F;
		g_mapColorNameToColorAnsi["LAWNGREEN"]          = 0x00FC7C;
		g_mapColorNameToColorAnsi["LIME"]               = 0x00FF00;
		g_mapColorNameToColorAnsi["GREENYELLOW"]        = 0x2FFFAD;
		g_mapColorNameToColorAnsi["MEDIUMTURQUOISE"]    = 0xCCD148;
		g_mapColorNameToColorAnsi["DARKCYAN"]           = 0x8B8B00;
		g_mapColorNameToColorAnsi["NAVY"]               = 0x800000;
		
		g_mapColorNameToColorAnsi["MEDIUMSPRINGGREEN"]  = 0x9AFA00;
		g_mapColorNameToColorAnsi["AQUAMARINE"]         = 0xD4FF7F;
		g_mapColorNameToColorAnsi["CYAN"]               = 0xFFFF00;
		g_mapColorNameToColorAnsi["LIGHTSLATEGRAY"]     = 0x998877;
		g_mapColorNameToColorAnsi["STEELBLUE"]          = 0xB48246;
		g_mapColorNameToColorAnsi["MIDNIGHTBLUE"]       = 0x701919;
		g_mapColorNameToColorAnsi["INDIGO"]             = 0x82004B;
		g_mapColorNameToColorAnsi["DARKSLATEBLUE"]      = 0x8B3D48;
		g_mapColorNameToColorAnsi["DARKBLUE"]           = 0x8B0000;
		g_mapColorNameToColorAnsi["MEDIUMSLATEBLUE"]    = 0xEE687B;
		
		g_mapColorNameToColorAnsi["BLUE"]               = 0xFF0000;
		g_mapColorNameToColorAnsi["MEDIUMBLUE"]         = 0xCD0000;
		g_mapColorNameToColorAnsi["ROYALBLUE"]          = 0xE16941;
		g_mapColorNameToColorAnsi["SLATEBLUE"]          = 0xCD5A6A;
		g_mapColorNameToColorAnsi["CADETBLUE"]          = 0xA09E5F;
		g_mapColorNameToColorAnsi["LIGHTSKYBLUE"]       = 0xFACE87;
		g_mapColorNameToColorAnsi["LIGHTBLUE"]          = 0xE6D8AD;
		g_mapColorNameToColorAnsi["DEEPSKYBLUE"]        = 0xFFBF00;
		g_mapColorNameToColorAnsi["SKYBLUE"]            = 0xEBCE87;
		g_mapColorNameToColorAnsi["CORNFLOWERBLUE"]     = 0xED9564;
		
		g_mapColorNameToColorAnsi["DODGERBLUE"]         = 0xFF901E;
		g_mapColorNameToColorAnsi["SILVER"]             = 0xC0C0C0;
		g_mapColorNameToColorAnsi["DARKGRAY"]           = 0xA9A9A9;    
		g_mapColorNameToColorAnsi["LIGHTGREY"]          = 0xD3D3D3;
		g_mapColorNameToColorAnsi["GAINSBORO"]          = 0xDCDCDC;
		g_mapColorNameToColorAnsi["GRAY"]               = 0x808080;    
		g_mapColorNameToColorAnsi["SLATEGRAY"]          = 0x908070;
		g_mapColorNameToColorAnsi["BLACK"]              = 0x000000;
		g_mapColorNameToColorAnsi["DIMGRAY"]            = 0x696969;
		g_mapColorNameToColorAnsi["THISTLE"]            = 0xD8BFD8;
		
		g_mapColorNameToColorAnsi["LAVENDER"]           = 0xFAE6E6;
		g_mapColorNameToColorAnsi["PLUM"]               = 0xDDA0DD;
		g_mapColorNameToColorAnsi["MEDIUMORCHID"]       = 0xD355BA;
		g_mapColorNameToColorAnsi["ORCHID"]             = 0xD670DA;
		g_mapColorNameToColorAnsi["MEDIUMPURPLE"]       = 0xDB7093;
		g_mapColorNameToColorAnsi["FUCHSIA"]            = 0xFF00FF;
		g_mapColorNameToColorAnsi["DARKVIOLET"]         = 0xD30094;    
		g_mapColorNameToColorAnsi["VIOLET"]             = 0xEE82EE;
		g_mapColorNameToColorAnsi["DARKORCHID"]         = 0xCC3299;    
		g_mapColorNameToColorAnsi["PALEVIOLETRED"]      = 0x9370DB;
		
		g_mapColorNameToColorAnsi["BLUEVIOLET"]         = 0xE22B8A;
		g_mapColorNameToColorAnsi["MEDIUMVIOLETRED"]    = 0x8515C7;
		g_mapColorNameToColorAnsi["PURPLE"]             = 0x800080;
		g_mapColorNameToColorAnsi["LIGHTSTEELBLUE"]     = 0xDEC4B0;
		g_mapColorNameToColorAnsi["POWDERBLUE"]         = 0xE6E0B0;
		g_mapColorNameToColorAnsi["GOLD"]               = 0x00D7FF;
		g_mapColorNameToColorAnsi["YELLOW"]             = 0x00FFFF;
		g_mapColorNameToColorAnsi["DARKKHAKI"]          = 0x6BB7BD;
		g_mapColorNameToColorAnsi["KHAKI"]              = 0x8CE6F0;
		g_mapColorNameToColorAnsi["NAVAJOWHITE"]        = 0xADDEFF;
		
		g_mapColorNameToColorAnsi["BISQUE"]             = 0xC4E4FF;
		
		// UNICODE COLORS
		g_mapColorNameToColorUnicode[L"ORANGERED"]          = 0x0045FF;
		g_mapColorNameToColorUnicode[L"HOTPINK"]            = 0xB469FF;
		g_mapColorNameToColorUnicode[L"DEEPPINK"]           = 0x9314FF;
		g_mapColorNameToColorUnicode[L"CRIMSON"]            = 0x3C14DC;
		g_mapColorNameToColorUnicode[L"RED"]                = 0x0000FF;
		g_mapColorNameToColorUnicode[L"ROSYBROWN"]          = 0x8F8FBC;
		g_mapColorNameToColorUnicode[L"INDIANRED"]          = 0x5C5CCD;
		g_mapColorNameToColorUnicode[L"DARKMAGENTA"]        = 0x8B008B;
		g_mapColorNameToColorUnicode[L"DARKRED"]            = 0x00008B;
		g_mapColorNameToColorUnicode[L"LIGHTSALMON"]        = 0x7AA0FF;
		
		g_mapColorNameToColorUnicode[L"LIGHTPINK"]          = 0xC1B6FF;
		g_mapColorNameToColorUnicode[L"CORAL"]              = 0x507FFF;
		g_mapColorNameToColorUnicode[L"LIGHTCORAL"]         = 0x8080F0;
		g_mapColorNameToColorUnicode[L"TOMATO"]             = 0x4763FF;
		g_mapColorNameToColorUnicode[L"SALMON"]             = 0x7280FA;
		g_mapColorNameToColorUnicode[L"SIENNA"]             = 0x2D52A0;
		g_mapColorNameToColorUnicode[L"SADDLEBROWN"]        = 0x13458B;
		g_mapColorNameToColorUnicode[L"MAROON"]             = 0x000080;
		g_mapColorNameToColorUnicode[L"PINK"]               = 0xCBC0FF;
		g_mapColorNameToColorUnicode[L"MISTYROSE"]          = 0xE1E4FF;
		
		g_mapColorNameToColorUnicode[L"DARKGOLDENROD"]      = 0x0B86B8;
		g_mapColorNameToColorUnicode[L"BURLYWOOD"]          = 0x87B8DE;
		g_mapColorNameToColorUnicode[L"CHOCOLATE"]          = 0x1E69D2;
		g_mapColorNameToColorUnicode[L"PERU"]               = 0x3F85CD;
		g_mapColorNameToColorUnicode[L"FIREBRICK"]          = 0x2222B2;
		g_mapColorNameToColorUnicode[L"BROWN"]              = 0x2A2AA5;
		g_mapColorNameToColorUnicode[L"DARKORANGE"]         = 0x008CFF;
		g_mapColorNameToColorUnicode[L"ORANGE"]             = 0x00A5FF;
		g_mapColorNameToColorUnicode[L"GOLDENROD"]          = 0x20A5DA;
		g_mapColorNameToColorUnicode[L"SANDYBROWN"]         = 0x60A4F4;
		
		g_mapColorNameToColorUnicode[L"PALEGOLDENROD"]      = 0xAAE8EE;
		g_mapColorNameToColorUnicode[L"TAN"]                = 0x8CB4D2;
		g_mapColorNameToColorUnicode[L"FORESTGREEN"]        = 0x228B22;
		g_mapColorNameToColorUnicode[L"DARKSLATEGRAY"]      = 0x4F4F2F;
		g_mapColorNameToColorUnicode[L"DARKGREEN"]          = 0x006400;
		g_mapColorNameToColorUnicode[L"MOCCASIN"]           = 0xB5E4FF;
		g_mapColorNameToColorUnicode[L"PEACHPUFF"]          = 0xB9DAFF;
		g_mapColorNameToColorUnicode[L"LIGHTSEAGREEN"]      = 0xAAB220;
		g_mapColorNameToColorUnicode[L"TEAL"]               = 0x808000;
		
		g_mapColorNameToColorUnicode[L"MEDIUMSEAGREEN"]     = 0x71B33C;
		g_mapColorNameToColorUnicode[L"SEAGREEN"]           = 0x578B2E;
		g_mapColorNameToColorUnicode[L"OLIVEDRAB"]          = 0x238E6B;
		g_mapColorNameToColorUnicode[L"GREEN"]              = 0x008000;
		g_mapColorNameToColorUnicode[L"LIMEGREEN"]          = 0x32CD32;
		g_mapColorNameToColorUnicode[L"LIGHTGREEN"]         = 0x90EE90;
		g_mapColorNameToColorUnicode[L"DARKSEAGREEN"]       = 0x8BBC8F;
		g_mapColorNameToColorUnicode[L"YELLOWGREEN"]        = 0x32CD9A;
		g_mapColorNameToColorUnicode[L"MEDIUMAQUAMARINE"]   = 0xAACD66;
		
		g_mapColorNameToColorUnicode[L"OLIVE"]              = 0x008080;
		g_mapColorNameToColorUnicode[L"PALEGREEN"]          = 0x98FB98;
		g_mapColorNameToColorUnicode[L"SPRINGGREEN"]        = 0x7FFF00;
		g_mapColorNameToColorUnicode[L"CHARTREUSE"]         = 0x00FF7F;
		g_mapColorNameToColorUnicode[L"LAWNGREEN"]          = 0x00FC7C;
		g_mapColorNameToColorUnicode[L"LIME"]               = 0x00FF00;
		g_mapColorNameToColorUnicode[L"GREENYELLOW"]        = 0x2FFFAD;
		g_mapColorNameToColorUnicode[L"MEDIUMTURQUOISE"]    = 0xCCD148;
		g_mapColorNameToColorUnicode[L"DARKCYAN"]           = 0x8B8B00;
		g_mapColorNameToColorUnicode[L"NAVY"]               = 0x800000;
		
		g_mapColorNameToColorUnicode[L"MEDIUMSPRINGGREEN"]  = 0x9AFA00;
		g_mapColorNameToColorUnicode[L"AQUAMARINE"]         = 0xD4FF7F;
		g_mapColorNameToColorUnicode[L"CYAN"]               = 0xFFFF00;
		g_mapColorNameToColorUnicode[L"LIGHTSLATEGRAY"]     = 0x998877;
		g_mapColorNameToColorUnicode[L"STEELBLUE"]          = 0xB48246;
		g_mapColorNameToColorUnicode[L"MIDNIGHTBLUE"]       = 0x701919;
		g_mapColorNameToColorUnicode[L"INDIGO"]             = 0x82004B;
		g_mapColorNameToColorUnicode[L"DARKSLATEBLUE"]      = 0x8B3D48;
		g_mapColorNameToColorUnicode[L"DARKBLUE"]           = 0x8B0000;
		g_mapColorNameToColorUnicode[L"MEDIUMSLATEBLUE"]    = 0xEE687B;
		
		g_mapColorNameToColorUnicode[L"BLUE"]               = 0xFF0000;
		g_mapColorNameToColorUnicode[L"MEDIUMBLUE"]         = 0xCD0000;
		g_mapColorNameToColorUnicode[L"ROYALBLUE"]          = 0xE16941;
		g_mapColorNameToColorUnicode[L"SLATEBLUE"]          = 0xCD5A6A;
		g_mapColorNameToColorUnicode[L"CADETBLUE"]          = 0xA09E5F;
		g_mapColorNameToColorUnicode[L"LIGHTSKYBLUE"]       = 0xFACE87;
		g_mapColorNameToColorUnicode[L"LIGHTBLUE"]          = 0xE6D8AD;
		g_mapColorNameToColorUnicode[L"DEEPSKYBLUE"]        = 0xFFBF00;
		g_mapColorNameToColorUnicode[L"SKYBLUE"]            = 0xEBCE87;
		g_mapColorNameToColorUnicode[L"CORNFLOWERBLUE"]     = 0xED9564;
		
		g_mapColorNameToColorUnicode[L"DODGERBLUE"]         = 0xFF901E;
		g_mapColorNameToColorUnicode[L"SILVER"]             = 0xC0C0C0;
		g_mapColorNameToColorUnicode[L"DARKGRAY"]           = 0xA9A9A9;    
		g_mapColorNameToColorUnicode[L"LIGHTGREY"]          = 0xD3D3D3;
		g_mapColorNameToColorUnicode[L"GAINSBORO"]          = 0xDCDCDC;
		g_mapColorNameToColorUnicode[L"GRAY"]               = 0x808080;    
		g_mapColorNameToColorUnicode[L"SLATEGRAY"]          = 0x908070;
		g_mapColorNameToColorUnicode[L"BLACK"]              = 0x000000;
		g_mapColorNameToColorUnicode[L"DIMGRAY"]            = 0x696969;
		g_mapColorNameToColorUnicode[L"THISTLE"]            = 0xD8BFD8;
		
		g_mapColorNameToColorUnicode[L"LAVENDER"]           = 0xFAE6E6;
		g_mapColorNameToColorUnicode[L"PLUM"]               = 0xDDA0DD;
		g_mapColorNameToColorUnicode[L"MEDIUMORCHID"]       = 0xD355BA;
		g_mapColorNameToColorUnicode[L"ORCHID"]             = 0xD670DA;
		g_mapColorNameToColorUnicode[L"MEDIUMPURPLE"]       = 0xDB7093;
		g_mapColorNameToColorUnicode[L"FUCHSIA"]            = 0xFF00FF;
		g_mapColorNameToColorUnicode[L"DARKVIOLET"]         = 0xD30094;    
		g_mapColorNameToColorUnicode[L"VIOLET"]             = 0xEE82EE;
		g_mapColorNameToColorUnicode[L"DARKORCHID"]         = 0xCC3299;    
		g_mapColorNameToColorUnicode[L"PALEVIOLETRED"]      = 0x9370DB;
		
		g_mapColorNameToColorUnicode[L"BLUEVIOLET"]         = 0xE22B8A;
		g_mapColorNameToColorUnicode[L"MEDIUMVIOLETRED"]    = 0x8515C7;
		g_mapColorNameToColorUnicode[L"PURPLE"]             = 0x800080;
		g_mapColorNameToColorUnicode[L"LIGHTSTEELBLUE"]     = 0xDEC4B0;
		g_mapColorNameToColorUnicode[L"POWDERBLUE"]         = 0xE6E0B0;
		g_mapColorNameToColorUnicode[L"GOLD"]               = 0x00D7FF;
		g_mapColorNameToColorUnicode[L"YELLOW"]             = 0x00FFFF;
		g_mapColorNameToColorUnicode[L"DARKKHAKI"]          = 0x6BB7BD;
		g_mapColorNameToColorUnicode[L"KHAKI"]              = 0x8CE6F0;
		g_mapColorNameToColorUnicode[L"NAVAJOWHITE"]        = 0xADDEFF;
		
		g_mapColorNameToColorUnicode[L"BISQUE"]             = 0xC4E4FF;
		
		// more
		g_strItalicAnsi = "<I>";
		g_strItalicUnicode = L"<I>";
		
		g_strBoldAnsi = "<B>";
		g_strBoldUnicode = L"<B>";
		
		g_strUnderLineAnsi = "<U>";
		g_strUnderLineUnicode = L"<U>";
		
		g_strFontAnsi = "<FONT ";
		g_strFontUnicode = L"<FONT ";
		
		g_strColorAnsi = " COLOR";
		g_strColorUnicode = L" COLOR";
		
		g_strBlankAnsi = " ";
		g_strBlankUnicode = L" ";
	}
	
	static int wtoi(char* pUTF16LE, int iLen)
	{
		char sz[20] = {0};
		CIConvWrapper iConvWrapper("UTF-16LE", "CP1252");
		iConvWrapper.Convert(pUTF16LE, iLen, sz, sizeof(sz));
		return atoi(sz);
	}
};

set<SCodePage> CSrtSubTitleReader::m_setSCodePage;
CSrtSubTitleReader::CSrtSubTitleReader()
{
	InitCodePage();
	Reset();
}

// lxy 2009-12-4 14:39:20  need test by srt files
unsigned char* CSrtSubTitleReader::FindString(const unsigned char* const pStart, int iMaxLen, const unsigned char* const pStrWillBeFind, int iStrWillBeFindLen)
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

int CSrtSubTitleReader::StringToIntUnicode(const unsigned char* const pStart, const unsigned char* const pEnd)
{
    int iLen = pEnd - pStart;
    if (iLen > 16)//不要超过千万单位
    {
        return 0;
    }
    wchar_t strTemp[9] = {0};
    memcpy(strTemp, pStart, iLen*2);
	return CSrtSubTitleReaderAssist<char>::wtoi((char*)strTemp, sizeof(strTemp));
}

int CSrtSubTitleReader::StringToIntUnicodeBigEndin(const unsigned char* const pStart, const unsigned char* const pEnd)
{
    int iLen = pEnd - pStart;
    if (iLen > 16)//不要超过千万单位
    {
        return 0;
    }
    wchar_t strTemp[9] = {0};
    unsigned char* pStrTemp = (unsigned char*)strTemp;
    unsigned char* pStartTemp = (unsigned char*)pStart;

    for (int i = 0; i< iLen; i+=2)
    {
        *pStrTemp = *(pStartTemp+1);
        pStrTemp += 2;
        pStartTemp += 2;
    }
    return CSrtSubTitleReaderAssist<char>::wtoi((char*)strTemp, sizeof(strTemp));
}

// UTF8的数字表示和Ansi的一样
int CSrtSubTitleReader::StringToIntUTF8(const unsigned char* const pStart, const unsigned char* const pEnd)
{
    int iLen = pEnd - pStart;
    if (iLen > 8)//不要超过千万单位
    {
        return 0;
    }

    char strTemp[9] = {0};
    memcpy(strTemp, pStart, iLen);
    return atoi(strTemp);
}

int CSrtSubTitleReader::StringToIntAnsi(const unsigned char* const pStart, const unsigned char* const pEnd)
{    
    int iLen = pEnd - pStart;
    if (iLen > 8)//不要超过千万单位
    {
        return 0;
    }

    char strTemp[9] = {0};
    memcpy(strTemp, pStart, iLen);
    return atoi(strTemp);
}


bool  CSrtSubTitleReader::StringToTimeUnicode(const unsigned char* const pStart, const unsigned char* const pEnd, int* pTimeStart, int* pTimeEnd)
{
    *pTimeStart = 0;
    *pTimeEnd = 0;

    int* pTime[2] = {pTimeStart, pTimeEnd};    
    int iValidSize = min((pEnd - pStart), 40*2);
   
	wstring strTime;
	char wszTime[400] = {0};
	if (sizeof(wchar_t) == 2)
	{
		memcpy(wszTime, pStart, iValidSize);
		strTime = (wchar_t*)wszTime;
	}
	else if(sizeof(wchar_t) == 4)
	{
		CIConvWrapper iConvWrapper("UTF-32LE", "UTF-16LE");
		iConvWrapper.Convert((char*)pStart, iValidSize, wszTime, sizeof(wszTime));
		strTime = (wchar_t*)wszTime;
	}
	else
	{
		return false;
	}

    vector<wstring> vctString1;
    vector<wstring> vctString2;
    vector<wstring> vctString3;

	CSrtSubTitleReaderAssist<wchar_t>::SplitString(strTime, g_strSep1Unicode, vctString1);
    if (vctString1.size() != 2)
    {
        return false;
    }
    for (int i = 0; i < 2; i ++)
    {
        CSrtSubTitleReaderAssist<wchar_t>::SplitString(vctString1[i], g_strSep2Unicode, vctString2);
        if (vctString2.size() != 3)
        {
            return false;
        }
        CSrtSubTitleReaderAssist<wchar_t>::SplitString(vctString2[2], g_strSep3Unicode, vctString3);
        if (vctString3.size() != 2)
        {
            return false;
        }

        int iHour = CSrtSubTitleReaderAssist<wchar_t>::StringToTime(vctString2[0], 2);
        int iMinute = CSrtSubTitleReaderAssist<wchar_t>::StringToTime(vctString2[1], 2);
        int iSecond = CSrtSubTitleReaderAssist<wchar_t>::StringToTime(vctString3[0], 2);
        int iMinSecond = CSrtSubTitleReaderAssist<wchar_t>::StringToTime(vctString3[1], 3);
        *pTime[i] = iMinSecond + iSecond * 1000 + iMinute * 60 * 1000 + iHour * 60 * 60 * 1000;    
    }
    return true;
}

bool  CSrtSubTitleReader::StringToTimeUnicodeBigEndin(const unsigned char* const pStart, const unsigned char* const pEnd, int* pTimeStart, int* pTimeEnd)
{
    *pTimeStart = 0;
    *pTimeEnd = 0;

    int* pTime[2] = {pTimeStart, pTimeEnd};    
    
	int iValidSize = min((pEnd - pStart), 40*2);
	char szTime[80] = {0};
	memcpy(szTime, pStart, iValidSize);
	char* pszTimeStart = (char*)szTime;
    for (int iCharIndex = 0; iCharIndex < iValidSize; iCharIndex += 2)
    {
		char cTmp = *pszTimeStart;
		pszTimeStart[iCharIndex] = pszTimeStart[iCharIndex + 1];
		pszTimeStart[iCharIndex + 1] = cTmp;
    }
	
    wstring strTime;
	char wszTime[400] = {0};
	if (sizeof(wchar_t) == 2)
	{
		memcpy(wszTime, szTime, iValidSize);
		strTime = (wchar_t*)wszTime;
	}
	else if(sizeof(wchar_t) == 4)
	{
		CIConvWrapper iConvWrapper("UTF-32LE", "UTF-16LE");
		iConvWrapper.Convert((char*)szTime, iValidSize, wszTime, sizeof(wszTime));
		strTime = (wchar_t*)wszTime;
	}
	else
	{
		return false;
	}

    vector<wstring> vctString1;
    vector<wstring> vctString2;
    vector<wstring> vctString3;

    CSrtSubTitleReaderAssist<wchar_t>::SplitString(strTime, g_strSep1Unicode, vctString1);
    if (vctString1.size() != 2)
    {
        return false;
    }
    for (int i = 0; i < 2; i ++)
    {
        CSrtSubTitleReaderAssist<wchar_t>::SplitString(vctString1[i], g_strSep2Unicode, vctString2);
        if (vctString2.size() != 3)
        {
            return false;
        }
        CSrtSubTitleReaderAssist<wchar_t>::SplitString(vctString2[2], g_strSep3Unicode, vctString3);
        if (vctString3.size() != 2)
        {
            return false;
        }

        int iHour = CSrtSubTitleReaderAssist<wchar_t>::StringToTime(vctString2[0], 2);
        int iMinute = CSrtSubTitleReaderAssist<wchar_t>::StringToTime(vctString2[1], 2);
        int iSecond = CSrtSubTitleReaderAssist<wchar_t>::StringToTime(vctString3[0], 2);
        int iMinSecond = CSrtSubTitleReaderAssist<wchar_t>::StringToTime(vctString3[1], 3);
        *pTime[i] = iMinSecond + iSecond * 1000 + iMinute * 60 * 1000 + iHour * 60 * 60 * 1000;    
    }
    return true;
}

bool  CSrtSubTitleReader::StringToTimeUTF8(const unsigned char* const pStart, const unsigned char* const pEnd, int* pTimeStart, int* pTimeEnd)
{
    return StringToTimeAnsi(pStart, pEnd, pTimeStart, pTimeEnd);
}

bool  CSrtSubTitleReader::StringToTimeAnsi(const unsigned char* const pStart, const unsigned char* const pEnd, int* pTimeStart, int* pTimeEnd)
{
    *pTimeStart = 0;
    *pTimeEnd = 0;

    int* pTime[2] = {pTimeStart, pTimeEnd};    
    int iValidSize = min((pEnd - pStart), 40);
    string strTime;
    strTime.resize(iValidSize + 1);
    memcpy((char*)strTime.c_str(), pStart, iValidSize);

    vector<string> vctString1;
    vector<string> vctString2;
    vector<string> vctString3;
      
    CSrtSubTitleReaderAssist<char>::SplitString(strTime, g_strSep1Ansi, vctString1);
    if (vctString1.size() != 2)
    {
        return false;
    }
    for (int i = 0; i < 2; i ++)
    {
        CSrtSubTitleReaderAssist<char>::SplitString(vctString1[i], g_strSep2Ansi, vctString2);
        if (vctString2.size() != 3)
        {
            return false;
        }
        CSrtSubTitleReaderAssist<char>::SplitString(vctString2[2], g_strSep3Ansi, vctString3);
        if (vctString3.size() != 2)
        {
            return false;
        }
        
        int iHour = CSrtSubTitleReaderAssist<char>::StringToTime(vctString2[0], 2);
        int iMinute = CSrtSubTitleReaderAssist<char>::StringToTime(vctString2[1], 2);
        int iSecond = CSrtSubTitleReaderAssist<char>::StringToTime(vctString3[0], 2);
        int iMinSecond = CSrtSubTitleReaderAssist<char>::StringToTime(vctString3[1], 3);
        *pTime[i] = iMinSecond + iSecond * 1000 + iMinute * 60 * 1000 + iHour * 60 * 60 * 1000;    
    }
    return true;
}



// 每一个Item，以数字开始，以空白行结束,例如，而且序号部分和时间部分的格式固定
/* 
2
00:00:22,310 --> 00:00:24,510
变形金刚家喻户晓

*/
bool CSrtSubTitleReader::AnalySrtSubTitleItem(unsigned char* const pStart, int iMaxLen, SSrtSubTitleItemReaded& sSrtSubTitleItemReaded, int& iLenUsed, bool& bValid)
{
    bValid = false;
    // m_pNextLineFlag m_iNextLineFlagLen
    const unsigned char* pFindStart = pStart;
    unsigned char* const pFindEnd = pStart + iMaxLen;
    unsigned char* pFindPos = NULL;
    

    // 去掉多余的换行符号
    while (true)
    {
        pFindPos = FindString(pFindStart, iMaxLen, m_pNextLineFlag, m_iNextLineFlagLen);
        if (pFindPos != pFindStart)
        {
            break;
        }
        else
        {
            pFindStart += m_iNextLineFlagLen;
        }
    }
    if (pFindPos == NULL)
    {
        return false;
    }

    // 在pFindPos和pFindStart之间的是序号
    sSrtSubTitleItemReaded.iIndex = m_pFunStringToInt(pFindStart, pFindPos);
    pFindStart = pFindPos + m_iNextLineFlagLen; // 运行这句后，pFindStart指向了"00:00:22,310 --> 00:00:24,510"
    pFindPos = FindString(pFindStart, pFindEnd - pFindStart, m_pNextLineFlag, m_iNextLineFlagLen); 
    if (pFindPos == NULL)
    {
        return false;
    }
    
    // 得到显示的时间信息
    bValid = m_pFunStringToTime(pFindStart, pFindPos, &sSrtSubTitleItemReaded.iStartTime, &sSrtSubTitleItemReaded.iEndTime);
    pFindStart = pFindPos + m_iNextLineFlagLen; // 运行这句后，pFindStart指向了"变形金刚家喻户晓"
    
    // 得到文本
    const unsigned char* const pTextStart = pFindStart;
    while (true)
    {
        pFindPos = FindString(pFindStart, pFindEnd - pFindStart, m_pNextLineFlag, m_iNextLineFlagLen);
        if (pFindPos == NULL)
        {
            return false;
        }
        if (pFindPos == pFindStart)
        {
            pFindStart = pFindPos + m_iNextLineFlagLen;
            break; //找到了空白的换行
        }
        else
        {
            pFindStart = pFindPos + m_iNextLineFlagLen;
        }
    }

    // 这时pFindStart 指向了空白行的下一行，pFindPos指向了空白行，pTextStart指向了文本的开始
    int iTextLen = pFindPos - pTextStart;
    sSrtSubTitleItemReaded.strSubTitleReaded.resize(iTextLen);
    memcpy((char*)sSrtSubTitleItemReaded.strSubTitleReaded.c_str(), pTextStart, iTextLen);
    iLenUsed = pFindStart - pStart;    
    return true;
}

bool CSrtSubTitleReader::LoadFile(const string& strSrtFileName)
{
    Reset();
    m_bLoaded = false;
    list<SSrtSubTitleItemReaded> lstSrtSubTitleItemReaded;  //用于排序
    
	SFileFormatType sFileFormatType = GuessFileFormatType(strSrtFileName);
	if (sFileFormatType.eTextEncode == eTextEncodeUnicode)
	{
		m_eTextEncode = eTextEncodeUnicode;                        
		m_pFunStringToInt = &CSrtSubTitleReader::StringToIntUnicode;
		m_pFunStringToTime = &CSrtSubTitleReader::StringToTimeUnicode;
		if (sFileFormatType.eEndofLine == eEndOfLineWindows)
		{
			m_pNextLineFlag = &g_strEndOfLineUnicodeWindows[0];
			m_iNextLineFlagLen = 4;
		}
		else if (sFileFormatType.eEndofLine == eEndOfLineMac)
		{
			m_pNextLineFlag = &g_strEndOfLineUnicodeMac[0];
			m_iNextLineFlagLen = 2;
		}
		else if (sFileFormatType.eEndofLine == eEndOfLineUnix)
		{
			m_pNextLineFlag = &g_strEndOfLineUnicodeUnix[0];
			m_iNextLineFlagLen = 2;
		}
		else
		{
			// 不应该运行到这里
		}
	}
	else if (sFileFormatType.eTextEncode == eTextEncodeUnicodeBigEndian)
	{
		m_eTextEncode = eTextEncodeUnicodeBigEndian;            
		m_pFunStringToInt = &CSrtSubTitleReader::StringToIntUnicodeBigEndin;
		m_pFunStringToTime = &CSrtSubTitleReader::StringToTimeUnicodeBigEndin;

		if (sFileFormatType.eEndofLine == eEndOfLineWindows)
		{
			m_pNextLineFlag = &g_strEndOfLineUnicodeBigEndianWindows[0];
			m_iNextLineFlagLen = 4;
		}
		else if (sFileFormatType.eEndofLine == eEndOfLineMac)
		{
			m_pNextLineFlag = &g_strEndOfLineUnicodeBigEndianMac[0];
			m_iNextLineFlagLen = 2;
		}
		else if (sFileFormatType.eEndofLine == eEndOfLineUnix)
		{
			m_pNextLineFlag = &g_strEndOfLineUnicodeBigEndianUnix[0];
			m_iNextLineFlagLen = 2;
		}
		else
		{
			// 不应该运行到这里
		}            
	}        
	else if (sFileFormatType.eTextEncode == eTextEncodeUTF8)
	{
		m_eTextEncode = eTextEncodeUTF8;
		m_pFunStringToInt = &CSrtSubTitleReader::StringToIntUTF8;
		m_pFunStringToTime = &CSrtSubTitleReader::StringToTimeUTF8;                         

		if (sFileFormatType.eEndofLine == eEndOfLineWindows)
		{
			m_pNextLineFlag = &g_strEndOfLineAnsiWindows[0];
			m_iNextLineFlagLen = 2;
		}
		else if (sFileFormatType.eEndofLine == eEndOfLineMac)
		{
			m_pNextLineFlag = &g_strEndOfLineAnsiMac[0];
			m_iNextLineFlagLen = 1;
		}
		else if (sFileFormatType.eEndofLine == eEndOfLineUnix)
		{
			m_pNextLineFlag = &g_strEndOfLineAnsiUnix[0];
			m_iNextLineFlagLen = 1;
		}
		else
		{
			// 不应该运行到这里
		}           
	}
	else
	{            
		if (sFileFormatType.eCodePage == eCodePageEnglish)
		{                
			m_eTextEncode = eTextEncodeAnsi;
			m_pFunStringToInt = &CSrtSubTitleReader::StringToIntAnsi;
			m_pFunStringToTime = &CSrtSubTitleReader::StringToTimeAnsi;
		}
		else if (sFileFormatType.eCodePage == eCodePageOEM)
		{                
			m_eTextEncode = eTextEncodeAnsi;
			m_pFunStringToInt = &CSrtSubTitleReader::StringToIntAnsi;
			m_pFunStringToTime = &CSrtSubTitleReader::StringToTimeAnsi;
		}
		else if (sFileFormatType.eCodePage == eCodePageUnicode)
		{
			m_eTextEncode = eTextEncodeUTF8;                
			m_pFunStringToInt = &CSrtSubTitleReader::StringToIntUTF8;
			m_pFunStringToTime = &CSrtSubTitleReader::StringToTimeUTF8;
		}

		if (sFileFormatType.eEndofLine == eEndOfLineWindows)
		{
			m_pNextLineFlag = &g_strEndOfLineAnsiWindows[0];
			m_iNextLineFlagLen = 2;
		}
		else if (sFileFormatType.eEndofLine == eEndOfLineMac)
		{
			m_pNextLineFlag = &g_strEndOfLineAnsiMac[0];
			m_iNextLineFlagLen = 1;
		}
		else if (sFileFormatType.eEndofLine == eEndOfLineUnix)
		{
			m_pNextLineFlag = &g_strEndOfLineAnsiUnix[0];
			m_iNextLineFlagLen = 1;
		}
		else
		{
			// 不应该运行到这里
		}           
	}
	
	// 开始分析
	SSrtSubTitleItemReaded sSrtSubTitleItemReaded;        
	sSrtSubTitleItemReaded.iCodePage = 0;
	// MAX_BUFFER_LENGTH 必须要大于一个项目的字符个数，一个项目的字符个数的最大值估计有200字节
#define MAX_BUFFER_LENGTH 4096
	unsigned char strReaded[MAX_BUFFER_LENGTH] = {0};    

	int iReaded = 0;
	int iTotalAnaliedCount = sFileFormatType.iBOMFlagCount;

	FILE* pFile = fopen(strSrtFileName.c_str(), "rb");
	if (pFile == NULL)
	{
		return false;
	}
	fseek(pFile, 0, SEEK_END); 
	const int iLength = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	

	bool bErrorHappened = false;
	while (((iReaded = fread(strReaded, 1, MAX_BUFFER_LENGTH, pFile)) > 0) && !bErrorHappened)
	{
		int iAnaliedCount = 0;     
		if (iTotalAnaliedCount + iReaded >= iLength)
		{
			//读完了
			if (iReaded > MAX_BUFFER_LENGTH - 4)    //可能会丢失"\r\n",宁愿多读一次
			{
				iReaded = MAX_BUFFER_LENGTH - 4;
			}
			else //文件末尾可能没有"\r\n",手工加上
			{
				memcpy(&strReaded[iReaded], m_pNextLineFlag, m_iNextLineFlagLen);
				iReaded += m_iNextLineFlagLen;
			}
		}
		while (iAnaliedCount <= iReaded)
		{
			int iUsed = 0;
			bool bValid = false;
			if (AnalySrtSubTitleItem(&strReaded[iAnaliedCount], iReaded - iAnaliedCount, sSrtSubTitleItemReaded, iUsed, bValid))
			{
				// 成功了
				iAnaliedCount += iUsed;
				if (bValid)
				{                        
					lstSrtSubTitleItemReaded.push_back(sSrtSubTitleItemReaded);    
				}                    
				if (lstSrtSubTitleItemReaded.size() == 63)
				{
					int i = 0;
					i++;
				}
			}
			else
			{
				// 失败说明剩下的不够生成一个，这个时候需要调整文件指针重新读，如果是第一次就失败，说明这个文件有问题，不要继续分析了
				if (iAnaliedCount == 0)
				{
					bErrorHappened = true; //不分析了，发生错误
					break;
				}
				else
				{
					iTotalAnaliedCount += iAnaliedCount;
					fseek(pFile, iTotalAnaliedCount, SEEK_SET);//调整文件指针
					break;
				}
			}// end if (ReadSrtSubTitleItem(...))
		} // end  while (iAnaliedCount < iReaded)        
	} // end while (((iReaded = file.Read(strReaded, MAX_BUFFER_LENGTH)) > 0) && !bErrorHappened)
    
    fclose(pFile);
    lstSrtSubTitleItemReaded.sort();
    m_vctSrtSubTitleItemReaded.assign(lstSrtSubTitleItemReaded.begin(), lstSrtSubTitleItemReaded.end());
	int iSubTitleCount = m_vctSrtSubTitleItemReaded.size();
    return (iSubTitleCount > 0);
}

void CSrtSubTitleReader::SetCodePage(int iCodePage)
{
    m_iCodePage = iCodePage;
}

int CSrtSubTitleReader::GetDuration()
{
    if (m_vctSrtSubTitleItemReaded.size() == 0)
    {
        return 0;
    }
    return m_vctSrtSubTitleItemReaded.rbegin()->iEndTime - m_iOffset;
}

void CSrtSubTitleReader::GetString(int iTime, wstring& strSubTitleItem, SSubTitleFormat* pSubTitleFormat/* = NULL*/)
{
    iTime += m_iOffset;
    strSubTitleItem.clear();
    const int iCount = m_vctSrtSubTitleItemReaded.size();
    if (iCount == 0)
    {
        return;
    }

    // 先检查缓存
    if (m_iLastGetStringCache < iCount)
    {
        SSrtSubTitleItemReaded* pSrtSubTitleItemReaded = &(m_vctSrtSubTitleItemReaded[m_iLastGetStringCache]);
        if (iTime >= pSrtSubTitleItemReaded->iStartTime && iTime <= pSrtSubTitleItemReaded->iEndTime)
        {
            // 找到了，就地解决
            ConvertStringToDecoded(*pSrtSubTitleItemReaded);
            strSubTitleItem = pSrtSubTitleItemReaded->strSubTitleDecoded;
			CSrtSubTitleReaderAssist<wchar_t>::GetSrtStyles(strSubTitleItem, pSubTitleFormat);
            return;
        }
#define SRT_SUBTITLE_REFERENCE_COUNT 5
        // 如果最后的缓存没有找到，先找找缓存的前5个和后5个，看能不能命中，在编码或者播放阶段，这种情况很普遍     
        if (iTime < pSrtSubTitleItemReaded->iStartTime)
        {            
            // 往前找
            int iLastGetStringCache = m_iLastGetStringCache;
            for (int i = 0; i < SRT_SUBTITLE_REFERENCE_COUNT; i++)
            {
                iLastGetStringCache --;
                if (iLastGetStringCache < 0)
                {
                    break;
                }
                SSrtSubTitleItemReaded* pSrtSubTitleItemReaded = &(m_vctSrtSubTitleItemReaded[m_iLastGetStringCache]);
                if (iTime > pSrtSubTitleItemReaded->iEndTime)
                {
                    m_iLastGetStringCache = iLastGetStringCache;
                    return; //当前时间处于空白期
                }
                else if (iTime >= pSrtSubTitleItemReaded->iStartTime)
                {
                    m_iLastGetStringCache = iLastGetStringCache;//找到了，保存缓存
                    ConvertStringToDecoded(*pSrtSubTitleItemReaded);    
                    strSubTitleItem = pSrtSubTitleItemReaded->strSubTitleDecoded;
					CSrtSubTitleReaderAssist<wchar_t>::GetSrtStyles(strSubTitleItem, pSubTitleFormat);
                    return; //当前时间处于空白期
                }
            }
        }
        else // if (iTime > pSrtSubTitleItemReaded->iEndTime)
        {
            // 往后找
            int iLastGetStringCache = m_iLastGetStringCache;
            for (int i = 0; i < SRT_SUBTITLE_REFERENCE_COUNT; i++)
            {
                iLastGetStringCache ++;
                if (iLastGetStringCache > iCount - 1)
                {
                    break;
                }
                SSrtSubTitleItemReaded* pSrtSubTitleItemReaded = &(m_vctSrtSubTitleItemReaded[m_iLastGetStringCache]);
                if (iTime < pSrtSubTitleItemReaded->iStartTime)
                {
                    m_iLastGetStringCache = iLastGetStringCache;
                    return; //当前时间处于空白期
                }
                else if (iTime <= pSrtSubTitleItemReaded->iEndTime)
                {
                    m_iLastGetStringCache = iLastGetStringCache;//找到了，保存缓存
                    ConvertStringToDecoded(*pSrtSubTitleItemReaded);    
                    strSubTitleItem = pSrtSubTitleItemReaded->strSubTitleDecoded;
					CSrtSubTitleReaderAssist<wchar_t>::GetSrtStyles(strSubTitleItem, pSubTitleFormat);
                    return; //当前时间处于空白期
                }
            }
        }
    }

    

    // 这个查找方法有个新名词叫做差不多查找吧，根据当前时间和总时间，定位到一个差不多的位置，然后向前或者向后查找
    int iDuration = GetDuration();
    int iMayBePos = iTime * 1.0 / iDuration * iCount;
    iMayBePos = max(min(iMayBePos, iCount - 1), 0);
    SSrtSubTitleItemReaded* pSrtSubTitleItemReaded = &(m_vctSrtSubTitleItemReaded[iMayBePos]);
    if (iTime >= pSrtSubTitleItemReaded->iStartTime && iTime <= pSrtSubTitleItemReaded->iEndTime)
    {
        //刚好这么巧找到了
        ConvertStringToDecoded(*pSrtSubTitleItemReaded);
        strSubTitleItem = pSrtSubTitleItemReaded->strSubTitleDecoded;
		CSrtSubTitleReaderAssist<wchar_t>::GetSrtStyles(strSubTitleItem, pSubTitleFormat);
        m_iLastGetStringCache = iMayBePos;
        return;
    }
    else if (iTime < pSrtSubTitleItemReaded->iStartTime)
    {
        //向前查找
        for (iMayBePos --; iMayBePos >= 0; iMayBePos --)
        {
            SSrtSubTitleItemReaded* pSrtSubTitleItemReaded = &(m_vctSrtSubTitleItemReaded[iMayBePos]);
            if (iTime > pSrtSubTitleItemReaded->iEndTime)
            {
                m_iLastGetStringCache = iMayBePos;
                return; //当前时间处于空白期
            }
            else if (iTime >= pSrtSubTitleItemReaded->iStartTime)
            {
                //找到了
                m_iLastGetStringCache = iMayBePos;
                ConvertStringToDecoded(*pSrtSubTitleItemReaded);                  
                strSubTitleItem = pSrtSubTitleItemReaded->strSubTitleDecoded;
                CSrtSubTitleReaderAssist<wchar_t>::GetSrtStyles(strSubTitleItem, pSubTitleFormat);
                return; //当前时间处于空白期
            }
        }
    }
    else // if (iTime > pSrtSubTitleItemReaded->iEndTime)
    {
        //向后查找
        for (iMayBePos ++; iMayBePos < iCount; iMayBePos ++)
        {
            SSrtSubTitleItemReaded* pSrtSubTitleItemReaded = &(m_vctSrtSubTitleItemReaded[iMayBePos]);
            if (iTime < pSrtSubTitleItemReaded->iStartTime)
            {
                m_iLastGetStringCache = iMayBePos;
                return; //当前时间处于空白期
            }
            else if (iTime <= pSrtSubTitleItemReaded->iEndTime)
            {
                //找到了
                m_iLastGetStringCache = iMayBePos;
                ConvertStringToDecoded(*pSrtSubTitleItemReaded);    
                strSubTitleItem = pSrtSubTitleItemReaded->strSubTitleDecoded;
				CSrtSubTitleReaderAssist<wchar_t>::GetSrtStyles(strSubTitleItem, pSubTitleFormat);
                return; //当前时间处于空白期
            }
        }
    }
}


// srt文件是用Unicode存的吗，是eTextEncodeAnsi返回false, 其余返回true
bool CSrtSubTitleReader::IsUnicodeFileType()
{
    if (m_eTextEncode == eTextEncodeAnsi)
    {
        return false;
    }
    return true;
}

void CSrtSubTitleReader::ConvertStringToDecoded(SSrtSubTitleItemReaded& sSrtSubTitleItemReaded)
{       
    if (m_eTextEncode == eTextEncodeAnsi)
    {
        if (sSrtSubTitleItemReaded.strSubTitleDecoded.size() != 0 &&  (sSrtSubTitleItemReaded.iCodePage == m_iCodePage))
        {
            return; // no need deal with
        }

		sSrtSubTitleItemReaded.strSubTitleDecoded.resize(sSrtSubTitleItemReaded.strSubTitleReaded.size() + 1);
		sSrtSubTitleItemReaded.strSubTitleDecoded[sSrtSubTitleItemReaded.strSubTitleDecoded.size() - 1] = 0;
		const char* pConvToFormat = NULL;
		if (sizeof(wchar_t) == 2)
		{
			pConvToFormat = "UTF-16LE";
		}
		else if(sizeof(wchar_t) == 4)
		{
			pConvToFormat = "UTF-32LE";
		}
		else
		{
			return;
		}
		const char* pCodePageFrom = GetCodePage(m_iCodePage).c_str();
		CIConvWrapper iConvWrpper(pConvToFormat, pCodePageFrom);
		iConvWrpper.Convert((char*)sSrtSubTitleItemReaded.strSubTitleReaded.c_str(), sSrtSubTitleItemReaded.strSubTitleReaded.size(),
							(char*)sSrtSubTitleItemReaded.strSubTitleDecoded.c_str(), sSrtSubTitleItemReaded.strSubTitleDecoded.size() * sizeof(wchar_t));
		
		sSrtSubTitleItemReaded.iCodePage = m_iCodePage;
    }
    else if ((m_eTextEncode == eTextEncodeUnicode) || (m_eTextEncode == eTextEncodeUnicodeBigEndian))
    {
		string strReaded;
		if (m_eTextEncode == eTextEncodeUnicodeBigEndian)
		{
			// Unicode Big Endian 读的和解出来的的数据每两个字节是颠倒的，需要转换
			strReaded = sSrtSubTitleItemReaded.strSubTitleReaded;
			int iReadedLen = strReaded.size();
			for (int i = 0; i < iReadedLen / 2; i ++)
			{
				unsigned short iShort = strReaded[i];
				iShort = (iShort << 8) | (iShort >> 8); // 颠倒位置
				strReaded[i] = iShort;
			}  
		}
		else
		{
			strReaded = sSrtSubTitleItemReaded.strSubTitleReaded;
		}

		
        if (sizeof(wchar_t) == 2)
		{
			if (sSrtSubTitleItemReaded.strSubTitleDecoded.size() == 0)
			{
				int iReadedLen = strReaded.size();
				sSrtSubTitleItemReaded.strSubTitleDecoded.resize(iReadedLen / 2 + 1);
				sSrtSubTitleItemReaded.strSubTitleDecoded[sSrtSubTitleItemReaded.strSubTitleDecoded.size() - 1] = 0;
				memcpy((char*)sSrtSubTitleItemReaded.strSubTitleDecoded.c_str(), 
							   strReaded.c_str(), 
							   iReadedLen);   
			}
		}
		else if(sizeof(wchar_t) == 4)
		{
			if (sSrtSubTitleItemReaded.strSubTitleDecoded.size() == 0)
			{
				int iReadedLen = strReaded.size();
				sSrtSubTitleItemReaded.strSubTitleDecoded.resize(iReadedLen / 2 + 1);
				sSrtSubTitleItemReaded.strSubTitleDecoded[sSrtSubTitleItemReaded.strSubTitleDecoded.size() - 1] = 0;
				
				CIConvWrapper iConvWrpper("UTF-32LE", "UTF-16LE");
				iConvWrpper.Convert((char*)strReaded.c_str(), iReadedLen,
									(char*)sSrtSubTitleItemReaded.strSubTitleDecoded.c_str(), sSrtSubTitleItemReaded.strSubTitleDecoded.size() * sizeof(wchar_t));
			}
		}
		else
		{
			return;
		}
    }
    else // if (m_eTextEncode == eTextEncodeUTF8)
    {
        
        if (sSrtSubTitleItemReaded.strSubTitleDecoded.size() == 0)
        {
			int iReadedLen = sSrtSubTitleItemReaded.strSubTitleReaded.size();
			sSrtSubTitleItemReaded.strSubTitleDecoded.resize(iReadedLen);
			sSrtSubTitleItemReaded.strSubTitleDecoded[sSrtSubTitleItemReaded.strSubTitleDecoded.size() - 1] = 0;
			const char* pConvToFormat = NULL;
			if (sizeof(wchar_t) == 2)
			{
				pConvToFormat = "UTF-16LE";
			}
			else if(sizeof(wchar_t) == 4)
			{
				pConvToFormat = "UTF-32LE";
			}
			else
			{
				return;
			}
			CIConvWrapper iConvWrpper(pConvToFormat, "UTF-8");
			iConvWrpper.Convert((char*)sSrtSubTitleItemReaded.strSubTitleReaded.c_str(), iReadedLen,
								(char*)sSrtSubTitleItemReaded.strSubTitleDecoded.c_str(), sSrtSubTitleItemReaded.strSubTitleDecoded.size() * sizeof(wchar_t));
        }        
    }
    
}

void CSrtSubTitleReader::CopyReadedToDecoded(SSrtSubTitleItemReaded& sSrtSubTitleItemReaded, SSrtSubTitleItemDecoded& sSrtSubTitleItemDecoded)
{
    sSrtSubTitleItemDecoded.iIndex = sSrtSubTitleItemReaded.iIndex;
    sSrtSubTitleItemDecoded.iStartTime = sSrtSubTitleItemReaded.iStartTime - m_iOffset;
    sSrtSubTitleItemDecoded.iEndTime = sSrtSubTitleItemReaded.iEndTime - m_iOffset;    
    sSrtSubTitleItemDecoded.strSubTitleDecoded = sSrtSubTitleItemReaded.strSubTitleDecoded;
    CSrtSubTitleReaderAssist<wchar_t>::GetSrtStyles(sSrtSubTitleItemDecoded.strSubTitleDecoded, &sSrtSubTitleItemDecoded.sSubTitleFormat);    
}

void CSrtSubTitleReader::Reset()
{
    m_bLoaded = false;
    m_eTextEncode = eTextEncodeAnsi;
    m_iCodePage = WESTERN_EUROPEAN_WINDOWS_CODE_PAGE;
    m_pNextLineFlag = NULL;
    m_iNextLineFlagLen = 0;
    m_pFunStringToInt = NULL;
    m_pFunStringToTime = NULL;    
    m_iLastGetStringCache = NULL;
    m_vctSrtSubTitleItemReaded.clear();
    m_iOffset = 0;
	strCodePage = GetCodePage(m_iCodePage);
}

int CSrtSubTitleReader::GetCount()
{
    return m_vctSrtSubTitleItemReaded.size();
}

void CSrtSubTitleReader::GetIndex(int iIndex, SSrtSubTitleItemDecoded& sSrtSubTitleDecoded)
{    
    if (iIndex < 0 || iIndex >= m_vctSrtSubTitleItemReaded.size())
    {
        return;
    }
    SSrtSubTitleItemReaded* pSrtSubTitleItemReaded = &m_vctSrtSubTitleItemReaded[iIndex];
    ConvertStringToDecoded(*pSrtSubTitleItemReaded);
    CopyReadedToDecoded(*pSrtSubTitleItemReaded, sSrtSubTitleDecoded);
}

void CSrtSubTitleReader::SetOffset(int iOffset)
{
    m_iOffset = iOffset;
}

bool CSrtSubTitleReader::IsEnglishSubTitle()
{
#define PRE_READ_COUNT 3
    int iPreReadCount = min(PRE_READ_COUNT, (int)m_vctSrtSubTitleItemReaded.size());
    int iPreReadIndex = 0;
    bool bEnglish = true;

    wstring strSubTitle;
    for (iPreReadIndex = 0; iPreReadIndex < iPreReadCount; iPreReadIndex ++)
    {
        if (!bEnglish)
        {
            break;
        }
        SSrtSubTitleItemReaded* pSrtSubTitleItemReaded = &m_vctSrtSubTitleItemReaded[iPreReadIndex];
        ConvertStringToDecoded(*pSrtSubTitleItemReaded);
        strSubTitle = pSrtSubTitleItemReaded->strSubTitleDecoded;        
        int iCharCount = strSubTitle.size();
        int iCharIndex = 0;
        for (iCharIndex = 0; iCharIndex < iCharCount; iCharIndex ++)
        {
            unsigned short iChar = strSubTitle[iCharIndex];
            if (iChar > 0xFF)
            {
                bEnglish = false;
                break;
            }                    
        }
    } // e
    return bEnglish;
}


SFileFormatType CSrtSubTitleReader::GuessFileFormatType(const string& strFileName)
{
    SFileFormatType sFileFormatType;
    sFileFormatType.eCodePage = eCodePageEnglish;
    sFileFormatType.eEndofLine = eEndOfLineWindows;
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
		return sFileFormatType;
	}

	// 确定编码格式
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
	sFileFormatType.eEndofLine = GuessEndOfLine(sReaded, iLenRead, sFileFormatType.eTextEncode);         

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
EnumGuessEncode CSrtSubTitleReader::GuessTextEncode(unsigned char* pText, int iLen)
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

EnumEndOfLine CSrtSubTitleReader::GuessEndOfLine(unsigned char* pText, int iLen, EnumTextEncode eTextEncode)
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


const set<SCodePage>& CSrtSubTitleReader::GetCodePage()
{
	InitCodePage();
	return m_setSCodePage;
}

void CSrtSubTitleReader::InitCodePage()
{
	if (m_setSCodePage.size() != 0)
	{
		return;
	}
	int iCustomCodePageNo = 0x1000000;
	m_setSCodePage.insert(SCodePage("European", iCustomCodePageNo++, "ASCII", "ASCII"));
	m_setSCodePage.insert(SCodePage("European", 28591, "ISO-8859-1", "Western European(28591)"));
	m_setSCodePage.insert(SCodePage("European", 28592, "ISO-8859-2", "Central European(28592)"));
	m_setSCodePage.insert(SCodePage("European", 28593, "ISO-8859-3", "Latin 3(28593)"));
	m_setSCodePage.insert(SCodePage("European", 28594, "ISO-8859-4", "Baltic(28594)"));
	m_setSCodePage.insert(SCodePage("European", 28595, "ISO-8859-5", "Cyrillic(28595)"));
	m_setSCodePage.insert(SCodePage("European", 28597, "ISO-8859-7", "Greek(28597)"));
	m_setSCodePage.insert(SCodePage("European", 28599, "ISO-8859-9", "Turkish(28599)"));
	m_setSCodePage.insert(SCodePage("European", iCustomCodePageNo++, "ISO-8859-10", "ISO-8859-10"));
	m_setSCodePage.insert(SCodePage("European", 28603, "ISO-8859-13", "Estonian(28603)"));
	m_setSCodePage.insert(SCodePage("European", iCustomCodePageNo++, "ISO-8859-14", "ISO-8859-14"));
	m_setSCodePage.insert(SCodePage("European", 28605, "ISO-8859-15", "Latin 9(28605)"));
	m_setSCodePage.insert(SCodePage("European", iCustomCodePageNo++, "ISO-8859-16", "ISO-8859-16"));
	m_setSCodePage.insert(SCodePage("European", 20866, "KOI8-R", "Russian (KOI8-R); Cyrillic (KOI8-R)"));
	m_setSCodePage.insert(SCodePage("European", 21866, "KOI8-U", "Ukrainian (KOI8-U); Cyrillic (KOI8-U)"));
	m_setSCodePage.insert(SCodePage("European", iCustomCodePageNo++, "KOI8-RU", "KOI8-RU"));
	m_setSCodePage.insert(SCodePage("European", 1250, "CP1250", "Central European(1250)"));
	m_setSCodePage.insert(SCodePage("European", 1251, "CP1251", "Cyrillic(1251)"));
	m_setSCodePage.insert(SCodePage("European", 1252, "CP1252", "Western European(1252)"));
	m_setSCodePage.insert(SCodePage("European", 1253, "CP1253", "Greek(1253)"));
	m_setSCodePage.insert(SCodePage("European", 1254, "CP1254", "Turkish(1254)"));
	m_setSCodePage.insert(SCodePage("European", 1257, "CP1257", "Baltic(1257)"));
	m_setSCodePage.insert(SCodePage("European", 850, "CP850", "Western European(850)"));
	m_setSCodePage.insert(SCodePage("European", 866, "CP866", "Russian; Cyrillic(866)"));
	m_setSCodePage.insert(SCodePage("European", iCustomCodePageNo++, "CP1131", "CP1131"));
	m_setSCodePage.insert(SCodePage("European", 10000, "MacRoman", "Western European (Mac)"));
	m_setSCodePage.insert(SCodePage("European", 10029, "MacCentralEurope", "Central European (Mac)"));
	m_setSCodePage.insert(SCodePage("European", 10079, "MacIceland", "Icelandic (Mac)"));
	m_setSCodePage.insert(SCodePage("European", 10082, "MacCroatian", "Croatian (Mac)"));
	m_setSCodePage.insert(SCodePage("European", 10010, "MacRomania", "Romanian (Mac)"));
	m_setSCodePage.insert(SCodePage("European", 10007, "MacCyrillic", "Cyrillic (Mac)"));
	m_setSCodePage.insert(SCodePage("European", 10017, "MacUkraine", "Ukrainian (Mac)"));
	m_setSCodePage.insert(SCodePage("European", 10006, "MacGreek", "Greek (Mac)"));
	m_setSCodePage.insert(SCodePage("European", 10081, "MacTurkish", "Turkish (Mac)"));
	m_setSCodePage.insert(SCodePage("European", iCustomCodePageNo++, "Macintosh", "Macintosh"));
	
	m_setSCodePage.insert(SCodePage("Semitic", 28596, "ISO-8859-6", "Arabic(28596)"));
	m_setSCodePage.insert(SCodePage("Semitic", 28598, "ISO-8859-8", "Hebrew (ISO-Visual)"));
	m_setSCodePage.insert(SCodePage("Semitic", 1255, "CP1255", "Hebrew (Windows)"));
	m_setSCodePage.insert(SCodePage("Semitic", 1256, "CP1256", "Arabic (Windows)"));
	m_setSCodePage.insert(SCodePage("Semitic", 862, "CP862", "Hebrew (DOS)"));
	m_setSCodePage.insert(SCodePage("Semitic", 10005, "MacHebrew", "Hebrew (Mac)"));
	m_setSCodePage.insert(SCodePage("Semitic", 10004, "MacArabic", "Arabic (Mac)"));
	
	m_setSCodePage.insert(SCodePage("Japanese", 20932, "EUC-JP", "Japanese (JIS 0208-1990 and 0121-1990)"));
	m_setSCodePage.insert(SCodePage("Japanese", 932, "SHIFT_JIS", "Japanese (Shift-JIS)"));
	m_setSCodePage.insert(SCodePage("Japanese", iCustomCodePageNo++, "CP932", "CP932"));
	m_setSCodePage.insert(SCodePage("Japanese", 50220, "ISO-2022-JP", "Japanese (JIS)"));
	m_setSCodePage.insert(SCodePage("Japanese", iCustomCodePageNo++, "ISO-2022-JP-2", "ISO-2022-JP-2"));
	m_setSCodePage.insert(SCodePage("Japanese", iCustomCodePageNo++, "ISO-2022-JP-1", "ISO-2022-JP-1"));
	m_setSCodePage.insert(SCodePage("Japanese", iCustomCodePageNo++, "EUC-JISX0213", "EUC-JISX0213"));
	m_setSCodePage.insert(SCodePage("Japanese", iCustomCodePageNo++, "Shift_JISX0213", "Shift_JISX0213"));
	m_setSCodePage.insert(SCodePage("Japanese", iCustomCodePageNo++, "ISO-2022-JP-3", "ISO-2022-JP-3"));
	
	m_setSCodePage.insert(SCodePage("Chinese", 51936, "EUC-CN", "Chinese Simplified (EUC)"));
	m_setSCodePage.insert(SCodePage("Chinese", 52936, "HZ", "Chinese Simplified (HZ)"));
	m_setSCodePage.insert(SCodePage("Chinese", iCustomCodePageNo++, "GBK", "GBK"));
	m_setSCodePage.insert(SCodePage("Chinese", 936, "CP936", "Simplified Chinese (PRC, Singapore, GB2312)"));
	m_setSCodePage.insert(SCodePage("Chinese", 54936, "GB18030", "Chinese Simplified (GB18030)"));
	m_setSCodePage.insert(SCodePage("Chinese", iCustomCodePageNo++, "EUC-TW", "EUC-TW"));
	m_setSCodePage.insert(SCodePage("Chinese", 950, "BIG5", "Traditional Chinese (Big5, Taiwan; Hong Kong SAR, PRC)"));
	m_setSCodePage.insert(SCodePage("Chinese", iCustomCodePageNo++, "CP950", "CP950"));
	m_setSCodePage.insert(SCodePage("Chinese", iCustomCodePageNo++, "BIG5-HKSCS", "BIG5-HKSCS"));
	m_setSCodePage.insert(SCodePage("Chinese", iCustomCodePageNo++, "BIG5-HKSCS:2001", "BIG5-HKSCS:2001"));
	m_setSCodePage.insert(SCodePage("Chinese", iCustomCodePageNo++, "BIG5-HKSCS:1999", "BIG5-HKSCS:1999"));
	m_setSCodePage.insert(SCodePage("Chinese", iCustomCodePageNo++, "ISO-2022-CN", "ISO-2022-CN"));
	m_setSCodePage.insert(SCodePage("Chinese", iCustomCodePageNo++, "ISO-2022-CN-EXT", "ISO-2022-CN-EXT"));
	m_setSCodePage.insert(SCodePage("Chinese", iCustomCodePageNo++, "BIG5-2003", "BIG5-2003"));
	
	m_setSCodePage.insert(SCodePage("Korean", 51949, "EUC-KR", "EUC Korean"));
	m_setSCodePage.insert(SCodePage("Korean", 949, "CP949", "Korean (Unified Hangul Code)"));
	m_setSCodePage.insert(SCodePage("Korean", 50225, "ISO-2022-KR", "Korean ISO 2022 "));
	m_setSCodePage.insert(SCodePage("Korean", 1361, "JOHAB", "Korean (Johab)"));
	
	m_setSCodePage.insert(SCodePage("Armenian", iCustomCodePageNo++, "ARMSCII-8", "ARMSCII-8"));
	
	m_setSCodePage.insert(SCodePage("Georgian", iCustomCodePageNo++, "Georgian-Academy", "Georgian-Academy"));
	m_setSCodePage.insert(SCodePage("Georgian", iCustomCodePageNo++, "Georgian-PS", "Georgian-PS"));
	
	m_setSCodePage.insert(SCodePage("Tajik", iCustomCodePageNo++, "KOI8-T", "KOI8-T"));
	
	m_setSCodePage.insert(SCodePage("Kazakh", iCustomCodePageNo++, "PT154", "PT154"));
	m_setSCodePage.insert(SCodePage("Kazakh", iCustomCodePageNo++, "RK1048", "RK1048"));
	
	m_setSCodePage.insert(SCodePage("Thai", iCustomCodePageNo++, "ISO-8859-11", "ISO-8859-11"));
	m_setSCodePage.insert(SCodePage("Thai", iCustomCodePageNo++, "TIS-620", "TIS-620"));
	m_setSCodePage.insert(SCodePage("Thai", 874, "CP874", "Thai (Windows)"));
	m_setSCodePage.insert(SCodePage("Thai", 10021, "MacThai", "Thai (Mac)"));
	
	m_setSCodePage.insert(SCodePage("Laotian", iCustomCodePageNo++, "MuleLao-1", "MuleLao-1"));
	m_setSCodePage.insert(SCodePage("Laotian", iCustomCodePageNo++, "CP1133", "CP1133"));
	
	m_setSCodePage.insert(SCodePage("Vietnamese", iCustomCodePageNo++, "VISCII", "VISCII"));
	m_setSCodePage.insert(SCodePage("Vietnamese", iCustomCodePageNo++, "TCVN", "TCVN"));
	m_setSCodePage.insert(SCodePage("Vietnamese", 1258, "CP1258", "Vietnamese (Windows)"));
}

string CSrtSubTitleReader::GetCodePage(int iCodePage)
{
	for (set<SCodePage>::const_iterator it = m_setSCodePage.begin();
										it != m_setSCodePage.end();
										it ++)
	{
		if (it->iCodePage == iCodePage)
		{
			return it->strCodePage;
		}
	}
	return "CP1252";
}
