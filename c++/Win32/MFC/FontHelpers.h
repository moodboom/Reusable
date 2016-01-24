//-------------------------------------------------------------------//
// FontHelpers.h   : MFC font helper functions
//
//	Copyright © 2002 A better Software.
//-------------------------------------------------------------------//

#ifndef FONT_HELPERS_H
	#define FONT_HELPERS_H

class SimpleFont
{
public:
	SimpleFont(
		CString	strFace		= _T(""),
		int		nSize			= 0,
		int		nWeight		= 0,
		COLORREF	crColor		= RGB( 0, 0, 0 ),
		bool		bItalic		= false,
		bool		bUnderline	= false,
		bool		bStrikeout	= false
	) :
		// Init vars.
		m_strFace	( strFace		),
		m_nSize		( nSize			),
		m_nWeight	( nWeight		),
		m_crColor	( crColor		),
		m_bItalic	( bItalic		),
		m_bUnderline( bUnderline	),
		m_bStrikeout( bStrikeout	)
	{}

	bool SimpleFont::operator==( const SimpleFont& sfSource )
	{
		return
		(
				m_strFace	 == sfSource.m_strFace	 
			&& m_nSize		 == sfSource.m_nSize		 
			&& m_nWeight	 == sfSource.m_nWeight	 
			&& m_crColor	 == sfSource.m_crColor	 
			&& m_bItalic	 == sfSource.m_bItalic	 
			&& m_bUnderline == sfSource.m_bUnderline 
			&& m_bStrikeout == sfSource.m_bStrikeout 
		);
	}

	bool SimpleFont::operator!=( const SimpleFont& sfSource )
	{ return !( *this == sfSource ); }
	
	CString	m_strFace	;
	int		m_nSize		;
	int		m_nWeight	;
	COLORREF	m_crColor	;
	bool		m_bItalic	;
	bool		m_bUnderline;
	bool		m_bStrikeout;
};


// This gets a LOGFONT based on a SimpleFont.
// It is typically used by other functions, not
// called directly.
void GetLOGFONT(
	const SimpleFont& sfSource,
	LOGFONT&				lfDest,
	CDC*					pDC			= 0
);


// This puts up the common font-selection dialog
// and simplifies the user's selection into a SimpleFont.
bool GetFontFromUser(
	SimpleFont& sf,
	CDC*			pDC	= 0
);


// This gets the equivalent CFont from a SimpleFont.
// After calling GetFontFromUser(), use this to get
// a CFont you can assign to CWnd's, etc.
void GetCFont(
	const SimpleFont& sfSource,
	CFont&				cfDestination,
	CDC*					pDC			= 0
);


// Can't do this with a CFont, no copy constructor,
// how convenient.  This is untested.
inline HFONT GetHFONT(
	const SimpleFont& sfSource,
	CDC*					pDC			= 0
) {
	CFont cfDest;
	GetCFont(
		sfSource,
		cfDest,
		pDC
	);
	return (HFONT)cfDest.Detach();
}

#endif	// FONT_HELPERS_H
