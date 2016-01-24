//-------------------------------------------------------------------//
// FontHelpers.cpp : MFC font helper functions
//
//	Copyright © 2002 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "FontHelpers.h"


//-------------------------------------------------------------------//
// GetLOGFONT()																		//
//-------------------------------------------------------------------//
// Here we get a logical font from a simple one.
//-------------------------------------------------------------------//
void GetLOGFONT(
	const SimpleFont& sfSource,
	LOGFONT&				lfDest,
	CDC*					pDC
) {
	// Set all the stuff we don't use.
	lfDest.lfWidth				= 0;
	lfDest.lfEscapement		= 0;
	lfDest.lfOrientation		= 0;
	lfDest.lfOutPrecision	= OUT_DEFAULT_PRECIS;
	lfDest.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
	lfDest.lfQuality			= PROOF_QUALITY;
	lfDest.lfCharSet			= ANSI_CHARSET;
	lfDest.lfPitchAndFamily	= VARIABLE_PITCH | FF_DONTCARE;

	// Now set the stuff we use.
	_tcscpy( lfDest.lfFaceName, LPCTSTR( sfSource.m_strFace ) );
	lfDest.lfWeight			= sfSource.m_nWeight		;
	lfDest.lfItalic			= sfSource.m_bItalic		;
	lfDest.lfUnderline		= sfSource.m_bUnderline	;
	lfDest.lfStrikeOut		= sfSource.m_bStrikeout	;

	// To get the logical font size, we need a DC.
	if ( pDC )
		lfDest.lfHeight = -MulDiv( sfSource.m_nSize / 10, pDC->GetDeviceCaps( LOGPIXELSY ), 72 );
	else
		lfDest.lfHeight = 0;
}


//-------------------------------------------------------------------//
// GetFontFromUser()																	//
//-------------------------------------------------------------------//
bool GetFontFromUser(
	SimpleFont& sf,
	CDC*			pDC
) {
	
	LOGFONT lf;
	GetLOGFONT(
		sf,
		lf,
		pDC
	);

	CFontDialog fd( &lf );
	
	// Some properties aren't available in the LOGFONT.
	// We need to set them here, just before displaying dlg.
	fd.m_cf.rgbColors		= sf.m_crColor	;
	fd.m_cf.iPointSize	= sf.m_nSize	;
	
	// Throw up the common dialog.
	// Don't get results if the user cancels.
	if ( fd.DoModal() == IDOK ) 
	{
		// Copy out results.
		sf.m_strFace		= fd.GetFaceName();
		sf.m_nSize			= fd.GetSize();
		sf.m_nWeight		= fd.GetWeight();
		sf.m_crColor		= fd.GetColor();
		sf.m_bItalic		= ( fd.IsItalic()		!= 0 );
		sf.m_bUnderline	= ( fd.IsUnderline()	!= 0 );
		sf.m_bStrikeout	= ( fd.IsStrikeOut()	!= 0 );

		return true;

	}

	return false;
}


//-------------------------------------------------------------------//
// GetCFont()																			//
//-------------------------------------------------------------------//
// Here we get a CFont from a simple one.
// This can then be used with CWnd's, etc.
//-------------------------------------------------------------------//
void GetCFont(
	const SimpleFont& sfSource,
	CFont&				cfDest,
	CDC*					pDC
) {
	LOGFONT lf;
	GetLOGFONT(
		sfSource,
		lf,
		pDC
	);

	// CFonts are messy little beasts, clean up if needed...
	if ( cfDest.GetSafeHandle() )
		cfDest.DeleteObject();

	cfDest.CreateFontIndirect( &lf );
}
