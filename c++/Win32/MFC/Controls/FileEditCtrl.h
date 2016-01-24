// FileEditCtrl.h : header file for CFileEditCtrl control class
// written by PJ Arends
// pja@telus.net
//

#if !defined(AFX_FILEEDITCTRL_H__F15965B0_B05A_11D4_B625_A1459D96AB20__INCLUDED_)
#define AFX_FILEEDITCTRL_H__F15965B0_B05A_11D4_B625_A1459D96AB20__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Controls\EditTrans.h>														// For transparent background edit controls.


/////////////////////////////////////////////////////////////////////////////
// CFECFileDialog dialog

// The sole purpose for this class is to change the
// text of the "OPEN" button to "OK".

class CFECFileDialog : public CFileDialog
{
	DECLARE_DYNAMIC(CFECFileDialog)

public:
	CFECFileDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

protected:
	//{{AFX_MSG(CFECFileDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	virtual void OnInitDone();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CFileEditCtrl control

class CFileEditCtrl : public CEditTrans
{
	typedef CEditTrans inherited;

	DECLARE_DYNAMIC (CFileEditCtrl)
// Construction
public:
	CFileEditCtrl(BOOL bAutoDelete = FALSE);	// Constructor

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileEditCtrl)
	public:
	//}}AFX_VIRTUAL
	virtual BOOL Create(BOOL bFindFolder, DWORD dwExStyle, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Implementation
public:
	virtual ~CFileEditCtrl();				// Destructor
	BOOL GetFindFolder();					// returns the controls functionality
	BROWSEINFO* GetBrowseInfo() const;		// returns a pointer to the internal BROWSEINFO structure
	CString GetNextPathName(POSITION &pos);	// get the file at pos and then update pos
	OPENFILENAME* GetOpenFileName() const;	// returns a pointer to the internal OPENFILENAME structure
	POSITION GetStartPosition();			// get the starting position for GetNextPathName()
	BOOL SetFindFolder(BOOL bFindFolder);	// changes the controls functionality

	// Generated message map functions
protected:
	void ButtonClicked();					// handles a mouse click on the button
	void DrawButton (int nButtonState = 0);	// draws the button
	void DrawDots (CDC *pDC, COLORREF CR, int nOffset = 0);	// draws the dots on the button
	BOOL FECBrowseForFolder();				// starts and handles the returns from the SHBrowseForFolder() shell function
	BOOL FECOpenFile();						// starts and handles the returns from the CFileDialog
	void FillBuffers();						// fills the buffers used by GetStartPosition() and GetNextPathName() functions
	//{{AFX_MSG(CFileEditCtrl)
	afx_msg void OnChange();
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL			m_bAutoDelete;			// delete 'this' in OnDestroy() handler?
	BOOL			m_bFindFolder;			// browse for files or folders?
	BOOL			m_bMouseCaptured;		// button has captured the mouse?
	BOOL			m_bTextChanged;			// window text changed since last time FillBuffers() was called
	LPTSTR			m_lpstrFiles;			// buffer for storing file names
	int				m_nButtonState;			// current button state (up, down, or disabled)
	BROWSEINFO*		m_pBROWSEINFO;			// only active when m_bFindFolder is TRUE
	CFECFileDialog* m_pCFileDialog;			// only active when m_bFindFolder is FALSE
	CRect			m_rcButtonArea;			// window coordinates for drawing the button
	CString			m_szCaption;			// caption of CFileDialog
	CString			m_szFolder;				// absolute path to first file in m_lpstrFiles. Empty
};											// if only one file has been entered (maybe).

/////////////////////////////////////////////////////////////////////////////
// DDV_/DDX_FileEditCtrl functions

void DDV_FileEditCtrl (CDataExchange *pDX, int nIDC);	// verify that the file/folder entered exists
void DDX_FileEditCtrl (CDataExchange *pDX, int nIDC, CFileEditCtrl &rCFEC, BOOL bFindFolder);
void DDX_FileEditCtrl (CDataExchange *pDX, int nIDC, CString& rStr, BOOL bFindFolder);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEEDITCTRL_H__F15965B0_B05A_11D4_B625_A1459D96AB20__INCLUDED_)
