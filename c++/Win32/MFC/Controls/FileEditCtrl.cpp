// FileEditCtrl.cpp : implementation file for CFileEditCtrl control class
// written by PJ Arends
// pja@telus.net
//

#include "stdafx.h"
// #include "resource.h"
#include "FileEditCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// These strings should be entered into a string table resource with the 
// FEC_IDS_* identifiers defined here, although this class will work
// properly if they are not.
//
// string usage:
//
// FEC_IDS_ALLFILES			- default file filter for CFileDialog
// FEC_IDS_FILEDIALOGTITLE	- default dialog caption for CFileDialog
// FEC_IDS_SEPERATOR		- character used to seperate files when OFN_ALLOWMULTISELECT flag is used
// FEC_IDS_NOFILE			- Error message for DDV_FileEditCtrl() when no files or folders are entered
// FEC_IDS_NOTEXIST			- Error message for DDV_FileEditCtrl() when the specified file or folder could not be found
// FEC_IDS_NOTFILE			- Error message for DDV_FileEditCtrl() when the specified file is actually a folder
// FEC_IDS_NOTFOLDER		- Error message for DDV_FileEditCtrl() when the specified folder is actually a file
// FEC_IDS_OKBUTTON			- Text for the 'OK' (Open) button on CFileDialog
//

// FEC_IDS_ALLFILES will be defined in resource.h if these strings
// are in a string table resource
#if !defined FEC_IDS_ALLFILES
	#define FEC_NORESOURCESTRINGS so this class knows how to handle these strings
	#define FEC_IDS_ALLFILES		_T("All Files (*.*)|*.*||")
	#define FEC_IDS_FILEDIALOGTITLE	_T("Browse for File")
	#define FEC_IDS_SEPERATOR		_T(";")
	#define FEC_IDS_NOFILE			_T("Enter an existing file.")
	#define FEC_IDS_NOTEXIST		_T("%s does not exist.")
	#define FEC_IDS_NOTFILE			_T("%s is not a file.")
	#define FEC_IDS_NOTFOLDER		_T("%s is not a folder.")
	#define FEC_IDS_OKBUTTON		_T("OK")
#endif

/////////////////////////////////////////////////////////////////////////////
// Button states

#define BTN_UP			0
#define BTN_DOWN		1
#define BTN_DISABLED	2

/////////////////////////////////////////////////////////////////////////////
// Helper function

BOOL IsWindow (CWnd *pWnd)
{
	if (!pWnd)
		return (FALSE);
	return ::IsWindow (pWnd->m_hWnd);
}

/////////////////////////////////////////////////////////////////////////////
// CFileEditCtrl

IMPLEMENT_DYNAMIC (CFileEditCtrl, CEdit)

CFileEditCtrl::CFileEditCtrl(BOOL bAutoDelete /* = FALSE */)
{
	// If bAutoDelete is TRUE, this class object will be deleted
	// when it's window is destroyed (in CFileEditCtrl::OnDestroy).
	// The only time this should be used is when the control is
	// created dynamicly in the
	// DDX_FileEditCtrl(CDataExchange*,int,CString&,BOOL) function.

	// Initialize all variables
	m_bAutoDelete		= bAutoDelete;
	m_bFindFolder		= FALSE;
	m_bMouseCaptured	= FALSE;
	m_bTextChanged		= FALSE;
	m_lpstrFiles		= NULL;
	m_nButtonState		= BTN_UP;
	m_pBROWSEINFO		= NULL;
	m_pCFileDialog		= NULL;
	m_rcButtonArea.SetRectEmpty();
	m_szCaption.Empty();
	m_szFolder.Empty();
}

CFileEditCtrl::~CFileEditCtrl()
{
	// clean up all pointer variables
	if (m_lpstrFiles)	delete m_lpstrFiles;
	if (m_pBROWSEINFO)  delete m_pBROWSEINFO;
	if (m_pCFileDialog) delete m_pCFileDialog;
}

BEGIN_MESSAGE_MAP(CFileEditCtrl, CEdit)
	//{{AFX_MSG_MAP(CFileEditCtrl)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_WM_ENABLE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCPAINT()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileEditCtrl message handlers

void CFileEditCtrl::ButtonClicked()
{
	// Called to handle a mouse click on the button
	BOOL bResult = FALSE;
	if (m_bFindFolder)
		bResult = FECBrowseForFolder();
	else
		bResult = FECOpenFile();
	// Post BN_CLICKED notification to parent window
	if (bResult && IsWindow(GetParent()))
		GetParent()->PostMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM)GetSafeHwnd());
}

