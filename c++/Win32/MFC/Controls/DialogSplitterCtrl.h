#if !defined DIALOG_SPLITTER_CTRL_H
#define DIALOG_SPLITTER_CTRL_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


// DialogSplitterCtrl.h : header file
//
//	To use this class in a dialog:
//		1) Declare a DialogSplitterCtrl member.
//		2) Place a button in the dlg template.
//		3) Bind the DialogSplitterCtrl member to the button
//			in OnInitDialog(), after calling CDialog::OnInitDialog().
//		4) Process the following msgs in the parent:
//
//				WM_SPLITTER_MOVED					Sent as splitter is dragged
//				WM_SPLITTER_FINISHED_MOVE		Sent after user finishes splitter drag
//
//			Typically, you will track "pane sizes", and adjust them in
//			handlers for the above msgs.  Then, just do your OnSize() override
//			anticipating that the pane size variables are all set for you.
//
//	To use this class in a CWnd (such as CView):
//		1) Declare a DialogSplitterCtrl member m_SplitterCtrl.
//		2)	Declare a CButton member m_SplitterRoot.
//		3) Bind the DialogSplitterCtrl member to the button
//			in CView::OnCreate() as follows:
			
			/*

			// Set up the splitter control.
			if ( 
				!m_SplitterRoot.Create( 
					NULL,
					WS_CHILD | WS_VISIBLE,
					CRect(),
					this,
					IDC_EXPLORER_SPLITTER
				)
			) {
				TRACE0( "Unable to create splitter button.\n" );
				return -1;
			}

			// If we want to subclass an HWND, it cannot have any
			// MFC object attached to it.  We used MFC to create it,
			// avoiding having to get this working:
			//
			// 	HWND m_hSplitterControl = ::CreateWindow(
			// 		_T("BUTTON"),
			// 		NULL,
			// 		WS_CHILD | WS_VISIBLE,
			// 		0, 0, 10, 10,
			// 		GetSafeHwnd(),
			// 		(HMENU)IDC_EXPLORER_SPLITTER,
			// 		AfxGetInstanceHandle(),
			// 		NULL
			//		);
			//
			// Now unhook MFC and it is as if we did the hard work
			// of creating from scratch ourselves.  Pretty cool.
			m_SplitterRoot.UnsubclassWindow();

			m_SplitterCtrl.BindWithControl( this, IDC_EXPLORER_SPLITTER );

			*/
//
//		4) (see step 4 for dialogs, above)
//
//
//	NOTE: YOU WILL NOT SEE ANY SPLITTER CURSORS WITH THE DEFAULT APPWIZARD-GENERATED PROJECT.
//			YOU MUST REMOVE THE FOLLOWING LINE IN THE "Resource Includes" SECTION OF YOUR RESOURCE FILE:
//
//		#define _AFX_NO_SPLITTER_RESOURCES
//
// Select "View|Resource Includes" and remove the line.
//
//


/////////////////////////////////////////////////////////////////
// Globals Statics Constants												//
/////////////////////////////////////////////////////////////////
//
#define WM_SPLITTER_MOVED				(WM_USER+1100)
#define WM_SPLITTER_FINISHED_MOVE	(WM_USER+1101)
//
/////////////////////////////////////////////////////////////////


class DialogSplitterCtrl : public CWnd
{
// Construction
public:
	DialogSplitterCtrl( bool bVertical = true );

// Attributes
public:

// Operations
public:
	BOOL BindWithControl(CWnd *parent, DWORD ctrlId);
	void Unbind(void);

	void SetLeftBorder	( int nLeft		)	{ 	ASSERT( m_bVertical == true	);	m_nLeftBorder	= nLeft;		}
	void SetRightBorder	( int nRight	)	{	ASSERT( m_bVertical == true	);	m_nRightBorder	= nRight;	}

	void SetTopBorder		( int nTop		)	{ 	ASSERT( m_bVertical == false	);	m_nLeftBorder	= nTop;		}
	void SetBottomBorder	( int nBottom	)	{	ASSERT( m_bVertical == false	);	m_nRightBorder	= nBottom;	}

	/*
	BOOL AttachAsLeftPane(DWORD ctrlId);
	BOOL AttachAsRightPane(DWORD ctrlId);
	BOOL DetachAllPanes(void);
	void RecalcLayout(void);
	BOOL GetMouseClipRect(LPRECT rectClip, CPoint point);
	*/

protected:

	// This variable determines the orientation of the
	// splitter bar ( which is either vertical or horizontal).
	bool m_bVertical;

	CWnd        *m_Parent;
	int         m_nLeftBorder, m_nRightBorder;

	// CDWordArray m_leftIds, m_rightIds;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogSplitterCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~DialogSplitterCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(DialogSplitterCtrl)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};







/*

/////////////////////////////////////////////////////////////////////////////
// CySplitterWnd window

class CySplitterWnd : public CWnd
{
// Construction
public:
	CySplitterWnd();

// Attributes
public:

// Operations
public:
    BOOL BindWithControl(CWnd *parent, DWORD ctrlId);
    void Unbind(void);

    void SetMinHeight(int above, int below);

    BOOL AttachAsAbovePane(DWORD ctrlId);
    BOOL AttachAsBelowPane(DWORD ctrlId);
    BOOL DetachAllPanes(void);
    void RecalcLayout(void);

protected:
    BOOL GetMouseClipRect(LPRECT rectClip, CPoint point);

    CWnd        *m_Parent;
    CDWordArray m_aboveIds, m_belowIds;
    int         m_minAbove, m_minBelow;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CySplitterWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CySplitterWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CySplitterWnd)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

*/


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined DIALOG_SPLITTER_CTRL_H
