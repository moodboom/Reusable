#if !defined(LIST_DATA_BYTE_COMBO_H)
#define LIST_DATA_BYTE_COMBO_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ListDataByteCombo.h : header file
//

#include "ListDataControlData.h"


/////////////////////////////////////////////////////////////////////////////
// ListDataByteCombo window

class ListDataByteCombo : public CComboBox, public ListDataControlData< BYTE >
{
// Construction
public:
	ListDataByteCombo();

	void Initialize(
		BYTE*						pNewDBVar,
		BYTE*						pNewAltDBVar,
		int						nNewListColumn,
		int						nNewMaxIndex
	);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ListDataByteCombo)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ListDataByteCombo();

	void ExtractControlValue();

	CString GetValueAsString();
	CString GetAltVarAsString();

	CString GetTextAtIndex( BYTE Index );

	int nMaxIndex;

protected:

	void DisplayControlValue();

	// Generated message map functions
	//{{AFX_MSG(ListDataByteCombo)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSelchange();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(LIST_DATA_BYTE_COMBO_H)
