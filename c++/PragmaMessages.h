#ifndef PRAGMA_MESSAGES_H
#define PRAGMA_MESSAGES_H

//-------------------------------------------------------------------//
// These defines will place nicely formatted messages in the compile 
// output window.
//
// I could never get the silly "#" to be output by a macro, believe 
// it or not, so you still have to type "#pragma " yourself before 
// each of these macros.  :<  E.g.:
//
//		#pragma _TODO("This is a ToDo statement")
//		#pragma _TOTHINK("This is a ToThink statement")
//		#pragma _TODOCUMENT("This is a ToDocument statement")
//		#pragma _DEBUGCODE("Make sure you remove this before shipping")
//
// The Visual Studio 6 output window will let you click on these
// to take you to the correct line - nice!
//-------------------------------------------------------------------//
#define __MDM_STR(x) #x

// MDM Why did we need this?  I'm taking it out, as
// it's colliding with another #define.
// Nope, doesn't work without it.
#define _MDM_STR(x) __MDM_STR(x)

#define _TODO(_message) message ("                          «TODO»       " ##_message "\t\n                                       " __FILE__ "(" _MDM_STR(__LINE__) ") : \t" )	

#define _TOTHINK(_message) message ("                          «TOTHINK»    " ##_message "\t\n                                       " __FILE__ "(" _MDM_STR(__LINE__) ") : \t" )	
	
#define _TODOCUMENT(_message) message ("                          «TODOCUMENT» " ##_message "\t\n                                       " __FILE__ "(" _MDM_STR(__LINE__) ") : \t" )	

#define _DEBUGCODE(_message) message ("                          «DEBUG CODE (REMOVE!)» " ##_message "\t\n                                       " __FILE__ "(" _MDM_STR(__LINE__) ") : \t" )	

/*
#elif defined LINUX

#define _TODO(_message) message ("                          «TODO»       " ##_message "\t\n                                       " __FILE__ "(" _STR(__LINE__) ") : \t" )	

#define _TOTHINK(_message) message ("                          «TOTHINK»    " ##_message "\t\n                                       " __FILE__ "(" _STR(__LINE__) ") : \t" )	
	
#define _TODOCUMENT(_message) message ("                          «TODOCUMENT» " ##_message "\t\n                                       " __FILE__ "(" _STR(__LINE__) ") : \t" )	

#define _DEBUGCODE(_message) message ("                          «DEBUG CODE (REMOVE!)» " ##_message "\t\n                                       " __FILE__ "(" _STR(__LINE__) ") : \t" )	


#elif OSX
// TODO
#endif
*/

//-------------------------------------------------------------------//


// This pragma define makes the code more readable when you turn off
// warnings re Microsoft's lame 255-char name limitation in the 
// "browser information".
#define _NO_VC_NAMELENGTH_WARNINGS warning( disable : 4786 )

#endif // PRAGMA_MESSAGES_H