BOOL CFileEditCtrl::Create(BOOL bFindFolder, DWORD dwExStyle, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID) 
{
	// Create this control in any window
	BOOL bResult = CreateEx(dwExStyle, _T("EDIT"), NULL, dwStyle, rect, pParentWnd, nID);
	if (bResult)
	{
		// Force a call to CFileEditCtrl::OnNcCalcSize() to calculate button size
		SetWindowPos(NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		// call CFileEditCtrl::FindFolder() to initialize the internal data structures
		bResult = SetFindFolder(bFindFolder);
		// set the font to the font used by the parent window
		if (pParentWnd)
			SetFont (pParentWnd->GetFont());
	}
	return bResult;
}

void CFileEditCtrl::DrawButton(int nButtonState)
{
	ASSERT (IsWindow(this));
	
	// if the control is disabled, ensure the button is drawn disabled
	if (GetStyle() & WS_DISABLED)
		nButtonState = BTN_DISABLED;

	// Draw the button in the specified state (Up, Down, or Disabled)
	CWindowDC DC(this);		// get the DC for drawing

	CBrush theBrush(GetSysColor(COLOR_3DFACE));		// the colour of the button background
	CBrush *pOldBrush = DC.SelectObject(&theBrush);

	if (nButtonState == BTN_DOWN)	// Draw button as down
	{
		// draw the border
		CPen thePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
		CPen *pOldPen = DC.SelectObject(&thePen);
		DC.Rectangle(&m_rcButtonArea);
		DC.SelectObject(pOldPen);
		thePen.DeleteObject();

		// draw the dots
		DrawDots (&DC, GetSysColor(COLOR_BTNTEXT), 1);
	}
	else	// draw button as up
	{
		CPen thePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
		CPen *pOldPen = DC.SelectObject(&thePen);
		DC.Rectangle(&m_rcButtonArea);
		DC.SelectObject(pOldPen);
		thePen.DeleteObject();

		// draw the border
		DC.DrawEdge(&m_rcButtonArea, EDGE_RAISED, BF_RECT);

		// draw the dots
		if (nButtonState == BTN_DISABLED)
		{
			DrawDots (&DC, GetSysColor(COLOR_3DHILIGHT), 1);
			DrawDots (&DC, GetSysColor(COLOR_GRAYTEXT));
		}
		else if (nButtonState == BTN_UP)
			DrawDots (&DC, GetSysColor(COLOR_BTNTEXT));
		else
			ASSERT (FALSE);	// Invalid nButtonState
	}
	DC.SelectObject(pOldBrush);
	theBrush.DeleteObject();

	// update m_nButtonState
	m_nButtonState = nButtonState;
}

void CFileEditCtrl::DrawDots(CDC *pDC, COLORREF CR, int nOffset /* = 0 */)
{
	// draw the dots on the button
	int width = m_rcButtonArea.Width();			// width of the button
	div_t divt = div (width, 4);
	int delta = divt.quot;						// space between dots
	int left = m_rcButtonArea.left + width / 2 - delta - (divt.rem ? 0 : 1); // left side of first dot
	width = width / 10;							// width and height of one dot
	int top = m_rcButtonArea.Height() / 2 - width / 2 + 1;	// top of dots
	left += nOffset;							// draw dots shifted? ( for button pressed )
	top += nOffset;
	// draw the dots
	if (width < 2)
	{
		pDC->SetPixel(left, top, CR);
		left += delta;
		pDC->SetPixel(left, top, CR);
		left += delta;
		pDC->SetPixel(left, top, CR);
	}
	else
	{
		CPen thePen(PS_SOLID, 1, CR);			// set the dot colour
		CPen *pOldPen = pDC->SelectObject(&thePen);
		CBrush theBrush(CR);
		CBrush *pOldBrush = pDC->SelectObject(&theBrush);
		pDC->Ellipse(left, top, left + width, top + width);
		left += delta;
		pDC->Ellipse(left, top, left + width, top + width);
		left += delta;
		pDC->Ellipse(left, top, left + width, top + width);
		pDC->SelectObject(pOldBrush);			// reset the DC
		theBrush.DeleteObject();
		pDC->SelectObject(pOldPen);
		thePen.DeleteObject();
	}
}

int CALLBACK FECFolderProc(HWND hWnd, UINT nMsg, LPARAM, LPARAM lpData)
{
	// This is the default callback procedure for the SHBrowseForFolder function.
	// It sets the current selection to the directory specified in the edit control
    if (nMsg == BFFM_INITIALIZED)
        ::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
    return 0;
}

BOOL CFileEditCtrl::FECBrowseForFolder()
{
	// Set up and call SHBrowseForFolder().
	// return TRUE if user clicks OK, return FALSE otherwise
	BOOL bReturnValue = FALSE;
	BOOL bProcedure = TRUE;		// assume user of this class has set a callback procedure
	BOOL bDisplay = TRUE;		// assume user of this class has set a DisplayName
	TCHAR lpstrDisplay[_MAX_PATH];
	if (!m_pBROWSEINFO->pszDisplayName)			// user has not set a display name
	{											// flag it, and use our own buffer
		bDisplay = FALSE;
		m_pBROWSEINFO->pszDisplayName = lpstrDisplay;
	}
	CString szPath;
	LPARAM oldLP = m_pBROWSEINFO->lParam;
	if (!m_pBROWSEINFO->lpfn)					// user has not set a callback procedure
	{											// flag it, and use the default procedure
		bProcedure = FALSE;
		POSITION pos = GetStartPosition();
		if (pos)
			szPath = GetNextPathName(pos);		// get the path in the edit control
		m_pBROWSEINFO->lParam = (LPARAM)(LPCTSTR)szPath;	// set lParam to point to szPath
		m_pBROWSEINFO->lpfn = FECFolderProc;	// set the callback procedure
	}
	ITEMIDLIST *idl = SHBrowseForFolder (m_pBROWSEINFO);
	if (idl)
	{
		if(SHGetPathFromIDList (idl, lpstrDisplay))	// get path string from ITEMIDLIST
		{
			SetWindowText(lpstrDisplay);		// update edit control
			bReturnValue = TRUE;
		}
		LPMALLOC lpm;
		if (SHGetMalloc (&lpm) == NOERROR)
			lpm->Free(idl);						// free memory returned by SHBrowseForFolder
	}
	if (!bDisplay)								// reset m_pBROWSEINFO to clear the DisplayName
		m_pBROWSEINFO->pszDisplayName = NULL;
	if (!bProcedure)							// reset m_pBROWSEINFO to clear the default callback proc.
	{
		m_pBROWSEINFO->lpfn = NULL;
		m_pBROWSEINFO->lParam = oldLP;
	}
	SetFocus();									// ensure focus returns to this control
	return bReturnValue;
}

BOOL CFileEditCtrl::FECOpenFile()
{
	// Set up the CFileDialog and call CFileDialog::DoModal().
	// return TRUE if the user clicked the OK button, return FALSE otherwise
	BOOL bReturnValue = FALSE;
	BOOL bDirectory = TRUE;						// assume user of this class has set the initial directory
	TCHAR lpstrDirectory[_MAX_PATH] = _T("");
	if (m_pCFileDialog->m_ofn.lpstrInitialDir == NULL)	// user has not set the initial directory
	{											// flag it, set initial directory to
		bDirectory = FALSE;						// directory in edit control
		POSITION pos = GetStartPosition();
		if (pos)
			_tcscpy(lpstrDirectory, GetNextPathName(pos));
		m_pCFileDialog->m_ofn.lpstrInitialDir = lpstrDirectory;
	}
	if (m_pCFileDialog->DoModal() == IDOK)		// Start the CFileDialog
	{											// user clicked OK, enter files selected into edit control
		CString szFileSeperator;
#if defined FEC_NORESOURCESTRINGS
		szFileSeperator = (CString)FEC_IDS_SEPERATOR;
#else
		szFileSeperator.LoadString(FEC_IDS_SEPERATOR);
#endif
		ASSERT (_tcslen(szFileSeperator) == 1);	// must be one character only
		szFileSeperator += _T(" ");
		CString szPath;
		POSITION pos = m_pCFileDialog->GetStartPosition();
		if (pos)
			szPath = m_pCFileDialog->GetNextPathName(pos);	// first file has complete path
		while (pos)
		{
			CString szTempPath = m_pCFileDialog->GetNextPathName(pos);
			TCHAR lpstrName[_MAX_PATH];			// remaining files are name and extension only
			TCHAR lpstrExt[_MAX_PATH];
			_tsplitpath(szTempPath, NULL, NULL, lpstrName, lpstrExt);
			szPath += szFileSeperator + lpstrName + lpstrExt;
		}
		SetWindowText (szPath);
		bReturnValue = TRUE;
	}
	if (!bDirectory)							// reset OPENFILENAME
		m_pCFileDialog->m_ofn.lpstrInitialDir = NULL;
	SetFocus();									// ensure focus returns to this control
	return bReturnValue;
}

void CFileEditCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	inherited::OnSetFocus(pOldWnd);
	// Select all the text
	SetSel(0,-1);
}

