#if !defined(TAB_DLG_CTRL_H)
#define TAB_DLG_CTRL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "TabDlgCtrlPage.h"

#ifdef _DEBUG
#pragma pack (push, 1)

typedef struct {
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
} DLGTEMPLATEEX;

#pragma pack (pop)
#endif // _DEBUG

/////////////////////////////////////////////////////////////////////////////
// TabDlgCtrl window

class TabDlgCtrl : public CTabCtrl 
{
typedef CTabCtrl inherited;

public:
// Construction
	TabDlgCtrl ();
// Destruction
	virtual ~TabDlgCtrl (void);
// Page Functions
	int		AddTabPage (LPCTSTR pszTitle, int nPageID, TabDlgCtrlPage* pTabPage, int nImageIndex = -1 );

   // MDM I don't care to get this one working right now.
   // Wouldn't be hard, just need to get a version set up
   // in FontDlg class...
   //   int		AddTabPage (LPCTSTR pszTitle, int nPageID,	LPCTSTR pszTemplateName);
	// int		AddTabPage (LPCTSTR pszTitle, int nPageID, int nTemplateID);

	BOOL	RemoveSSLPage (int nIndex);
	int		GetSSLPageCount (void);
	BOOL	GetSSLPageTitle (int nIndex, CString& strTitle);
	BOOL	SetSSLPageTitle (int nIndex, LPCTSTR pszTitle);
	int		GetSSLPageID (int nIndex);
	int		SetSSLPageID (int nIndex, int nPageID);
	BOOL	ActivateSSLPage (int nIndex);
	int		GetSSLActivePage (void);
	CWnd*	GetSSLPage (int nIndex);
	int		GetSSLPageIndex (int nPageID);

protected:
    struct TabDelete {
        TabDlgCtrlPage*   pTabPage;
        BOOL        bDelete;
    };
    CArray<TabDelete, TabDelete> m_tabs;
	CArray<HWND, HWND> m_hFocusWnd;
	CArray<int, int> m_nPageIDs;

	int AddPage (LPCTSTR pszTitle, int nPageID, TabDelete tabDelete, int nImageIndex = -1 );

    virtual BOOL OnInitPage (int nIndex, int nPageID);
	virtual void OnActivatePage (int nIndex, int nPageID);
	virtual void OnDeactivatePage (int nIndex, int nPageID);
	virtual void OnDestroyPage (int nIndex, int nPageID);
	virtual BOOL OnCommand (WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify (WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCmdMsg (UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);

#ifdef _DEBUG
	BOOL CheckDialogTemplate (LPCTSTR pszTemplateName);
#endif // _DEBUG
	void ResizeDialog (int nIndex, int cx, int cy);
	// Generated message map functions
protected:
	//{{AFX_MSG(TabDlgCtrl)
	afx_msg void OnDestroy (void);
	afx_msg void OnSetFocus (CWnd* pOldWnd);
	afx_msg void OnKillFocus (CWnd* pNewWnd);
	afx_msg void OnSelChanging (NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChange (NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(TAB_DLG_CTRL_H)
