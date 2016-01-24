#if !defined(AFX_DATETIMEEB_H__7E6BAE9C_2B93_4F52_8054_673108A9345E__INCLUDED_)
#define AFX_DATETIMEEB_H__7E6BAE9C_2B93_4F52_8054_673108A9345E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxdisp.h>				// For COleDateTime


// DateTimeEB.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DateTimeEB window

const UINT WM_DATETIME_EDIT = ::RegisterWindowMessage("WM_DATETIME_EDIT");
#define DTE_DATETIMECHANGED 0x0001


class DateTimeEB : public CEdit
{
// Construction
public:
	DateTimeEB( CString strMask = _T("MM/DD/YYYY hh:mm:ss") );

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DateTimeEB)
	protected:
   virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
   CString GetMask() const 
   {
      return m_strMask;
   }
   void SetMask(CString mask)
   {
      COleDateTime time = GetDateTime();
      m_strMask = mask;
      SetDateTime(time);
   }

    COleDateTime GetDateTime();
    COleDateTime GetDateTime(CString str);
    void SetDateTime(COleDateTime dt);

	virtual ~DateTimeEB()
   {}

	// Generated message map functions
protected:

   CString m_strMask;

	//{{AFX_MSG(DateTimeEB)
  	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATETIMEEB_H__7E6BAE9C_2B93_4F52_8054_673108A9345E__INCLUDED_)
