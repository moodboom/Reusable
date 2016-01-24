// VolumeSliderCtrl.h : Header file
//
// Copyright (c) 1999 Daniel Frey
// See VolumeSliderCtrl.cpp for details.

#if !defined(VOLUME_SLIDER_CTRL_H)
#define VOLUME_SLIDER_CTRL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class VolumeSliderCtrl : public CSliderCtrl
{
protected:
	CPoint	m_ptCenter;
	int		m_nRadius;
	CRgn		m_rgn;

	int		m_nZero;
	bool		m_bInverted;
	int		m_nDeadDegrees;	// Amount of dead space CCW of zero.

	CString	m_strText;
	int		m_nKnobRadius;

	bool		m_bDragging;
	bool		m_bDragChanged;

	CPoint	m_ptLast;

public:
	DECLARE_DYNAMIC( VolumeSliderCtrl )

	VolumeSliderCtrl();
	virtual ~VolumeSliderCtrl();

	// You may use '%ld' to show the slider value. Default: "%ld°"
	virtual void SetText(const CString& strNewText);
	virtual CString GetText() const;

	// Override this function for customized output
	virtual CString OnFormatText();

	// Default: 7, Minimum: 5
	virtual void SetKnobRadius( const int nNewKnobRadius );
	virtual int GetKnobRadius() const;

	// 0-359, 0 = Top, 90 = Right, 180 = Bottom, 270 = Left
	virtual void SetZero(const int nZero);
	virtual int GetZero() const;

	virtual void SetInverted(const bool bNewInverted = true);
	virtual bool GetInverted();
	
	//{{AFX_VIRTUAL(VolumeSliderCtrl)
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:
	virtual void Init();

	// Set a default custom cursor.
	HCURSOR  m_hCursor;
	virtual void SetDefaultCursor();

	virtual bool SetKnob(const CPoint& pt);
	virtual void PostMessageToParent(const int nTBCode) const;

	// This determines if we are crossing from 0 to 360 or
	// vice versa, so we can prevent it.
	void PreventCrossingZero( CPoint& point );

	//{{AFX_MSG(VolumeSliderCtrl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif // _DEBUG
};

//{{AFX_INSERT_LOCATION}}

#endif // VOLUME_SLIDER_CTRL_H
