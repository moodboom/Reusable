// VolumeSliderCtrl.cpp: Implementation file
//
// Copyright (c) 1999 Daniel Frey
//
// Distribute and use freely, except:
// a) Don't alter or remove this notice.
// b) Mark the changes you make.
//
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk. Expect bugs.
//
// Send bug reports, bug fixes, enhancements, request, etc. to:
//
//	Daniel.Frey@hew-kabel-cdt.com
//
// History:
//
//	v1.0  19.04.1999  Initial release

#include "stdafx.h"

#include <math.h>

#include "..\MemDC.h"
#define USE_MEM_DC // Remove this, if you don't want to use CMemDC

#include "VolumeSliderCtrl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const double pi = 3.141592653589793238462643383279;

const int cnMaxSmallDrawingRadius = 20;

// The following 3 functions were taken from 'CRoundButton.cpp', written and
// copyright (c) 1997,1998 by Chris Maunder (chrismaunder@codeguru.com).
// To be honest, I never had a look at their implementation, I just use them.
// This is cut-and-paste-programming at its best... if it works... :)

// prototypes
COLORREF GetColour(double dAngle, COLORREF crBright, COLORREF crDark);
void DrawCircle(CDC* pDC, CPoint p, LONG lRadius, COLORREF crColour, BOOL bDashed = FALSE);
void DrawCircle(CDC* pDC, CPoint p, LONG lRadius, COLORREF crBright, COLORREF crDark);


// Calculate colour for a point at the given angle by performing a linear
// interpolation between the colours crBright and crDark based on the cosine
// of the angle between the light source and the point.
//
// Angles are measured from the +ve x-axis (i.e. (1,0) = 0 degrees, (0,1) = 90 degrees )
// But remember: +y points down!

COLORREF GetColour(double dAngle, COLORREF crBright, COLORREF crDark)
{
#define Rad2Deg	180.0/3.1415 
#define LIGHT_SOURCE_ANGLE	-2.356		// -2.356 radians = -135 degrees, i.e. From top left

	ASSERT(dAngle > -3.1416 && dAngle < 3.1416);
	double dAngleDifference = LIGHT_SOURCE_ANGLE - dAngle;

	if (dAngleDifference < -3.1415) dAngleDifference = 6.293 + dAngleDifference;
	else if (dAngleDifference > 3.1415) dAngleDifference = 6.293 - dAngleDifference;

	double Weight = 0.5*(cos(dAngleDifference)+1.0);

	BYTE Red   = (BYTE) (Weight*GetRValue(crBright) + (1.0-Weight)*GetRValue(crDark));
	BYTE Green = (BYTE) (Weight*GetGValue(crBright) + (1.0-Weight)*GetGValue(crDark));
	BYTE Blue  = (BYTE) (Weight*GetBValue(crBright) + (1.0-Weight)*GetBValue(crDark));

	//TRACE("LightAngle = %0.0f, Angle = %3.0f, Diff = %3.0f, Weight = %0.2f, RGB%3d,%3d,%3d\n", 
	//	  LIGHT_SOURCE_ANGLE*Rad2Deg, dAngle*Rad2Deg, dAngleDifference*Rad2Deg, Weight,Red,Green,Blue);

	return RGB(Red, Green, Blue);
}

void DrawCircle(CDC* pDC, CPoint p, LONG lRadius, COLORREF crColour, BOOL bDashed)
{
	const int nDashLength = 1;
	LONG lError, lXoffset, lYoffset;
	int  nDash = 0;
	BOOL bDashOn = TRUE;

	//Check to see that the coordinates are valid
	ASSERT( (p.x + lRadius <= LONG_MAX) && (p.y + lRadius <= LONG_MAX) );
	ASSERT( (p.x - lRadius >= LONG_MIN) && (p.y - lRadius >= LONG_MIN) );

	//Set starting values
	lXoffset = lRadius;
	lYoffset = 0;
	lError	 = -lRadius;

	do {
		if (bDashOn) {
			pDC->SetPixelV(p.x + lXoffset, p.y + lYoffset, crColour);
			pDC->SetPixelV(p.x + lXoffset, p.y - lYoffset, crColour);
			pDC->SetPixelV(p.x + lYoffset, p.y + lXoffset, crColour);
			pDC->SetPixelV(p.x + lYoffset, p.y - lXoffset, crColour);
			pDC->SetPixelV(p.x - lYoffset, p.y + lXoffset, crColour);
			pDC->SetPixelV(p.x - lYoffset, p.y - lXoffset, crColour);
			pDC->SetPixelV(p.x - lXoffset, p.y + lYoffset, crColour);
			pDC->SetPixelV(p.x - lXoffset, p.y - lYoffset, crColour);
		}

		//Advance the error term and the constant X axis step
		lError += lYoffset++;

		//Check to see if error term has overflowed
		if ((lError += lYoffset) >= 0)
			lError -= --lXoffset * 2;

		if (bDashed && (++nDash == nDashLength)) {
			nDash = 0;
			bDashOn = !bDashOn;
		}

	} while (lYoffset <= lXoffset);	//Continue until halfway point
} 

