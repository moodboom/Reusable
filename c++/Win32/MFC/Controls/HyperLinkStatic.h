// HyperLinkStatic.h : header file
//
//
// HyperLinkStatic static control. Will open the default browser with the given URL
// when the user clicks on the link.
//
// Copyright Chris Maunder, 1997
// Feel free to use and distribute. May not be sold for profit. 
/////////////////////////////////////////////////////////////////////////////
/****************************************************************************
 *
 * $Date: 2002/08/23 12:01:17 $
 * $Revision: 1.1 $
 * $Archive: /CodeJock/Include/HyperLinkStatic.h $
 *
 * $History: HyperLinkStatic.h $
 * 
 * *****************  Version 6  *****************
 * User: Kirk Stowell Date: 10/26/99   Time: 10:42p
 * Updated in $/CodeJock/Include
 * Made class methods virtual for inheritance purposes.
 * 
 * *****************  Version 5  *****************
 * User: Kirk Stowell Date: 10/14/99   Time: 12:26p
 * Updated in $/CodeJock/Include
 * Added source control history to file header.
 *
 ***************************************************************************/
/////////////////////////////////////////////////////////////////////////////

#ifndef HYPER_LINK_STATIC_H
#define HYPER_LINK_STATIC_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// A CStatic derived class will open the default browser with the 
// given URL when the user clicks on the link. 

// Stoopid CJ macro to export function under dll build.  I always
// prefer static builds.
// class _CJX_EXT_CLASS HyperLinkStatic : public CStatic

class HyperLinkStatic : public CStatic
{
// Construction/destruction
public:
    HyperLinkStatic();
    virtual ~HyperLinkStatic();

// Attributes
public:

// Operations
public:

    virtual void SetURL(CString strURL);
    virtual CString GetURL() const;

    virtual void SetColors(COLORREF crLinkColor, COLORREF crVisitedColor, 
                    COLORREF crHoverColor = -1);
    virtual COLORREF GetLinkColor() const;
    virtual COLORREF GetVisitedColor() const;
    virtual COLORREF GetHoverColor() const;

    virtual void SetVisited(BOOL bVisited = TRUE);
    virtual BOOL GetVisited() const;

    virtual void SetLinkCursor(HCURSOR hCursor);
    virtual HCURSOR GetLinkCursor() const;

    virtual void SetUnderline(BOOL bUnderline = TRUE);
    virtual BOOL GetUnderline() const;

    virtual void SetAutoSize(BOOL bAutoSize = TRUE);
    virtual BOOL GetAutoSize() const;

    // Made this a standalone static, for use without 
	 // the need for a "static text" object.
	 // virtual HINSTANCE GotoURL(LPCTSTR url, int showcmd);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HyperLinkStatic)
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    protected:
    virtual void PreSubclassWindow();
    //}}AFX_VIRTUAL

// Implementation
protected:
    virtual void ReportError(int nError);

    // Made this a standalone static, for use without 
	 // the need for a "static text" object.
    // virtual LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata);

    virtual void PositionWindow();
    virtual void SetDefaultCursor();

// Protected attributes
protected:
    COLORREF m_crLinkColor, m_crVisitedColor;     // Hyperlink Colors
    COLORREF m_crHoverColor;                       // Hover Color
    BOOL     m_bOverControl;                        // cursor over control?
    BOOL     m_bVisited;                            // Has it been visited?
    BOOL     m_bUnderline;                          // underline hyperlink?
    BOOL     m_bAdjustToFit;                        // Adjust window size to fit text?
    CString  m_strURL;                              // hyperlink URL
    CFont    m_Font;                                // Underline font if necessary
    HCURSOR  m_hLinkCursor;                         // Cursor for hyperlink
    CToolTipCtrl m_ToolTip;                         // The tooltip

    // Generated message map functions
protected:
    //{{AFX_MSG(HyperLinkStatic)
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    //}}AFX_MSG
    afx_msg void OnClicked();
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////


// Made these standalone global functions, for use without 
// the need for a "static text" object.
HINSTANCE GotoURL(LPCTSTR url, int showcmd);
LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata);

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // HYPER_LINK_STATIC_H
