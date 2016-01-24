//-------------------------------------------------------------------//
// Macros.h - header file															//
//																							//
// Useful defines and constants.													//
//-------------------------------------------------------------------//

#ifndef EM_MACROS_H
#define EM_MACROS_H


//-------------------------------------------------------------------//
// Extraction of bytes from a long word										//
//-------------------------------------------------------------------//
#define BYTE0(l)	((BYTE)(l))
#define BYTE1(l)	((BYTE)(((long)(l) >> 8) & 0xFF))
#define BYTE2(l)	((BYTE)(((long)(l) >> 16) & 0xFF))
#define BYTE3(l)	((BYTE)(((long)(l) >> 24) & 0xFF))


//-------------------------------------------------------------------//
// Square macro																		//
//-------------------------------------------------------------------//
#define sqr(a) (pow((double)(a),2))


//-------------------------------------------------------------------//
// PI and 2*PI constant doubles													//
//-------------------------------------------------------------------//
const double	cdPI		= ( 3.14159265359 );			// PI
const double	cd2PI		= ( 2.0 * 3.14159265359 );	// Two times PI


//-------------------------------------------------------------------//
// Pointer assertion macros using typeid from RTTI							//
//-------------------------------------------------------------------//
#define ASSERT_PTR_TYPE(p, type) \
	ASSERT( ((p) != NULL) && ( typeid(*(p)) == typeid(type) ) )

#define ASSERT_NULL_OR_PTR_TYPE(p, type) \
	ASSERT( ((p) == NULL) || ( typeid(*(p)) == typeid(type) ) )


#endif // EM_MACROS_H
