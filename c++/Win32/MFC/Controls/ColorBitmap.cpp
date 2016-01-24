//-------------------------------------------------------------------//
// ColorBitmap.cpp
//
// This class provides color bitmaps and associated functionality.
// Support for high-color, palette management, self-drawing, and
// drawing to buttons with automatically generated 
// up/down/disabled/focus version is all included.  What a deal!
//
// Palette management details
// ==========================
// The palette mechanism is based on the idea that the application
// uses one palette optimized for all resources.  This palette is
// referenced by the pPalette static member variable.  Typically,
// pPalette is initialized at the start of the app to point to 
// an application-wide palette object.  
//
// When the app receives notice that it can take control of the 
// palette ( see MainFrame::OnQueryNewPalette() ), the app palette is 
// re-initialized and blown out to the system palette through a
// FOREGROUND realization.  Then, all items that use the palette are 
// redrawn.
// When the app receives notice that another app has taken control 
// of the palette ( see MainFrame::OnPaletteChanged() ), the app 
// palette is re-initialized, and all palette items are redrawn.
// The heart of the palette mechanism lies within the redrawing
// of palette items.  ColorBitmap's are the prototypical palette
// items.
// When the ColorBitmaps are loaded, it is assured that the system
// palette has already been updated with the app palette/pPalette.  
// Therefore, all we need to do is get a temporary DC, select the 
// palette into it, and load.  No palette realization is required.
// When ColorBitmaps are drawn, the palette is selected into the DC
// and realized in the BACKGROUND.  If the app has the palette focus,
// this realization maps perfectly to the system palette, which has
// been updated with the app palette.  If the app is in the 
// background and must defer the palette control, the realization
// maps the bitmap palette to the system palette, which was set by
// another application.
//
// As mentioned above, the palette is typically initialized within
// the main frame's palette handling functions.  ColorBitmap
// provides the ability to extract the palette during creation.  This
// is how the main frame initializes the palette.

// This code originated (long ago :> ) from Q124947, 
// "Retrieving Palette Information from a Bitmap Resource".
// The knowledge base article also details how to extract a palette 
// from a bitmap, and how to handle the palette.  Another more
// in-depth resource is "The Palette Manager: How and Why".

