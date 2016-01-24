#if !defined(AFX_CURRENTPROCESSPRIORITYCB_H__23A0F11F_E4FB_4E22_BA8D_8C1031BA88A7__INCLUDED_)
#define AFX_CURRENTPROCESSPRIORITYCB_H__23A0F11F_E4FB_4E22_BA8D_8C1031BA88A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CurrentProcessPriorityCB.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CurrentProcessPriorityCB window

class CurrentProcessPriorityCB : public CComboBoxEx
{
// Construction
public:
	CurrentProcessPriorityCB( bool bImmediatelyUpdate = false );

// Attributes
public:

// Operations
public:

	DWORD SetProcessPriorityFromSelection();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CurrentProcessPriorityCB)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CurrentProcessPriorityCB();

protected:
	bool m_bImmediatelyUpdate;

	// Generated message map functions
protected:
	//{{AFX_MSG(CurrentProcessPriorityCB)
	afx_msg void OnSelChange();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CURRENTPROCESSPRIORITYCB_H__23A0F11F_E4FB_4E22_BA8D_8C1031BA88A7__INCLUDED_)