void DrawCircle(CDC* pDC, CPoint p, LONG lRadius, COLORREF crBright, COLORREF crDark)
{
	LONG lError, lXoffset, lYoffset;

	//Check to see that the coordinates are valid
	ASSERT( (p.x + lRadius <= LONG_MAX) && (p.y + lRadius <= LONG_MAX) );
	ASSERT( (p.x - lRadius >= LONG_MIN) && (p.y - lRadius >= LONG_MIN) );

	//Set starting values
	lXoffset = lRadius;
	lYoffset = 0;
	lError	 = -lRadius;

	do {
		const double Pi = 3.141592654, 
					 Pi_on_2 = Pi * 0.5,
					 Three_Pi_on_2 = Pi * 1.5;
		COLORREF crColour;
		double	 dAngle = atan2((double)lYoffset, (double)lXoffset);

		//Draw the current pixel, reflected across all eight arcs
		crColour = GetColour(dAngle, crBright, crDark);
		pDC->SetPixelV(p.x + lXoffset, p.y + lYoffset, crColour);

		crColour = GetColour(Pi_on_2 - dAngle, crBright, crDark);
		pDC->SetPixelV(p.x + lYoffset, p.y + lXoffset, crColour);

		crColour = GetColour(Pi_on_2 + dAngle, crBright, crDark);
		pDC->SetPixelV(p.x - lYoffset, p.y + lXoffset, crColour);

		crColour = GetColour(Pi - dAngle, crBright, crDark);
		pDC->SetPixelV(p.x - lXoffset, p.y + lYoffset, crColour);

		crColour = GetColour(-Pi + dAngle, crBright, crDark);
		pDC->SetPixelV(p.x - lXoffset, p.y - lYoffset, crColour);

		crColour = GetColour(-Pi_on_2 - dAngle, crBright, crDark);
		pDC->SetPixelV(p.x - lYoffset, p.y - lXoffset, crColour);

		crColour = GetColour(-Pi_on_2 + dAngle, crBright, crDark);
		pDC->SetPixelV(p.x + lYoffset, p.y - lXoffset, crColour);

		crColour = GetColour(-dAngle, crBright, crDark);
		pDC->SetPixelV(p.x + lXoffset, p.y - lYoffset, crColour);

		//Advance the error term and the constant X axis step
		lError += lYoffset++;

		//Check to see if error term has overflowed
		if ((lError += lYoffset) >= 0)
			lError -= --lXoffset * 2;

	} while (lYoffset <= lXoffset);	//Continue until halfway point
} 

/////////////////////////////////////////////////////////////////////////////
// VolumeSliderCtrl

IMPLEMENT_DYNAMIC(VolumeSliderCtrl, CSliderCtrl)

VolumeSliderCtrl::VolumeSliderCtrl()
{
	// No cursor as yet.
	m_hCursor       = NULL;                 

	m_strText = "%ld°";
	m_nKnobRadius = 5;
	m_bInverted = true;
	m_bDragging = false;

	// m_nZero = 180;
	m_nZero = 200;

	// TO DO 
	// Add this to the constructor, or at least
	// provide access.
	m_nDeadDegrees = 80;
}

VolumeSliderCtrl::~VolumeSliderCtrl()
{
	if ( m_hCursor )
		DestroyCursor( m_hCursor );
}

BEGIN_MESSAGE_MAP(VolumeSliderCtrl, CSliderCtrl)
	//{{AFX_MSG_MAP(VolumeSliderCtrl)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void VolumeSliderCtrl::PreSubclassWindow() 
{
	CSliderCtrl::PreSubclassWindow();

	SetRange(0, 359, FALSE);
	SetLineSize(1);
	SetPageSize(10);

	Init();

	// Try and load up a "hand" cursor.
	SetDefaultCursor();      
}