BOOL CFileEditCtrl::SetFindFolder(BOOL bFindFolder)
{
	// initialize the internal data to browse for files or folders.
	// returns TRUE on success, FALSE on failure
	if (bFindFolder)
	{
		// if bFindFolder is TRUE, sets the control to find folders
		if (m_bFindFolder == bFindFolder && m_pBROWSEINFO)
			return TRUE;						// already set to find folders
		m_pBROWSEINFO = new BROWSEINFO();
		if (!m_pBROWSEINFO)						// failed to create BROWSEINFO structure
			return FALSE;
		if (m_pCFileDialog)
		{
			delete m_pCFileDialog;				// delete the CFileDialog
			m_pCFileDialog = NULL;
		}
		// set up the browseinfo structure used by SHBrowseForFolder()
		::ZeroMemory(m_pBROWSEINFO, sizeof(BROWSEINFO));
		m_pBROWSEINFO->hwndOwner = GetSafeHwnd();
		m_pBROWSEINFO->ulFlags = BIF_RETURNONLYFSDIRS;
	}
	else
	{
		// if bFindFolder is FALSE, sets the control to find files
		if (m_bFindFolder == bFindFolder && m_pCFileDialog)
			return TRUE;						// already set to find files
		// create the CFileDialog
		CString szFilter;
#if defined FEC_NORESOURCESTRINGS
		szFilter = FEC_IDS_ALLFILES;
#else	
		szFilter.LoadString(FEC_IDS_ALLFILES);
#endif
		m_pCFileDialog = new CFECFileDialog(TRUE,
			NULL,
			NULL,
			OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
			szFilter,
			this);
		if (!m_pCFileDialog)					// failed to create the CFileDialog
			return FALSE;
		if (m_pBROWSEINFO)
		{
			delete m_pBROWSEINFO;				// delete the BROWSEINFO structure
			m_pBROWSEINFO = NULL;
		}
		// set up the CFileDialog
		m_pCFileDialog->m_ofn.hwndOwner = GetSafeHwnd();
#if defined FEC_NORESOURCESTRINGS
		m_pCFileDialog->m_ofn.lpstrTitle = FEC_IDS_FILEDIALOGTITLE;
#else
		m_szCaption.LoadString(FEC_IDS_FILEDIALOGTITLE);
		m_pCFileDialog->m_ofn.lpstrTitle = (LPCTSTR)m_szCaption;
#endif
	}
	m_bFindFolder = bFindFolder;
	return TRUE;
}

