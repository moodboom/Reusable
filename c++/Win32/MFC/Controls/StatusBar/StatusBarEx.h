//-------------------------------------------------------------------//
//
// NOTE: Derived from code provided by Pierre Mellinand on 
// CodeGuru, 10/99.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef StatusBarEx_h_
#define StatusBarEx_h_

#include "../../STLExtensions.h"					// For std::vector<>

class StatusBarProgress;							// We can drop one into one of our panes.


///////////////////////////////////////////////////////////////////////////////////
// StatusBarExPaneControlInfo
class StatusBarExPaneControlInfo
{
public:
	
	CWnd*		m_hWnd;
	int		m_nPaneID;
	BOOL		m_bAutodelete;

	StatusBarExPaneControlInfo();
};


///////////////////////////////////////////////////////////////////////////////////
// StatusBarExPaneControlInfo
class StatusBarExPane
{

public:

	UINT		nID;
	int		cxText;
	UINT		nStyle;
	UINT		nFlags;
	CString	strText;

	StatusBarExPane();
	StatusBarExPane( const StatusBarExPane& sbepSource );
	StatusBarExPane& operator=( const StatusBarExPane& sbepSource );

};


///////////////////////////////////////////////////////////////////////////////////
// StatusBarEx
class StatusBarEx:public CStatusBar
{
	typedef CStatusBar inherited;

	DECLARE_DYNCREATE(StatusBarEx)

public:

	StatusBarEx();
	virtual ~StatusBarEx();

	// Use these to display progress.
	// Always start with the first and end with the third.
	virtual void SetMaxProgress( int nMax );
	virtual void SetProgress( int nPos );
	virtual void EndProgress();

	BOOL AddDynamicPane( int position, UINT paneID, HICON hIcon = 0 );
	void RemovePane( int nPaneID );

	virtual void SetIcon( int nPaneID, HICON hIcon );
	
	BOOL AttachControl( CWnd * pWnd, int nPaneID, BOOL bAutodeleteControl = TRUE );

	void SetPaneWidth( int index, int cxWidth );

	void PositionControls();
	StatusBarExPaneControlInfo* GetStatusPaneControl( int nPaneID );
	BOOL GetStatusPane( int nIndex, StatusBarExPane & xfxpane );

	//{{AFX_MSG(MPCStatusPane)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:

	// This can let us know if one of our panes was clicked, etc.
	virtual BOOL OnChildNotify(UINT message,WPARAM wParam,LPARAM lParam,LRESULT* pResult);

	void UpdatePaneIcons();

	StatusBarProgress* m_pStatusProgress;

	std::vector<StatusBarExPaneControlInfo*>	m_vPanes;

	std::vector<HICON> m_vIcons;
	
};

#endif