// Sample code:
/*

BOOL TheApp::InitInstance()
{

	// TheApp uses one palette, AppPalette, that
	// is optimized across every item that uses a palette.
	// Items like ColorBitmaps that use palettes have static
	// member variables that point to the app's palette.  We
	// point them to it here.
	// The app palette is initialized in MainFrame::OnQueryNewPalette().
	ColorBitmap::pPalette = &theApp.AppPalette;

	etc.

}

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	////////////////////////////////////////////////////////////////////////
	// Call the ColorBitmap constructor to extract the
	// App palette - it will be needed for DIBs
	// that default to 8 bits of color regardless of the
	// display settings.
	{
		ColorBitmap TempBmp(
			IDB_WIZARD,				// uBitmapID,
			true,						//	bMapColors
			false,					// bLeaveWhite
			true						// bExtractPalette
		);
	}
	////////////////////////////////////////////////////////////////////////

	etc.

}

//-------------------------------------------------------------------//
// OnQueryNewPalette()																//
//-------------------------------------------------------------------//
// This function is called when re-entering the app after another
// app has changed the system palette.  We need to perform two
// steps:
//
// First, we want to reset the system palette (and the video card
// palette) to use our optimized palette.  We do this by selecting
// and realizing our palette in the FOREGROUND.  Now that we have the
// focus, other apps have to defer to using our palette.
//
// Then, we need to update all of our color items.  We can do this
// by simply repainting them, if we make sure that the drawing
// routines of all our color items select and realize their palettes
// in the BACKGROUND.  The background mapping will map perfectly to
// the system palette once we have done the FOREGROUND realization
// here (if they are using the optimized palette, of course).  And
// it will remap the palete as needed in other cases.
//
// See the class description in ColorBitmap.cpp for more details.
//-------------------------------------------------------------------//
BOOL MainFrame::OnQueryNewPalette()
{

	// We are getting the focus back after having lost it to another
	// app that changed the system palette on the video card.
	// We want to re-blow our palette out to the system palette on
	// the video card.
	int nColorsChanged = 0;
	if ( GetSafeHwnd() ) {

		// Refresh the palette.
		// We use a temp bitmap to do so.
		// Note that images have access to the app palette through
		// static pointers.
		{
			ColorBitmap TempBmp(
				IDB_WIZARD,				// uBitmapID,
				true,						//	bMapColors
				false,					// bLeaveWhite
				true						// bExtractPalette
			);
		}

		CClientDC dc( this );

		// FOREGROUND selection, so we rewrite the system palette.
		CPalette* pOldPal = dc.SelectPalette( &theApp.AppPalette, FALSE );
		nColorsChanged = dc.RealizePalette();
		if ( pOldPal )
			dc.SelectPalette( pOldPal, TRUE );

	}

	// Here, we pass notification on to all children that
	// need it.  We only do so if needed.
	if ( nColorsChanged != 0 )
		RefreshAfterPaletteChanges();

	return FALSE;

}

//-------------------------------------------------------------------//
// OnPaletteChanged()																//
//-------------------------------------------------------------------//
// This function is called to tell us when another app has changed
// the palette.
// See OnQueryNewPalette() and the class description in
// ColorBitmap.cpp for details.
//-------------------------------------------------------------------//
void MainFrame::OnPaletteChanged(CWnd* pFocusWnd)
{

	if ( pFocusWnd != this ) {

		// Another window in another app changed the system palette.
		// We know it was another app because we only change the
		// system palette within this window, in OnQueryNewPalette().
		// We need to redraw all color items in our app.  The redraw
		// code should always select and realize the appropriate
		// palette in the BACKGROUND.  See OnQueryNewPalette() for
		// more info.

		// Refresh the palette.
		// We use a temp bitmap to do the refresh.
		// Note that images have access to the app palette through
		// static pointers.
		{
			ColorBitmap TempBmp(
				IDB_MAIN_BMP,			// uBitmapID,
				true,						//	bMapColors
				false,					// bLeaveWhite
				true						// bExtractPalette
			);
		}

		RefreshAfterPaletteChanges();

	}

}


//-------------------------------------------------------------------//
// RefreshAfterPaletteChanges()													//
//-------------------------------------------------------------------//
// This function reloads bitmaps and image lists and repaints
// appropriate windows after a palette change.
//-------------------------------------------------------------------//
void MainFrame::RefreshAfterPaletteChanges()
{

	// Update the toolbar.
	if ( ColorBar.GetSafeHwnd() )
		ColorBar.Invalidate();

	// Update list controls.
	UpdateListControlImages();

	// Update tree controls.
	TreeControls->RefreshImages();

	etc.

}

*/
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "..\macros.h"					// For BYTE() macros.

#include "ColorBitmap.h"


// Initialize static members.
CPalette* ColorBitmap::pPalette = NULL;


//-------------------------------------------------------------------//
// construction																		//
//-------------------------------------------------------------------//
ColorBitmap::ColorBitmap(
	UINT			uBitmapResourceID,
	bool			bMapColors,
	bool			bLeaveWhite,
	bool			bExtractPalette
) {

	// Delete old bitmap, if present.
	DeleteObject();
	
	// We need a dc, so we can set its palette.
	// Get the entire screen DC.
	// Note: previously, we required a parent CWnd*
	// which we used to get the DC.  But this seems to
	// be working now, so we removed the CWnd* param
	// from the constructor.
	HDC hScreenDC = ::GetDC( NULL );
	ASSERT( hScreenDC );
	CDC* pDC = CDC::FromHandle( hScreenDC );
	ASSERT( pDC->GetSafeHdc() );
	
	// Load and attach the bitmap.
	Attach( 
		(HBITMAP) LoadColorBitmap(
			uBitmapResourceID,
			pDC,
			bMapColors,
			bLeaveWhite,
			bExtractPalette
		)
	);

	// Release the dc.
	// We allocated a temp CDC above.  Windows will get rid of it.
	// Don't try to clean up any more here, it will just cause 
	// problems.
	::ReleaseDC( NULL, hScreenDC );

	// Make sure we got it.
	ASSERT( m_hObject );

}