BOOL VolumeSliderCtrl::PreCreateWindow(CREATESTRUCT& cs) 
{
	if(!CSliderCtrl::PreCreateWindow(cs)) return FALSE;

	SetRange(0, 359, FALSE);
	SetLineSize(1);
	SetPageSize(10);

	Init();
	
	// "No longer supported"... whatever...
	// VerifyPos();

	return TRUE;
}


//-------------------------------------------------------------------//
// SetDefaultCursor()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void VolumeSliderCtrl::SetDefaultCursor()
{
	// m_hPointer = LoadCursor( (_Module.GetResourceInstance(), MAKEINTRESOURCE( IDC_HANDPOINTER ) );

	// The following appeared in Paul DiLascia's Jan 1998 MSJ articles.
	// It loads a "hand" cursor from the winhlp32.exe module
	// Get the windows directory
	CString strWndDir;
	GetWindowsDirectory(strWndDir.GetBuffer(MAX_PATH), MAX_PATH);
	strWndDir.ReleaseBuffer();

	strWndDir += _T("\\winhlp32.exe");
	// This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
	HMODULE hModule = LoadLibrary(strWndDir);
	if (hModule) 
	{
		HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
		if (hHandCursor)
			m_hCursor = CopyCursor(hHandCursor);
	}
	FreeLibrary(hModule);
}


//-------------------------------------------------------------------//
// OnSetCursor()																		//
//-------------------------------------------------------------------//
// Set the previously loaded cursor now.
//-------------------------------------------------------------------//
BOOL VolumeSliderCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    if ( m_hCursor )
    {
        ::SetCursor( m_hCursor );
        return TRUE;
    }
    return FALSE;
}


void VolumeSliderCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CSliderCtrl::OnSize(nType, cx, cy);
	
	Init();
}

void VolumeSliderCtrl::Init()
{
	CRect rc;
	GetClientRect(rc);

	// Resize the window to make it square
	rc.bottom = rc.right = min(rc.bottom, rc.right);

	// Get the vital statistics of the window
	m_ptCenter = rc.CenterPoint();
	m_nRadius = rc.bottom/2-(m_nKnobRadius+1);

	// Set the window region so mouse clicks only activate the round section 
	// of the slider
	m_rgn.DeleteObject(); 
	SetWindowRgn(NULL, FALSE);
	m_rgn.CreateEllipticRgnIndirect(rc);
	SetWindowRgn(m_rgn, TRUE);
}

#pragma warning(disable:4100) // Unreferenced formal parameter
BOOL VolumeSliderCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}
#pragma warning(default:4100)


