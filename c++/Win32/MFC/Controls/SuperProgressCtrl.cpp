//////////////////////////////////////////////////////////////////
// SuperProgressCtrl.cpp
// Implementation file of the CSuperProgressCtrlCtrl class

#include "stdafx.h"

#include "SuperProgressCtrl.h"

///////////////////////////////////////////////////////////////////
// CSuperProgressCtrl implementation
//

/////////////////////////////////////////////////////////////////////
// Converts a bitmap to a region
//
//	BitmapToRegion :	Create a region from the "non-transparent" pixels of a bitmap
//	Author :			Jean-Edouard Lachand-Robert (http://www.geocities.com/Paris/LeftBank/1160/resume.htm), June 1998.
//
//	hBmp :				Source bitmap
//	cTransparentColor :	Color base for the "transparent" pixels (default is black)
//	cTolerance :		Color tolerance for the "transparent" pixels.
//
//	A pixel is assumed to be transparent if the value of each of its 3 components (blue, green and red) is 
//	greater or equal to the corresponding value in cTransparentColor and is lower or equal to the 
//	corresponding value in cTransparentColor + cTolerance.
//
static
HRGN BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor = 0,
					COLORREF cTolerance = 0x101010)
{
	HRGN hRgn = NULL;

	if(hBmp)
	{
		// Create a memory DC inside which we will scan the bitmap content
		HDC hMemDC = CreateCompatibleDC(NULL);
		if(hMemDC)
		{
			// Get bitmap size
			BITMAP bm;
			GetObject(hBmp, sizeof(bm), &bm);

			// Create a 32 bits depth bitmap and select it into the memory DC 
			BITMAPINFOHEADER RGB32BITSBITMAPINFO = {	
					sizeof(BITMAPINFOHEADER),	// biSize 
					bm.bmWidth,					// biWidth; 
					bm.bmHeight,				// biHeight; 
					1,							// biPlanes; 
					32,							// biBitCount 
					BI_RGB,						// biCompression; 
					0,							// biSizeImage; 
					0,							// biXPelsPerMeter; 
					0,							// biYPelsPerMeter; 
					0,							// biClrUsed; 
					0							// biClrImportant; 
			};
			VOID * pbits32; 
			HBITMAP hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
			if(hbm32)
			{
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

				// Create a DC just to copy the bitmap into the memory DC
				HDC hDC = CreateCompatibleDC(hMemDC);
				if(hDC)
				{
					// Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits)
					BITMAP bm32;
					GetObject(hbm32, sizeof(bm32), &bm32);
					while(bm32.bmWidthBytes % 4)
						bm32.bmWidthBytes++;

					// Copy the bitmap into the memory DC
					HBITMAP holdBmp = (HBITMAP)SelectObject(hDC, hBmp);
					BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

					// For better performances, we will use the ExtCreateRegion() function to create the
					// region. This function take a RGNDATA structure on entry. We will add rectangles by
					// amount of ALLOC_UNIT number in this structure.
					#define ALLOC_UNIT	100
					DWORD maxRects = ALLOC_UNIT;
					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
					RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

					// Keep on hand highest and lowest values for the "transparent" pixels
					BYTE lr = GetRValue(cTransparentColor);
					BYTE lg = GetGValue(cTransparentColor);
					BYTE lb = GetBValue(cTransparentColor);
					BYTE hr = min(0xff, lr + GetRValue(cTolerance));
					BYTE hg = min(0xff, lg + GetGValue(cTolerance));
					BYTE hb = min(0xff, lb + GetBValue(cTolerance));

					// Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
					BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
					for(int y = 0; y < bm.bmHeight; y++)
					{
						// Scan each bitmap pixel from left to right
						for(int x = 0; x < bm.bmWidth; x++)
						{
							// Search for a continuous range of "non transparent pixels"
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while(x < bm.bmWidth)
							{
								BYTE b = GetRValue(*p);
								if(b >= lr && b <= hr)
								{
									b = GetGValue(*p);
									if(b >= lg && b <= hg)
									{
										b = GetBValue(*p);
										if(b >= lb && b <= hb)
											// This pixel is "transparent"
											break;
									}
								}
								p++;
								x++;
							}

							if(x > x0)
							{
								// Add the pixels (x0, y) to (x, y+1) as a new rectangle in the region
								if(pData->rdh.nCount >= maxRects)
								{
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
									pData = (RGNDATA *)GlobalLock(hData);
								}
								RECT *pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
								if(x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0;
								if(y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y;
								if(x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x;
								if(y+1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y+1;
								pData->rdh.nCount++;

								// On Windows98, ExtCreateRegion() may fail if the number of rectangles is too
								// large (ie: > 4000). Therefore, we have to create the region by multiple steps.
								if(pData->rdh.nCount == 2000)
								{
									HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
									if(hRgn)
									{
										CombineRgn(hRgn, hRgn, h, RGN_OR);
										DeleteObject(h);
									}
									else
										hRgn = h;
									pData->rdh.nCount = 0;
									SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
								}
							}
						}

						// Go to next row (remember, the bitmap is inverted vertically)
						p32 -= bm32.bmWidthBytes;
					}

					// Create or extend the region with the remaining rectangles
					HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
					if(hRgn)
					{
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					}
					else
						hRgn = h;

					// Clean up
					
					// ** Fix from CodeGuru member Steve Chia **
					GlobalUnlock(hData);   // <--- add this
   				
					GlobalFree(hData);
					SelectObject(hDC, holdBmp);
					DeleteDC(hDC);
				}

				DeleteObject(SelectObject(hMemDC, holdBmp));
			}

			DeleteDC(hMemDC);
		}	
	}

	return hRgn;
}

///////////////////////////////////////////////////////////////////
// InflateRegion - Inflates a region by the x and y values
// specified in nXInflate and nYInflate
// Creates a new region that represents the inflated region
// (retains the contents of the old region)
// Returns NULL if unsuccessfull
static HRGN InflateRegion(HRGN hRgn, int nXInflate, int nYInflate)
{
	// Local Variables
	LPRGNDATA lpData;	// The RGNDATA structure
	LPRECT lpRect;		// Pointer to the array of RECT structures
	DWORD BufSize;		// The amount of memory required
	DWORD i;			// General index variable
	HRGN hRgnNew;		// The newly created region

	// Get the number of rectangles in the region
	BufSize = GetRegionData(hRgn, 0, NULL);
	if(BufSize == 0)
		return NULL;
	// Allocate memory for the RGNDATA structure
	lpData = (LPRGNDATA)malloc(BufSize);
	// Set the location of the RECT structures
	lpRect = (LPRECT)(lpData->Buffer);
	// Get the region data
	if(!GetRegionData(hRgn, BufSize, lpData))
	{
		free(lpData);
		return NULL;
	}
	// Expand (or contract) all the rectangles in the data
	for(i=0; i<lpData->rdh.nCount; i++)
		InflateRect(&lpRect[i], nXInflate, nYInflate);
	// Create the new region
	hRgnNew = ExtCreateRegion(NULL, lpData->rdh.nCount, lpData);
	free((void*)lpData);
	return hRgnNew;
}

/////////////////////////////////////////////////////////////////////////////
// CSuperProgressCtrl

CSuperProgressCtrl::CSuperProgressCtrl()
{
}

CSuperProgressCtrl::~CSuperProgressCtrl()
{
	// ** Fix from CodeGuru member Steve Chia **
	// add this
   DeleteObject((HBITMAP)m_hbmArea);
   DeleteObject((HRGN)m_hRegion);
}


BEGIN_MESSAGE_MAP(CSuperProgressCtrl, CWnd)
	//{{AFX_MSG_MAP(CSuperProgressCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSuperProgressCtrl message handlers

CString CSuperProgressCtrl::s_Class = _T("");
void CSuperProgressCtrl::RegisterClass()
{
	// Register the window class, if not already registered.
	if ( s_Class.IsEmpty() )
		s_Class = AfxRegisterWndClass(
			CS_CLASSDC,
			AfxGetApp()->LoadStandardCursor(IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE+1), 
			NULL
		);
}

BOOL CSuperProgressCtrl::Create(CWnd* pParent, int x, int y,
							HBITMAP hbmArea, UINT nID)
{
	// Local Variables
	BITMAP bm;
	BOOL bRet;
	HRGN hRgn;

	// Get the dimensions of the given bitmap
	if(GetObject((HGDIOBJ)hbmArea, sizeof(BITMAP), (void*)&bm) == 0)
		return FALSE;
	// Create the window
	bRet = CWnd::Create(s_Class, NULL, WS_VISIBLE | WS_CHILD,
		CRect(x, y, x+bm.bmWidth-1, y+bm.bmHeight-1), pParent,
		nID, NULL);
	// Set the various member variables
	m_nMin = 0;			// Default minimum is 0
	m_nMax = 100;		// Default maximum is 100
	m_nPosition = 0;	// Initial position is at 0
	m_nStep = 10;		// Default step is 10
	m_Colour1 = RGB(0,0,255);	// Default start colour is blue
	m_Colour2 = RGB(255,0,0);	// Default end colour is red
	m_Background = GetSysColor(COLOR_3DFACE);	// Dialog Background
	m_nFillStyle = SP_FILL_HORZGRAD;	// Horizontal Gradient
	// If the window was created successfully, set the window region
	if(bRet)
	{
		m_hbmArea = hbmArea;
		m_hRegion = BitmapToRegion(hbmArea, RGB(255,255,255), 0x000000);
		hRgn = InflateRegion(m_hRegion, 1, 1);

		// ** Fix from CodeGuru member Steve Chia **
		::SetWindowRgn(m_hWnd, hRgn, TRUE);

		// We don't need to delete the region,
		// Windows does it for us when the
		// window is destroyed
	}
	// Return the status of the window creation
	return bRet;
}

void CSuperProgressCtrl::SetRange(int nMin, int nMax)
{
	// Make sure that max > min
	ASSERT(nMax > nMin);
	// Set the maximum and minimum values
	m_nMin = nMin;
	m_nMax = nMax;

	// Invalidate the window
	::InvalidateRgn(m_hWnd, m_hRegion, FALSE);
}

int CSuperProgressCtrl::SetPos(int nPos)
{
	int oldpos = m_nPosition;

	// Set the new position
	m_nPosition = nPos;

   // Invalidate the window if needed.
   if ( bBitmapRedrawNeeded( oldpos ) )
   {
	   ::InvalidateRgn(m_hWnd, m_hRegion, FALSE);
	}
		
	return oldpos;
}

int CSuperProgressCtrl::OffsetPos(int nOffset)
{
	int oldpos = m_nPosition;
	// Make sure that min <= pos <= max
	ASSERT(((nOffset+m_nPosition) >= m_nMin) &&
		((nOffset+m_nPosition) <= m_nMax));
	// Set the new position
	m_nPosition += nOffset;
	
	// Invalidate the window if needed.
	if ( bBitmapRedrawNeeded( oldpos ) )
	{
		::InvalidateRgn(m_hWnd, m_hRegion, FALSE);
	}
		
	return oldpos;
}

int CSuperProgressCtrl::SetStep(int nStep)
{
	int oldpos = m_nStep;
	// Make sure that step is less than
	// the difference between max and min
	ASSERT(nStep <= (m_nMax - m_nMin + 1));
	// Set the new step value
	m_nStep = nStep;
	return oldpos;
}

int CSuperProgressCtrl::StepIt()
{
	int oldpos = m_nPosition;
	// Only step if the new position is in range
	if(((m_nPosition+m_nStep) >= m_nMin) &&
		((m_nPosition+m_nStep) <= m_nMax))
		m_nPosition += m_nStep;
	else if((m_nPosition+m_nStep) < m_nMin)
		m_nPosition = m_nMin;
	else
		m_nPosition = m_nMax;

	// Invalidate the window if needed.
	if ( bBitmapRedrawNeeded( oldpos ) )
	{
		::InvalidateRgn(m_hWnd, m_hRegion, FALSE);
	}

	return oldpos;
}

int CSuperProgressCtrl::SetFillStyle(int nStyle)
{
	int oldstyle = m_nFillStyle;
	// Make sure the new style is valid
	ASSERT((nStyle >= 1) && (nStyle <= 4));
	// Set the new fill style
	m_nFillStyle = nStyle;

	// Invalidate the window if needed.
	if(m_nFillStyle != oldstyle)
		::InvalidateRgn(m_hWnd, m_hRegion, FALSE);

	// Return the old style
	return oldstyle;
}

int CSuperProgressCtrl::GetFillStyle() const
{
	// Return the current fill style
	return m_nFillStyle;
}

void CSuperProgressCtrl::SetColours(COLORREF Colour1, COLORREF Colour2)
{
	// Invalidate the window if necessary
	if((Colour1 != m_Colour1) || (Colour2 != m_Colour2))
		::InvalidateRgn(m_hWnd, m_hRegion, FALSE);
	// Set the new colours
	m_Colour1 = Colour1;
	m_Colour2 = Colour2;
}

void CSuperProgressCtrl::GetColours(COLORREF* Colour1,
									COLORREF* Colour2) const
{
	// Return the old colours
	if(Colour1) *Colour1 = m_Colour1;
	if(Colour2) *Colour2 = m_Colour2;
}

COLORREF CSuperProgressCtrl::SetBackColour(COLORREF Colour)
{
	COLORREF oldval = m_Background;
	// Set the background colour
	m_Background = Colour;
	return oldval;
}

COLORREF CSuperProgressCtrl::GetBackColour() const
{
	return m_Background;
}

BOOL CSuperProgressCtrl::MessageLoop() const
{
	// Local variables
	MSG msg;
	LONG lIdle;
	// Process all the messages in the message queue
	while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if(!AfxGetApp()->PumpMessage())
		{
			PostQuitMessage(0);
			return FALSE;	// Signal WM_QUIT received
		}
	}
	// let MFC do its idle processing
	lIdle = 0;
	while(AfxGetApp()->OnIdle(lIdle++));
	// Signal to continue processing
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////
// Emboss         - Creates a 3D embossed effect
// Taken directly from an article by Zafir Anjum entitled
// "Emboss text and other shape on your bitmap"
///////////////////////////////////////////////////////////////////////////////////
// Returns                - A new bitmap containing the resulting effect
// hBitmap                - Bitmap that contains the basic text & shapes
// hbmBackGnd             - Contains the color image 
// hPal                   - Handle of palette associated with hbmBackGnd
// bRaised                - True if raised effect is desired. False for sunken effect
// xDest          - x coordinate - used to offset hBitmap
// yDest          - y coordinate - used to offset hBitmap
// clrHightlight  - Color used for the highlight edge
// clrShadow              - Color used for the shadow
//
// Note                   - 1. Neither of the bitmap handles passed in should be selected 
//                          in a device context.
//                          2. The pixel at 0,0 in hBitmap is considered the background color
//
static
HBITMAP Emboss( HBITMAP hBitmap, HBITMAP hbmBackGnd,
				HPALETTE hPal, BOOL bRaised, int xDest, int yDest, 
				COLORREF clrHighlight, COLORREF clrShadow )
{
        const DWORD PSDPxax = 0x00B8074A;
        BITMAP   bmInfo ;
        HBITMAP  hbmOld, hbmShadow, hbmHighlight, hbmResult, hbmOldMem ;
        HBRUSH   hbrPat ;
        HDC      hDC, hColorDC, hMonoDC, hMemDC ;

        if( !bRaised )
        {
                // Swap the highlight and shadow color
                COLORREF clrTemp = clrShadow;
                clrShadow = clrHighlight;
                clrHighlight = clrTemp;
        }
        
        // We create two monochrome bitmaps. One of them will contain the
        // highlighted edge and the other will contain the shadow. These
        // bitmaps are then used to paint the highlight and shadow on the
        // background image.
        
        hbmResult = NULL ;
        hDC = GetDC( NULL ) ;

        // Create a compatible DCs
        hMemDC = ::CreateCompatibleDC( hDC );
        hMonoDC = CreateCompatibleDC( hDC );
        hColorDC = CreateCompatibleDC( hDC );

        if( hMemDC == NULL || hMonoDC == NULL || hColorDC == NULL )
        {
                if( hMemDC ) DeleteDC( hMemDC );
                if( hMonoDC ) DeleteDC( hMonoDC );
                if( hColorDC ) DeleteDC( hColorDC );

                return NULL;
        }

        // Select the background image into memory DC so that we can draw it
        hbmOldMem = (HBITMAP)::SelectObject( hMemDC, hbmBackGnd );
        
        // Get dimensions of the background image
        BITMAP bm;
        ::GetObject( hbmBackGnd, sizeof( bm ), &bm );
        
        
        
        // Create the monochrome and compatible color bitmaps 
        GetObject( hBitmap, sizeof( BITMAP ), (LPSTR) &bmInfo ) ;
        hbmShadow =
                CreateBitmap( bmInfo.bmWidth, bmInfo.bmHeight, 1, 1, NULL ) ;
        hbmHighlight =
                CreateBitmap( bmInfo.bmWidth, bmInfo.bmHeight, 1, 1, NULL ) ;
        hbmResult =
                CreateCompatibleBitmap( hDC, bm.bmWidth, bm.bmHeight ) ;
        
        hbmOld = (HBITMAP)SelectObject( hColorDC, hBitmap ) ;
        
        // Set background color of bitmap for mono conversion
        // We assume that the pixel in the top left corner has the background color
        SetBkColor( hColorDC, GetPixel( hColorDC, 0, 0 ) ) ;
        
        // Create the highlight bitmap.
        hbmHighlight = (HBITMAP)SelectObject( hMonoDC, (HGDIOBJ) hbmHighlight ) ;
        PatBlt( hMonoDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, WHITENESS ) ;
        BitBlt( hMonoDC, 0, 0, bmInfo.bmWidth - 1, bmInfo.bmHeight - 1,
                hColorDC, 1, 1, SRCCOPY ) ;
        BitBlt( hMonoDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight,
                hColorDC, 0, 0, MERGEPAINT ) ;
        hbmHighlight = (HBITMAP)SelectObject( hMonoDC, (HGDIOBJ) hbmHighlight ) ;

        
        // create the shadow bitmap
        hbmShadow = (HBITMAP)SelectObject( hMonoDC, (HGDIOBJ) hbmShadow ) ;
        PatBlt( hMonoDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, WHITENESS ) ;
        BitBlt( hMonoDC, 1, 1, bmInfo.bmWidth-1, bmInfo.bmHeight-1,
                hColorDC, 0, 0, SRCCOPY ) ;
        BitBlt( hMonoDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight,
                hColorDC, 0, 0, MERGEPAINT ) ;
        hbmShadow = (HBITMAP)SelectObject( hMonoDC, (HGDIOBJ) hbmShadow ) ;

        
        // Now let's start working on the final image
        SelectObject( hColorDC, hbmResult ) ;
        // Select and realize the palette if one is supplied
        if( hPal && GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE )
        {
                ::SelectPalette( hColorDC, hPal, FALSE );
                ::RealizePalette(hColorDC);
        }
        // Draw the background image
        BitBlt(hColorDC, 0, 0, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0,SRCCOPY);
        // Restore the old bitmap in the hMemDC
        ::SelectObject( hMemDC, hbmOldMem );
        
        
        // Set the background and foreground color for the raster operations
        SetBkColor( hColorDC, RGB(255,255,255) ) ;
        SetTextColor( hColorDC, RGB(0,0,0) ) ;
        
        // blt the highlight edge
        hbrPat = CreateSolidBrush( clrHighlight ) ;
        hbrPat = (HBRUSH)SelectObject( hColorDC, hbrPat ) ;
        hbmHighlight = (HBITMAP)SelectObject( hMonoDC, (HGDIOBJ) hbmHighlight ) ;
        BitBlt( hColorDC, xDest, yDest, bmInfo.bmWidth, bmInfo.bmHeight,
                hMonoDC, 0, 0, PSDPxax ) ;
        DeleteObject( SelectObject( hColorDC, hbrPat ) ) ;
        hbmHighlight = (HBITMAP)SelectObject( hMonoDC, (HGDIOBJ) hbmHighlight ) ;
        
        // blt the shadow edge
        hbrPat = CreateSolidBrush( clrShadow ) ;

        hbrPat = (HBRUSH)SelectObject( hColorDC, hbrPat ) ;
        hbmShadow = (HBITMAP)SelectObject( hMonoDC, (HGDIOBJ) hbmShadow ) ;
        BitBlt( hColorDC, xDest, yDest, bmInfo.bmWidth, bmInfo.bmHeight,
                hMonoDC, 0, 0, PSDPxax ) ;
        DeleteObject( SelectObject( hColorDC, hbrPat ) ) ;
        hbmShadow = (HBITMAP)SelectObject( hMonoDC, (HGDIOBJ) hbmShadow ) ;
        
        // select old bitmap into color DC 
        SelectObject( hColorDC, hbmOld ) ;
        
        DeleteObject( (HGDIOBJ) hbmShadow ) ;
        DeleteObject( (HGDIOBJ) hbmHighlight ) ;
        
			// ** Fix from CodeGuru member Steve Chia **
			// to be added at the end of method
			DeleteObject((HDC)hColorDC);  // <---  add this
			DeleteObject((HDC)hMonoDC);   // <---  add this
			DeleteObject((HDC)hMemDC);    // <---  add this

		  ReleaseDC( NULL, hDC ) ;

        return ( hbmResult ) ;
}

void CSuperProgressCtrl::OnPaint() 
{
	// Note: It is very easy to fill in the shape of the bitmap,
	//       a result of the SetWindowRgn() call in the Create()
	//       function. The only clipping we have to perform is
	//       with a call to SelectClipRgn()
	
	// Local Variables
	CPaintDC dc(this);	// device context for painting
	CDC memDC;			// memory device context (eliminate flicker)
	HBITMAP hbmBack;	// bitmap for emboss background
	HBITMAP hbmEmboss;	// bitmap for the embossing
	HBITMAP hOldBitmap;	// Save the device context state...
	CRect rect;			// client/region area rectangle
	int nRange;			// The range of the progress control
	BYTE rs, gs, bs;	// Starting red, green and blue values
	BYTE re, ge, be;	// Ending red, green and blue values
	float red, green, blue;	// Current colour components
	float rstep, gstep, bstep;	// Colour step values
	int xPos, yPos;		// Counters for drawing gradients quickly

	// Get the client area
	GetClientRect(rect);
	// Create the memory device contexts
	memDC.CreateCompatibleDC(&dc);
	// Create and select a bitmap to use for the device context
	hbmBack =  CreateCompatibleBitmap(dc.GetSafeHdc(), rect.Width(), rect.Height());
	if(!hbmBack)
		return;
	hOldBitmap = (HBITMAP)SelectObject(memDC.GetSafeHdc(), hbmBack);
	// Colour in the bitmap background
	memDC.FillSolidRect(rect, m_Background);
	SelectClipRgn(memDC.GetSafeHdc(), m_hRegion);

	// Fill the area
	nRange = m_nMax - m_nMin + 1;
	if((m_nFillStyle == SP_FILL_HORZGRAD) ||
		(m_nFillStyle == SP_FILL_VERTGRAD))
	{
		// Get the starting and ending colour components
		rs = GetRValue(m_Colour1); re = GetRValue(m_Colour2);
		gs = GetGValue(m_Colour1); ge = GetGValue(m_Colour2);
		bs = GetBValue(m_Colour1); be = GetBValue(m_Colour2);
	}
	switch(m_nFillStyle)
	{
	case SP_FILL_VERT:
		// Fill in the rectangle
		memDC.FillSolidRect(
		   rect.left, 
		   rect.top-1,
			rect.Width(), 
			GetDrawUpperLimit( rect ) + 1,
			m_Colour1
		);
		break;
	case SP_FILL_HORZ:
		// Fill in the rectangle
		memDC.FillSolidRect(
		   rect.left-1, 
		   rect.top,
			GetDrawUpperLimit( rect ) + 1, 
			rect.Height(),
			m_Colour1
		);
		break;
	case SP_FILL_VERTGRAD:
		// Get the initial colour values
		red = (float)rs;
		green = (float)gs;
		blue = (float)bs;
		// Get the colour step values
		rstep = (float)(re - rs) / rect.Height();
		gstep = (float)(ge - gs) / rect.Height();
		bstep = (float)(be - bs) / rect.Height();
		yPos = rect.top-1;
		while ( yPos+1 <= GetDrawUpperLimit( rect ) )
		{
			// Fill in the current rectangle
			memDC.FillSolidRect(rect.left, yPos, rect.Width(), 1,
				RGB((int)red, (int)green, (int)blue));
			// Get the next colour to use
			red += rstep; green += gstep; blue += bstep;
			// Get the next rectangle
			yPos++;
		}
		break;
	case SP_FILL_HORZGRAD:
		// Get the initial colour values
		red = (float)rs;
		green = (float)gs;
		blue = (float)bs;
		// Get the colour step values
		rstep = (float)(re - rs) / rect.Width();
		gstep = (float)(ge - gs) / rect.Width();
		bstep = (float)(be - bs) / rect.Width();
		xPos = rect.left-1;
		while ( xPos+1 <= GetDrawUpperLimit( rect ) )
		{
			// Fill in the current rectangle
			memDC.FillSolidRect(xPos, rect.top, 1, rect.Height(),
				RGB((int)red, (int)green, (int)blue));
			// Get the next colour to use
			red += rstep; green += gstep; blue += bstep;
			// Get the next rectangle
			xPos++;
		}
		break;
	}

	// Emboss the bitmap on the device context
	SelectObject(memDC.GetSafeHdc(), hOldBitmap);
	hbmEmboss = Emboss(m_hbmArea, hbmBack, NULL, FALSE, 0, 0,
					   GetSysColor(COLOR_3DHILIGHT),
					   GetSysColor(COLOR_3DSHADOW));
	SelectObject(memDC.GetSafeHdc(), hbmEmboss);

	// Copy to the screen
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
	// Cleanup
	SelectObject(memDC.GetSafeHdc(), hOldBitmap);
	DeleteObject(hbmBack);
	DeleteObject(hbmEmboss);
}


int CSuperProgressCtrl::GetDrawUpperLimit( CRect& rectClient, int nPos )
{
	int nRange = m_nMax - m_nMin + 1;
	
   switch ( m_nFillStyle )
   {
	   case SP_FILL_VERT:
	   case SP_FILL_VERTGRAD:
		   return (nPos-m_nMin)*(rectClient.Height()+2)/nRange;
	   case SP_FILL_HORZ:
   	case SP_FILL_HORZGRAD:
   		return (nPos-m_nMin)*(rectClient.Width() +2)/nRange;
      default:
         ASSERT( false );
         return 0;
   }
   
}


int CSuperProgressCtrl::GetDrawUpperLimit( CRect& rectClient )
{
   return GetDrawUpperLimit( rectClient, m_nPosition );
}


bool CSuperProgressCtrl::bBitmapRedrawNeeded( int nOldPos )
{
   CRect rect;
   GetClientRect( rect );
   return ( GetDrawUpperLimit( rect ) != GetDrawUpperLimit( rect, nOldPos ) );
}
