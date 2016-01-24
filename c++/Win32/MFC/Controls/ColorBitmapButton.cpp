//-------------------------------------------------------------------//
// ColorBitmapButton.cpp															//
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "ColorBitmapButton.h"


BEGIN_MESSAGE_MAP(ColorBitmapButton, CButton)
	//{{AFX_MSG_MAP(ColorBitmapButton)
	//}}AFX_MSG_MAP

	// Do the "_EX" thing so the parent gets the message, too!
	ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)

END_MESSAGE_MAP()

//-------------------------------------------------------------------//
// construction																		//
//-------------------------------------------------------------------//
ColorBitmapButton::ColorBitmapButton (
	UINT			uNewCommandID,
	UINT			uNewTooltipID,
	UINT			uNewLargeBitmapResourceID,
	UINT			uNewSmallBitmapResourceID,
	UINT			uNewSizeToResourceID,
	bool			bNewMapColors,
	bool			bNewSunkenPressed,
	bool			bNewGrayedOut,
	bool			bNewFocusVersion,
	bool			bNewExtractPalette
) :

	// Init vars.
	uCommandID           			( uNewCommandID						),            
	uTooltipID            			( uNewTooltipID						),            
	uLargeBitmapResourceID			( uNewLargeBitmapResourceID		),
	uSmallBitmapResourceID			( uNewSmallBitmapResourceID		),
	uSizeToResourceID	   			( uNewSizeToResourceID				),   
	bMapColors            			( bNewMapColors						),            
	bSunkenPressed        			( bNewSunkenPressed					),        
	bGrayedOut            			( bNewGrayedOut						),            
	bFocusVersion         			( bNewFocusVersion					),         
	bExtractPalette        			( bNewExtractPalette					),
	pBitmap								( 0										),
	m_bCheckBoxStyle					( false									),
	m_bFirstTimeCheck					( true									),
	m_bCheckState						( false									)
{
}


//-------------------------------------------------------------------//
// Create()																				//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ColorBitmapButton::Create( 
	CWnd* pParent,
	bool	bCheckBoxStyle,
	bool	bInitialCheckState
) {

	// Keep the style type around.
	// We use it in our trap of OnClicked().
	// It would be nice if we could just use BS_AUTOCHECKBOX | BS_PUSHLIKE
	// in place of BS_PUSHBUTTON in our call to CButton::Create(), but
	// Windows does not respect that.  Why did I expect it to be simple?
	if ( bCheckBoxStyle )
	{
		m_bCheckBoxStyle = true;
		bSunkenPressed = false;
		m_bCheckState = bInitialCheckState;
		m_bFirstTimeCheck = true;
	}
	
	// We have not tested using a focus version within the bitmap,
	// I know that DrawItem (at least) needs math updating.
	// GetSize() should already be done.
	ASSERT( !bFocusVersion );
		
	// Call the base class Create().
	CButton::Create(
		LPCTSTR( CString( (LPSTR) uTooltipID ) ),
		BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
		CRect(),
		pParent,
		uCommandID		
	);

	// Construct the bitmap.
	pBitmap = new ColorBitmap(
		uLargeBitmapResourceID,
		bMapColors,				
		false,					// bLeaveWhite - used for image list transparency
		bExtractPalette
	);

	// Make sure we succeeded in loading the bitmap.
	ASSERT( pBitmap->m_hObject != NULL);

	// Now we resize it.  If uSizeToResourceID was specified, we size to the
	// control referenced by the ID.  Otherwise, we size the button to the bitmap.
	if ( uSizeToResourceID ) {
	
		// Get the rect of the specified ResourceID.
		// We get the rect in screen coordinates (client would give 0, 0 origin).
		// Then we convert to client of the parent.
		CRect rect;
		GetParent()->GetDlgItem( uSizeToResourceID )->GetWindowRect( &rect );
		GetParent()->ScreenToClient( &rect );
	
		// Resize the button into the SizeTo control's rect.
		VERIFY(
			SetWindowPos(
				NULL, 
				rect.left, 
				rect.top, 
				rect.Width(), 
				rect.Height(),
				SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE			// SWP_NOMOVE | 
			)
		);

	} else {
		
		// Resize button to the bitmap.  
		// Note that we don't change the origin.
		// This is a nearly direct copy of CBitmapButton::SizeToContent().
		CSize bitmapSize;
		BITMAP bmInfo;
		VERIFY( pBitmap->GetObject(sizeof(bmInfo), &bmInfo) == sizeof(bmInfo));
		VERIFY(SetWindowPos(NULL, -1, -1, bmInfo.bmWidth, bmInfo.bmHeight,
			SWP_NOMOVE|SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE));

	}	

}