//-------------------------------------------------------------------//
// LoadColorBitmap()																	//
//-------------------------------------------------------------------//
// Here, we take the given DC and make sure the palette is selected.
// Then we use CreateDIBitmap to load the bitmap.  
// We have to "crack open" the bitmap internals so we can get at
// the number of colors.  Therefore, we also handle color
// mapping and extracting the palette here, since we have all the 
// internal data required for those functions.
//
// This code originated (long ago :> ) from Q124947, 
// "Retrieving Palette Information from a Bitmap Resource".
// The knowledge base article also details how to extract a palette 
// from a bitmap, and how to handle the palette.  Another more
// in-depth resource is "The Palette Manager: How and Why".
//-------------------------------------------------------------------//
HBITMAP ColorBitmap::LoadColorBitmap(
	UINT			uBitmapID,
	CDC*			pButtonDC,
	bool			bMapColors,
	bool			bLeaveWhite,
	bool			bExtractPalette
) {


	// This was a function parameter in the sample code.
	HINSTANCE hInstance = AfxGetInstanceHandle();
	
	HRSRC  hRsrc;
	HGLOBAL hGlobal;
	HBITMAP hBitmapFinal = NULL;
	
	// These are used to determine the number of colors in the bitmap.
	LPBITMAPINFOHEADER  lpbih;
	int iNumColors;
   
	CPalette* pOldPalette = NULL;

	hRsrc = FindResource( 
		hInstance, 
		MAKEINTRESOURCE( uBitmapID ),
		RT_BITMAP 
	);

	// Make sure we found it.
	ASSERT( hRsrc );

	// Make sure the palette is ready.
	ASSERT( pPalette );
		
	hGlobal = LoadResource(hInstance, hRsrc);
	lpbih = (LPBITMAPINFOHEADER)LockResource(hGlobal);

	// Make sure that icon was defined.
	if ( !lpbih ) {
		ASSERT( false );
		return 0;
	}

	// Get the number of colors in the bitmap.
	if (lpbih->biBitCount <= 8)
		 iNumColors = (1 << lpbih->biBitCount);
	else
		 iNumColors = 0;  // No palette needed for 24 BPP DIB

	if (lpbih->biClrUsed > 0)
		 iNumColors = lpbih->biClrUsed;  // Use biClrUsed

	// Remap if requested.
	// This will change the color in the bitmap data block,
	// before it is copied to the palette.
	if ( bMapColors )
		RemapBitmap(
			(LPBITMAPINFO)lpbih,
			iNumColors,
			bLeaveWhite
		);
	
	// We may need to extract the palette, but not under
	// standard conditions.
	if ( bExtractPalette )
		ExtractPalette( 
			(LPBITMAPINFO)lpbih,
			iNumColors
		);

	// Select the palette.
	// Note that we use a static palette for all bitmaps.
	// This palette is typically maintained by the app's
	// main frame.  It is already set when we reach this
	// point.  We just need to select it during
	// creation of the bitmap, we don't need to realize it.
	pOldPalette = pButtonDC->SelectPalette(
		pPalette,
		TRUE
	);          
	
	// If we have extracted the palette, we should
	// realize it, in the BACKGROUND, since it may be
	// different than the application-wide palette.
	// If we extract a new palette here, subsequent bitmaps
	// that do not re-extract will use this new palette.
	// So be careful this is what you want.  
	// Currently, the only time we extract the palette is
	// to initialize it when the main frame gets notification
	// that we need to update.  No painting is done with
	// the bitmap, it is only temporarily created to 
	// extract the palette info.  So the realize is not needed
	// in that case.
	if ( bExtractPalette ) {
		pButtonDC->RealizePalette();
	}

	hBitmapFinal = CreateDIBitmap(
		pButtonDC->GetSafeHdc(),
		lpbih,
		(LONG)CBM_INIT,
		(LPSTR)lpbih + lpbih->biSize + iNumColors * sizeof(RGBQUAD),
		(LPBITMAPINFO)lpbih,
		DIB_RGB_COLORS
	);

	// Re-select and re-realize the default palette.
	// We are done with the temporary DC (typically a memory
	// DC), so there may not be a need to re-realize the palette.
	pButtonDC->SelectPalette( pOldPalette, TRUE );
	pButtonDC->RealizePalette();

	UnlockResource(hGlobal);
	FreeResource(hGlobal);
   
	return (hBitmapFinal);
}


