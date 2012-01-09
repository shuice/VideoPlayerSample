#define COMMON_DLL
#include <DllPort.h>


#include <afx.h>
#include <cmath>
#include "CommonInterface.h"
#include <atlimage.h>
#include <gdiplus.h>  
#include <afxwin.h>
#include <windows.h>
#include <shlobj.h>

// convert from * to *, may loss of data
#pragma warning(disable: 4244)

using   namespace   Gdiplus; 


bool Common::SplitPath(const CString& sFileName, 
                                CString* pstrFolder,
                                CString* pstrFileTitle,
                                CString* pstrFileExtName,
                                CString* pstrDrive)
{
    TCHAR strFolder[_MAX_DIR] = {0};
    TCHAR strFileTitle[_MAX_FNAME] = {0};    
    TCHAR strFileExtName[_MAX_EXT] = {0};
    TCHAR strDrive[_MAX_DRIVE] = {0};
    if (0 != _tsplitpath_s<_MAX_DRIVE, _MAX_DIR, _MAX_FNAME, _MAX_EXT>
                            ((LPCTSTR)sFileName, strDrive, strFolder, strFileTitle, strFileExtName))
    {
        return false;
    }
#define CSTRING_ASSIGN(x, y) if ((x) != NULL) {*(x) = (y);};

    CSTRING_ASSIGN(pstrFolder, strFolder);
    CSTRING_ASSIGN(pstrFileTitle, strFileTitle);
    CSTRING_ASSIGN(pstrFileExtName, strFileExtName);
    CSTRING_ASSIGN(pstrDrive, strDrive);    
    return true;
}


bool Common::RectAdapt(const CRect* pInnerRect, const CRect* pOuterRect, CRect* pAdaptRect)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (pInnerRect==NULL || pOuterRect==NULL || pAdaptRect==NULL)
    {
        return false;
    }
    if (IsRectEmpty(pInnerRect) || IsRectEmpty(pOuterRect))
    {
        return false;
    }

    long iInnerRectWidth = pInnerRect->Width();
    long iInnerRectHeight = pInnerRect->Height();
    long iOuterRectWidth = pOuterRect->Width();
    long iOuterRectHeight = pOuterRect->Height();


    if (iOuterRectWidth >= iInnerRectWidth
        && iOuterRectHeight >= iInnerRectHeight)
    {
        pAdaptRect->left = (iOuterRectWidth-iInnerRectWidth)/2;
        pAdaptRect->top = (iOuterRectHeight-iInnerRectHeight)/2;
        pAdaptRect->right = pAdaptRect->left + iInnerRectWidth;
        pAdaptRect->bottom = pAdaptRect->top + iInnerRectHeight;
        pAdaptRect->OffsetRect(pOuterRect->left, pOuterRect->top);
        return true;
    }

    if (iOuterRectWidth >= iInnerRectWidth
        && iOuterRectHeight < iInnerRectHeight)
    {
        double fAspec = iInnerRectWidth/(double)iInnerRectHeight;
        pAdaptRect->top = 0;
        pAdaptRect->bottom = iOuterRectHeight;
        double fWidth = iOuterRectHeight * fAspec;
        pAdaptRect->left = (iOuterRectWidth-fWidth)/2;
        pAdaptRect->right = pAdaptRect->left + fWidth;
    }
    else if (iOuterRectWidth < iInnerRectWidth
        && iOuterRectHeight >= iInnerRectHeight)
    {
        double fAspec = iInnerRectHeight/(double)iInnerRectWidth;
        pAdaptRect->left = 0;
        pAdaptRect->right = iOuterRectWidth;
        double fHeight = iOuterRectWidth * fAspec;
        pAdaptRect->top = (iOuterRectHeight-fHeight)/2;
        pAdaptRect->bottom = pAdaptRect->top + fHeight;
    }
    else if (iOuterRectWidth < iInnerRectWidth
        && iOuterRectHeight < iInnerRectHeight)
    {
        double fWidthtRate = iOuterRectWidth / (double)iInnerRectWidth;
        double fHeightRate = iOuterRectHeight / (double)iInnerRectHeight;
        if (fWidthtRate > fHeightRate)
        {
            double fAspec = iInnerRectWidth/(double)iInnerRectHeight;
            pAdaptRect->top = 0;
            pAdaptRect->bottom = iOuterRectHeight;
            double fWidth = iOuterRectHeight * fAspec;
            pAdaptRect->left = (iOuterRectWidth-fWidth)/2;
            pAdaptRect->right = pAdaptRect->left + fWidth;
        }
        else
        {
            double fAspec = iInnerRectHeight/(double)iInnerRectWidth;
            pAdaptRect->left = 0;
            pAdaptRect->right = iOuterRectWidth;
            double fHeight = iOuterRectWidth * fAspec;
            pAdaptRect->top = (iOuterRectHeight-fHeight)/2;
            pAdaptRect->bottom = pAdaptRect->top + fHeight;
        }
    }
    pAdaptRect->OffsetRect(pOuterRect->left, pOuterRect->top);
    return true;
}

