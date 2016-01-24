// SuperProgressDlg.h : header file
//

//////////////////////////////////////////////////////////////
// WARNING - See "DERIVED CLASS NOTE" in constructor body
// before deriving from this class.
//////////////////////////////////////////////////////////////

#include "BaseProgressDlg.h"					// Base class

#include "Controls\SuperProgressCtrl.h"	// Our "super" ctrl


#if !defined SUPER_PROGRESS_DLG_H
	#define SUPER_PROGRESS_DLG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// SuperProgressDlg dialog

class SuperProgressDlg : public BaseProgressDlg
{
	typedef BaseProgressDlg inherited;

public:

	// Constructor
	SuperProgressDlg( 
		CWnd* pParent							= 0,
		UINT	TextID							= 0,
		UINT	DialogID							= 0,
		int	nRespondTimeInMS				= 200,
		UINT	IconID							= 0,
		bool	bCalledFromDerivedClass		= false
	);

	// Destructor.
	~SuperProgressDlg();

	virtual void UpdateProgress( int nNewProgress );

	// Call this function to change the max pos
	// from the default of 100.
	virtual void SetMaxPos( int nMaxPos )
	{ 
		inherited::SetMaxPos( nMaxPos );
		m_SuperCtrl.SetRange( 0, nMaxPos );
	}

// Dialog Data
	//{{AFX_DATA(SuperProgressDlg)
	enum { IDD = IDD_PROGRESS_STD };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SuperProgressDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CSuperProgressCtrl m_SuperCtrl;

	// Generated message map functions
	//{{AFX_MSG(SuperProgressDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.


#endif	// SUPER_PROGRESS_DLG_H
