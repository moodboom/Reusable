//-------------------------------------------------------------------//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//


class CSortClass
{
public:
	CSortClass(CListCtrl * _pWnd, const int _iCol, const bool _bIsNumeric);
	//CSortClass(CListCtrl * _pWnd, const int _iCol, const bool _bIsNumeric, int iStartingItem, int iEndingItem );
	virtual ~CSortClass();		
	
	int iCol;	
	int m_iStartingItem;
	int m_iEndingItem;

	CListCtrl * pWnd;	
	bool bIsNumeric;
	
	void Sort(const bool bAsc);	
	
	static int CALLBACK CompareAsc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareDes(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareAscI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareDesI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

public:	
	class CSortItem	
	{	
		public:		
			virtual  ~CSortItem();
			CSortItem(const DWORD _dw, const CString &_txt);		
			CString txt;		
			DWORD dw;	
	};
	class CSortItemInt	
	{	
		public:
			CSortItemInt(const DWORD _dw, CString &_txt);		
			int iInt ;		
			DWORD dw;	
	};
};