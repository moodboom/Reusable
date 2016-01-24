#include "..\CJLibrary\GfxSplitterWnd.h"


// DynSplitterWnd.h file
//
// class CDynSplitterWnd : public CSplitterWnd
class CDynSplitterWnd : public CGfxSplitterWnd
{
	// Construction
	public:
		CDynSplitterWnd();

		// Attributes
	public:
		BOOL	IsDynamic()

		{ return m_bDynSplit; }
		void	SetDynamic(BOOL bDynSplit = TRUE)
		{ m_bDynSplit = bDynSplit; }

		// Operations
	public:

		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CDynSplitterWnd)
		virtual void OnInvertTracker(const CRect& rect);
		virtual void StartTracking(int ht);
		virtual void StopTracking(BOOL bAccept);
		//}}AFX_VIRTUAL

		// Implementation
	public:
		virtual ~CDynSplitterWnd();

	protected:
		CPoint  m_OldPoint;
		BOOL	m_bDynSplit;

		enum HitTestValue
		{
			noHit                   = 0,
			vSplitterBox            = 1,
			hSplitterBox            = 2,
			bothSplitterBox         = 3,        // just for keyboard
			vSplitterBar1           = 101,
			vSplitterBar15          = 115,
			hSplitterBar1           = 201,
			hSplitterBar15          = 215,
			splitterIntersection1   = 301,
			splitterIntersection225 = 525
		};


		// Generated message map functions
	protected:
		//{{AFX_MSG(CDynSplitterWnd)
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};