void CFileEditCtrl::FillBuffers()
{
	// initializes the m_szFolder and m_lpstrFiles member variables
	// these variables are used by the GetStartPosition() and 
	// GetNextPathName() functions to retrieve the file names entered
	// by the user.
	ASSERT(IsWindow(this));						// Control window must exist
#if defined FEC_NORESOURCESTRINGS
	m_szFolder = FEC_IDS_SEPERATOR;
#else
	m_szFolder.LoadString(FEC_IDS_SEPERATOR);
#endif
	TCHAR chSeperator = m_szFolder[0];			// get the character used to seperate the files

	m_szFolder.Empty();							// empty the buffers of old data
	if (m_lpstrFiles)
	{
		delete m_lpstrFiles;
		m_lpstrFiles = NULL;
	}

	int len = GetWindowTextLength();
	if (!len)									// no files entered, leave buffers empty
		return;
	LPTSTR lpstrWindow = new TCHAR[len + 1];	// working buffer
	GetWindowText(lpstrWindow, len + 1);
	LPTSTR lpstrStart = lpstrWindow;			// points to the first character in a file name
	LPTSTR lpstrEnd = NULL;						// points to the next seperator character
	while(*lpstrStart == chSeperator || _istspace(*lpstrStart))
		lpstrStart++;							// skip over leading spaces and seperator characters
	if (!*lpstrStart)
	{											// no files entered, leave buffers empty
		delete lpstrWindow;
		return;
	}
	lpstrEnd = _tcschr(lpstrStart, chSeperator);// find seperator character
	if (lpstrEnd)								// mark end of string
		*lpstrEnd = 0;
	if (!lpstrEnd || m_bFindFolder || (!m_bFindFolder && !(m_pCFileDialog->m_ofn.Flags & OFN_ALLOWMULTISELECT)))
//	if (only one entry || find folder || (find file && find only one file))
	{
		m_lpstrFiles = new TCHAR[_MAX_PATH];
		ZeroMemory(m_lpstrFiles,_MAX_PATH);
		_tfullpath(m_lpstrFiles, lpstrStart, _MAX_PATH); // get absolute path
/////////////////////////////////////////////////////////////////////////////////////
// uncomment this code to add a trailing slash to folders if it is not already there
//		int len = _tcslen(m_lpstrFiles);
//		if (m_bFindFolder && m_lpstrFiles[len - 1] != 0x5c)
//			m_lpstrFiles[len] = 0x5c;
/////////////////////////////////////////////////////////////////////////////////////
		delete lpstrWindow;
		return;
	}
	_TCHAR Drive[_MAX_DRIVE];
	_TCHAR Folder[_MAX_PATH];
	_TCHAR File[_MAX_PATH];
	_TCHAR Ext[_MAX_PATH];
	_tsplitpath(lpstrStart, Drive, Folder, File, Ext);
	m_szFolder = (CString)Drive + Folder;		// drive and directory placed in m_szFolder
	m_lpstrFiles = new TCHAR[len + 1];
	ZeroMemory(m_lpstrFiles, len + 1);
	_tcscpy(m_lpstrFiles, File);				// file and extension of first file placed in m_lpstrFiles
	_tcscat(m_lpstrFiles, Ext);
	lpstrStart = lpstrEnd + 1;					// reset to the start of the next string
	LPTSTR pointer = m_lpstrFiles + _tcslen(m_lpstrFiles) + 1;	// location where next file will be placed in m_lpstrFiles
	while (lpstrEnd)
	{	// add the rest of the files m_lpstrFiles as they have been typed (include any path information)
		while(*lpstrStart == chSeperator || _istspace(*lpstrStart))
			lpstrStart++;						// remove leading spaces and seperator characters
		if (!*lpstrStart)						// last file was followed by spaces and seperator characters,
			break;								// there are no more files entered
		lpstrEnd = _tcschr(lpstrStart, chSeperator); // find next seperator character
		if (lpstrEnd)
			*lpstrEnd = 0;		// mark end of string
		_tcscpy(pointer, lpstrStart);			// copy string to its location in m_lpstrFiles
		pointer += _tcslen(pointer) + 1;		// reset pointer to accept next file
		if (lpstrEnd)
			lpstrStart = lpstrEnd + 1;			// reset to the start of the next string
	}
	delete lpstrWindow;							// delete working buffer
}