//-------------------------------------------------------------------//
// ExtractPalette()																	//
//-------------------------------------------------------------------//
// This is used internally, and requires detailed bitmap
// info.	To extract a palette externally, just create a temporary
// ColorBitmap, specifying the bExtractPalette as true.
//-------------------------------------------------------------------//
void ColorBitmap::ExtractPalette(
	LPBITMAPINFO			lpbi,
	int						iNumColors
) {

	LPLOGPALETTE			lpPal;
	HANDLE					hLogPal;
	HPALETTE 				hPal = NULL;
	int						i;

	if (iNumColors) {
		hLogPal = GlobalAlloc (GHND, sizeof (LOGPALETTE) +
									  sizeof (PALETTEENTRY) * (iNumColors));
		lpPal = (LPLOGPALETTE) GlobalLock (hLogPal);
		lpPal->palVersion 	= 0x300;
		lpPal->palNumEntries = iNumColors;

		// Copy data from bitmap data to newly allocated logical palette.
		for (i = 0;  i < iNumColors;	i++) {

			// Copy.
			lpPal->palPalEntry[i].peRed	= lpbi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue	= lpbi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;

		}

		// If we are re-extracting for the purpose of refreshing the 
		// palette, we need to release the previously attached data.
		if ( pPalette->m_hObject )
			pPalette->DeleteObject();

		VERIFY( pPalette->CreatePalette( lpPal ) );
		
		GlobalUnlock (hLogPal);
		GlobalFree	 (hLogPal);
	}
	
}


