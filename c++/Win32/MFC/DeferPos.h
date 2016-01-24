//-------------------------------------------------------------------//
// DeferPos.h interface																//
//
// This class wraps the BeginDeferWindowPos/DeferWindowPos/EndDeferWindowPos
// APIs using a "resource allocation is acquisition" idiom.
//
// There used to be MFC and Win32 versions of most functions.  I commented
// out the Win32 stuff cause I didn't wanna maintain it.  Sorry!
//
// This class is derived from code posted on CodeGuru quite a
// while back.
//-------------------------------------------------------------------//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#if !defined(DEFER_POS_H)
#define DEFER_POS_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <vector>
using namespace std;													// REQD for STL classes!


class CDeferPos
{
public:

	CDeferPos(
		int	nWindows		= 1
	);

	~CDeferPos();

	BOOL MoveWindow( 
		CWnd* pWnd, 
		int x, 
		int y, 
		int nWidth, 
		int nHeight, 
		BOOL bRepaint
	);
	
	BOOL SetWindowPos( 
		CWnd* pWnd, 
		CWnd* pWndAfter, 
		int x, 
		int y, 
		int nWidth, 
		int nHeight, 
		UINT uFlags
	);

protected:

	HDWP		m_hdwp;
};


// SIZING HELPER CLASS
//
// We always work our way from left to right across "touching"
// controls.  Each control that must be moved is added to a list.  
// When we have completed the list, we will draw left-to-right or
// right-to-left, depending on the resize direction.
//
// See existing instances of OnSize() functions for details.
class MoveData{
public:
	MoveData(
		CWnd*	pCtrl,
		int	nX,
		int	nY,
		int	nWidth,
		int	nHeight
	) :
		// Init vars.
		m_pCtrl	( pCtrl		),
		m_nX		( nX			),
		m_nY		( nY			),
		m_nWidth	( nWidth		),
		m_nHeight( nHeight	)
	{}

	CWnd*	m_pCtrl	;
	int	m_nX		;
	int	m_nY		;
	int	m_nWidth	;
	int	m_nHeight;

};


typedef vector< MoveData > MoveDataArray;


#endif // !defined(DEFER_POS_H)
