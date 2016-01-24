#if !defined(AFX_CHEVBAR_H__ADA7D7F6_E2C1_11D3_84C7_0080450EA020__INCLUDED_)
#define AFX_CHEVBAR_H__ADA7D7F6_E2C1_11D3_84C7_0080450EA020__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Added it here just in case somebody dont have the latest headers 
// Have to update the build environ, copied from commctrl.h
#define RBBS_USECHEVRON				0x00000200 
#define RBN_CHEVRONPUSHED			(RBN_FIRST - 10)
/*
typedef struct tagNMREBARCHEVRON
{
    NMHDR hdr;
    UINT uBand;
    UINT wID;
    LPARAM lParam;
    RECT rc;
    LPARAM lParamNM;

} NMREBARCHEVRON, *LPNMREBARCHEVRON;
*/

#include "ChevDrop.h"

//	This is the rebar class. The rebar to support chevrons have to derive from this class
//	and have to call AddBar( ). It is preferred to have toolbars as child windows. 

class CChevBar : public CReBar
{

	CChevDrop	*m_pDropWindow;

public:
	CChevBar();

	// This method is to add a band to the Rebar
	// Toolbars are preferred now. The iID has to be unique for the toolbars that are added
	BOOL	AddBar( CWnd* pBar, int iID=-1, 
					LPCTSTR lpszText = NULL, 
					DWORD dwStyle = RBBS_GRIPPERALWAYS | RBBS_USECHEVRON );

public:

	//{{AFX_VIRTUAL(CChevBar)
	//}}AFX_VIRTUAL

public:
	virtual ~CChevBar();

protected:
	//{{AFX_MSG(CChevBar)
	//}}AFX_MSG

	/*
		Reflection handler for the Chevron push
	*/
	void OnChevronPushed( NMHDR * pNotifyStruct, LRESULT* result );

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CHEVBAR_H__ADA7D7F6_E2C1_11D3_84C7_0080450EA020__INCLUDED_)