//-------------------------------------------------------------------//
// RemapBitmap()																		//
//-------------------------------------------------------------------//
// This function remaps the colors in a bitmap to match the 			//
// user-selected system colors, ala CreateMappedBitmap.  				//
// MS did not separate the functionality of creating a bitmap			//
// and mapping it (duh).  So we duplicate the mapping function 		//
// here, since we have specialized creation code.							//
// This function will leave the white pixels alone if requested.
// This is useful when using white as the transparent color in 
// image lists created with this bitmap.
//-------------------------------------------------------------------//
void ColorBitmap::RemapBitmap(
	LPBITMAPINFO	lpbi,
	int				iNumColors,
	bool				bLeaveWhite
) {

	// This does not work, the structure fields end up reversed.
	// RGBQUAD rgbText;
	// *( (DWORD*) &rgbText ) = GetSysColor( COLOR_BTNTEXT );

	// Set up variables.
	RGBQUAD rgbText;
	DWORD dText = GetSysColor( COLOR_BTNTEXT );
	rgbText.rgbRed =			BYTE0( dText );
	rgbText.rgbGreen =		BYTE1( dText );
	rgbText.rgbBlue =			BYTE2( dText );
	rgbText.rgbReserved =	BYTE3( dText );
	
	RGBQUAD rgbShadow;
	DWORD dShadow = GetSysColor( COLOR_BTNSHADOW );
	rgbShadow.rgbRed =		BYTE0( dShadow );
	rgbShadow.rgbGreen =		BYTE1( dShadow );
	rgbShadow.rgbBlue =		BYTE2( dShadow );
	rgbShadow.rgbReserved =	BYTE3( dShadow );
	
	RGBQUAD rgbFace;
	DWORD dFace = GetSysColor( COLOR_BTNFACE );
	rgbFace.rgbRed =			BYTE0( dFace );
	rgbFace.rgbGreen =		BYTE1( dFace );
	rgbFace.rgbBlue =			BYTE2( dFace );
	rgbFace.rgbReserved =	BYTE3( dFace );
	
	RGBQUAD rgbHigh;
	DWORD dHigh = GetSysColor( COLOR_BTNHIGHLIGHT );
	rgbHigh.rgbRed =			BYTE0( dHigh );
	rgbHigh.rgbGreen =		BYTE1( dHigh );
	rgbHigh.rgbBlue =			BYTE2( dHigh );
	rgbHigh.rgbReserved =	BYTE3( dHigh );

	// Rip through bitmap and remap colors.
	for ( int i = 0;  i < iNumColors;	i++ ) {

		// Text test.
		if ( 			lpbi->bmiColors[i].rgbRed == 0
					&&	lpbi->bmiColors[i].rgbGreen == 0
					&& lpbi->bmiColors[i].rgbBlue == 0 ) {

			lpbi->bmiColors[i] = rgbText;

		// Shadow test.
		} else if ( lpbi->bmiColors[i].rgbRed == 128
					&&	lpbi->bmiColors[i].rgbGreen == 128
					&& lpbi->bmiColors[i].rgbBlue == 128 ) {

			lpbi->bmiColors[i] = rgbShadow;

		// Face test.
		} else if ( lpbi->bmiColors[i].rgbRed == 192
					&&	lpbi->bmiColors[i].rgbGreen == 192
					&& lpbi->bmiColors[i].rgbBlue == 192 ) {

			lpbi->bmiColors[i] = rgbFace;

		// MYSTERIES OF THE UNKNOWN
		// IN SEARCH OF... CODE ANOMALIES
		//	If I put this code in, all of a sudden
		// the tree bitmaps lose their transparency.
		// I can't figure it out!  So white mapping
		// is currently always skipped, until we
		// figure this out.
		// It's weird - the tree bitmaps always have
		// bLeaveWhite set to true, so they always
		// bypass this remap, but they lose transparency
		// unless you remove this code.
		/*
		// Highlight test.
		// We may have been asked to bypass this.
		} else if (	!bLeaveWhite
					&& lpbi->bmiColors[i].rgbRed == 255
					&&	lpbi->bmiColors[i].rgbGreen == 255
					&& lpbi->bmiColors[i].rgbBlue == 255 ) {

			lpbi->bmiColors[i] = rgbHigh;
		*/

		}
	}

}


