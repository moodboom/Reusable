// ------------- key.h

#ifndef KEY_H
#define KEY_H

// ============================
// PdyKey abstract base class
// ============================
class PdyKey	{
	friend class BaseDatabase;
	friend class PdyBtree;
	friend class TNode;
	friend class Persistent;
	NodeNbr fileaddr;	 // object address -> by this key
	NodeNbr lowernode;   // lower node of keys > this key

	// These extract the key data from the specified 
	// node's buffer.
	virtual void GetFromNode( TNode* pNode ) = 0;
	virtual void PutToNode( TNode* pNode ) = 0;

	virtual bool isNullValue() const = 0;
	virtual void CopyKeyData(const PdyKey *key) = 0;
	virtual bool isObjectAddress() const = 0;
	virtual const ObjAddr *ObjectAddress() const = 0;
	virtual PdyKey *MakeKey() const = 0;
protected:

	// const Type_info *relatedclass; // related class
	const type_info *relatedclass; // related class
	
	IndexNo indexno;	 // 0=primary key, >0 =secondary key
	KeyLength keylength; // length of the key
public:
	PdyKey(NodeNbr fa = 0);
	virtual ~PdyKey()
		{ /* ... */ }
	
	virtual int operator>(const PdyKey& key) const = 0;
	virtual int operator==(const PdyKey& key) const = 0;
	virtual PdyKey& operator=(const PdyKey& key);

	// MDM Added, as required by MS template code.
	virtual int operator<(const PdyKey& key) const = 0;
	
	// void Relate(const Type_info *ti)
	void Relate(const type_info *ti)
		{ relatedclass = ti; }
	
	KeyLength GetKeyLength() const
		{ return keylength; }
	void SetKeyLength(KeyLength kylen)
		{ keylength = kylen; }
};

// ============================
// Key class
// ============================
template <class T>
class Key : public PdyKey	{
	T ky;
	bool isObjectAddress() const

		// MDM Changed for VC++ 5.0.
		// This causes a "performance warning" due to the required int-bool 
		// conversion.
		// The result of a typeid expression is a const type_info&.
		// The type_info == operator returns an int.
		// We need to check the value of this int, instead of letting the 
		// compiler convert it.
		// We are assuming that any non-zero return is true (could not find 
		// any documentation or source code to verify).
		// { return typeid(T) == typeid(ObjAddr); }
		{ return ( ( typeid(T) == typeid(ObjAddr) ) != 0 ); }

	const ObjAddr *ObjectAddress() const
		{ return reinterpret_cast<const ObjAddr*>(&ky); }
	void CopyKeyData(const PdyKey *key);
    PdyKey *MakeKey() const;
public:
	Key(const T& key);
	virtual ~Key()
		{ /* ... */ }
	PdyKey& operator=(const PdyKey& key);
    int operator>(const PdyKey& key) const;
    int operator==(const PdyKey& key) const;
	
	 // MDM Added.
	 int operator<(const PdyKey& key) const;
    
	 T& KeyValue()
		{ return ky; }
	void SetKeyValue(const T& key)
		{ ky = key; }
	const T& KeyValue() const
		{ return ky; }

	// These extract the key data from the specified 
	// node's buffer.
	virtual void GetFromNode( TNode* pNode );
	virtual void PutToNode( TNode* pNode );

    bool isNullValue() const;
};

template <class T>
Key<T>::Key(const T& key) : ky(key)
{
	keylength = sizeof(T);
}

template <class T>
void Key<T>::CopyKeyData(const PdyKey *key)
{
	
	const Key<T> *kp =
		static_cast<const Key<T>*>(key);
	ky = kp->ky;
	
}

template <class T>
PdyKey& Key<T>::operator=(const PdyKey& key)
{
	if (this != &key)	{
		PdyKey::operator=(key);
		CopyKeyData(&key);
	}
	return *this;
}

template <class T>
int Key<T>::operator>(const PdyKey& key) const
{
	const Key<T> *kp =
		static_cast<const Key<T>*>(&key);
	return ky > kp->ky;
}


template <class T>
int Key<T>::operator<(const PdyKey& key) const
{

	// MDM MS template implementation implements the calculation of X > Y by
	// actually calculating Y < X.  So we have to supply the < operator in 
	// order to get > functionality.  Dumb.
	// Here, we reverse the order and switch operands, to cover for MS's bad 
	// implementation, so that any override of > will be called, if available.
	// Of course, this will screw you up if you were really trying to get at 
	// a < function.  Oh well.

	const Key<T> *kp =
		static_cast<const Key<T>*>(&key);
	
	// Reverse the order and switch operands, to cover for MS's bad 
	// implementation.
	// Same result, different operand:  return ky < kp->ky;
	return kp->ky > ky;

}


template <class T>
int Key<T>::operator==(const PdyKey& key) const
{
	const Key<T> *kp =
		static_cast<const Key<T>*>(&key);
	return ky == kp->ky;
}

template <class T>
PdyKey *Key<T>::MakeKey() const
{
	PdyKey *newkey = new Key<T>(T(0));
	newkey->SetKeyLength(keylength);
	return newkey;
}


//-------------------------------------------------------------------//
// GetFromNode()																		//
//-------------------------------------------------------------------//
// This gets the key data from the specified node's buffer.
// This must be specialized if key != simple data type.
//-------------------------------------------------------------------//
template <class T>
void Key<T>::GetFromNode( TNode* pNode )
{
	if ( keylength > 0 )
		pNode->GetFromBuffer( &ky, keylength );
}