BROWSEINFO* CFileEditCtrl::GetBrowseInfo() const
{
	// returns a pointer to the BROWSEINFO structure
	// so the user of this control can set their own
	// parameters for SHBrowseForFolder
	return m_pBROWSEINFO;
}

BOOL CFileEditCtrl::GetFindFolder()
{
	// returns TRUE if this control is used to find folders,
	// FALSE if it is used to find files.
	return m_bFindFolder;
}

CString CFileEditCtrl::GetNextPathName(POSITION &pos)
{
	// Returns the file name at the specified position in the buffer.
	// The starting position is retrieved using the GetStartPosition() function.
	// The position is updated to point to the next file, or set to NULL if
	// there are no more files.
	ASSERT (pos);								// pos must not be NULL
	LPTSTR str = (LPTSTR)pos;					// str points to file name at pos
	TCHAR lpstrReturnString[_MAX_PATH];
	CString szTemp;
	if (str[1] == ':')
		szTemp = str;							// str contains complete path
	else
		szTemp = m_szFolder + str;				// build the path
	_tfullpath(lpstrReturnString, szTemp, _MAX_PATH);	// get absolute path from any relative paths
	str += _tcslen(str) + 1;					// set pos to next file
	pos = *str ? (POSITION)str : NULL;
	return (CString)lpstrReturnString;
}

OPENFILENAME* CFileEditCtrl::GetOpenFileName() const
{
	// returns a pointer to the OPENFILENAME structure so the
	// user can modify the Open File dialog
	if (m_pCFileDialog)
		return (&m_pCFileDialog->m_ofn);
	return NULL;
}