//-------------------------------------------------------------------//
// Draw()																				//
//-------------------------------------------------------------------//
// This routine draws the bitmap with only a point.
// It looks up the size of the bitmap automatically.
//-------------------------------------------------------------------//
BOOL ColorBitmap::Draw( 
	CDC*		pDC, 
	CPoint*	pLocation,
	CSize*	pSize
) {

	CBitmap *	pbmpOrig;
	CDC			MemoryDC;
 	
	// Make sure the user has already LOADED the bitmap!
	ASSERT( m_hObject );
	
	// If a palette was provided, we select and realize it in the BACKGROUND.
	// This should be identical to selecting it in the foreground when the
	// app has the focus, because the app's main frame did a FOREGROUND
	// realize when we got the focus.
	if ( pPalette ) {
		pDC->SelectPalette( pPalette, TRUE );
		pDC->RealizePalette();
	}
	
	// Create a compatible memory DC.
	// Note that OLE Items' Draw functions use a CMetaFileDC,
	// not a CDC.  CreateCompatibleDC will fail for this type
	// of DC.
	if( pDC->IsKindOf( RUNTIME_CLASS( CMetaFileDC ) ) ) {

		MemoryDC.CreateCompatibleDC( NULL );
	
	} else {

		// CCDC requires RC_BITBLT capability.
		ASSERT( pDC->GetDeviceCaps( RC_BITBLT ) );

		MemoryDC.CreateCompatibleDC( pDC );
	
	}
 
   // Select the bitmap into the DC.
	pbmpOrig = MemoryDC.SelectObject( this );
 
   // If we weren't given the size, get the dimensions of the bitmap.
	CSize ActualSize;
	if ( pSize == NULL ) {
		BITMAP BitmapData;
		GetBitmap( &BitmapData );
		ActualSize.cx = BitmapData.bmWidth;
		ActualSize.cy = BitmapData.bmHeight;
		pSize = &ActualSize;
	}

	// Copy (BitBlt) bitmap from memory DC to screen DC
   BOOL bReturn = pDC->BitBlt( 
			pLocation->x, 
			pLocation->y, 
			pSize->cx, 
			pSize->cy, 
			&MemoryDC, 
			0, 0, 
			SRCCOPY );
 
	// Clean up.
	// Reselect the original.
	MemoryDC.SelectObject( pbmpOrig );

	// You don't usually have to delete standard DC's in MFC,
	// but you do have to delete this one, as it was created
	// with "CreateCompatibleDC" as a memory DC.
	MemoryDC.DeleteDC();
	
	return bReturn;

}


//-------------------------------------------------------------------//
// Draw()																				//
//-------------------------------------------------------------------//
// This routine draws the bitmap, given a complete
// rectangle encapsulating it.  It can be used in
// situations where the bitmap dimensions are already
// known, so that the bitmap data structure does not 
// need to be loaded.
//-------------------------------------------------------------------//
BOOL ColorBitmap::Draw( CDC* pDC, CRect* pLocation )
{

	return Draw( pDC, &(pLocation->TopLeft()), &(pLocation->Size()) );
	
}


