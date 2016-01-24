// HyperLinkStatic.cpp : implementation file
//
// HyperLinkStatic static control. Will open the default browser with the given URL
// when the user clicks on the link.
//
// Copyright (C) 1997, 1998 Chris Maunder
// All rights reserved. May not be sold for profit.
//
// Thanks to Pål K. Tønder for auto-size and window caption changes.
//
// "GotoURL" function by Stuart Patterson
// As seen in the August, 1997 Windows Developer's Journal.
// Copyright 1997 by Miller Freeman, Inc. All rights reserved.
// Modified by Chris Maunder to use TCHARs instead of chars.
//
// "Default hand cursor" from Paul DiLascia's Jan 1998 MSJ article.
//
/////////////////////////////////////////////////////////////////////////////
/****************************************************************************
 *
 * $Date: 2002/08/23 12:01:17 $
 * $Revision: 1.1 $
 * $Archive: /CodeJock/CJLibrary/HyperLinkStatic.cpp $
 *
 * $History: HyperLinkStatic.cpp $
 * 
 * *****************  Version 5  *****************
 * User: Kirk Stowell Date: 10/14/99   Time: 11:46p
 * Updated in $/CodeJock/CJLibrary
 * Fixed memory leak caused by not freeing link cursor. Thanks to Matt
 * Davies [matt.davies@codehammer.com] for help with this.
 * 
 * *****************  Version 4  *****************
 * User: Kirk Stowell Date: 10/14/99   Time: 12:26p
 * Updated in $/CodeJock/CJLibrary
 * Added source control history to file header.
 *
 ***************************************************************************/
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HyperLinkStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TOOLTIP_ID 1

/////////////////////////////////////////////////////////////////////////////
// HyperLinkStatic

HyperLinkStatic::HyperLinkStatic()
{
    m_hLinkCursor       = NULL;                 // No cursor as yet
    m_crLinkColor      = RGB(  0,   0, 238);   // Blue
    m_crVisitedColor   = RGB( 85,  26, 139);   // Purple
    m_crHoverColor     = ::GetSysColor(COLOR_HIGHLIGHT);
    m_bOverControl      = FALSE;                // Cursor not yet over control
    m_bVisited          = FALSE;                // Hasn't been visited yet.
    m_bUnderline        = TRUE;                 // Underline the link?
    m_bAdjustToFit      = TRUE;                 // Resize the window to fit the text?
    m_strURL.Empty();
}

HyperLinkStatic::~HyperLinkStatic()
{
    m_Font.DeleteObject();
	if (m_hLinkCursor) {
		DestroyCursor(m_hLinkCursor);
	}
}

BEGIN_MESSAGE_MAP(HyperLinkStatic, CStatic)
    //{{AFX_MSG_MAP(HyperLinkStatic)
    ON_CONTROL_REFLECT(STN_CLICKED, OnClicked)
    ON_WM_CTLCOLOR_REFLECT()
    ON_WM_SETCURSOR()
    ON_WM_MOUSEMOVE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HyperLinkStatic message handlers

BOOL HyperLinkStatic::PreTranslateMessage(MSG* pMsg) 
{
    m_ToolTip.RelayEvent(pMsg);
    return CStatic::PreTranslateMessage(pMsg);
}

void HyperLinkStatic::OnClicked()
{
    int result = (int)GotoURL(m_strURL, SW_SHOW);
    m_bVisited = (result > HINSTANCE_ERROR);
    if (!m_bVisited) {
        MessageBeep(MB_ICONEXCLAMATION);     // Unable to follow link
        ReportError(result);
    } else 
        SetVisited();                        // Repaint to show visited Color
}

HBRUSH HyperLinkStatic::CtlColor(CDC* pDC, UINT nCtlColor) 
{
#ifdef _DEBUG
	ASSERT(nCtlColor == CTLCOLOR_STATIC);
#else
	UNUSED_ALWAYS( nCtlColor );
#endif
    
    if (m_bOverControl)
        pDC->SetTextColor(m_crHoverColor);
    else if (m_bVisited)
        pDC->SetTextColor(m_crVisitedColor);
    else
        pDC->SetTextColor(m_crLinkColor);

    // transparent text.
    pDC->SetBkMode(TRANSPARENT);
    return (HBRUSH)GetStockObject(NULL_BRUSH);
}

