/*===========================================================================
====                                                                     ====
====    File name           :  EditTrans.cpp                             ====
====    Creation date       :  7/10/2001                                 ====
====    Author(s)           :  Dany Cantin                               ====
====                                                                     ====
===========================================================================*/

#include "stdafx.h"
#include "EditTrans.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditTrans

CEditTrans::CEditTrans()
{
    m_TextColor = RGB(0, 0, 0);
    m_BackColor = TRANS_BACK;
}

CEditTrans::~CEditTrans()
{
}


BEGIN_MESSAGE_MAP(CEditTrans, CEdit)
	//{{AFX_MSG_MAP(CEditTrans)
	ON_WM_CTLCOLOR_REFLECT()
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
	ON_WM_LBUTTONDOWN()
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnKillfocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditTrans message handlers

HBRUSH CEditTrans::CtlColor(CDC* pDC, UINT nCtlColor) 
{
    m_Brush.DeleteObject();

    if (m_BackColor == TRANS_BACK) {
        m_Brush.CreateStockObject(HOLLOW_BRUSH);
        pDC->SetBkMode(TRANSPARENT);
    }
    else {
        m_Brush.CreateSolidBrush(m_BackColor);
        pDC->SetBkColor(m_BackColor);
    }

    pDC->SetTextColor(m_TextColor);

    return (HBRUSH)m_Brush;
}


void CEditTrans::OnKillfocus() 
{
    UpdateCtrl();
}


void CEditTrans::OnUpdate() 
{
    UpdateCtrl();
}


void CEditTrans::OnLButtonDown(UINT nFlags, CPoint point) 
{
    UpdateCtrl();
    
    inherited::OnLButtonDown(nFlags, point);
}


void CEditTrans::UpdateCtrl()
{
    CWnd* pParent = GetParent();
    CRect rect;
    
    GetWindowRect(rect);
    pParent->ScreenToClient(rect);
    rect.DeflateRect(2, 2);
    
    pParent->InvalidateRect(rect, FALSE);    
}


//-------------------------------------------------------------------//
// OnKeyUp()																			//
//-------------------------------------------------------------------//
// Fix as posted in CodeProject comment.
//-------------------------------------------------------------------//
void CEditTrans::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ( nChar == VK_PRIOR || nChar == VK_NEXT || 
	nChar == VK_END || nChar == VK_HOME || 
	nChar == VK_LEFT || nChar == VK_UP || 
	nChar == VK_RIGHT || nChar == VK_DOWN )
	{
		UpdateCtrl();
	}
	inherited::OnKeyUp( nChar, nRepCnt, nFlags );
}


// Further code...
/*
void CTransEdit::onkeyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
if ( nChar == VK_PRIOR || nChar == VK_NEXT || 
nChar == VK_END || nChar == VK_HOME || 
nChar == VK_LEFT || nChar == VK_UP || 
nChar == VK_RIGHT || nChar == VK_DOWN )
{
UpdateCtrl();
}
CEdit::onkeyup( nChar, nRepCnt, nFlags );
}

Another different approach could be if we use a pattern brush with a bitmap which duplicate the drawing on the parent window. The code for the CtlColor message will look like:

HBRUSH CTransEdit::CtlColor(CDC* pDC, UINT nCtlColor) 
{
m_Brush.DeleteObject();

if ( m_BackColor == TRANS_BACK )
{
m_Brush.CreatePatternBrush( &m_Background );
m_Brush.UnrealizeObject();

pDC->SelectObject( &m_Brush );

pDC->SetBrushOrg( 0, 0 );

pDC->SetBkMode(TRANSPARENT);
else
{
m_Brush.CreateSolidBrush( m_BackColor ); 
}

return m_Brush;
}

To create the background bitmap we can use a code like the above:
(The rectangle "pos" is the position of the editor inside the parent window. We assume here that we have the bitmap m_drawBmp in the parent window, with the whole window content in it).

void CTransEdit::PrepareBackground( CRect pos )
{
ASSERT_VALID( m_pView );

CClientDC thisDC( this );
CBitmap* pOldThisBmp;
CDC thisMem;

CClientDC viewDC( m_pView );
CBitmap* pOldViewBmp;
CDC viewMem;

m_Background.DeleteObject();
m_Background.CreateCompatibleBitmap( &thisDC, pos.Size().cx, pos.Size().cy );

thisMem.CreateCompatibleDC( &thisDC );
pOldThisBmp = thisMem.SelectObject( &m_Background );

viewMem.CreateCompatibleDC( &viewDC );
pOldViewBmp = viewMem.SelectObject( &m_pView->m_drawBmp );

thisMem.BitBlt( 0, 0, pos.Size().cx, pos.Size().cy, &viewMem, pos.left, pos.top, SRCCOPY );

thisMem.SelectObject( pOldThisBmp );
viewMem.SelectObject( pOldViewBmp );
} 

Jose Reyes.
email: jreyes_97@yahoo.com
*/