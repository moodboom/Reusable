#ifndef _BACKGROUNDUTIL_H_
#define _BACKGROUNDUTIL_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////
// Globals Statics Constants												//
/////////////////////////////////////////////////////////////////
//

typedef enum 
{
	BDBS_TILE,
	BDBS_CENTER,

	BDBS_COUNT,
} BD_BACKGROUND_STYLE;

//
/////////////////////////////////////////////////////////////////


class CBackgroundUtil  
{
public:
	BOOL SetBitmap(UINT uResourceID = 0);
	CBackgroundUtil();
	virtual ~CBackgroundUtil();

	void SetOffset( int nX = 0, int nY = 0 )
	{
		m_nXOffset = nX; m_nYOffset = nY;
	}

	void SetBackgroundBmpStyle( BD_BACKGROUND_STYLE bkbs = BDBS_TILE )
	{
		m_BkgndStyle = bkbs;
	}

protected:
	BOOL TileBitmap(CDC* pDC, CRect rc);
	BOOL GetBitmapAndPalette(UINT nIDResource, CBitmap& bitmap, CPalette& pal);
	// For background bitmap
	CBitmap m_BmpPattern;
	CPalette m_BmpPalette;
	int m_nBmpWidth;
	int m_nBmpHeight;

	int m_nXOffset;
	int m_nYOffset;
	BD_BACKGROUND_STYLE m_BkgndStyle;

};

#endif