bool Common::RectStretchAdapt(const CRect* pInnerRect, const CRect* pOuterRect, CRect* pAdaptRect)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (pInnerRect==NULL || pOuterRect==NULL || pAdaptRect==NULL)
    {
        return false;
    }
    if (IsRectEmpty(pInnerRect) || IsRectEmpty(pOuterRect))
    {
        return false;
    }

    double fInnerRate = pInnerRect->Width() / (double)pInnerRect->Height();
    double fOuterRate = pOuterRect->Width() / (double)pOuterRect->Height();
    *pAdaptRect = *pOuterRect;

    if (fInnerRate < fOuterRate)
    {
        int iAdaptRectWidth = pInnerRect->Width() * pOuterRect->Height()/(double)pInnerRect->Height();
        pAdaptRect->left = (pOuterRect->Width() - iAdaptRectWidth) / 2 + pOuterRect->left;
        pAdaptRect->right = pAdaptRect->left + iAdaptRectWidth;

    }
    else
    {
        int iAdaptRectHeight = pInnerRect->Height() * pOuterRect->Width()/(double)pInnerRect->Width();
        pAdaptRect->top = (pOuterRect->Height() - iAdaptRectHeight) / 2 + pOuterRect->top;
        pAdaptRect->bottom = pAdaptRect->top + iAdaptRectHeight;
    }
    return true;
}

bool Common::SaveBitmapToFile(HBITMAP hBitmap, const CString& sFileName)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CImage img;
    bool bSuccess = true;
    try
    {
        img.Attach(hBitmap);
        img.Save(sFileName);
        img.Detach();
    }
    catch (...)
    {
        bSuccess =  false;
    }
    return bSuccess;
}

HBITMAP Common::OpenPicture(const CString& sFileName)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CImage img;
    try
    {
        HRESULT rs = img.Load(sFileName);
        if (rs != S_OK)
        {
            return NULL;
        }
        return img.Detach();

    }
    catch (...)
    {
        ;
    }
    return NULL;
}

CSize Common::GetBitmapSize(HBITMAP hBitmap)
{
    CSize size(0, 0);
    if (hBitmap == NULL)
    {
        return size;
    }
    CBitmap bitmap;
    try
    {
        if (!bitmap.Attach(hBitmap))
        {
            return size;
        }
        BITMAP bBitmap;
        bitmap.GetBitmap(&bBitmap);
        size.cx = bBitmap.bmWidth;
        size.cy = bBitmap.bmHeight;
        bitmap.Detach();
        return size;

    }
    catch (...)
    {

    }
    return size;
}

void Common::SafeDeleteObject(HANDLE hObj)
{
    if (hObj == NULL)
    {
        return;
    }
    ::DeleteObject(hObj);
    hObj = NULL;
}


bool Common::SeprateWidthHeight(const CString& sVideoSize, int& iWidth, int &iHeight)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    iWidth = 0;
    iHeight = 0;
    if (sVideoSize.GetLength() < 3)
    {
        return false;
    }
    int iPos = sVideoSize.Find(_T("x"));

    if (iPos == -1)
    {
        return false;
    }
    CString sWidth = sVideoSize.Mid(0, iPos);
    int iLeft = sVideoSize.GetLength() - iPos - 1;
    CString sHeight = sVideoSize.Mid(iPos+1, iLeft);

    iWidth = _ttoi(sWidth);
    iHeight = _ttoi(sHeight);
    if (iWidth==0 || iHeight ==0)
    {
        return false;
    }
    return true;
}

bool Common::SeprateWidthHeight(const CString& sVideoSize, long& iWidth, long &iHeight)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    int iWidthTmp = iWidth;
    int iHeightTmp = iHeight;
    bool bSuccess = SeprateWidthHeight(sVideoSize, iWidthTmp, iHeightTmp);
    iWidth = iWidthTmp;
    iHeight = iHeightTmp;
    return bSuccess;
}

int Common::GetMaxWindowCharLen(CWnd* pWnd, const CString& str)
{
    if (pWnd == NULL)
    {
        return 0;
    }
    CDC* pDC = pWnd->GetDC();
    if (pDC == NULL)
    {
        return 0;
    }
    CFont* pFont = pWnd->GetFont();
    if (pFont == NULL)
    {
        pWnd->ReleaseDC(pDC);
        return 0;
    }

    CFont* pOldFont = pDC->SelectObject(pFont);
    // 二分法最好了，不过二分法难写，容易出错，如果以后有效率的需求再改成二分法
    int iCount = str.GetLength();
    int iIndex = 0;
    CRect rect;
    pWnd->GetClientRect(&rect);
    const int iWidth = rect.Width();

    for (iIndex = 0; iIndex < iCount; iIndex ++)
    {
        CSize size =  pDC->GetTextExtent(str, iIndex);
        if (size.cx > iWidth)
        {
            if (iIndex == 0)
            {
                pDC->SelectObject(pOldFont);
                pWnd->ReleaseDC(pDC);
                return 0;
            }
            else
            {
                pDC->SelectObject(pOldFont);
                pWnd->ReleaseDC(pDC);
                return iIndex - 1;
            }
        }
    }

    pDC->SelectObject(pOldFont);
    pWnd->ReleaseDC(pDC);
    return iCount;
}

void Common::FastByteCopy(void* pDest, void* pSource, int iCopyCount)
{
    _asm {
        mov ecx, iCopyCount
            mov esi, pSource
            mov edi, pDest
            rep movsb
    }
}

int Common::AlignUp(int x, int a)
{
    if (a == 0)
    {
        return 0;
    }
    int i = x % a;
    if (i == 0)
    {
        x = x + a - i;
    }
    return x;
}

int Common::AlignDown(int x, int a)
{
    if (a == 0)
    {
        return 0;
    }
    return x - x % a;
}
