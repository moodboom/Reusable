//-------------------------------------------------------------------//
// SubstringSearcher																	//
//
//	This module contains a class to assist with searching
// a large number of strings for a pattern.  You provide
// the pattern up front, and we optimize for subsequent
// pattern matching.
//
// It uses a Self-Tuning Boyer-Moore string search.
// This version was developed after studying an implementation
// by behoffski (Brenton Hoff) for "Grouse".  This author coined 
// the name and was the original STBM author, to the best of my 
// knowledge.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "SubstringSearcher.h"


// SubstringSearcher Globals/Statics/Consts									//
BYTE SubstringSearcher::CaseFold[];

/*Macro to provide very-low-cost case folding to lower case*/
#define STBM_TOLOWER(c)  (CaseFold[c])



//-------------------------------------------------------------------//
// SubstringSearcher()																//
//-------------------------------------------------------------------//
// Constructor to set up for subsequent substring searches - say that
// 10 times fast.
//-------------------------------------------------------------------//
SubstringSearcher::SubstringSearcher( 
	ustring	strSubstring,
	int		nMaxSourceStringLength
) :
	// Init vars.
	m_strSubstring				(	strSubstring				),
	m_nMaxSourceStringLength(	nMaxSourceStringLength	)
{
	// Set up the length of the substring.
	m_nSubstringLength = strSubstring.size();

	ASSERT( m_nSubstringLength > 0 );
	if ( m_nSubstringLength <= 0 )
		return;
	
	// Set up the substring buffer.
	// This contains the substring, AS WELL AS ENOUGH ROOM FOR THE SEARCH ALGORITHM!!
	// See the algorithm documentation for details.
	// NOTE: we cast away the const, as we need to modify the buffer.
	// No worries there, we made a copy of the originally supplied string.
	// TO DO
	// I cranked it up to be safe 'cause I didn't feel like determining the need better.
	// Reduce this size later, if ya feel up to it, punk.
	// m_szSubstringBuffer = m_strSubstring.GetBufferSetLength( m_nMaxSourceStringLength * 2 + 1 );
	m_strSubstring.reserve( m_nMaxSourceStringLength * 2 + 1 );
	m_szSubstringBuffer = const_cast<TCHAR*>( m_strSubstring.c_str() );

	// Set up a table of lowercase versions of chars on first instance.
	if ( CaseFold[1] != tolower((BYTE) 1 ) )
		for ( int nA = 0; nA < 256; nA++)
			CaseFold[(BYTE) nA] = tolower((BYTE) nA);

			// Set up the Spec.
			Spec.pPattern = (BYTE *) m_szSubstringBuffer;
			Spec.PatLen   = m_nSubstringLength;
			
			// Not used.
			// Spec.Flags    = Flags;
			
        BYTE *pPat;
        BYTE *pPatEnd;
        UINT i;

        pPat = (BYTE *) m_szSubstringBuffer;
        pPatEnd = pPat + m_nSubstringLength - 1;
        
		  // for (i = 0; i < DIM(pSpec->SkipTable); i++) {
        for (i = 0; i < STBM_ALPHABET_SIZE; i++) {

					 // pSpec->SkipTable[i] = m_nSubstringLength;
					 Spec.SkipTable[i] = m_nSubstringLength;
        }

        // if ((Flags & STBM_SEARCH_CASE_INSENSITIVE) == 0) {
        if (false) {

                /*Exact match required (case sensitive)*/
                for (; pPat < pPatEnd; pPat++) {
                        // pSpec->SkipTable[*pPat] = pPatEnd - pPat;
                        Spec.SkipTable[*pPat] = pPatEnd - pPat;								
                }

                // pSpec->md2 = pSpec->SkipTable[*pPat]; 
                // pSpec->SkipTable[*pPat] = 0;

                Spec.md2 = Spec.SkipTable[*pPat]; 
                Spec.SkipTable[*pPat] = 0;
        
		  } else {
                /*Case-insensitive search -- set up table accordingly*/
                for (; pPat < pPatEnd; pPat++) {
                        /*Force supplied pattern to have consistent case*/
                        *pPat = tolower(*pPat);

                        // pSpec->SkipTable[*pPat]          = pPatEnd - pPat;
                        // pSpec->SkipTable[toupper(*pPat)] = pPatEnd - pPat;
                        Spec.SkipTable[*pPat]          = pPatEnd - pPat;
                        Spec.SkipTable[toupper(*pPat)] = pPatEnd - pPat;

                }

                *pPat = tolower(*pPat);
                // pSpec->md2 = pSpec->SkipTable[*pPat]; 
                // pSpec->SkipTable[*pPat]          = 0;
                // pSpec->SkipTable[toupper(*pPat)] = 0;
                Spec.md2 = Spec.SkipTable[*pPat]; 
                Spec.SkipTable[*pPat]          = 0;
                Spec.SkipTable[toupper(*pPat)] = 0;
                
        }

        /*Prepared for search successfully*/
        // *ppSpec = pSpec;

}