//-------------------------------------------------------------------//
// OnPaint()																			//
//-------------------------------------------------------------------//
// Here we go!
//-------------------------------------------------------------------//
void VolumeSliderCtrl::OnPaint() 
{
	const int nMin = GetRangeMin();
	const int nMax = GetRangeMax()+1;

	CPaintDC dc(this); // device context for painting

	#ifdef USE_MEM_DC
		CMemDC pDC(&dc);
	#else
		CDC* pDC = &dc;
	#endif

	int nRadius = m_nRadius;

	// Draw (clear) the background
	CRect rc;
	GetClientRect(rc);
	pDC->SelectStockObject(NULL_BRUSH);
	pDC->FillSolidRect(rc, ::GetSysColor(COLOR_BTNFACE));

	// TO DO
	// Update this to limit the range of the knob, eg, from 200 to 160,
	// instead of allowing the full circle.
	// DONE, NOOOOOO    now we need to update PreventZero...()...
	
	int nPos = (((GetPos()-nMin)*360/(nMax-nMin)) + m_nZero + 360) % 360;
	// int nPos = (((GetPos()-nMin)* ( 360 - m_nDeadDegrees ) /(nMax-nMin)) + m_nZero + 360) % 360;

	if(m_bInverted) nPos = 360-nPos;

	const double dPos = ((double)(nPos))*pi/180.0;

	bool bSmallDraw = ( nRadius < cnMaxSmallDrawingRadius );
	#pragma warning(disable:4244) // Disable warning "Converting 'double' to 'int', possible loss of data"

		int nKnobRadius = m_nKnobRadius*0.8;
		if ( !bSmallDraw ) nKnobRadius++;
		CPoint	ptKnobCenter;
		ptKnobCenter = CPoint(	
			// m_ptCenter.x + ( nRadius - m_nKnobRadius ) * sin( dPos ), 
			// m_ptCenter.y - ( nRadius - m_nKnobRadius ) * cos( dPos )
			m_ptCenter.x + ( nRadius - m_nKnobRadius - ( bSmallDraw ? 0 : 1 ) + 0.5 ) * sin( dPos ), 
			m_ptCenter.y - ( nRadius - m_nKnobRadius - ( bSmallDraw ? 0 : 1 ) + 0.5 ) * cos( dPos ) + 1
		);

	#pragma warning(default:4244)

	/*
	// MDM	3/13/00 12:24:18 AM
	// Fill with black.
	CRect rcVolume( m_ptCenter.x - nRadius, m_ptCenter.y - nRadius, 
						m_ptCenter.x + nRadius, m_ptCenter.y + nRadius);
	rcVolume.InflateRect( 2, 2, 3, 3 );
	CRgn rgnVolume;
	rgnVolume.CreateEllipticRgnIndirect(rcVolume);
	CBrush brVolume(::GetSysColor(COLOR_BTNFACE));	// Std
	// CBrush brVolume( RGB( 32, 32, 32 ) );				// Black
	// CBrush brVolume( RGB( 160, 160, 160 ) );			// Gray
	// CBrush brVolume( RGB( 32, 32, 128 ) );				// Blue
	// CBrush brVolume( RGB( 255, 224, 0 ) );				// Yellow
	pDC->FillRgn(&rgnVolume, &brVolume);
	rgnVolume.DeleteObject();
	*/

	// Draw the edge.
	// nRadius+=2;
	// DrawCircle(pDC, m_ptCenter, nRadius--, ::GetSysColor(COLOR_3DHIGHLIGHT),::GetSysColor(COLOR_3DDKSHADOW));
	// DrawCircle(pDC, m_ptCenter, nRadius--, ::GetSysColor(COLOR_3DLIGHT), ::GetSysColor(COLOR_3DSHADOW));

	// Do we want the outer "smoothing" shading?
	/*
	DrawCircle(pDC, m_ptCenter						, nRadius + 4, ::GetSysColor(COLOR_3DLIGHT), ::GetSysColor(COLOR_3DSHADOW));
	DrawCircle(pDC, m_ptCenter + CSize( 0, 1 ), nRadius + 4, ::GetSysColor(COLOR_3DLIGHT), ::GetSysColor(COLOR_3DSHADOW));
	DrawCircle(pDC, m_ptCenter - CSize( 1, 0 ), nRadius + 4, ::GetSysColor(COLOR_3DLIGHT), ::GetSysColor(COLOR_3DSHADOW));
	DrawCircle(pDC, m_ptCenter						, nRadius + 3, ::GetSysColor(COLOR_3DLIGHT), ::GetSysColor(COLOR_3DSHADOW));
	DrawCircle(pDC, m_ptCenter + CSize( 0, 1 ), nRadius + 3, ::GetSysColor(COLOR_3DLIGHT), ::GetSysColor(COLOR_3DSHADOW));
	DrawCircle(pDC, m_ptCenter - CSize( 1, 0 ), nRadius + 3, ::GetSysColor(COLOR_3DLIGHT), ::GetSysColor(COLOR_3DSHADOW));
	*/
	
	COLORREF cr3DLight = ::GetSysColor(COLOR_3DHIGHLIGHT);
	COLORREF cr3DDark  = ::GetSysColor(COLOR_3DDKSHADOW );

	// MDM	5/21/00 6:05:15 PM
	// Let's try filling the circle!
	//
	//	RGB	x
	//			x
	//			x
	//			 x
	//			 x
	//			  x
	//			   x
	//			    x
	//			     xx
	//			       xxxxxxxxxx
	//										Radius
	//
	// As the radius gets smaller, we move the RGB values to black (0).
	// Use
	//
	//		y = Ax^B
	//
	// where
	//	
	//		A = coef as defined by starting Radius and RGB
	//		B = power that creates pleasant gradiant effect

	nRadius += 2;
	int nA;
	const double cdGradientLog = 2.8;
	const BYTE bBGoal = 75;
	double dGradientRLCoef =   GetRValue( cr3DLight )					/ pow( nRadius, cdGradientLog );
	double dGradientGLCoef =   GetGValue( cr3DLight )					/ pow( nRadius, cdGradientLog );
	double dGradientBLCoef = ( GetBValue( cr3DLight ) - bBGoal )	/ pow( nRadius, cdGradientLog );
	double dGradientRDCoef =   GetRValue( cr3DDark  )					/ pow( nRadius, cdGradientLog );
	double dGradientGDCoef =   GetGValue( cr3DDark  )					/ pow( nRadius, cdGradientLog );
	double dGradientBDCoef = ( GetBValue( cr3DDark  ) - bBGoal )	/ pow( nRadius, cdGradientLog );
	for ( nA = nRadius; nA >= 0; nA-- )
	{
		double dTemp = pow( nA, cdGradientLog );
		BYTE bRL = (BYTE)( dGradientRLCoef * dTemp				);
		BYTE bGL = (BYTE)( dGradientGLCoef * dTemp				);
		BYTE bBL = (BYTE)( dGradientBLCoef * dTemp + bBGoal	);
		BYTE bRD = (BYTE)( dGradientRDCoef * dTemp				);
		BYTE bGD = (BYTE)( dGradientGDCoef * dTemp				);
		BYTE bBD = (BYTE)( dGradientBDCoef * dTemp + bBGoal	);
		COLORREF crLight = RGB( bRL, bGL, bBL );
		COLORREF crDark  = RGB( bRD, bGD, bBD );
		DrawCircle(	pDC, m_ptCenter						, nA				, crLight, crDark );
		DrawCircle(	pDC, m_ptCenter + CSize( 0, 1 )	, nA				, crLight, crDark );
		DrawCircle(	pDC, m_ptCenter - CSize( 1, 0 )	, nA				, crLight, crDark );
	}	
	
	/*
	// Thicken up the edge on larger controls.
	if ( nRadius > cnMaxSmallDrawingRadius )
	{
		DrawCircle(	pDC, m_ptCenter						, nRadius		, cr3DLight, cr3DDark );
		DrawCircle(	pDC, m_ptCenter + CSize( 0, 1 )	, nRadius		, cr3DLight, cr3DDark );
		DrawCircle(	pDC, m_ptCenter - CSize( 1, 0 )	, nRadius		, cr3DLight, cr3DDark );
	}
	DrawCircle(		pDC, m_ptCenter						, nRadius + 1	, cr3DLight, cr3DDark );
	DrawCircle(		pDC, m_ptCenter + CSize( 0, 1 )	, nRadius + 1	, cr3DLight, cr3DDark );
	DrawCircle(		pDC, m_ptCenter - CSize( 1, 0 )	, nRadius + 1	, cr3DLight, cr3DDark );
	DrawCircle(		pDC, m_ptCenter						, nRadius + 2	, cr3DLight, cr3DDark );
	DrawCircle(		pDC, m_ptCenter + CSize( 0, 1 )	, nRadius + 2	, cr3DLight, cr3DDark );
	DrawCircle(		pDC, m_ptCenter - CSize( 1, 0 )	, nRadius + 2	, cr3DLight, cr3DDark );
	*/
	
	// Draw the knob
	CRect rcKnob(	ptKnobCenter.x - nKnobRadius, ptKnobCenter.y - nKnobRadius, 
						ptKnobCenter.x + nKnobRadius, ptKnobCenter.y + nKnobRadius);
	rcKnob.OffsetRect( 1, 1 );
	rcKnob.DeflateRect( 0, 0, 0, 1 );

	CRgn rgnKnob;
	rgnKnob.CreateEllipticRgnIndirect(rcKnob);
	// CBrush brKnob(::GetSysColor(COLOR_BTNFACE));
	CBrush brKnob(RGB(255, 0, 0 ));
	pDC->FillRgn(&rgnKnob, &brKnob);
	rgnKnob.DeleteObject();

	DrawCircle(pDC, ptKnobCenter, --nKnobRadius, ::GetSysColor(COLOR_3DDKSHADOW),::GetSysColor(COLOR_3DHIGHLIGHT));
	DrawCircle(pDC, ptKnobCenter, --nKnobRadius, ::GetSysColor(COLOR_3DSHADOW),::GetSysColor(COLOR_3DLIGHT));

	// MDM	3/9/2000 7:16:34 PM
	/*
	// Draw the text
	const CString strFormattedText = OnFormatText();

	if(!strFormattedText.IsEmpty())
	{
		CWnd* pOwner = GetParentOwner();
		if(pOwner)
		{
			CFont* pFont = pOwner->GetFont();
			pDC->SelectObject(pFont);
		}

		const CSize szExtent = pDC->GetTextExtent(strFormattedText);
		const CPoint ptText = CPoint(m_ptCenter.x - szExtent.cx/2, m_ptCenter.x - szExtent.cy/2);

		pDC->SetBkMode(TRANSPARENT);
		if(!IsWindowEnabled())
		{
			pDC->DrawState(ptText, szExtent, strFormattedText, DSS_DISABLED, TRUE, 0, (HBRUSH)NULL);
		}
		else
		{
			pDC->TextOut(ptText.x, ptText.y, strFormattedText);
		}
	}
	*/

	// Don't call CSliderCtrl::OnPaint()
}

void VolumeSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(!m_bDragging)
	{
		m_bDragging = true;
		m_bDragChanged = false;
		SetCapture();
		SetFocus();
		if( SetKnob(point) )
		{
			m_bDragChanged = true;
			PostMessageToParent(TB_THUMBTRACK);
			m_ptLast = point;
		}
		RedrawWindow();
	}
	else
	{
		CSliderCtrl::OnLButtonDown(nFlags, point);
	}
}

void VolumeSliderCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(m_bDragging)
	{
		PreventCrossingZero( point );
		if ( SetKnob(point) )
		{
			m_bDragChanged = true;
			PostMessageToParent(TB_THUMBTRACK);
			RedrawWindow();
			m_ptLast = point;
		}
	}
	else
	{
		CSliderCtrl::OnMouseMove(nFlags, point);
	}
}

void VolumeSliderCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(m_bDragging)
	{
		m_bDragging = false;
		::ReleaseCapture();
		PreventCrossingZero( point );
		if ( SetKnob(point) )
		{
			PostMessageToParent(TB_THUMBTRACK);
			m_bDragChanged = true;
		}
		if(m_bDragChanged)
		{
			PostMessageToParent(TB_THUMBPOSITION);
			m_bDragChanged = false;
		}
		RedrawWindow();

	}
	else
	{
		CSliderCtrl::OnLButtonUp(nFlags, point);
	}
}