void HyperLinkStatic::OnMouseMove(UINT nFlags, CPoint point) 
{
    CStatic::OnMouseMove(nFlags, point);

    if (m_bOverControl)        // Cursor is currently over control
    {
        CRect rect;
        GetClientRect(rect);

        if (!rect.PtInRect(point))
        {
            m_bOverControl = FALSE;
            ReleaseCapture();
            RedrawWindow();
            return;
        }
    }
    else                      // Cursor has just moved over control
    {
        m_bOverControl = TRUE;
        RedrawWindow();
        SetCapture();
    }
}

BOOL HyperLinkStatic::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
    if (m_hLinkCursor)
    {
        ::SetCursor(m_hLinkCursor);
        return TRUE;
    }
    return FALSE;
}

void HyperLinkStatic::PreSubclassWindow() 
{
    // We want to get mouse clicks via STN_CLICKED
    DWORD dwStyle = GetStyle();
    ::SetWindowLong(GetSafeHwnd(), GWL_STYLE, dwStyle | SS_NOTIFY);
    
    // Set the URL as the window text
    if (m_strURL.IsEmpty())
        GetWindowText(m_strURL);

    // Check that the window text isn't empty. If it is, set it as the URL.
    CString strWndText;
    GetWindowText(strWndText);
    if (strWndText.IsEmpty()) {
        ASSERT(!m_strURL.IsEmpty());    // Window and URL both NULL. DUH!
        SetWindowText(m_strURL);
    }

    // Create the font
    LOGFONT lf;
    GetFont()->GetLogFont(&lf);
    lf.lfUnderline = (char)m_bUnderline;
    m_Font.CreateFontIndirect(&lf);
    SetFont(&m_Font);

    PositionWindow();        // Adjust size of window to fit URL if necessary
    SetDefaultCursor();      // Try and load up a "hand" cursor

    // Create the tooltip
    CRect rect; 
    GetClientRect(rect);
    m_ToolTip.Create(this);
    m_ToolTip.AddTool(this, m_strURL, rect, TOOLTIP_ID);

    CStatic::PreSubclassWindow();
}

/////////////////////////////////////////////////////////////////////////////
// HyperLinkStatic operations

void HyperLinkStatic::SetURL(CString strURL)
{
    m_strURL = strURL;

    if (::IsWindow(GetSafeHwnd())) {
        PositionWindow();
        m_ToolTip.UpdateTipText(strURL, this, TOOLTIP_ID);
    }
}

CString HyperLinkStatic::GetURL() const
{ 
    return m_strURL;   
}

void HyperLinkStatic::SetColors(COLORREF crLinkColor, COLORREF crVisitedColor,
                            COLORREF crHoverColor /* = -1 */) 
{ 
    m_crLinkColor    = crLinkColor; 
    m_crVisitedColor = crVisitedColor;

	if (crHoverColor == -1)
		m_crHoverColor = ::GetSysColor(COLOR_HIGHLIGHT);
	else
		m_crHoverColor = crHoverColor;

    if (::IsWindow(m_hWnd))
        Invalidate(); 
}

COLORREF HyperLinkStatic::GetLinkColor() const
{ 
    return m_crLinkColor; 
}

COLORREF HyperLinkStatic::GetVisitedColor() const
{
    return m_crVisitedColor; 
}

COLORREF HyperLinkStatic::GetHoverColor() const
{
    return m_crHoverColor;
}

void HyperLinkStatic::SetVisited(BOOL bVisited /* = TRUE */) 
{ 
    m_bVisited = bVisited; 

    if (::IsWindow(GetSafeHwnd()))
        Invalidate(); 
}

BOOL HyperLinkStatic::GetVisited() const
{ 
    return m_bVisited; 
}

