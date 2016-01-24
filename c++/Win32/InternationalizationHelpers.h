#ifndef INTERNATIONALIZATION_HELPERS_H
#define INTERNATIONALIZATION_HELPERS_H


// Get the current Windows keyboard input locale,
// in LCID format.
LCID GetCurrentInputLanguage()
{
	// wLang includes language and "sub-language".
	HKL hklCurrent = GetKeyboardLayout( 0 );
	WORD wLang = LOWORD( hklCurrent );
	return MAKELCID(
		wLang,
		SORT_DEFAULT
	);
}



#endif	// INTERNATIONALIZATION_HELPERS_H