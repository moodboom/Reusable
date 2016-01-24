#ifndef _MEMDC_H_
#define _MEMDC_H_

//////////////////////////////////////////////////
// CMemDC - memory DC
//
// Author: Keith Rule
// Email:  keithr@europa.com
// Copyright 1996-1999, Keith Rule
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
// History - 10/3/97 Fixed scrolling bug.
//                   Added print support. - KR
//
//           11/3/99 Fixed most common complaint. Added
//                   background color fill. - KR
//
//           11/3/99 Added support for mapping modes other than
//                   MM_TEXT as suggested by Lee Sang Hun. - KR
//
// MDM	2/8/2001 11:28:19 AM
//			Incorporated changes required by ProgressCtrlX:
//				BOOL fCopyOld			= FALSE
//				BOOL fUsePalette		= FALSE
//
// This class implements a memory Device Context which allows
// flicker free drawing.

class CMemDC : public CDC {
private:	
	CBitmap	m_bitmap;		// Offscreen bitmap
	CBitmap*	m_oldBitmap;	// bitmap originally found in CMemDC
	CDC*		m_pDC;			// Saves CDC passed in constructor
	CRect		m_rect;			// Rectangle of drawing area.
	BOOL		m_bMemDC;		// TRUE if CDC really is a Memory DC.
	BOOL     m_fUsePalette; // if necessary create and select palette
public:
	
	CMemDC(
		CDC* pDC, 
		const CRect* pRect	= NULL,
		BOOL fCopyOld			= FALSE, 
		BOOL fUsePalette		= FALSE
	) : 
		// Call base class.
		CDC()
	{
		ASSERT(pDC != NULL); 

		// Some initialization
		m_pDC = pDC;
		m_oldBitmap = NULL;
		m_bMemDC = !pDC->IsPrinting();
		m_fUsePalette = fUsePalette;

		// Get the rectangle to draw
		if (pRect == NULL) {
			pDC->GetClipBox(&m_rect);
		} else {
			m_rect = *pRect;
		}
		
		if (m_bMemDC) {
			// Create a Memory DC
			CreateCompatibleDC(pDC);
			pDC->LPtoDP(&m_rect);

			m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
			m_oldBitmap = SelectObject(&m_bitmap);
			
			SetMapMode(pDC->GetMapMode());
			pDC->DPtoLP(&m_rect);
			SetWindowOrg(m_rect.left, m_rect.top);

			// MDM	2/8/2001 11:34:40 AM
			// Added.
			if(fCopyOld)
				BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
				       m_pDC, m_rect.left, m_rect.top, SRCCOPY);

		} else {
			// Make a copy of the relevent parts of the current DC for printing
			m_bPrinting = pDC->m_bPrinting;
			m_hDC       = pDC->m_hDC;
			m_hAttribDC = pDC->m_hAttribDC;
		}

		// Fill background 
		FillSolidRect(m_rect, pDC->GetBkColor());
	}

	
	~CMemDC()	
	{		
		if (m_bMemDC) 
		{
			// MDM	2/8/2001 11:35:35 AM
			// Added.
			CPalette pal, *pOldPal=NULL;
			if(m_fUsePalette && CreatePalette(pal))
			{
				pOldPal = m_pDC->SelectPalette(&pal, FALSE);
				m_pDC->RealizePalette();
			}

			// Copy the offscreen bitmap onto the screen.
			m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
				this, m_rect.left, m_rect.top, SRCCOPY);			
			
			// MDM	2/8/2001 11:35:35 AM
			// Added.
			if(pOldPal)
				m_pDC->SelectPalette(pOldPal, TRUE);

			//Swap back the original bitmap.
			SelectObject(m_oldBitmap);		

		} else {
			// All we need to do is replace the DC with an illegal value,
			// this keeps us from accidently deleting the handles associated with
			// the CDC that was passed to the constructor.			
			m_hDC = m_hAttribDC = NULL;
		}	
	}
	
	// MDM	2/8/2001 11:35:35 AM
	// Added.
	BOOL CreatePalette(CPalette& pal, CDC* pDC = NULL)
	{
		if(!pDC) pDC = this;
		if(!(pDC->GetDeviceCaps(RASTERCAPS) & RC_PALETTE))
			return FALSE;

		//Get the DIBSection's color table
		const int nColors = 236;
		RGBQUAD rgb[nColors];
		::GetDIBColorTable(pDC->m_hDC, 0, nColors, rgb);

		//Create a palette from the color table
		LPLOGPALETTE lpPal = (LPLOGPALETTE) new BYTE[sizeof(LOGPALETTE) + (nColors*sizeof(PALETTEENTRY))];
		lpPal->palVersion = 0x300;       
		lpPal->palNumEntries = nColors;

		for (int i = 0; i < nColors; i++)       
		{
			lpPal->palPalEntry[i].peRed = rgb[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = rgb[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue = rgb[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}
		VERIFY(pal.CreatePalette(lpPal));
  
		//Clean up
		delete lpPal;
		return TRUE;
	}

	// Allow usage as a pointer	
	CMemDC* operator->() 
	{
		return this;
	}	

	// Allow usage as a pointer	
	operator CMemDC*() 
	{
		return this;
	}
};

#endif