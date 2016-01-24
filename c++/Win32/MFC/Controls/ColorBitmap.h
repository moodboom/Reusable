//-------------------------------------------------------------------//
// ColorBitmap.h interface															//
//																							//
//-------------------------------------------------------------------//

#ifndef COLOR_BITMAP_H
#define COLOR_BITMAP_H

class ColorBitmap : public CBitmap
{

public:

	// Our constructor.
	ColorBitmap(
		UINT			uBitmapResourceID,
		bool			bMapColors				= true,
		bool			bLeaveWhite				= false,
		bool			bExtractPalette		= false
	);

	BOOL Draw( 
		CDC*		pDC, 
		CPoint*	pLocation,
		CSize*	pSize = NULL
	);

	BOOL Draw( 
		CDC* pDC, 
		CRect*  pLocation 
	);

	void DrawBitmapToButton(
		CDC			*pDC,				// Device context to draw in.
		CRect			rect,				// Rectangle to draw in.
		UINT			state,		 	// Button state: pressed, focus, disabled.
		bool			bHasGrayOut,	// Has a grayed out version in the bitmap?
		bool			bSunken			// Want that extremely sunken look?
	);

	// Returns a color reference based upon the best
	// available color for the passed device context.
	// This function can be used wit printer DC's, and
	// takes into account the palette.
	static COLORREF BestAvailableColor( CDC *pDC, COLORREF Color );

	// We keep a pointer to the palette, as we'll need it to draw.
	// There is only one palette, shared by the entire app.
	// Be aware that if we trash this palette with a BACKGROUND
	// realization, it needs to be reloaded.
	static CPalette* pPalette;

protected:

	// This remaps the entire bitmap memory to system colors.
	// If white is used for transparency, it can be excluded
	// from the mapping.
	void RemapBitmap(
		LPBITMAPINFO	lpbi,
		int				iNumColors,
		bool				bLeaveWhite
	);

	// This function loads the given bitmap, extracting
	// data about the bitmap so that it can be created
	// using all its colors.
	HBITMAP LoadColorBitmap(
		UINT			uBitmapID,
		CDC*			pButtonDC,
		bool			bMapColors,
		bool			bLeaveWhite,
		bool			bExtractPalette
	);

	void ExtractPalette(
		LPBITMAPINFO			lpbi,
		int						iNumColors
	);

};

/////////////////////////////////////////////////////////////////////////////

#endif	// COLOR_BITMAP_H