void HyperLinkStatic::SetLinkCursor(HCURSOR hCursor)
{ 
    m_hLinkCursor = hCursor;
    if (m_hLinkCursor == NULL)
        SetDefaultCursor();
}

HCURSOR HyperLinkStatic::GetLinkCursor() const
{
    return m_hLinkCursor;
}

void HyperLinkStatic::SetUnderline(BOOL bUnderline /* = TRUE */)
{
    m_bUnderline = bUnderline;

    if (::IsWindow(GetSafeHwnd()))
    {
        LOGFONT lf;
        GetFont()->GetLogFont(&lf);
        lf.lfUnderline = (char)m_bUnderline;

        m_Font.DeleteObject();
        m_Font.CreateFontIndirect(&lf);
        SetFont(&m_Font);

        Invalidate(); 
    }
}

BOOL HyperLinkStatic::GetUnderline() const
{ 
    return m_bUnderline; 
}

void HyperLinkStatic::SetAutoSize(BOOL bAutoSize /* = TRUE */)
{
    m_bAdjustToFit = bAutoSize;

    if (::IsWindow(GetSafeHwnd()))
        PositionWindow();
}

BOOL HyperLinkStatic::GetAutoSize() const
{ 
    return m_bAdjustToFit; 
}


// Move and resize the window so that the window is the same size
// as the hyperlink text. This stops the hyperlink cursor being active
// when it is not directly over the text. If the text is left justified
// then the window is merely shrunk, but if it is centred or right
// justified then the window will have to be moved as well.
//
// Suggested by Pål K. Tønder 

void HyperLinkStatic::PositionWindow()
{
    if (!::IsWindow(GetSafeHwnd()) || !m_bAdjustToFit) 
        return;

    // Get the current window position
    CRect rect;
    GetWindowRect(rect);

    CWnd* pParent = GetParent();
    if (pParent)
        pParent->ScreenToClient(rect);

    // Get the size of the window text
    CString strWndText;
    GetWindowText(strWndText);

    CDC* pDC = GetDC();
    CFont* pOldFont = pDC->SelectObject(&m_Font);
    CSize Extent = pDC->GetTextExtent(strWndText);
    pDC->SelectObject(pOldFont);
    ReleaseDC(pDC);

    // Get the text justification via the window style
    DWORD dwStyle = GetStyle();

    // Recalc the window size and position based on the text justification
    if (dwStyle & SS_CENTERIMAGE)
        rect.DeflateRect(0, (rect.Height() - Extent.cy)/2);
    else
        rect.bottom = rect.top + Extent.cy;

    if (dwStyle & SS_CENTER)   
        rect.DeflateRect((rect.Width() - Extent.cx)/2, 0);
    else if (dwStyle & SS_RIGHT) 
        rect.left  = rect.right - Extent.cx;
    else // SS_LEFT = 0, so we can't test for it explicitly 
        rect.right = rect.left + Extent.cx;

    // Move the window
    SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
}

/////////////////////////////////////////////////////////////////////////////
// HyperLinkStatic implementation

// The following appeared in Paul DiLascia's Jan 1998 MSJ articles.
// It loads a "hand" cursor from the winhlp32.exe module
void HyperLinkStatic::SetDefaultCursor()
{
    if (m_hLinkCursor == NULL)                // No cursor handle - load our own
    {
        // Get the windows directory
        CString strWndDir;
        GetWindowsDirectory(strWndDir.GetBuffer(MAX_PATH), MAX_PATH);
        strWndDir.ReleaseBuffer();

        strWndDir += _T("\\winhlp32.exe");
        // This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
        HMODULE hModule = LoadLibrary(strWndDir);
        if (hModule) {
            HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
            if (hHandCursor)
                m_hLinkCursor = CopyCursor(hHandCursor);
        }
        FreeLibrary(hModule);
    }
}