//-------------------------------------------------------------------//
// PreventCrossingZero()															//
//-------------------------------------------------------------------//
// This tells us if we are crossing from the first to fourth
// quadrant or vice versa, so we can prevent it.
//-------------------------------------------------------------------//
void VolumeSliderCtrl::PreventCrossingZero( CPoint& point )
{
	// First, check if last point was lower-left and this is lower-right.
	if (
			m_ptLast.x <= m_ptCenter.x && m_ptLast.y > m_ptCenter.y
		&&	   point.x >= m_ptCenter.x &&    point.y > m_ptCenter.y
	) {
		// Set to extreme lower left.
		point.x = m_ptCenter.x - 1;
		point.y += 200;
	}

	// Next, check if last point was lower-right and this is lower-left.
	if (
			m_ptLast.x >= m_ptCenter.x && m_ptLast.y > m_ptCenter.y
		&&	   point.x <= m_ptCenter.x &&    point.y > m_ptCenter.y
	) {
		// Set to extreme lower right.
		point.x = m_ptCenter.x + 1;
		point.y += 200;
	}

}


//-------------------------------------------------------------------//
// SetKnob()																			//
//-------------------------------------------------------------------//
// Set that little red knobby job.
//-------------------------------------------------------------------//
bool VolumeSliderCtrl::SetKnob(const CPoint& pt)
{
	const int nMin = GetRangeMin();
	const int nMax = GetRangeMax()+1;

	CSize szDelta = pt - m_ptCenter;
	if(m_bInverted) szDelta.cx *= -1;

	double dNewPos = 0.0;

	if(szDelta.cx != 0)
	{
		dNewPos = 90.0 - atan(-(double)szDelta.cy / (double)szDelta.cx) * 180.0 / pi;
	}

	if(((szDelta.cx == 0) && (szDelta.cy >= 0)) || (szDelta.cx < 0))
	{
		dNewPos += 180.0;
	}

	dNewPos -= m_nZero;

	while(dNewPos < 0.0) dNewPos += 360.0;
	while(dNewPos >= 360.0) dNewPos -= 360.0;

	const int nNewPos = nMin + (int)(dNewPos*(nMax-nMin)/360.0);
	const bool bChanged = (nNewPos != GetPos());

	if(bChanged)
	{
		SetPos(nNewPos);
	}

	return bChanged;
}

void VolumeSliderCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	const int nMin = GetRangeMin();
	const int nMax = GetRangeMax()+1;

	switch(nChar)
	{
	case VK_LEFT:
	case VK_UP:
		{
			int nNewPos = GetPos()-GetLineSize();
			while(nNewPos < nMin) nNewPos += (nMax - nMin);
			SetPos(nNewPos);
			RedrawWindow();
			PostMessageToParent(TB_LINEUP);
		}
		break;
	
	case VK_RIGHT:
	case VK_DOWN:
		{
			int nNewPos = GetPos()+GetLineSize();
			while(nNewPos >= nMax) nNewPos -= (nMax - nMin);
			SetPos(nNewPos);
			RedrawWindow();
			PostMessageToParent(TB_LINEDOWN);
		}
		break;

	case VK_PRIOR:
		{
			int nNewPos = GetPos()-GetPageSize();
			while(nNewPos < nMin) nNewPos += (nMax - nMin);
			SetPos(nNewPos);
			RedrawWindow();
			PostMessageToParent(TB_PAGEUP);
		}
		break;

	case VK_NEXT:
		{
			int nNewPos = GetPos()+GetPageSize();
			while(nNewPos >= nMax) nNewPos -= (nMax - nMin);
			SetPos(nNewPos);
			RedrawWindow();
			PostMessageToParent(TB_PAGEDOWN);
		}
		break;

	case VK_HOME:
	case VK_END:
		// Do nothing (ignore keystroke)
		break;

	default:
		CSliderCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void VolumeSliderCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch(nChar)
	{
	case VK_LEFT:
	case VK_UP:
	case VK_RIGHT:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
		PostMessageToParent(TB_ENDTRACK);
		break;

	case VK_HOME:
	case VK_END:
		// Do nothing
		break;

	default:
		CSliderCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
	}
}

void VolumeSliderCtrl::PostMessageToParent(const int nTBCode) const
{
	CWnd* pWnd = GetParent();
	if(pWnd) pWnd->PostMessage(WM_VSCROLL, (WPARAM)((GetPos() << 16) | nTBCode),
(LPARAM)GetSafeHwnd());
}

void VolumeSliderCtrl::SetText(const CString& strNewText)
{
	m_strText = strNewText;
}

CString VolumeSliderCtrl::GetText() const
{
	return m_strText;
}

void VolumeSliderCtrl::SetKnobRadius(const int nNewKnobRadius)
{
	ASSERT(nNewKnobRadius > 4);

	m_nKnobRadius = nNewKnobRadius;
}

int VolumeSliderCtrl::GetKnobRadius() const
{
	return m_nKnobRadius;
}

void VolumeSliderCtrl::SetZero(const int nZero)
{
	ASSERT(nZero >= 0 && nZero < 360);

	m_nZero = nZero;
}

int VolumeSliderCtrl::GetZero() const
{
	return m_nZero;
}

void VolumeSliderCtrl::SetInverted(const bool bNewInverted)
{
	m_bInverted = bNewInverted;
}

bool VolumeSliderCtrl::GetInverted()
{
	return m_bInverted;
}

// Overrideables
CString VolumeSliderCtrl::OnFormatText()
{
	CString strFormattedText;
	strFormattedText.Format(GetText(), GetPos());
	return strFormattedText;
}

#ifdef _DEBUG
void VolumeSliderCtrl::AssertValid() const
{
	CSliderCtrl::AssertValid();

	ASSERT(m_nZero >= 0 && m_nZero < 360);
	ASSERT(m_nKnobRadius > 4);
}

void VolumeSliderCtrl::Dump(CDumpContext& dc) const
{
	CSliderCtrl::Dump(dc);

	dc	<< "m_strText = '" << m_strText << "'\n"
		<< "m_nKnobRadius = " << m_nKnobRadius << "\n"
		<< "m_nZero = " << m_nZero;
}
#endif // _DEBUG



