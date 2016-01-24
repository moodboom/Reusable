#if !defined(LIST_DATA_DATE_EDIT_H)
#define LIST_DATA_DATE_EDIT_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ListDataDateEdit.h : header file
//

#include <Controls\EditTrans.h>														// For transparent background edit controls.

#include "..\..\BaseDateTime.h"			// This is our var type.  It also does our output for us.

#include "ListDataControlData.h"


/////////////////////////////////////////////////////////////////////////////
// ListDataDateEdit window

class ListDataDateEdit : public CEditTrans, public ListDataControlData< BaseDateTime >
{
	typedef CEditTrans inherited;

// Construction
public:
	ListDataDateEdit();

	void Initialize(
		BaseDateTime*			pNewDBVar,
		BaseDateTime*			pNewAltDBVar,
		int						nNewListColumn
	);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ListDataDateEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ListDataDateEdit();

	void ExtractControlValue();

	CString GetValueAsString();
	CString GetAltVarAsString();

protected:

	// This indicates when the text has been changed and we are ready
	// for an update.  Note that database data is never changed unless
	// a change has been made within the edit text.
	bool m_bUpdated;

	bool m_bMultiPrompt;

	void DisplayControlValue();

	// This is the helper that does the formatting.
	// Some derived classes override for special display formatting.
	virtual CString strOutput( BaseDateTime dtValue );

	// Generated message map functions
	//{{AFX_MSG(ListDataDateEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnUpdate();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(LIST_DATA_DATE_EDIT_H)
