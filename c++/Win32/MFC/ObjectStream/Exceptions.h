#ifndef EXCEPTIONS_H
	#define EXCEPTIONS_H


// =============================================
// ObjectStream exceptions representing program errors
// =============================================
class PdyExceptions {};
// --- ReadObject from non-ctor
class NotInConstructor : public PdyExceptions {};
// --- WriteObject from non-dtor
class NotInDestructor : public PdyExceptions {};
// --- No database open
class NoDatabase : public PdyExceptions {};
// --- LoadObject was not called
class NotLoaded : public PdyExceptions {};
// --- SaveObject was not called
class NotSaved : public PdyExceptions {};
// --- Multireference object deleted
class MustDestroy : public PdyExceptions {};
// --- string key w/out size
class ZeroLengthKey : public PdyExceptions {};
// --- Key length != btree key length
class BadKeylength : public PdyExceptions {};
// --- Bad ObjAddr specified
class BadObjAddr : public PdyExceptions {};


#endif	// EXCEPTIONS_H
