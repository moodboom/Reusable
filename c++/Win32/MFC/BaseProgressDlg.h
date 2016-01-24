//-------------------------------------------------------------------//
// BaseProgressDlg.h : header file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

//////////////////////////////////////////////////////////////
// WARNING - See "DERIVED CLASS NOTE" in constructor body
// before deriving from this class.
//////////////////////////////////////////////////////////////


#if !defined BASE_PROGRESS_DLG_H
	#define BASE_PROGRESS_DLG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module:
//

#define IDD_PROGRESS_STD						6101		// IDD_PROGRESS_STD DIALOG DISCARDABLE  0, 0, 186, 50
#define IDC_PROGRESS_METER						6102		// STYLE DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_VISIBLE
#define IDC_PROGRESS_TEXT						6103		// FONT 8, "MS Sans Serif"
																	// BEGIN
																	// 	 CONTROL         "Progress1",IDC_PROGRESS_METER,"msctls_progress32",
																	// 						  PBS_SMOOTH | WS_BORDER,10,24,167,14
																	// 	 CTEXT           "(operation in progress)",IDC_PROGRESS_TEXT,0,9,185,8
																	// END

// 
// Ideally, resources would be specified on a class-by-class basis.  Unfortunately,
// Visual Studio does not handle that well.  Different projects have different resource
// files that contain all of the project's resources in one place.  Make sure
// you provide the resources matching the above defines in your resource file.
// You must also include this file in the resource file's "Read-only symbol
// directives" ( see "Resource Includes" on the View menu ).
//
// We want to exclude the remainder of the include file when dealing with App Studio.
#ifndef RC_INVOKED
/////////////////////////////////////////////////////////////////////////


#include "FontDlg.h"													// Base class, allows custom font.

#include "Controls\ProgressCtrlX\ProgressCtrlX.h"			// For our contained progress control.


/////////////////////////////////////////////////////////////////////////////
// BaseProgressDlg dialog

class BaseProgressDlg : public FontDlg
{
	typedef FontDlg inherited;

public:

	// Constructor
	BaseProgressDlg( 
		CWnd*		pParent						= 0,
		CWinApp* pParentApp					= 0,			// Specify this if you want us to process app msgs during progress.
		UINT		TextID						= 0,
		UINT		DialogID						= 0,
		int		nRespondTimeInMS			= 200,
		UINT		IconID						= 0,
		bool		bCalledFromDerivedClass	= false
	);

	// Destructor.
	virtual ~BaseProgressDlg();

	virtual void UpdateProgress( int nNewProgress );

	// Call this function to change the max pos
	// from the default of 100.
	virtual void SetMaxPos( int nMaxPos )
	{ 
		ProgressMeter.SetRange( 0, nMaxPos );
	}

	// Set the progress text directly, any which way you want.
	void SetText( UINT TextID			) { SetText( CString( (LPTSTR) TextID )	);	}
	// void SetText( LPCTSTR szFormat	) { SetText( CString( szFormat )				);	}
	void SetText( CString& strText	);

	// This function allows us to do message strings that include 
	// the progress number.  It displays the current progress value
	// in the text string using the format specified in m_strFormat.
	void UpdateProgressAndDisplayAsText( int nNewProgress );

	// These set the format string for UpdateProgressAndDisplayAsText().
	void SetTextFormat( LPCTSTR szFormat	) { m_strFormat = szFormat;				}
	void SetTextFormat( UINT TextID			) { m_strFormat.LoadString( TextID );	}
	void SetTextFormat( CString& strFormat	) { m_strFormat = strFormat;				}

	// This sets the step size, i.e., how often we actually update the text.
	// 1 = every nProgressUpdate, 5 = every 5 nProgressUpdates, etc.
	// Typically only used when there are a  LOT of steps.
	void SetTextUpdateStepSize( int nStep	) { m_nTextUpdateStepSize = nStep;		}

	void StartSnake( int nMsPerStep )
	{
		// What's with all the silly setup steps???  Dammmmmit!
		ProgressMeter.SetRange( 0, 1000 );
		ProgressMeter.SetStep( 50 );
		ProgressMeter.SetSnake();
		ProgressMeter.SetSnakeTail( 500 );
		ProgressMeter.RunSnake( nMsPerStep );
	}

	void StopSnake()
	{
		ProgressMeter.StopSnake();
	}

// Dialog Data
	//{{AFX_DATA(BaseProgressDlg)
	enum { IDD = IDD_PROGRESS_STD };
	CProgressCtrlX	ProgressMeter;
	CString	strProgressText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BaseProgressDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void PurgeMessages();

	HICON m_hIcon;

	CWinApp* m_pParentApp;

	// These are used by UpdateProgressAndDisplayAsText().
	CString m_strFormat;
	int m_nTextUpdateStepSize;

	int m_nRespondTimeInMS;
	
	// Generated message map functions
	//{{AFX_MSG(BaseProgressDlg)
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Standard cancel (via ESC, for example) is not enabled
	// by default for this dlg.  To allow Cancel, just override
	// this function in the derived class, call CDialog::OnCancel(),
	// and perform your cancel steps.
	virtual void OnCancel();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.


#endif		// RC_INVOKED

#endif	// BASE_PROGRESS_DLG_H