//-------------------------------------------------------------------//
// ~SubstringSearcher()																//
//-------------------------------------------------------------------//
SubstringSearcher::~SubstringSearcher()
{
	// Let my people go!
	// m_strSubstring.ReleaseBuffer( m_nMaxSourceStringLength * 2 + 1 );
}


//-------------------------------------------------------------------//
// bContainsSubstring()																//
//-------------------------------------------------------------------//
// This function uses a "Tuned Boyer-Moore" generic search algorithm.
// 
// I believe we have a pretty fast algorithm here, but I have not
// actually compared it to others.  Here are what seem like the best:
//	
//		Tuned Boyer-Moore (TBM)						The classic benchmark
//		"FastGenericSearchAlgorithm" (HAL)		better worst-case	(who cares)
//		"Self-Tuning Boyer-Moore" (STBM)			potentially faster!!  used by grep
//
// See Software Development\Documentation\SubstringSearching\
// for details.
//-------------------------------------------------------------------//
bool SubstringSearcher::bContainsSubstring(
	ustring& strSource
) {
	
	STBM_SearchSpec *pSpec = &Spec;
	UINT BufferLength = strSource.size();

	// TO DO
	// Make sure we don't need to release this puppy.
	// Since we won't be changing its length we should be okay...
	TCHAR* pBuffer = const_cast<TCHAR*>( strSource.c_str() );

        UINT *pSkip;
        BYTE *pText;
        UINT Skip;
        BYTE *pEnd;
        INT GuardOffset;
        BYTE GuardChar;
        BYTE *pPattern = pSpec->pPattern;
        BYTE *pPatEnd;

        INT i;
        INT NegLen;

        /*Prepare for search*/
        pEnd = (BYTE *) (pBuffer + BufferLength);
        pSkip = &pSpec->SkipTable[0];
        pText = (BYTE *) pBuffer;
        pPatEnd = pPattern + pSpec->PatLen - 1;
        
		  // Getting a warning, is this working correctly?
		  // NegLen = -pSpec->PatLen;
		  NegLen = -(INT)pSpec->PatLen;

        GuardOffset = -1;
        if (pSpec->PatLen == 1) {
                GuardOffset = 0;
        }

        GuardChar = STBM_TOLOWER(pPatEnd[GuardOffset]);
        goto SkipLoop;

CheckGuard:
        // if (pText >= pEnd) return NULL;
        if (pText >= pEnd) return false;

        if (STBM_TOLOWER(pText[GuardOffset]) == GuardChar) goto TryMatch;
        pText += pSpec->md2;
        /* FALLTHROUGH */

        /*Main search is in "fast" skip loop*/
SkipLoop:
        for (;;) {

                /*Note: When (re-)entering skip loop, checks more often*/
                Skip = pSkip[*pText];
                if (Skip == 0) goto CheckGuard;
                pText += Skip;

                pText += pSkip[*pText];
                pText += pSkip[*pText];
                Skip = pSkip[*pText];
                if (Skip == 0) goto CheckGuard;
                pText += Skip;

                pText += pSkip[*pText];
                pText += pSkip[*pText];

                /*May need end-of-buffer check here in some applications*/
                // MDM	6/21/01 7:19:52 PM
					 // Well I know *I* didn't pad the source string...
					 // We COULD pad...not so sure that would help speed much...
					 // allocating additional buffer, copying, zeroing...
                if (pText >= pEnd) return NULL;

        }

TryMatch:

        /*Check pattern from (Guard-1)..0*/
        for (i = GuardOffset - 1; i > NegLen; i--) {
                if (pPatEnd[i] != STBM_TOLOWER(pText[i])) goto Mismatch;
        }

        /*Now check pattern from (PatLen - 1) to (Guard + 1)*/
        for (i = -1; i > GuardOffset; i--) {
                if (pPatEnd[i] != STBM_TOLOWER(pText[i])) goto Mismatch;
        }

        /*?? Stash current guard position to use for next search*/

        /*Match found: return pointer to start of text in buffer*/
		  // Nah we just want true;
		  // return pText + NegLen + 1;
			return true;

Mismatch:
        /*Use last mismatch as new guard character*/
        GuardChar = STBM_TOLOWER(pPatEnd[i]);
        GuardOffset = i;

        pText += pSpec->md2;
        goto SkipLoop;
	
	/*

	int m = strSource.GetLength();

	const TCHAR*	x	= LPCTSTR( strSource		);
	TCHAR*			y	= m_szSubstringBuffer;

	int i, j, k, shift;
	
	// Fix this some time...we need a dynamic creation.
	// int bc[ m_nMaxSourceStringLength ];
	// int bc[ 255 ];
	int* bc = new int[ m_nMaxSourceStringLength ];

	// Preprocessing
	
	// TO DO
	// What the hell is going on here?  This HAS to be moved to preprocessing
	// code in constructor.  But it uses m!!!
	// TO DO
	// memcpy should be faster, but we need a Unicode-capable version.
	// memcpy( 
	for (i = 0; i < m_nMaxSourceStringLength; ++i) 
		bc[i] = m;

	for (i = 0; i < m - 1; ++i) 
		bc[x[i]] = m - i - 1;

	shift = bc[x[m - 1]];
	bc[x[m - 1]] = 0;

	for (i = m_nSubstringLength; i < m_nSubstringLength + m; ++i) 
		y[i] = x[m - 1];

	// DEBUG
	y[i] = _T('\0');
	ustring strM;
	// for (i = 0; i < m_nMaxSourceStringLength; ++i) 
	for (i = 0; i < 100; ++i) 
	{
		strM.AppendNumber( bc[i] );
		strM += _T(",");
	}
	TRACE( _T("X=%s\nY=%s\nBC=%s\n\n"), x, y, strM.c_str() );
	ustring strMsg;
	strMsg  = (LPTSTR)x;
	strMsg += y;
	strMsg += strM;
	DisplayMessage( strMsg.c_str() );

	// Searching
	j = m - 1;
	while ( j < m_nSubstringLength ) 
	{
		k = bc[ y[j] ];
		while (k !=  0) 
		{
			j += k; k = bc[y[j]];
			j += k; k = bc[y[j]];
			j += k; k = bc[y[j]];
		}
		i = 0;
		while (i < m - 1 && x[i] == y[j - m + 1 + i] ) 
			++i;
		if (j < m_nSubstringLength && i >= m - 1) 
		{
			// We found a match!
			// That's all we care about in this version, return immediately.
			// OUTPUT(j - m + 1);
			delete bc;
			return true;
		}

		j += shift;                                  // shift
	}

	delete bc;
	return false;
	*/

}