//-------------------------------------------------------------------//
// ~ColorBitmapButton()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ColorBitmapButton::~ColorBitmapButton()
{
	delete pBitmap;
}


//-------------------------------------------------------------------//
// OnClicked()																			//
//-------------------------------------------------------------------//
// Windows buttons force BS_PUSHBUTTON-style handling for all 
// BN_OWNERDRAW buttons.  We want to allow the BS_CHECKBOX style,
// if requested in Create().
// Here's where we can toggle our internally-maintained state.
//-------------------------------------------------------------------//
BOOL ColorBitmapButton::OnClicked() 
{
	
	if ( m_bCheckBoxStyle )
	{
		// Toggle state.
		m_bCheckState = !m_bCheckState;

		// We need to redraw NOW!
		Invalidate();
	}
	
	if ( m_bFirstTimeCheck )
	{
		m_bFirstTimeCheck = false;
	}

	// Let the parent get the click message too.
	return FALSE;

}


//-------------------------------------------------------------------//
// DrawItem()																			//
//-------------------------------------------------------------------//
// This function was copied from CBitmapButton's source (in 			//
// WinBtn.cpp). It was modified according to knowledge base article:	//
// "How to Use CBitmapButton to Resize a Bitmap to Fit a Button".		//
//-------------------------------------------------------------------//
void ColorBitmapButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	ASSERT(lpDIS != NULL);
	
	// Have to have loaded the bitmap.
	ASSERT( pBitmap->m_hObject != NULL );

	UINT state = lpDIS->itemState;

	// Update the style manually for checkbox-style buttons.
	if ( m_bCheckBoxStyle )
		if ( m_bCheckState )
			state |= ODS_SELECTED;
		else 
			state &= ~ODS_SELECTED;

	CRect rect;
	rect.CopyRect(&lpDIS->rcItem);

	// If we are Sizing to bitmap, then
	// the rectangle needs to be juuuuuust a little bit
	// bigger than the bitmap to leave room for the border.
	if ( uSizeToResourceID == 0 ) {

		// determine bitmaps size
		BITMAP bits;
		pBitmap->GetObject(sizeof(BITMAP),&bits);

		if ( bGrayedOut )
			rect.right = rect.left + (bits.bmWidth>>1) + 4;
		else
			rect.right = rect.left + bits.bmWidth + 4;
		rect.bottom = rect.top + bits.bmHeight + 4;
	
	}

	pBitmap->DrawBitmapToButton( 
		CDC::FromHandle(lpDIS->hDC),
		rect, 
		state, 
		bGrayedOut, 
		bSunkenPressed
	);

}


//-------------------------------------------------------------------//
// GetSize()																			//
//-------------------------------------------------------------------//
CSize ColorBitmapButton::GetSize( bool bIncludeBorder )
{
	BITMAP bmInfo;

	VERIFY(
		pBitmap->GetObject(sizeof(bmInfo), &bmInfo) == sizeof(bmInfo)
	);

	if ( uSizeToResourceID == 0 ) {
		int nImages = 1;
		if ( bGrayedOut ) nImages++;
		if ( bFocusVersion ) nImages++;
		bmInfo.bmWidth /= nImages;
		bmInfo.bmWidth += 4;
		bmInfo.bmHeight += 4;
	}

	if ( bIncludeBorder )
	{
		bmInfo.bmWidth += 2;
		bmInfo.bmHeight += 3;
	}

	return CSize( bmInfo.bmWidth, bmInfo.bmHeight );

}