POSITION CFileEditCtrl::GetStartPosition()
{
	// if the window is active, fills the buffers with the text in the window.
	// returns a pointer to the buffer, type-casted as an MFC POSITION structure
	if (IsWindow(this) && m_bTextChanged)
	{
		FillBuffers();
		m_bTextChanged = FALSE;
	}
	return (POSITION)m_lpstrFiles;
}

void CFileEditCtrl::OnChange() 
{
	// Flags when the window text has changed since FillBuffers() was called.
	// If it has, then FillBuffers will be called in GetStartPosition()
	m_bTextChanged = TRUE;
}

void CFileEditCtrl::OnDestroy() 
{
	if (m_bAutoDelete)	// delete this CFileEditCtrl object? Only used when control was created
		delete this;	// in DDX_FileEditCtrl(CDataExchange*,int,CString&,BOOL)
}

void CFileEditCtrl::OnDropFiles(HDROP hDropInfo) 
{
	// handles drag and drop file entry, control must have the
	// WS_EX_ACCEPTFILES extended style set.
	CString szSeperator;
#if defined FEC_NORESOURCESTRINGS
	szSeperator = FEC_IDS_SEPERATOR;
#else
	szSeperator.LoadString(FEC_IDS_SEPERATOR);
#endif
	ASSERT (_tcslen(szSeperator) == 1);			// must be one character only
	szSeperator += _T(" ");						// get the file seperator character
	CString szDroppedFiles;						// buffer to contain all the dropped files

	TCHAR lpstrDropBuffer[_MAX_PATH];
	UINT nDropCount = DragQueryFile(hDropInfo,0xffffffff,NULL,0);
	if (nDropCount && (m_bFindFolder || (!m_bFindFolder && !(m_pCFileDialog->m_ofn.Flags & OFN_ALLOWMULTISELECT))))
		nDropCount = 1;
	if (nDropCount)
	{
		DragQueryFile(hDropInfo, 0, lpstrDropBuffer, _MAX_PATH);
		szDroppedFiles = lpstrDropBuffer;
	}
	for (UINT x = 1; x < nDropCount; x++)
	{
		DragQueryFile(hDropInfo, x, lpstrDropBuffer, _MAX_PATH);
		szDroppedFiles += szSeperator;
		szDroppedFiles += lpstrDropBuffer;
	}
	DragFinish (hDropInfo);
	SetWindowText (szDroppedFiles);
}

void CFileEditCtrl::OnEnable(BOOL bEnable) 
{
	// enables/disables the control
	inherited::OnEnable(bEnable);
	DrawButton (bEnable ? BTN_UP : BTN_DISABLED);
}

void CFileEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// check for CTRL + 'period' keystroke, if found, simulate a mouse click on the button
	if ((nChar == 0xBE || nChar == 0x6E) && GetKeyState(VK_CONTROL) < 0)
		ButtonClicked();
	inherited::OnKeyDown(nChar, nRepCnt, nFlags);
}

//--------------------------------------------------------------------------//
// Because the mouse is captured in OnNcLButtonDown(), we have to respond	//
// to WM_LBUTTONUP and WM_MOUSEMOVE messages.								//
// The m_bMouseCaptured variable is used because inherited::OnLButtonDown()		//
// also captures the mouse, so using GetCapture() could give an invalid		//
// response.																//
// The point is offset by one pixel for aesthetic purposes					//
//--------------------------------------------------------------------------//

void CFileEditCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	inherited::OnLButtonUp(nFlags, point);

	// Release the mouse capture and draw the button as normal. If the
	// cursor is over the button, simulate a click by carrying
	// out the required action.
	if (m_bMouseCaptured)
	{
		ReleaseCapture();
		m_bMouseCaptured = FALSE;
		if (m_nButtonState != BTN_UP)
			DrawButton(BTN_UP);
		point.Offset(1,1);
		if (m_rcButtonArea.PtInRect(point))
			ButtonClicked();
	}
}

void CFileEditCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	inherited::OnMouseMove(nFlags, point);

	// presses and releases the button as the mouse is moved over and
	// off the button. we check the current button state to avoid
	// unnecessary flicker
	if (m_bMouseCaptured)
	{
		point.Offset(1,1);
		if (m_rcButtonArea.PtInRect(point))
		{
			if (m_nButtonState != BTN_DOWN)
				DrawButton (BTN_DOWN);
		}
		else if (m_nButtonState != BTN_UP)
			DrawButton (BTN_UP);
	}
}

void CFileEditCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	// calculate the size of the client area and the button.
	inherited::OnNcCalcSize(bCalcValidRects, lpncsp);
	// set button area equal to client area of edit control
	m_rcButtonArea = lpncsp->rgrc[0];
	// shrink right side of client area by 80% of the height of client area
	lpncsp->rgrc[0].right -= (lpncsp->rgrc[0].bottom - lpncsp->rgrc[0].top) * 8/10;
	// shrink the button so its left side is at the right side of client area
	m_rcButtonArea.left = lpncsp->rgrc[0].right;
	if (bCalcValidRects)
		// convert button coordinates from parent client coordinates to control window coordinates
		m_rcButtonArea.OffsetRect(-lpncsp->rgrc[1].left, -lpncsp->rgrc[1].top);
	m_rcButtonArea.NormalizeRect();

	// MDM	2/8/2001 2:40:00 PM
	// I like it a bit bigger so the button is "outside" the edit box.
	m_rcButtonArea.InflateRect( 0, 2, 2, 2 );
}

UINT CFileEditCtrl::OnNcHitTest(CPoint point) 
{
	// If the mouse is over the button, OnNcHitTest() would normally return
	// HTNOWHERE, and we would not get any mouse messages. So we return 
	// HTBORDER to ensure we get them.
	UINT where = inherited::OnNcHitTest(point);
	if (where == HTNOWHERE)
	{
		ScreenToClient(&point);
		if (m_rcButtonArea.PtInRect(point))
			where = HTBORDER;
	}
	return where;
}

void CFileEditCtrl::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	inherited::OnNcLButtonDown(nHitTest, point);

	// Capture mouse input, set the focus to this control,
	// and draw the button as pressed
	ScreenToClient(&point);
	point.Offset(1,1);
	if (m_rcButtonArea.PtInRect(point))
	{
		SetCapture();
		m_bMouseCaptured = TRUE;
		SetFocus();
		DrawButton(BTN_DOWN);
	}
}

void CFileEditCtrl::OnNcPaint() 
{
	inherited::OnNcPaint();				// draws the border around the control
	DrawButton (m_nButtonState);	// draw the button in its current state
}

/////////////////////////////////////////////////////////////////////////////
// DDV_FileEditCtrl & DDX_FileEditCtrl

void DDV_FileEditCtrl(CDataExchange *pDX, int nIDC)
{
	// verify the files or folder entered actually exists
	// verify that files are entered when looking for files
	// and that folders are entered when looking for folders
	CWnd *pWnd = pDX->m_pDlgWnd->GetDlgItem(nIDC);
	if(!pWnd->IsKindOf(RUNTIME_CLASS(CFileEditCtrl)))	// is this control a CFileEditCtrl control?
	{
		TRACE (_T("Control %d not subclassed to CFileEditCtrl, must first call DDX_FileEditCtrl()"),nIDC);
		ASSERT(FALSE);
		AfxThrowNotSupportedException();
	}
	if(!pDX->m_bSaveAndValidate)				// not saving data
		return;

	if ( !pWnd->IsWindowEnabled() )
		return;

	CFileEditCtrl *pFEC = (CFileEditCtrl *)pWnd;
	pDX->PrepareEditCtrl(nIDC);
	POSITION pos = pFEC->GetStartPosition();
	if (!pos)
	{	// no name entered
		AfxMessageBox(FEC_IDS_NOFILE);
		pDX->Fail();
	}
	while(pos)
	{
		CString szMessage;
		CString szFile = pFEC->GetNextPathName(pos);
		DWORD dwAttribute = GetFileAttributes(szFile);
		if (dwAttribute == 0xFFFFFFFF)			// GetFileAttributes() failed
		{										// does not exist
			szMessage.Format(FEC_IDS_NOTEXIST, szFile);
			AfxMessageBox(szMessage);
			pDX->Fail();
		}
		if (pFEC->GetFindFolder() && !(dwAttribute & FILE_ATTRIBUTE_DIRECTORY))
		{										// not a folder
			szMessage.Format(FEC_IDS_NOTFOLDER, szFile);
			AfxMessageBox(szMessage);
			pDX->Fail();
		}
		if (!pFEC->GetFindFolder() && dwAttribute & FILE_ATTRIBUTE_DIRECTORY)
		{										// not a file
			szMessage.Format(FEC_IDS_NOTFILE, szFile);
			AfxMessageBox(szMessage);
			pDX->Fail();
		}
	}
}

