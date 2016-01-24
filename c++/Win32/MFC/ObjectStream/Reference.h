
#ifndef REFERENCE_H
	#define REFERENCE_H


// =====================================
// Reference template
// =====================================
template <class T>
class Reference	{
public:
	T *obj;
	Reference();
	~Reference();
	void ReadObject();
	void WriteObject();
	void operator=(T& to) throw (BadReference);
	void RemoveReference();
};

template <class T>
Reference<T>::Reference()
{
	obj = 0;
}

template <class T>
Reference<T>::~Reference()
{
	Persistent::Destroy(obj);
}

template <class T>
void Reference<T>::ReadObject()
{
	Persistent::Destroy(obj);
	obj = 0;

	ObjAddr oa;
	::ReadObject(oa);
	if (oa != 0)
		obj = new T(oa);
}

template <class T>
void Reference<T>::WriteObject()
{
	ObjAddr oa = 0;
	if (obj != 0)
		oa = obj->ObjectAddress();
	::WriteObject(oa);
}

template <class T>
void Reference<T>::operator=(T& to)

		// throw (BadReference)

{
	Persistent *po = dynamic_cast<Persistent*>(&to);
	if (po == 0)
		throw BadReference();
	Persistent::Destroy(obj);
	obj = static_cast<T*>(po);
	obj->AddReference();
}

template <class T>
void Reference<T>::RemoveReference()
{
	Persistent::Destroy(obj);
	obj = 0;
}

#endif	// REFERENCE_H