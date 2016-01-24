//-------------------------------------------------------------------//
// MultiColumnSortClass
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"
#include "MultiColumnSortClass.h"
/////////////////////////////////////////////////////////////////////////////
// CSortClass
CSortClass::CSortClass(CListCtrl * _pWnd, const int _iCol, const bool _bIsNumeric )
{	
	iCol = _iCol;	
	pWnd = _pWnd;	
	bIsNumeric = _bIsNumeric;		
	ASSERT(pWnd);
	int max = pWnd->GetItemCount();	
	DWORD dw;	
	CString txt;	
	if (bIsNumeric)	
	{
		for (int t = 0; t < max; t++)		
		{			
			dw = pWnd->GetItemData(t);
			txt = pWnd->GetItemText(t, iCol);
			pWnd->SetItemData(t, (DWORD) new CSortItemInt(dw, txt));		
		}	
	}	
	else
	{
		for (int t = 0; t < max; t++)	
		{			
			dw = pWnd->GetItemData(t);
			txt = pWnd->GetItemText(t, iCol);
			pWnd->SetItemData(t, (DWORD) new CSortItem(dw, txt));
		}
	}
}
CSortClass::~CSortClass()
{	
	ASSERT(pWnd);
	int max = pWnd->GetItemCount();
	if (bIsNumeric)	
	{		
		CSortItemInt * pItem;		
		for (int t = 0; t < max; t++)	
		{
			pItem = (CSortItemInt *) pWnd->GetItemData(t);			ASSERT(pItem);
			pWnd->SetItemData(t, pItem->dw);	
			delete pItem;	
		}
	}	
	else
	{
		CSortItem * pItem;
		for (int t = 0; t < max; t++)
		{
			pItem = (CSortItem *) pWnd->GetItemData(t);
			ASSERT(pItem);
			pWnd->SetItemData(t, pItem->dw);
			delete pItem;
		}
	}
}

void CSortClass::Sort(const bool bAsc)
{	
	if (bIsNumeric)	
	{		
		if (bAsc)
			pWnd->SortItems(CompareAscI, 0L);
		else	
			pWnd->SortItems(CompareDesI, 0L);
	}
	else
	{	
		if (bAsc)	
			pWnd->SortItems(CompareAsc, 0L);	
		else
			pWnd->SortItems(CompareDes, 0L);
	}
}

int CALLBACK CSortClass::CompareAsc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	
	CSortItem * i1 = (CSortItem *) lParam1;
	CSortItem * i2 = (CSortItem *) lParam2;	
	ASSERT(i1 && i2);
	return i1->txt.CompareNoCase(i2->txt);
}

int CALLBACK CSortClass::CompareDes(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	
	CSortItem * i1 = (CSortItem *) lParam1;
	CSortItem * i2 = (CSortItem *) lParam2;	
	ASSERT(i1 && i2);
	return i2->txt.CompareNoCase(i1->txt);
}

int CALLBACK CSortClass::CompareAscI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	CSortItemInt * i1 = (CSortItemInt *) lParam1;
	CSortItemInt * i2 = (CSortItemInt *) lParam2;
	ASSERT(i1 && i2);
	if (i1->iInt == i2->iInt) 
		return 0;	
	return i1->iInt > i2->iInt ? 1 : -1;
}

int CALLBACK CSortClass::CompareDesI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	
	CSortItemInt * i1 = (CSortItemInt *) lParam1;
	CSortItemInt * i2 = (CSortItemInt *) lParam2;
	ASSERT(i1 && i2);
	if (i1->iInt == i2->iInt)
		return 0;	
	return i1->iInt < i2->iInt ? 1 : -1;
}

CSortClass::CSortItem::CSortItem(const DWORD _dw, const CString & _txt)
{
	dw = _dw;
	txt = _txt;
}

CSortClass::CSortItem::~CSortItem()
{
}

CSortClass::CSortItemInt::CSortItemInt(const DWORD _dw, CString & _txt)
{
	// MDM	7/13/2000 6:31:32 PM
	// If we remove colons we get great sorting for times, too!
	// Hopefully this isn't a big performance hit on this already-sad
	// mechanism.  :>
	// We can also remove "/" to help SOMEWHAT with dates.  Still pretty
	// scragged using stupido Americana-style dates.
	// Also remove decimals, so we get good sorting down to the last
	// decimal digit, as long as output #decimals is standardized.
	// Isn't there some way to scan in one pass???  :<
	_txt.Remove( _T(':') );
	_txt.Remove( _T('/') );
	_txt.Remove( _T('.') );

	iInt = atoi( _txt );	
	dw = _dw;
}