void HyperLinkStatic::ReportError(int nError)
{
    CString str;
    switch (nError) {
        case 0:                       str = _T("The operating system is out\nof memory or resources."); break;
        case SE_ERR_PNF:              str = _T("The specified path was not found."); break;
        case SE_ERR_FNF:              str = _T("The specified file was not found."); break;
        case ERROR_BAD_FORMAT:        str = _T("The .EXE file is invalid\n(non-Win32 .EXE or error in .EXE image)."); break;
        case SE_ERR_ACCESSDENIED:     str = _T("The operating system denied\naccess to the specified file."); break;
        case SE_ERR_ASSOCINCOMPLETE:  str = _T("The filename association is\nincomplete or invalid."); break;
        case SE_ERR_DDEBUSY:          str = _T("The DDE transaction could not\nbe completed because other DDE transactions\nwere being processed."); break;
        case SE_ERR_DDEFAIL:          str = _T("The DDE transaction failed."); break;
        case SE_ERR_DDETIMEOUT:       str = _T("The DDE transaction could not\nbe completed because the request timed out."); break;
        case SE_ERR_DLLNOTFOUND:      str = _T("The specified dynamic-link library was not found."); break;
        case SE_ERR_NOASSOC:          str = _T("There is no application associated\nwith the given filename extension."); break;
        case SE_ERR_OOM:              str = _T("There was not enough memory to complete the operation."); break;
        case SE_ERR_SHARE:            str = _T("A sharing violation occurred. ");
        default:                      str.Format(_T("Unknown Error (%d) occurred."), nError); break;
    }
    str = _T("Unable to open hyperlink:\n\n") + str;
    AfxMessageBox(str, MB_ICONEXCLAMATION | MB_OK);
}


//-------------------------------------------------------------------//
// GotoURL()																			//
//-------------------------------------------------------------------//
// The wheat gluten and potatoes of it all.
//
// This and GetRegKey were made standalone global functions, 
// for use without the need for a "static text" object.
//-------------------------------------------------------------------//
// HINSTANCE HyperLinkStatic::GotoURL( LPCTSTR url, int showcmd )
HINSTANCE GotoURL( LPCTSTR url, int showcmd )
{
    TCHAR key[MAX_PATH + MAX_PATH];

    // First try ShellExecute()
    HINSTANCE result = ShellExecute( NULL, _T("open"), url, NULL, NULL, showcmd );

    // If it failed, get the .htm regkey and lookup the program
    if((UINT)result <= HINSTANCE_ERROR)
	{
        if( GetRegKey( HKEY_CLASSES_ROOT, _T(".htm"), key ) == ERROR_SUCCESS )
		{
            lstrcat( key, _T("\\shell\\open\\command"));

            if( GetRegKey( HKEY_CLASSES_ROOT, key, key ) == ERROR_SUCCESS )
			{
                LPTSTR pos;

                pos = _tcsstr(key, _T("\"%1\""));

                if( pos == NULL )						// No quotes found
				{
                    pos = _tcsstr( key, _T("%1"));		// Check for %1, without quotes 
                    
					if( pos == NULL )					// No parameter at all...
					{
                        pos = key + _tcslen( key ) - 1;
					}
                    else
					{
                        *pos = _T('\0');				// Remove the parameter
					}
                }
                else
				{
                    *pos = _T('\0');					// Remove the parameter
				}

                lstrcat( pos, _T(" "));
                lstrcat( pos, url );

				PROCESS_INFORMATION pi;
		
				STARTUPINFO			si;

				::ZeroMemory( &si, sizeof( si )); 
		
				si.cb = sizeof( si ); 

				if( ::CreateProcess( NULL,
									 pos,
									 NULL,
									 NULL,
									 FALSE,
									 0,
									 NULL,
									 NULL,
									 &si,
									&pi ) == FALSE )
				{
					return NULL;
				}

                result = (HINSTANCE) pi.hProcess;
            }
        }
    }

    return result;
}


//-------------------------------------------------------------------//
// GetRegKey()																			//
//-------------------------------------------------------------------//
// LONG HyperLinkStatic::GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
    HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS) {
        long datasize = MAX_PATH;
        TCHAR data[MAX_PATH];
        RegQueryValue(hkey, NULL, data, &datasize);
        _tcscpy(retdata,data);
        RegCloseKey(hkey);
    }

    return retval;
}

