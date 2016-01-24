//-------------------------------------------------------------------//
//
// NOTE: Derived from code provided by Chris Maunder and 
// Michael Martin on CodeGuru, 8/98.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef STATUS_BAR_PROGRESS_H
#define STATUS_BAR_PROGRESS_H


/////////////////////////////////////////////////////////////////////////////
// StatusBarProgress -  status bar progress control
//

class StatusBarProgress: public CProgressCtrl
// Creates a ProgressBar in the status bar
{
public:
	StatusBarProgress();
	StatusBarProgress(LPCTSTR strMessage, int nSize=100, int MaxValue=100, 
                 BOOL bSmooth=FALSE, int nPane=0);
	~StatusBarProgress();
	BOOL Create(LPCTSTR strMessage, int nSize=100, int MaxValue=100, 
                BOOL bSmooth=FALSE, int nPane=0);

	DECLARE_DYNCREATE(StatusBarProgress)

// operations
public:
	BOOL SetRange(int nUpper, int nLower = 0, int nStep = 1);
	BOOL SetText(LPCTSTR strMessage);
	BOOL SetSize(int nSize);
	COLORREF SetBarColour(COLORREF clrBar);
	COLORREF SetBkColour(COLORREF clrBk);
	int  SetPos(int nPos);
	int  OffsetPos(int nPos);
	int  SetStep(int nStep);
	int  StepIt();
	void Clear();

// Overrides
	//{{AFX_VIRTUAL(StatusBarProgress)
	//}}AFX_VIRTUAL

// implementation
protected:
	int		m_nSize;		// Percentage size of control
	int		m_nPane;		// ID of status bar pane progress bar is to appear in
	CString	m_strMessage;	// Message to display to left of control
    CString m_strPrevText;  // Previous text in status bar
	CRect	m_Rect;			// Dimensions of the whole thing

	CStatusBar *GetStatusBar();
	BOOL Resize();

// Generated message map functions
protected:
	//{{AFX_MSG(StatusBarProgress)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
/////////////////////////////////////////////////////////////////////////////
