//-------------------------------------------------------------------//
// ListDataLPTSTREdit
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#if !defined(LIST_DATA_LPTSTR_EDIT_H)
#define LIST_DATA_LPTSTR_EDIT_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ListDataLPTSTREdit.h : header file
//

#include <Controls\EditTrans.h>														// For transparent background edit controls.

#include "ListDataControlData.h"


/////////////////////////////////////////////////////////////////////////////
// ListDataLPTSTREdit window

class ListDataLPTSTREdit : public CEditTrans, public ListDataControlData< LPTSTR >
{
	typedef CEditTrans inherited;

// Construction
public:
	ListDataLPTSTREdit();

	void Initialize(
		LPTSTR*					pNewDBVar,
		LPTSTR*					pNewAltDBVar,
		int						nNewListColumn
	);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ListDataLPTSTREdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ListDataLPTSTREdit();

	void ExtractControlValue();

	CString GetValueAsString();
	CString GetAltVarAsString();

	// We need to override these to do allocating as needed.
	virtual void SetDBFromControl();
	virtual void SetControlFromDB();

protected:

	// This indicates when the text has been changed and we are ready
	// for an update.  Note that database data is never changed unless
	// a change has been made within the edit text.
	bool m_bUpdated;

	bool m_bMultiPrompt;

	void DisplayControlValue();

	// Generated message map functions
	//{{AFX_MSG(ListDataLPTSTREdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnUpdate();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(LIST_DATA_LPTSTR_EDIT_H)
