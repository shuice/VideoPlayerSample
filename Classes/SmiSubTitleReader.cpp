#include "SmiSubTitleReader.h"
#include "SplitString.h"

bool CSmiSubTitleReader::LoadFile(const string& strSrtFileName)
{
    FILE* pFile = fopen(strSrtFileName.c_str(), "rb");
    if (pFile == NULL)
    {
        return false;
    }
    fseek(pFile, 0, SEEK_END);
    int iLength = ftell(pFile);
    char* pData = new char[iLength + 1];
    if (pData == NULL)
    {
        fclose(pFile);
        return false;
    }
    memset(pData, 0, iLength+1);
    fseek(pFile, 0, SEEK_SET);
    fread(pData, iLength, 1, pFile);
    fclose(pFile);
    
    vector<string> vectStr;
    SplitString(pData, "<SYNC ", vectStr);
    delete[] pData;
    const int iCount = vectStr.size();
    if (iCount < 2)
    {
        return false;
    }
    
    for (int iIndex = 1; iIndex < iCount; iIndex ++)
    {
        /*
        <SYNC Start=32507><P Class=UNKNOWNCC>
        字幕翻译者: bei 校对:tangyu98<br>英文字幕提供http://www.topcmm.com/friends/
        <SYNC Start=37706><P Class=UNKNOWNCC>&nbsp;
        <SYNC Start=57335><P Class=UNKNOWNCC>
        没什么好说的!
        <SYNC Start=59431><P Class=UNKNOWNCC>&nbsp;
        */
        const string& strItem = vectStr[iIndex];
        SSrtSubTitleItemReaded sSrtSubTitleItemReaded;
        sSrtSubTitleItemReaded.iIndex = m_vctSrtSubTitleItemReaded.size();
        sSrtSubTitleItemReaded.iCodePage = 0;
        char* pszEqulPos = strchr(strItem.c_str(), '=');
        if (pszEqulPos == NULL)
        {
            continue;
        }
        char* pszNumberPos = pszEqulPos + 1;
        while (*pszNumberPos != '\0' && !isdigit(*pszNumberPos)) 
        {
            pszNumberPos ++;
        }
        if (!isdigit(*pszNumberPos))
        {
            continue;
        }
        sSrtSubTitleItemReaded.iStartTime = atoi(pszNumberPos);
        sSrtSubTitleItemReaded.strSubTitleReaded = TripMarkFlags(strItem);
        m_vctSrtSubTitleItemReaded.push_back(sSrtSubTitleItemReaded);
    }
    
    int iSubTitleCount = m_vctSrtSubTitleItemReaded.size();
    for (int iSubTitleIndex = 0; iSubTitleIndex < iSubTitleCount - 1; iSubTitleIndex ++)
    {
        m_vctSrtSubTitleItemReaded[iSubTitleIndex].iEndTime = m_vctSrtSubTitleItemReaded[iSubTitleIndex+1].iStartTime - 1;
    }
    if (iSubTitleCount != 0)
    {
        m_vctSrtSubTitleItemReaded.rbegin()->iEndTime = INT_MAX;
    }
    return true;
}

string CSmiSubTitleReader::TripMarkFlags(const string& str)
{
    int iStrLen = str.size();
    if (iStrLen == 0)
    {
        return "";
    }
    char* pszTmp = new char[iStrLen + 1];
    memset(pszTmp, 0, iStrLen + 1);
    string strRet;
    const char* p = str.c_str();
    while (*p != '\0')
    {
        char* pLeft = strchr(p, '>');
        char* pRight = strchr(p, '<');
        if ((pLeft == NULL) && (pRight == NULL))  
        {
            strRet += p;
            break;
        }
        else if ((pLeft == NULL) && (pRight != NULL))
        {
            break;
        }
        else if ((pLeft != NULL) && (pRight == NULL))
        {
            strRet += (pLeft + 1);
            break;
        }
        else
        {
            if (pLeft < pRight)
            {
                if (pRight - pLeft > 1)
                {
                    memcpy(pszTmp, pLeft + 1, pRight - pLeft - 1);
                    pszTmp[pRight - pLeft - 1] = '\0';
                    strRet += pszTmp;
                }
                p = pRight + 1;
            }
            else
            {
                break;
            }
        }
    }
    return strRet;
}