//-------------------------------------------------------------------//
// DrawBitmapToButton()															//
//-------------------------------------------------------------------//
// Draws the bitmap within the given rectangle, with the requested 
// style.
//-------------------------------------------------------------------//
void ColorBitmap::DrawBitmapToButton(
	CDC			*pDC,				// Device context to draw in.
	CRect			rect,				// Rectangle to draw in.
	UINT			state,		 	// Button state: pressed, focus, disabled.
	bool			bHasGrayOut,	// Has a grayed out version in the bitmap?
	bool			bSunken			// Want that extremely sunken look?
) {

	int		nShift = 0;

	// Make sure the user has already LOADED the bitmap!
	ASSERT( m_hObject );
	
	// If a palette was provided, we select and realize it in the BACKGROUND.
	// This should be identical to selecting it in the foreground when the
	// app has the focus, because the app's main frame did a FOREGROUND
	// realize when we got the focus.  RealizePalette() has no net
	// effect.
	// When the app is in the background, RealizePalette() actually causes
	// a remap of our palette to the palette of the app with the focus.
	if ( pPalette ) {
		
		pDC->SelectPalette( pPalette, TRUE );
		pDC->RealizePalette();
	}
	
	// Short color ramp for grayed-out bitmaps to map to.
	// Picked 12 for the size because of the math:  If brightness
	// is a measure of R+G+B for a range of 0 to (256*3), then
	// we can shift this value to the right by six ( same as
	// dividing it by 64 ) to get a range of 0 to 11.  Using
	// the shift saves a divide operation.
	static DWORD dwGray[12];
	static bool bInitGray = false;
	static bool bIsTrueColor = true;

	if ( !bInitGray ) {

		if ( pDC->GetDeviceCaps( NUMCOLORS ) != -1 )
			bIsTrueColor = FALSE;

		// Create color ramp to map disabled bitmaps to.
		dwGray[0] = GetSysColor( COLOR_BTNSHADOW );
		dwGray[11] = GetSysColor( COLOR_BTNHIGHLIGHT );

		for ( int nA=1; nA<11; nA++ ) {
			dwGray[nA] = BYTE0( dwGray[0] ) +
				( ( ( BYTE0( dwGray[11] ) - BYTE0( dwGray[0] ) ) * nA ) / 11 );
			dwGray[nA] += ( BYTE1( dwGray[0] ) +
				( ( ( BYTE1( dwGray[11] ) - BYTE1( dwGray[0] ) ) * nA ) / 11 ) ) << 8;
			dwGray[nA] += ( BYTE2( dwGray[0] ) +
				( ( ( BYTE2( dwGray[11] ) - BYTE2( dwGray[0] ) ) * nA ) / 11 ) ) << 16;

			// Map color ramp to palette
			if ( !bIsTrueColor ) {
				if ( !ColorBitmap::pPalette ) {
					ASSERT( false );	// Must have palette to make this work.
				} else {
					dwGray[ nA ] = PALETTEINDEX(
						ColorBitmap::pPalette->GetNearestPaletteIndex(
							dwGray[ nA ] ) );
				}
			}

		}

		bInitGray = true;
	}


	// If there is a grayed out version in the bitmap,
	// then we need to split the bitmap down the middle.
	if ( bHasGrayOut ) nShift = 1;

	// Get a memDC to play with.
	// This will maintain the palette info.
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	if ( !memDC.GetSafeHdc() )
		return;
	
	CBitmap* pOld = memDC.SelectObject( this );

	// determine bitmaps size for use in StretchBlt         
	BITMAP bits;
	GetObject(sizeof(BITMAP),&bits);

	// Create some pens and brushes to use for drawing the button.
	CPen		pText( PS_SOLID, 0, GetSysColor( COLOR_BTNTEXT ) );
	CBrush	brBack( GetSysColor( COLOR_BTNFACE ) );

	// Clear background.
	pDC->SelectObject( brBack );
	pDC->SelectObject( pText );
	pDC->Rectangle( &rect );
	pDC->SelectObject( GetStockObject( NULL_BRUSH ) );
	pDC->SelectObject( GetStockObject( BLACK_PEN ) );

	int xOff, yOff;

	// Draw selected (pushed in) button shape.
	if ( state & ODS_SELECTED ) {
		if ( bSunken ) {
			pDC->DrawEdge( &rect, EDGE_SUNKEN, BF_BOTTOMRIGHT );
			pDC->DrawEdge( &rect, BDR_SUNKENINNER, BF_TOPLEFT );
			// pDC->DrawEdge( &rect, EDGE_SUNKEN, BF_RECT );
		}
		// Simulate putton press by moving bitmap down and to the right.
		xOff = yOff = 1;
	}

	// Draw 3d button shape.
	else {
		pDC->DrawEdge( &rect, BDR_RAISEDINNER, BF_TOPLEFT );
		pDC->DrawEdge( &rect, EDGE_RAISED, BF_BOTTOMRIGHT );
		xOff = yOff = 0;
	}

	xOff += rect.left + ( ( rect.Width() - (bits.bmWidth>>nShift) ) >> 1 );
	yOff += rect.top + ( ( rect.Height() - bits.bmHeight ) >> 1 );
	
	// If there is a gray button provided, and the button is disabled,
	// the draw the grayed version of the bitmap.
	if ( ( state & ODS_DISABLED ) && bHasGrayOut ) {

		pDC->BitBlt( xOff, yOff, bits.bmWidth>>1, bits.bmHeight,
			&memDC, bits.bmWidth>>1, 0, SRCCOPY);

	} else {

		// If there is no gray button provided, create one on the fly....
		if ( state & ODS_DISABLED ) {

			DWORD dwShadow = GetSysColor( COLOR_BTNSHADOW );
			DWORD dwHigh = GetSysColor( COLOR_BTNHIGHLIGHT );

			// True color mode - Use the gray color ramp defined in
			// the static variables above.  Looks really smooth!

			// NOTE: because of the GetNearestPaletteIndex() code
			// above we can now use the gray ramp in 256 color mode!
			// Hence the mass of commented out code below.

			//if ( bIsTrueColor ) {
				for ( int nX = 0; nX < bits.bmWidth>>nShift; nX ++ ) {
					for ( int nY = 0; nY < bits.bmHeight; nY ++ ) {
						COLORREF color;
						color = memDC.GetPixel( nX, nY );
						int nLevel = BYTE0(color) + BYTE1(color) + BYTE2(color);
						pDC->SetPixel( nX + xOff, nY + yOff, dwGray[nLevel>>6] );
					}
				}
			//}

			/*
			// 256 (or 16) color mode - Map all the pixels to the three
			// system colors: the face, shadow and highlight.
			else {
				for ( int nX = 0; nX < bits.bmWidth>>nShift; nX ++ ) {
					for ( int nY = 0; nY < bits.bmHeight; nY ++ ) {
						COLORREF color;
						color = memDC.GetPixel( nX, nY );
						int nLevel = BYTE0(color) + BYTE1(color) + BYTE2(color);
						if ( nLevel > 450 )
							pDC->SetPixel( nX + xOff, nY + yOff, dwHigh );
						else if ( nLevel < 240 )
							pDC->SetPixel( nX + xOff, nY + yOff, dwShadow );
					}
				}
			}
			*/

		}

		// Normal button draw.
		else {
			pDC->BitBlt( xOff, yOff, bits.bmWidth>>nShift, bits.bmHeight,
				&memDC, 0, 0, SRCCOPY);
		}

	}

	// Draw focus rectangle.
	if ( state & ODS_FOCUS ) {
		CRect rclFocus( rect.left+3, rect.top+3, rect.right-3, rect.bottom-3 );
		pDC->DrawFocusRect( &rclFocus );
	}

	memDC.SelectObject(pOld);

}


