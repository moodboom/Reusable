// Scrap.h : main header file for the SCRAP application
//

#if !defined(AFX_SCRAP_H__62E26F3C_A5F5_11D5_BCC1_00D0B7E64F2B__INCLUDED_)
#define AFX_SCRAP_H__62E26F3C_A5F5_11D5_BCC1_00D0B7E64F2B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CScrapApp:
// See Scrap.cpp for the implementation of this class
//

class CScrapApp : public CWinApp
{
public:
	CScrapApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScrapApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CScrapApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRAP_H__62E26F3C_A5F5_11D5_BCC1_00D0B7E64F2B__INCLUDED_)
