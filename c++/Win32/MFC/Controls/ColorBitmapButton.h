// ColorBitmapButton interface

#ifndef COLOR_BITMAP_BUTTON_H
	#define COLOR_BITMAP_BUTTON_H

#include "ColorBitmap.h"						// Contains one.


//-------------------------------------------------------------------//
// class ColorBitmapButton															//
//-------------------------------------------------------------------//
class ColorBitmapButton : public CButton
{

public:

	// Our constructor.
	ColorBitmapButton(
		UINT			uNewCommandID,
		UINT			uNewTooltipID,
		UINT			uNewLargeBitmapResourceID,
		UINT			uNewSmallBitmapResourceID,
		UINT			uNewSizeToResourceID		= 0,				// 0 = SizeToBitmap
		bool			bNewMapColors				= true,
		bool			bNewSunkenPressed			= true,
		bool			bNewGrayedOut				= false,
		bool			bNewFocusVersion			= false,
		bool			bNewExtractPalette		= false
	);

	void Create( 
		CWnd* pParent,
		bool	bCheckBoxStyle			= false,
		bool	bInitialCheckState	= false
	);

	// destructor
	ColorBitmapButton::~ColorBitmapButton();

	CWindowDC* ColorBitmapButton::NewWindowDC()
		{ return new CWindowDC( this ); }

	CSize GetSize( bool bIncludeBorder = false );

	bool bIsPushed()
	{
		ASSERT( m_bCheckBoxStyle );
		return m_bCheckState;
	}

	void SetPushed( bool bPushed = true )
	{
		// Set state and redraw.
		m_bCheckState = bPushed;
		Invalidate();
	}

	// These are all specified in constructor, and saved for use
	// in the Create() call.
	UINT uCommandID;
	UINT uTooltipID;
	UINT uLargeBitmapResourceID;
	UINT uSmallBitmapResourceID;
protected:
	bool bMapColors;
	bool bExtractPalette;
	ColorBitmap* pBitmap;

	// This variable tells us to either size the button to the 
	// bitmap, if zero, or to size the bitmap to the control with
	// the given resource ID.
	// See DrawItem() for details.
	UINT uSizeToResourceID;

	// Determines whether to draw the sunken look when pressed, or
	// just the flattened look.
	bool bSunkenPressed;

	// Determines whether the bitmap has a grayed (disabled) version.
	bool bGrayedOut;

	// Determines whether bitmap has focused version in it.
	bool bFocusVersion;

	// This means the button can be left in a "pressed" state.
	bool m_bCheckBoxStyle;
	bool m_bCheckState;
	bool m_bFirstTimeCheck;

	// Override of base class.
	void DrawItem(LPDRAWITEMSTRUCT lpDIS);

public:

	// Generated message map functions
	//{{AFX_MSG(ColorBitmapButton)
	//}}AFX_MSG

	// This uses "_EX", so we don't use ClassWizard.
	afx_msg BOOL OnClicked();

	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

#endif	// COLOR_BITMAP_BUTTON_H
