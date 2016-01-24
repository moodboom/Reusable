//-------------------------------------------------------------------//
// SubstringSearcher.h																//
//
//	This module contains a class to assist with searching
// a large number of strings for a pattern.
//
// It uses a Self-Tuning Boyer-Moore string search.
// This version was developed after studying an implementation
// by behoffski (Brenton Hoff) for "Grouse".  This author coined 
// the name and was the original STBM author, to the best of my 
// knowledge.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef SUBSTRING_SEARCHER_H
	#define SUBSTRING_SEARCHER_H

#include "ustring.h"				// For StandardTypes and ustring


#define STBM_ALPHABET_SIZE              (UCHAR_MAX + 1)
typedef struct STBM_SearchSpec_Struct {
        /*Lookup table for "fast" loop*/
        UINT SkipTable[STBM_ALPHABET_SIZE];
        UINT md2;

        /*Original pattern, needed to complete match*/
        UINT PatLen;
        BYTE *pPattern;

        // LWORD Flags;
        Long Flags;

        /*Skip table for where guard fails to match*/
        /* ?? not sure if it's worth doing this*/

} STBM_SearchSpec;

// This class allows you to do fast searches for a substring
// in a set of source strings.
class SubstringSearcher
{
public:

	SubstringSearcher(
		ustring	strSubstring,
		int		nMaxSourceStringLength	= 255
	);

	virtual ~SubstringSearcher();
	
	bool bContainsSubstring( ustring& strSource );

protected:
	ustring	m_strSubstring;
	TCHAR*	m_szSubstringBuffer;
	int		m_nSubstringLength;
	int		m_nMaxSourceStringLength;

	/*Streamlined lowercase-folding table*/
	static BYTE CaseFold[256];

	STBM_SearchSpec Spec;

};


#endif		// SUBSTRING_SEARCHER_H