void DDX_FileEditCtrl(CDataExchange *pDX, int nIDC, CString& rStr, BOOL bFindFolder)
{
	// Subclasses the edit control with Id nIDC. Coordinates the window text
	// with the CString. bFindFolder determines if the control is used to
	// browse for files or folders.
	CWnd *pWnd = pDX->m_pDlgWnd->GetDlgItem(nIDC);
	if (pDX->m_bSaveAndValidate)				// update string with text from control
	{
		// ensure the control is a CFileEditCtrl control
		if (pWnd->IsKindOf(RUNTIME_CLASS(CFileEditCtrl)))
		{
			// copy the first file listed in the control to the string
			rStr.Empty();
			CFileEditCtrl *pFEC = (CFileEditCtrl *)pWnd;
			POSITION pos = pFEC->GetStartPosition();
			if (pos)
				rStr = pFEC->GetNextPathName(pos);
		}
	}
	else										// create the control if it is not already created
	{											// set the control text to the text in string
		CFileEditCtrl *pFEC = NULL;
		if (!pWnd->IsKindOf(RUNTIME_CLASS(CFileEditCtrl)))    // not subclassed yet
		{
			// create then subclass the control to the edit control with the ID nIDC
			HWND hWnd = pDX->PrepareEditCtrl(nIDC);
			pFEC = new CFileEditCtrl(TRUE);		// create the control with autodelete
			if (!pFEC->SubclassWindow(hWnd))
			{									// failed to subclass the edit control
				ASSERT(FALSE);
				AfxThrowNotSupportedException();
			}
			// Force a call to CFileEditCtrl::OnNcCalcSize() to calculate button size
			pFEC->SetWindowPos(NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			// call CFileEditCtrl::FindFolder() to initialize the internal data structures
			pFEC->SetFindFolder(bFindFolder);
		}
		else									// control already exists
			pFEC = (CFileEditCtrl *)pWnd;
		if (pFEC)
			pFEC->SetWindowText(rStr);			// set the control text
	}
}

void DDX_FileEditCtrl(CDataExchange *pDX, int nIDC, CFileEditCtrl &rCFEC, BOOL bFindFolder)
{
	// Subclass the specified CFileEditCtrl class object to the edit control
	// with the ID nIDC. bFindFolder determines if the control is used to
	// browse for files or folders.
	if (rCFEC.m_hWnd == NULL)					// not yet subclassed
	{
		ASSERT (!pDX->m_bSaveAndValidate);
		// subclass the control to the edit control with the ID nIDC
		HWND hWnd = pDX->PrepareEditCtrl(nIDC);
		if (!rCFEC.SubclassWindow(hWnd))
		{										// failed to subclass the edit control
			ASSERT(FALSE);
			AfxThrowNotSupportedException();
		}
		// Force a call to CFileEditCtrl::OnNcCalcSize() to calculate button size
		rCFEC.SetWindowPos(NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		// call CFileEditCtrl::FindFolder() to initialize the internal data structures
		rCFEC.SetFindFolder(bFindFolder);
	}
	else if (pDX->m_bSaveAndValidate)
		rCFEC.GetStartPosition();				// updates the data from the edit control
}

/////////////////////////////////////////////////////////////////////////////
// CFECFileDialog

IMPLEMENT_DYNAMIC(CFECFileDialog, CFileDialog)

CFECFileDialog::CFECFileDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
}


BEGIN_MESSAGE_MAP(CFECFileDialog, CFileDialog)
	//{{AFX_MSG_MAP(CFECFileDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CFECFileDialog::OnInitDialog() 
{
	CFileDialog::OnInitDialog();
	
	// Set the text of the IDOK button on an
	// old style dialog to 'OK'
	if (!(m_ofn.Flags & OFN_EXPLORER))
	{
		CString szOkButton;
#if defined FEC_NORESOURCESTRINGS
		szOkButton = FEC_IDS_OKBUTTON;
#else
		szOkButton.LoadString(FEC_IDS_OKBUTTON);
#endif
		GetDlgItem(IDOK)->SetWindowText(szOkButton);
	}
	return TRUE;
}

void CFECFileDialog::OnInitDone()
{
	// Set the text of the IDOK button on an
	// Explorer style dialog to 'OK'
	CString szOkButton;
#if defined FEC_NORESOURCESTRINGS
	szOkButton = FEC_IDS_OKBUTTON;
#else
	szOkButton.LoadString(FEC_IDS_OKBUTTON);
#endif
	CommDlg_OpenSave_SetControlText(GetParent()->m_hWnd, IDOK, (LPCTSTR)szOkButton);
}
