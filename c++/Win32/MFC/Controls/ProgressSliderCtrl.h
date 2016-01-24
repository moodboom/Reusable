#if !defined(PROGRESS_SLIDER_CTRL_H)
#define PROGRESS_SLIDER_CTRL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressSliderCtrl.h : header file
//
//	ProgressSliderCtrl class, version 1.0
//
//	Copyright (c) 1999 Paul M. Meidinger (pmmeidinger@yahoo.com)
//
// Feel free to modifiy and/or distribute this file, but
// do not remove this header.
//
// I would appreciate a notification of any bugs discovered or 
// improvements that could be made.
//
// This file is provided "as is" with no expressed or implied warranty.
//
//	History:
//		PMM	12/21/1999		Initial implementation.		

/////////////////////////////////////////////////////////////////////////////
// ProgressSliderCtrl window

class ProgressSliderCtrl : public CSliderCtrl
{
// Construction
public:
	ProgressSliderCtrl();

// Attributes
public:

// Operations
public:
	void SetChannelColor(COLORREF crColor);
	COLORREF GetChannelColor();
	void SetThumbColor(COLORREF crColor);
	COLORREF GetThumbColor();
	void SetSelectionColor(COLORREF crColor);
	COLORREF GetSelectionColor();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ProgressSliderCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ProgressSliderCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(ProgressSliderCtrl)
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	
	// MDM	3/9/2000 6:38:21 PM
	COLORREF LightenColor(const COLORREF crColor, UINT nIncreaseVal);
	COLORREF DarkenColor(const COLORREF crColor, UINT nReduceVal);
	/*
	COLORREF LightenColor(const COLORREF crColor, BYTE byIncreaseVal);
	COLORREF DarkenColor(const COLORREF crColor, BYTE byReduceVal);
	*/

	void DrawHorizontalThumb(CDC* pDC, const CRect &rect);
	void DrawVerticalThumb(CDC* pDC, const CRect &rect);
	void DrawChannel(CDC* pDC, const CRect &rect, BOOL bHoriz);
	void DeletePens();
	void CreatePens();
	CPen m_penThumb;
	CPen m_penThumbLight;
	CPen m_penThumbLighter;
	CPen m_penThumbDark;
	CPen m_penThumbDarker;
	CPen m_penDkShadow;
	CPen m_penShadow;
	CPen m_penHilight;
	void GetColors();
	COLORREF m_crSelection;
	COLORREF m_crChannel;
	COLORREF m_crThumb;
	COLORREF m_crThumbLight;
	COLORREF m_crThumbLighter;
	COLORREF m_crThumbDark;
	COLORREF m_crThumbDarker;
	COLORREF m_crDkShadow;
	COLORREF m_crShadow;
	COLORREF m_crHilight;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(PROGRESS_SLIDER_CTRL_H)
