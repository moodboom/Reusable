// ------------ linklist.h
// a template for a linked list

#ifndef LINKLIST_H
#define LINKLIST_H

// --- the linked list entry
template <class T>
class ListEntry    {

	// MDM All of a sudden, this stopped working in VC++ 5.0.
	// So I made everything public.
	// friend class LinkedList<T>;
public:
   
	T *thisentry;
   ListEntry<T> *nextentry;
   ListEntry<T> *preventry;
   ListEntry(T *entry);
};

template <class T>
// ---- the linked list
class LinkedList    {
    // --- the listhead
    ListEntry<T> *firstentry;
    ListEntry<T> *lastentry;
    ListEntry<T> *iterator;
    T *CurrentEntry();
	void RemoveEntry(ListEntry<T> *entry);
	void InsertEntry(T *entry);
	void InsertEntry(T *entry, short int pos);
	void RemoveEntry(short int pos);
public:
    LinkedList();
    virtual ~LinkedList()
		{ ClearList(); }
    void AppendEntry(T *entry);
	void InsertEntry(T *entry, T *curr);
	void RemoveEntry(T *entry);
    T *FindEntry(short int pos);
	short int FindEntry(T *entry);
    T *FirstEntry();
    T *LastEntry();
    T *NextEntry();
    T *PrevEntry();
    T *NextEntry(T *entry);
    T *PrevEntry(T *entry);
	void ClearList();
};

template <class T>
// ---- construct a linked list
LinkedList<T>::LinkedList()
{
    iterator = 0;
    firstentry = 0;
    lastentry = 0;
}

template <class T>
// ---- remove all entries from a linked list
void LinkedList<T>::ClearList()
{
	ListEntry<T> *lentry = firstentry;
	while (lentry != 0)	{
		ListEntry<T> *nxt = lentry->nextentry;
		delete lentry;
		lentry = nxt;
	}
    iterator = 0;
    firstentry = 0;
    lastentry = 0;
}

// ---- construct a linked list entry
template <class T>
ListEntry<T>::ListEntry(T *entry)
{
	thisentry = entry;
	nextentry = 0;
	preventry = 0;
}

template <class T>
// ---- append an entry to the linked list
void LinkedList<T>::AppendEntry(T *entry)
{
    ListEntry<T> *newentry = new ListEntry<T>(entry);
    newentry->preventry = lastentry;
    if (lastentry)
        lastentry->nextentry = newentry;
    if (firstentry == 0)
        firstentry = newentry;
    lastentry = newentry;
}

template <class T>
// ---- return the current linked list entry
T *LinkedList<T>::CurrentEntry()
{
    return iterator ? iterator->thisentry : 0;
}

template <class T>
// ---- return the first entry in the linked list
T *LinkedList<T>::FirstEntry()
{
    iterator = firstentry;
    return CurrentEntry();
}

template <class T>
// ---- return the last entry in the linked list
T *LinkedList<T>::LastEntry()
{
    iterator = lastentry;
    return CurrentEntry();
}

template <class T>
// ---- return the next entry following the specified one
T *LinkedList<T>::NextEntry(T *entry)
{
	FindEntry(entry);
	return NextEntry();
}

template <class T>
// ---- return the next entry in the linked list
T *LinkedList<T>::NextEntry()
{
    if (iterator == 0)
        iterator = firstentry;
    else
        iterator = iterator->nextentry;
    return CurrentEntry();
}

template <class T>
// ---- return the previous entry ahead of the specified one
T *LinkedList<T>::PrevEntry(T *entry)
{
	FindEntry(entry);
	return PrevEntry();
}

template <class T>
// ---- return the previous entry in the linked list
T *LinkedList<T>::PrevEntry()
{
    if (iterator == 0)
        iterator = lastentry;
    else
        iterator = iterator->preventry;
    return CurrentEntry();
}

template <class T>
// ---- remove an entry from the linked list by position
void LinkedList<T>::RemoveEntry(short int pos)
{
    FindEntry(pos);
    if (iterator != 0)
        RemoveEntry(iterator);
}

template <class T>
// ---- remove an entry from the linked list by entry address
void LinkedList<T>::RemoveEntry(ListEntry<T> *lentry)
{
    if (lentry == 0)
        return;
    if (lentry == iterator)
        iterator = lentry->preventry;
    // ---- repair any break made by this removal
    if (lentry->nextentry)
        lentry->nextentry->preventry = lentry->preventry;
    if (lentry->preventry)
        lentry->preventry->nextentry = lentry->nextentry;
    // --- maintain listhead if this is last and/or first
    if (lentry == lastentry)
        lastentry = lentry->preventry;
    if (lentry == firstentry)
        firstentry = lentry->nextentry;
    delete lentry;
}

template <class T>
// ---- remove an entry from the linked list by entry
void LinkedList<T>::RemoveEntry(T *entry)
{
	FindEntry(entry);
	RemoveEntry(iterator);
}

template <class T>
// ---- insert an entry into the linked list ahead of another
void LinkedList<T>::InsertEntry(T *entry, T *curr)
{
	FindEntry(curr);
	InsertEntry(entry);
}

template <class T>
// ---- insert an entry into the linked list by position
void LinkedList<T>::InsertEntry(T *entry, short int pos)
{
    FindEntry(pos);
	InsertEntry(entry);
}

template <class T>
// ---- insert an entry into the linked list ahead of iterator
void LinkedList<T>::InsertEntry(T *entry)
{
	if (iterator == 0)
		AppendEntry(entry);
	else	{
    	ListEntry<T> *newentry = new ListEntry<T>(entry);
    	newentry->nextentry = iterator;
    	if (iterator)    {
        	newentry->preventry = iterator->preventry;
        	iterator->preventry = newentry;
    	}
    	if (newentry->preventry)
        	newentry->preventry->nextentry = newentry;
    	if (iterator == firstentry)
        	firstentry = newentry;
		iterator = newentry;
	}
}

template <class T>
// ---- return a specific linked list entry
T *LinkedList<T>::FindEntry(short int pos)
{
    iterator = firstentry;
    while (iterator && pos--)
        iterator = iterator->nextentry;
    return CurrentEntry();
}

template <class T>
// ---- return a specific linked list entry number
short int LinkedList<T>::FindEntry(T *entry)
{
	short int pos = 0;
	if (entry != 0)	{
	   
		// We have hit an UAE here when using a bad database.
		iterator = firstentry;
    	
		while (iterator)	{
			if (entry == iterator->thisentry)
				break;
        	iterator = iterator->nextentry;
			pos++;
		}
	}
	return pos;
}

#endif
