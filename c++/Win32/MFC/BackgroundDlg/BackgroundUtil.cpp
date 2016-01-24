#include "stdafx.h"
#include "BackgroundUtil.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBackgroundUtil::CBackgroundUtil()
{
	// Set defaults.
	SetOffset();
	SetBackgroundBmpStyle();
}


CBackgroundUtil::~CBackgroundUtil()
{
	m_BmpPattern.Detach();
	m_BmpPalette.Detach();
}


BOOL CBackgroundUtil::SetBitmap(UINT uResourceID)
{
	BITMAP bm;
	BOOL bRet;

	// Detach previous resources
	m_BmpPattern.Detach();  
	m_BmpPalette.Detach();  

	// Default return value
	bRet = TRUE;

	// Load new bitmap
	if (uResourceID != 0)
	{
		bRet = GetBitmapAndPalette(uResourceID, m_BmpPattern, m_BmpPalette);
		// If all ok
		if (bRet == TRUE)
		{
			// Get dimension
			m_BmpPattern.GetBitmap(&bm);
			// Width of the bitmap
			m_nBmpWidth = bm.bmWidth;
			// Height of the bitmap
			m_nBmpHeight = bm.bmHeight;
		}
	}

	return bRet;
} // End of SetBitmap


BOOL CBackgroundUtil::GetBitmapAndPalette(UINT nIDResource, CBitmap & bitmap, CPalette & pal)
{
	LPCTSTR lpszResourceName = (LPCTSTR)nIDResource;

	HBITMAP hBmp = (HBITMAP)::LoadImage( AfxGetInstanceHandle(), 
		lpszResourceName, IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION);

	if (hBmp == NULL) return FALSE;

	bitmap.Attach(hBmp);

	// Create a logical palette for the bitmap
	DIBSECTION ds;
	BITMAPINFOHEADER &bmInfo = ds.dsBmih;
	bitmap.GetObject(sizeof(ds), &ds);

	int nColors = bmInfo.biClrUsed ? bmInfo.biClrUsed : 1 << bmInfo.biBitCount;

	// Create a halftone palette if colors > 256. 
	CClientDC dc(NULL); // Desktop DC

	if(nColors > 256)
		pal.CreateHalftonePalette(&dc);
	else
	{
		// Create the palette
		RGBQUAD *pRGB = new RGBQUAD[nColors];
		CDC memDC;
		memDC.CreateCompatibleDC(&dc);
		memDC.SelectObject( &bitmap );
		::GetDIBColorTable( memDC, 0, nColors, pRGB );
		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
		LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];
		pLP->palVersion = 0x300;
		pLP->palNumEntries = nColors;
		for (int i=0; i < nColors; i++)
		{
			pLP->palPalEntry[i].peRed = pRGB[i].rgbRed;
			pLP->palPalEntry[i].peGreen = pRGB[i].rgbGreen;
			pLP->palPalEntry[i].peBlue = pRGB[i].rgbBlue;
			pLP->palPalEntry[i].peFlags = 0;
		}
		pal.CreatePalette( pLP );
		delete[] pLP;
		delete[] pRGB;
	}
	return TRUE;
} // End of GetBitmapAndPalette


BOOL CBackgroundUtil::TileBitmap(
	CDC* pDC, 
	CRect rc
) {
	CDC MemDC;

	// If there is a bitmap loaded
	if (m_BmpPattern.m_hObject != NULL)
	{
		MemDC.CreateCompatibleDC(pDC);
		CBitmap* pOldBitmap = MemDC.SelectObject(&m_BmpPattern);

		// Clip the tiling to the requested rect.
		// Convert from screen coords to logical coords...
		CRgn rgn;
		CRect rectRgn = rc;
		pDC->DPtoLP( &rectRgn );
		rgn.CreateRectRgn(
			rectRgn.TopLeft().x, 
			rectRgn.TopLeft().y, 
			rectRgn.BottomRight().x, 
			rectRgn.BottomRight().y
		); 
		pDC->SelectClipRgn( &rgn ); 

		if ( m_BkgndStyle == BDBS_TILE )
		{
			// Tile the bitmap.
			// Note that it has to be offset as needed, but
			// we want to make sure we still tile the whole client area.

			// actual xoffset = -xoff + (xoff/xbmp)*xbmp
			int x = - m_nXOffset + ( m_nXOffset / m_nBmpWidth  ) * m_nBmpWidth ;
			int y = - m_nYOffset + ( m_nYOffset / m_nBmpHeight ) * m_nBmpHeight;

			while ( y < rc.Height() ) 
			{
				while( x < rc.Width() ) 
				{
					pDC->BitBlt(x, y, m_nBmpWidth, m_nBmpHeight, &MemDC, 0, 0, SRCCOPY);
					x += m_nBmpWidth;
				}
				x = - m_nXOffset + ( m_nXOffset / m_nBmpWidth  ) * m_nBmpWidth ;
				y += m_nBmpHeight;
			}
		
		} else if ( m_BkgndStyle == BDBS_CENTER )
		{
			pDC->BitBlt(
				m_nXOffset + ( rc.Width()  - m_nBmpWidth  ) / 2, 
				m_nYOffset + ( rc.Height() - m_nBmpHeight ) / 2, 
				m_nBmpWidth, 
				m_nBmpHeight, 
				&MemDC, 
				0, 
				0, 
				SRCCOPY
			);		
		}

		MemDC.SelectObject(pOldBitmap);

		pDC->SelectClipRgn( NULL ); 

		return TRUE;
	}

	return FALSE; // Normal behaviour
} // End of TileBitmap


/*
// These variables are required for clipping. 
 
static POINT ptUpperLeft; 
static POINT ptLowerRight; 
static POINT aptRect[5]; 
static POINT ptTmp; 
static POINTS ptsTmp; 
static BOOL fDefineRegion; 
static BOOL fRegionExists; 
static HRGN hrgn; 
static RECT rctTmp; 
int i; 
 
case WM_COMMAND: 
    switch (wParam) 
    { 
 
    case IDM_CLIP: 
 
    hdc = GetDC(hwnd); 
 
    // Retrieve the application's client rectangle and paint 
    // with the default (white) brush. 
 
    GetClientRect(hwnd, &rctTmp); 
    FillRect(hdc, &rctTmp, GetStockObject(WHITE_BRUSH)); 
 
    // Use the rect coordinates to define a clipping region. 
 
static HRGN hrgn; 
    hrgn = CreateRectRgn(aptRect[0].x, aptRect[0].y, 
        aptRect[2].x, aptRect[2].y); 
    SelectClipRgn(hdc, hrgn); 
 
    // Transfer (draw) the bitmap into the clipped rectangle. 
 
    BitBlt(hdc, 
       0, 0, 
       bmp.bmWidth, bmp.bmHeight, 
       hdcCompatible, 
       0, 0, 
       SRCCOPY); 
 
    ReleaseDC(hwnd, hdc); 
    break; 
    }
*/