//-------------------------------------------------------------------//
// BestAvailableColor()																//
//-------------------------------------------------------------------//
// Returns a color reference based upon the best available color for	//
// the passed device context.  This function can be used with			//
// printer DC's, and takes into account the palette.						//
//-------------------------------------------------------------------//
COLORREF ColorBitmap::BestAvailableColor( CDC *pDC, COLORREF Color ) {

	bool	bIsPalette	= ( 0 != ( pDC->GetDeviceCaps( RASTERCAPS ) & RC_PALETTE ) );
	bool	bIsPrinting	= ( pDC->IsPrinting() != 0 );

	int	nColorBits	= pDC->GetDeviceCaps( COLORRES );
	int	nColors		= pDC->GetDeviceCaps( NUMCOLORS );

	// If this is a screen draw to a paletted DC then use a palette color.
	if ( !bIsPrinting && ColorBitmap::pPalette && bIsPalette ) {
		Color = PALETTEINDEX( ColorBitmap::pPalette->GetNearestPaletteIndex( Color ) );
	}
	
	// If this is a b/w printer then convert Color to grayscale.
	else if ( bIsPrinting && ( nColors < 3 ) ) {

		// This averaging method was too dark.
		int nLevel = ( ((int)BYTE0(Color)) + ((int)BYTE1(Color)) + ((int)BYTE2(Color)) ) / 3;
		nLevel = min( nLevel, 255 );

		Color = nLevel + (nLevel<<8) + (nLevel<<16);
	}

	return Color;
}


