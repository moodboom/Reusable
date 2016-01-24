#if !defined(LIST_DATA_LONG_SLIDER_H)
#define LIST_DATA_LONG_SLIDER_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ListDataLongSlider.h : header file
//

#include "ListDataControlData.h"


/////////////////////////////////////////////////////////////////////////////
// ListDataLongSlider window

class ListDataLongSlider : public CSliderCtrl, public ListDataControlData< Long >
{
// Construction
public:
	ListDataLongSlider();

	void Initialize(
		Long*						pNewDBVar,
		Long*						pNewAltDBVar,
		int						nNewListColumn
	);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ListDataLongSlider)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ListDataLongSlider();

	void ExtractControlValue();

	CString GetValueAsString();
	CString GetAltVarAsString();

protected:

	void DisplayControlValue();

	// Generated message map functions
	//{{AFX_MSG(ListDataLongSlider)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(LIST_DATA_LONG_SLIDER_H)