//-------------------------------------------------------------------//
// PutToNode()																			//
//-------------------------------------------------------------------//
// This puts the key data into the specified node's buffer.
// This must be specialized if key != simple data type.
//-------------------------------------------------------------------//
template <class T>
void Key<T>::PutToNode( TNode* pNode )
{
	if ( keylength > 0 )
		pNode->PutToBuffer( &ky, keylength );
}


template <class T>
bool Key<T>::isNullValue() const
{
	return ky == T(0);
}


// =================================================
// specialized Key<string> template member functions
// =================================================


//-------------------------------------------------------------------//
// Key()																					//
//-------------------------------------------------------------------//
inline Key<string>::Key(const string& key) : ky(key)
{
	keylength = key.length();
}


//-------------------------------------------------------------------//
// GetFromNode()																		//
//-------------------------------------------------------------------//
// This gets the key data from the specified node's buffer.
//-------------------------------------------------------------------//
inline void Key<string>::GetFromNode( TNode* pNode )
{
	char *cp = new char[keylength+1];
	pNode->GetFromBuffer( cp, keylength );
	*(cp+keylength) = '\0';
	ky = string(cp);
	delete cp;
}


//-------------------------------------------------------------------//
// PutToNode()																			//
//-------------------------------------------------------------------//
// This puts the key data into the specified node's buffer.
//-------------------------------------------------------------------//
inline void Key<string>::PutToNode( TNode* pNode )
{
	ky.resize(keylength);
	pNode->PutToBuffer( ky.c_str(), keylength );
}


//-------------------------------------------------------------------//
// MakeKey()																			//
//-------------------------------------------------------------------//
inline PdyKey *Key<string>::MakeKey() const
{
	PdyKey *newkey =

		// new Key<string>(string('\0',keylength));
		new Key<string>(string( "", keylength ));

	newkey->SetKeyLength(keylength);
	return newkey;
}


//-------------------------------------------------------------------//
// isNullValue()																		//
//-------------------------------------------------------------------//
inline bool Key<string>::isNullValue() const
{

	// MDM Correct replacement?
	// return ky.is_null();
	return ky.empty();

}


//-------------------------------------------------------------------//
// operator >()																		//
//-------------------------------------------------------------------//
inline int Key<string>::operator>(const PdyKey& key) const
{

	const Key<string> *kp =
		static_cast<const Key<string>*>(&key);
	
	// Perform a case-insensitive string comparison.
	// This does not seem to call string's override, for some 
	// reason:  return ky > kp->ky;
	return ( ky.compare(kp->ky) > 0 );

}



// ==================================================
// Specialized Key<wstring> template member functions
// ==================================================


//-------------------------------------------------------------------//
// Key()																					//
//-------------------------------------------------------------------//
inline Key<wstring>::Key(const wstring& key) : ky(key)
{
	// Key length bytes = char len * bytes/char.
	keylength = key.length() * sizeof(wchar_t);
}


//-------------------------------------------------------------------//
// GetFromNode()																		//
//-------------------------------------------------------------------//
// This gets the key data from the specified node's buffer.
//-------------------------------------------------------------------//
inline void Key<wstring>::GetFromNode( TNode* pNode )
{
	
	// Be careful, keylength is in bytes, we need
	// characters here.  Divide by bytes/char.
	ASSERT( keylength % sizeof(wchar_t) == 0 );
	KeyLength keycharacters = keylength/sizeof(wchar_t);

	wchar_t *cp = new wchar_t[ keycharacters +1];
	
	pNode->GetFromBuffer( cp, keylength );
	
	// Add a terminating NULL.
	*(cp+keycharacters) = '\0';
	
	// Copy result.
	ky = wstring(cp);

	delete cp;
}


//-------------------------------------------------------------------//
// PutToNode()																			//
//-------------------------------------------------------------------//
// This puts the key data into the specified node's buffer.
//-------------------------------------------------------------------//
inline void Key<wstring>::PutToNode( TNode* pNode )
{

	// Be careful, keylength is in bytes, we need
	// characters here.  Divide by bytes/char.
	ASSERT( keylength % sizeof(wchar_t) == 0 );
	KeyLength keycharacters = keylength/sizeof(wchar_t);

	// Resize needs characters.
	ky.resize( keycharacters );

	// PutToBuffer needs bytes.
	pNode->PutToBuffer( ky.c_str(), keylength );

}


//-------------------------------------------------------------------//
// MakeKey()																			//
//-------------------------------------------------------------------//
inline PdyKey *Key<wstring>::MakeKey() const
{

	// Be careful, keylength is in bytes, we need
	// characters here.  Divide by bytes/char.
	ASSERT( keylength % sizeof(wchar_t) == 0 );
	KeyLength keycharacters = keylength/sizeof(wchar_t);

	PdyKey *newkey =
		new Key<wstring>(wstring( L"", keycharacters ));

	newkey->SetKeyLength(keylength);
	return newkey;

}


//-------------------------------------------------------------------//
// isNullValue()																		//
//-------------------------------------------------------------------//
inline bool Key<wstring>::isNullValue() const
{
	return ky.empty();
}


//-------------------------------------------------------------------//
// operator >()																		//
//-------------------------------------------------------------------//
inline int Key<wstring>::operator>(const PdyKey& key) const
{

	const Key<wstring> *kp =
		static_cast<const Key<wstring>*>(&key);
	
	// Perform a case-insensitive string comparison.
	// The first line does not seem to call string's override, for some 
	// reason:
	// return ky > kp->ky;
	return ( ky.compare(kp->ky) > 0 );

}


#endif


