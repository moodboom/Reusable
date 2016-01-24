// ScrapDlg.cpp : implementation file
//

//-------------------------------------------------------------------//
// Moved scrap to vs 2003.
// Had to add this to stdafx.h to get min/max working:
//
//		#define _MFC_VER 0x0700
//		#include <min_max_fix.h>
// 
//-------------------------------------------------------------------//
#include "stdafx.h"

#include "Scrap.h"
#include "ScrapDlg.h"

#define __STDC__ 1
#include <oci.h>												// For Oracle OCI interface
#undef __STDC__

#include <locale.h>											// For windows locale support???
#include <TCHAR_convert.h>
#include <TimeHelpers.h>									// For profiling the database load time.

// #include <CompInfo.h>									// For heap tracing

#include <ustring.h>

#include <DisplayMessage.h>								// For feedback

#include <STLExtensions.h>

// #define WINDOWS_UI 2										// Set WINDOWS_UI to 2 for private build
// #define JAVA_INTERFACE 0								// Set Java interface to 0 for default (no interface)
// #include <UnicodeBIDI/bidi.h>							// For BIDI algorithm

#include <math.h>												// for acos()

#include <RandomHelpers.h>									// For get_random_uLong()

// =======================									// For boost serialization
// BOOST SERIALIZE HEADERS
// =======================
// MDM NOTE that the order of these includes comes
// directly from the serialize demo code - I don't
// recommend changing them unless absolutely necessary,
// since they work as they are for cross-platform.
// =======================
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>

// MDM Not needed, and causes "already defined" errors if
// included in more than one module.
// #include <boost/archive/tmpdir.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
// =======================
#include <boost/serialization/vector.hpp>				// Needed to serialize standard vectors.

// Redirecting std::cout to go to a file or a string...
#include <fstream>
#include <sstream>
#include <iostream>


// MDM For Intel MKL, see test section below.
// extern "C" int main();


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




//-------------------------------------------------------------------//
// STATICS, GLOBALS, CONSTANTS
//-------------------------------------------------------------------//

/*
//------------------------------------------//
// Maps and their requisite < operators.
//------------------------------------------//
class A
{
public:
	int a;
};

bool operator< ( A a, A b )
{
	return true;
}

bool operator< (
	A& Left,
	A& Right
)
{
	return Left.a < Right.a;
}

#include <map>
std::map< A, int > test_map;

// WILL NOT COMPILE
// bool operator< ( int na, int nb )
// {
// 	return true;
// }

// WILL NOT COMPILE
// bool operator< (
// 	const A* pLeft,
// 	const A* pRight
// ) const 
// {
// 	return true;
// }


// If you set up this function for each of your
// map items, you can call TraceMap()...
void operator<<( std::ostringstream strm, A& info )
{
	strm << info.a;
}

//------------------------------------------//
*/


//------------------------------------------//
// Inheritance and const.
//
//------------------------------------------//

/*
class A
{
public:
	virtual int& foo() { return s_nA; }
	virtual const int& foo() const { return s_nA; }
protected:
	static int s_nA;
};
int A::s_nA = 5;
class B : public A
{
public:
	// virtual int& foo() { return s_nB; }
	virtual const int& foo() const { return s_nB; }
protected:
	static int s_nB;
};
int B::s_nB = 6;
*/

//------------------------------------------//


//------------------------------------------//
// Default params
//------------------------------------------//
// You must define default params in the function
// declaration, not the definition.  The compiler
// will ALLOW you to define the param in the 
// definition as long as you don't define it
// in the declaration.  But it will be ignored
// during runtime.
//------------------------------------------//
class A
{
public:

   A( int i );

};

A::A( int i = 2 )
{

}

//------------------------------------------//


//------------------------------------------//
// boost serialization
//------------------------------------------//
// I'm having trouble with TransferredQuestion
//	being archived by a container class.
//------------------------------------------//

class TransferredQuestion
{
    // =============
    // SERIALIZATION
    // =============
    // friend std::ostream & operator<<(std::ostream &os, const TransferredQuestion &to);
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int /* file_version */);
    // =============

public:
    TransferredQuestion() {}

	uShort m_sType;
	ustring m_str;
};
template<class Archive>
void TransferredQuestion::serialize(
 Archive & ar, 
 const unsigned int // file_version 
) {	
	ar & m_sType;
	ar & m_str;
}
class TQContainer
{
    // =============
    // SERIALIZATION
    // =============
    // friend std::ostream & operator<<(std::ostream &os, const TQContainer &to);
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int /* file_version */);
    // =============

public:
	TQContainer() {}

	TransferredQuestion m_tq;
};
template<class Archive>
void TQContainer::serialize(
 Archive & ar, 
 const unsigned int // file_version 
) {	
	ar & m_tq;
}

//------------------------------------------//
   

//------------------------------------------//
// Abstract template class in a container.
//------------------------------------------//

class BaseOfTemplate
{
};

template <class T>
class AbstractTemplateBase : public BaseOfTemplate
{
	// Construction
	public:		   
	AbstractTemplateBase();
};

template <class T>
AbstractTemplateBase<T>::AbstractTemplateBase()
{
}
//------------------------------------------//
	

//-------------------------------------------------------------------//
// Heather needs to pass a pointer to a string pointer, so she
// can deallocate and reallocate.
//-------------------------------------------------------------------//
void trashMyString( char** pszTrashMe )
{
   delete *pszTrashMe;
   *pszTrashMe = new char[30];
   strcpy( *pszTrashMe, "P0WNED!!!!!!" );
}
//-------------------------------------------------------------------//



//-------------------------------------------------------------------//


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScrapDlg dialog

CScrapDlg::CScrapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScrapDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScrapDlg)
	m_strInput = _T("");
	m_strOutput = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScrapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScrapDlg)
	DDX_Control(pDX, IDC_OUTPUT, m_OutputEdit);
	DDX_Text(pDX, IDC_INPUT, m_strInput);
	DDX_Text(pDX, IDC_OUTPUT, m_strOutput);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CScrapDlg, CDialog)
	//{{AFX_MSG_MAP(CScrapDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_GO, OnGo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScrapDlg message handlers

BOOL CScrapDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CScrapDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CScrapDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CScrapDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


//-------------------------------------------------------------------//
// GetRegKey()																			//
//-------------------------------------------------------------------//
LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
    HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS) {
        long datasize = MAX_PATH;
        TCHAR data[MAX_PATH];
        RegQueryValue(hkey, NULL, data, &datasize);
        
		  // Having trouble here...
		  _tcscpy(retdata,data);
        
		  RegCloseKey(hkey);
    }

    return retval;
}

//-------------------------------------------------------------------//
// GotoURL()																			//
//-------------------------------------------------------------------//
HINSTANCE GotoURL( LPCTSTR url, int showcmd, CString& strOutput )
{
    TCHAR key[MAX_PATH + MAX_PATH];
	 strOutput = _T("Attempting to open [");
	 strOutput += url;
	 strOutput += _T("]...\r\n");

    // First try ShellExecute()
    HINSTANCE result = ShellExecute( NULL, _T("open"), url, NULL, NULL, showcmd );

    // If it failed, get the .htm regkey and lookup the program
    if((UINT)result <= HINSTANCE_ERROR)
	{

		strOutput += _T("ShellExecute failed, attempting to find browser exe in registry...\r\n");
		if( GetRegKey( HKEY_CLASSES_ROOT, _T(".htm"), key ) == ERROR_SUCCESS )
		{
			lstrcat( key, _T("\\shell\\open\\command"));

			if( GetRegKey( HKEY_CLASSES_ROOT, key, key ) == ERROR_SUCCESS )
			{
                LPTSTR pos;

                pos = _tcsstr(key, _T("\"%1\""));

                if( pos == NULL )						// No quotes found
				{
                    pos = _tcsstr( key, _T("%1"));		// Check for %1, without quotes 
                    
					if( pos == NULL )					// No parameter at all...
					{
                        pos = key + _tcslen( key ) - 1;
					}
                    else
					{
                        *pos = _T('\0');				// Remove the parameter
					}
                }
                else
				{
                    *pos = _T('\0');					// Remove the parameter
				}

                lstrcat( pos, _T(" "));
                lstrcat( pos, url );

				PROCESS_INFORMATION pi;
		
				STARTUPINFO			si;

				::ZeroMemory( &si, sizeof( si )); 
		
				si.cb = sizeof( si ); 

				if( ::CreateProcess( NULL,
									 pos,
									 NULL,
									 NULL,
									 FALSE,
									 0,
									 NULL,
									 NULL,
									 &si,
									&pi ) == FALSE )
				{
					strOutput += _T("CreateProcess() failed...\r\n");
					return NULL;
				}

                result = (HINSTANCE) pi.hProcess;
					 strOutput += _T("Success!\r\n");
            }
        } else
		  {
				strOutput += _T("Couldn't find [\\shell\\open\\command] for the .htm extension...\r\n");
		  }
    } else
	 {
		 strOutput += _T("Success!\r\n");
	 }

	 return result;
}


//-------------------------------------------------------------------//
// OnGo()																				//
//-------------------------------------------------------------------//
void CScrapDlg::OnGo() 
{
	// Do whatevah ya like here.

	// Get the input.
	UpdateData();



	/*
	//-------------------------------------------------------------------//
	// Crack the silly XOR code.
	// Well, it turns out it ain't an XOR code.  We are using an 
	// Oracle package that includes the following:
	//
	//		mask.fs_e( plaintext, length? )
	//		mask.fs_d( cyphertext, length? )
	//
	//	use it to peek at the passwords as follows:
	//		select id, db, mask.fs_d( PASSWD, 12 ) from ADM_OPERATORS;
	//
	LPCTSTR szXOR = _T("operator");
	LPCTSTR szInput = m_strInput.GetBuffer(100);
	int nXOR = 0;
	for ( int nA = 0; nA < m_strInput.GetLength(); nA++ )
	{
		m_strOutput += szXOR[nXOR] ^ szInput[nA];
		nXOR++;
		if ( nXOR == 8 )
			nXOR = 0;
	}
	//-------------------------------------------------------------------//
	*/




	/*
	//-------------------------------------------------------------------//
	// We want to find a generic way to fire up IE on all Windows boxes.
	// See what there is...
	GotoURL( m_strInput, SW_SHOWMAXIMIZED, m_strOutput );
	//-------------------------------------------------------------------//
	*/



	/*
	//-------------------------------------------------------------------//
	// UNICODE!
	//	In order to enable Unicode in an app, we have to
	//	
	//		1) Go to regional settings in control panel
	//			General: add language settings
	//			Input Locales: add input locales
	//		2) Manually modify the dlg templates in the RC file to use
	//			a special hacky font - "MS Shell Dlg 2" - which forces
	//			a font that supports all installed languages.
	//			(The orig was called "MS Sans Serif"...)
	//			(I think this uses Tahoma?  see MSDN if ya want)
	//		3) Set up your UNICODE build env's if u didnt do so by default (BAD)
	//				Add a config
	//				Add _UNICODE, UNICODE
	//				REmove _MBCS
	//				Update the entry point, which is diff for UNICODE:
	//					In the Output category of the Link tab in the Project Settings dialog box, 
	//						set the Entry Point Symbol to wWinMainCRTStartup. 
	//					For Visual C++ .NET: In the Advanced category of the Linker folder in the 
	//						Project Properties dialog box, set the Entry Point to wWinMainCRTStartup. 
	//		4)	Set up the proper libs for your build environment.
	//			One good way that works for MFC/non-MFC code combinations:
	//				a) Add this to the top of stdafx.h
	//						#include <ForceCorrectMFCLinkageForNonMFCModules.h>
	//				b) set up the lib to link to manually, per configuration:
	//						Release				nafxcw.lib
	//						Debug					nafxcwd.lib
	//						Release UNICODE	uafxcw.lib
	//						Debug UNICODE		uafxcwd.lib
	//
	//					NOte: other libs i've had to add include
	//						winmm.lib odbc32.lib odbccp32.lib 
	//					Plus third party, of course
	//
	//		5) Build and run the UNICODE version of the app.
	
	// MDM	4/12/01 9:26:04 AM
	// Profile the time.
	
	sleep( 500 );

	struct _timeb timebuffer;
	_ftime( &timebuffer );

	// We want to do a test of conversion from/to wide chars.
	// Convert the input string to UNICODE repeatedly, timing the process...
	const int cnIterations = 1000000;
	const int cnMaxStringLength = 1000;

	// Sorry for the ugly cast...
	TCHAR* szInput = LPTSTR( LPCTSTR( m_strInput ) );

	for ( int nA = 0; nA < cnIterations; nA++ )
	{
		// Method 1
		// wstring strOutput( szInput );
		
		// Method 2
		// wchar_t szOutput[ cnMaxStringLength + 1 ];
		// convert_TCHAR_to_wchar( szOutput, szInput, cnMaxStringLength );

		// Method 3
		char szOutput[ cnMaxStringLength + 1 ];
		convert_TCHAR_to_char( szOutput, szInput, cnMaxStringLength );

		// Turn off display...
		//m_strOutput += szInput;
		//UpdateData( FALSE );
		//Invalidate();
		//RedrawWindow();
	}

	// MDM	4/12/01 9:26:04 AM
	// Finish profiling the time.

	struct _timeb timebuffer2;
	_ftime( &timebuffer2 );
	m_strOutput.Format(
		_T("% d strings converted in %.3f seconds..."),
		cnIterations,
		CalcTimeSpan( timebuffer, timebuffer2 )
	);

	// Test to make sure we can put UNICODE into title bar.
	SetWindowText(m_strInput);

	// BIDI test
	// For our hacky Hebrew Unix Ormap support, we stored Hebrew chars in
	// the database as they are viewed from left to right, not in correct
	// logical order.  Therefore, to display them correctly in the GUI,
	// we need to BIDI them.
	//
	// This is all taken verbatim from the reference sample code.
	// I just commented out the crap that really shouldn't be
	// in there.

	m_strOutput = m_strInput;
	TCHAR* pszInput = m_strOutput.GetBuffer( MAX_CCH );

	int cch = m_strInput.GetLength();

	// BEGIN reference code -----------

	// pszInput[cch] = 0;
	// fprintf(f, "Input    %2d: %s\n", realArg, pszInput);
 
	int types[MAX_CCH];
	int levels[MAX_CCH];
 
	// assign directional types
	classify(pszInput, types, cch);
 
	// if (beVerbose) 
	// {
	// 	fprintf(f, "Input Types: ");
	// 	ShowInputTypes(f, pszInput, cch); fprintf(f, "\n");
	// }
 
	// limit text to first block
	cch = resolveParagraphs(types, cch);
 
	// set base level and compute character types
	int baselevel = baseLevel(types, cch);
	// if (beVerbose) 
	// {
	// 	fprintf(f, "Base Level : %d\n", baselevel);
	// }
 
	// resolve explicit
	resolveExplicit(baselevel, N, types, levels, cch);
 
	// if (beVerbose) 
	// {
	// 	fprintf(f, "Levels (A) : ");
	// 	ShowLevels(f, levels, cch); fprintf(f, "\n");
	// }
 
	// resolve weak
	resolveWeak(baselevel, types, levels, cch);
 
	// if (beVerbose) 
	// {
	// 	fprintf(f, "Types (A)  : ");
	// 	ShowTypes(f, types, cch); fprintf(f, "\n");
	// }
 
	// resolve neutrals
	resolveNeutrals(baselevel,types, levels, cch);
 
	// if (beVerbose) 
	// {
	// 	fprintf(f, "Types (B)  : ");
	// 	ShowTypes(f, types, cch); fprintf(f, "\n");
	// }
 
	// resolveImplicit
	resolveImplicit(types, levels, cch);
 
	// if (beVerbose) 
	// {
	// 	fprintf(f, "Levels (B) : ");
	// 	ShowLevels(f, levels, cch); fprintf(f, "\n");
	// }
 
	// assign directional types again, but for WS, S this time
	classify(pszInput, types, cch, true);
 
	BidiLines(baselevel, pszInput, types, levels, cch, doMirror);
 
	// if (doClean) 
	// { 
	// 	cch = clean(pszInput, cch); 
	// }

	// fprintf(f, "Output   %2d: %s\n\n", realArg, pszInput);

	// END reference code -----------
	
	m_strOutput.ReleaseBuffer( );

	//-------------------------------------------------------------------//
	*/


	/*
	//-------------------------------------------------------------------//
	// BOOST sample
	//-------------------------------------------------------------------//
	// You will need to include the path to the VC++ compatible boost
	// libs.  Add the following to the project lib path 
	// (Proj->Settings->LInk->Input->Addl lib paths):
	//
	//		E:\Michael's Data\Software Development\Samples\boost_1_27_0\libs\regex\build\vc6
	//
	//-------------------------------------------------------------------//
	{
		// ustring text;
		std::string text;
		{
			CString strTemp;

			m_strOutput = _T("Processing input file...\r\n");
			map_type m;

			std::ifstream fs(	
			
				// Sorry for the ugly cast...
				LPTSTR( LPCTSTR( m_strInput ) )
			
			);

			load_file(text, fs);
			IndexClasses(m, text);
			
			strTemp.Format( _T("%s%d matches found...\r\n"), m_strOutput, m.size() );
			m_strOutput = strTemp;

			map_type::iterator c, d;
			c = m.begin();
			d = m.end();
			while(c != d)
			{
				// cout << "class \"" << (*c).first << "\" found at index: " << (*c).second << endl;
				strTemp.Format( 
					_T("%sclass \"%s\" found at index: %d\r\n"), 
					m_strOutput, 
					( (*c).first		).c_str(), 
					(*c).second
				);
				m_strOutput = strTemp;

				// Post results as we go...
				UpdateData( FALSE );

				++c;
			}
		}
	}
	//-------------------------------------------------------------------//
	*/


	/*
	//-------------------------------------------------------------------//
	// std::string efficiency
	// Comes out to 12 bytes.  Yucky.
	//-------------------------------------------------------------------//
	std::string text;
	std::wstring wtext;
	m_strOutput.Format(
		_T("string is %d, wstring is %d"),
		sizeof text,
		sizeof wtext
	);
	//-------------------------------------------------------------------//
	*/


	/*
	//-------------------------------------------------------------------//
	// bit shifting examples.
	//-------------------------------------------------------------------//
	m_strOutput = _T("Bit values\r\n");
	for ( int nA = 0; nA < 16; nA++ )
	{
		CString strBits;
		strBits.Format(
			_T("shifted left %d positions = %16d\r\n"),
			nA,
			1 << nA
		);
		m_strOutput += strBits;
	}
	//-------------------------------------------------------------------//
	*/

	/*
	//-------------------------------------------------------------------//
	// Memory allocation fragmentation test
	// Here, we allocate lots of small chunks of memory, until we have
	// used up our address space.  Hopefully the chunks will be fairly
	// contiguous, both pointer-address-wise and physical-heap-memory-wise.  
	// Then, we free every other allocation, creating a whole bunch of
	// small holes in the app address space (who knows what we create
	// in the system address space - based on the results, it looks 
	// like the system can collect and consolidate this memory).  
	// Then, we try to allocate one block that is 2 times the original 
	// chunk size, to verify that memory "fragmentation" occurs that can 
	// cause small allocations to fail even when large amounts of memory 
	// are available, if the app address space does not have a big
	// enough "hole".
	//
	// Results:
	//
	// 1) If you have less than 2GB available on your machine, the last
	// allocation will SUCCEED, because the system can gather the
	// deallocated memory up and provide a pointer to one of the many
	// application memory space "holes".
	//
	// 2) If you have 2GB or more of total memory (including virtual),
	// the last allocation will FAIL as expected, because you have
	// truly used up the entire application memory space and there are
	// no holes left big enough.
	// 
	//-------------------------------------------------------------------//
	class Test
	{
		int n1;
	};
	// Test* pTest = new Test[100000000];
	CString strLine;
	
	CCompInfo hInfo( _T("Heap.log") );
	
	const int cnBigChunkSize = 200 * 1024 * 1024;
	const int cnSmallChunkSize = 10 * 1024 * 1024;
	const int cnMaxAvail = 0x7fffffff;

	// Use up MOST of the avail memory with big chunks.
	// We just leave the last one available.
	// We'll catch the exception when we run out, this is expected.
	int nA;
	char** pBigChunks = new char*[ cnMaxAvail / cnBigChunkSize ];
	try
	{
		for ( nA = 0; true; nA++ )
		{
			strLine.Format( _T("Allocating large chunk %d...\r\n"), nA );
			m_strOutput += strLine;
			ForceRefresh();

			pBigChunks[nA] = new char[ cnBigChunkSize ];

			m_strOutput += hInfo.GetHeapAllocations( 100000 ).c_str();
			ForceRefresh();
		}
	}
	catch (...)
	{
		// Don't worry about it when we run out - we should!
	}

	// Use up the REST of the avail memory with small chunks.
	// We'll catch the exception when we run out, this is expected.
	char** pSmallChunks = new char*[ cnMaxAvail / cnSmallChunkSize ];
	try
	{
		for ( nA = 0; true; nA++ )
		{
			strLine.Format( _T("Allocating small chunk %d...\r\n"), nA );
			m_strOutput += strLine;
			ForceRefresh();

			pSmallChunks[nA] = new char[ cnSmallChunkSize ];

			m_strOutput += hInfo.GetHeapAllocations( 100000 ).c_str();
			ForceRefresh();
		}
	}
	catch (...)
	{
		// Don't worry about it when we run out - we should!
	}

	// The last slot was NOT allocated.
	// We need to step back TWO slots to make sure we don't 
	// leave any holes bigger than the small block.
	nA = nA - 2;
	
	// Now delete every other small chunk.
	for ( ; nA > 0; nA = nA - 2 )
	{
		strLine.Format( _T("Deallocating small chunk %d...\r\n"), nA );
		m_strOutput += strLine;
		ForceRefresh();

		delete pSmallChunks[nA];

		m_strOutput += hInfo.GetHeapAllocations( 100000 ).c_str();
		ForceRefresh();
	}

	// Now attempt to allocate a block 2 times the size of small.
	strLine = _T("Final allocation\r\n");
	m_strOutput += strLine;
	UpdateData( FALSE );

	// THIS SHOULD FAIL!
	sleep(4);
	char* pLast = new char[ cnSmallChunkSize * 2 ];

	m_strOutput += hInfo.GetHeapAllocations( 100000 ).c_str();
	//-------------------------------------------------------------------//
	*/


	//-------------------------------------------------------------------//
	// Font performance test
	// For this, we set up an Active X control that we draw fonts to.
	// The Active X control is called FontPerformanceTester.
	// It merely draws 1000000 1-character strings to the display,
	// timing the drawing.  We don't need to add any code, just drop
	// in the control and go.
	//
	// To enable this test, build FontPerformanceTester, which registers
	// the control, and make sure one is added somewhere in the main 
	// dialog.
	//
	// To remove this test, remove the ActiveX control from the dialog.
	//-------------------------------------------------------------------//


	//-------------------------------------------------------------------//
	// Scope test.
	//-------------------------------------------------------------------//
	/*
	{
		class ScopeTest
		{
		public:
			CString strFunction()
			{
				return CString( _T("ScopeTest function") );
			}

			~ScopeTest()
			{
				DisplayMessage( _T("ScopeTest object going out of scope...") );
			}
		};
		
		{
			DisplayMessage( ScopeTest().strFunction() );

			// NOTE: WE GET THE "GOING OUT OF SCOPE" MESSAGE HERE!
			// THE SCOPE IS LIMITED TO WITHIN THE FUNCTION PARENS!  
			// THIS IS GOOD AND COOL!

			DisplayMessage( _T("End inside bracket") );
		}
		DisplayMessage( _T("End outside bracket") );

	}
	*/	
	//-------------------------------------------------------------------//


	/*
	//-------------------------------------------------------------------//
	// Oracle Internationalization test
	//-------------------------------------------------------------------//
	// To set up for this, we did the following:
	//	
	//		1)	add Oracle include path
	//				F:\ORACLE\ORA9I\OCI\INCLUDE
	//		2)	add Oracle lib path
	//				F:\ORACLE\ORA9I\OCI\LIB\MSVC
	//		3)	add oci.lib to included libraries
	//		4) add the following to the includes section:
	//				#define __STDC__ 1
	//				#include <oci.h>
	//				#undef __STDC__
	//
	//	
	//-------------------------------------------------------------------//

	// ------------------
	// Set up environment.
	// ------------------
	#define LANG_STR_DATE _T("American")
	struct OCIEnv* envhp = NULL;
	OCIEnvCreate(
		(OCIEnv **)&envhp,
		
		#ifdef _UNICODE
			OCI_UTF16,
		#else
			OCI_DEFAULT,
		#endif

		0,
		0,
		0,
		0,
		0,
		0
	);
	struct OCIServer* srvhp;
	struct OCISvcCtx* svchp;
	struct OCISession* usrhp;
	OCIHandleAlloc(envhp,(void**)&srvhp,OCI_HTYPE_SERVER,0,NULL);
	OCIHandleAlloc(envhp,(void**)&svchp,OCI_HTYPE_SVCCTX,0,NULL);
	OCIHandleAlloc(envhp,(void**)&m_errhp,OCI_HTYPE_ERROR,0,NULL);
	// ------------------


	// ------------------
	// Start connection.
	// ------------------
	const TCHAR* user = _T("admin");
	const TCHAR* pass = _T("marvin");
	const TCHAR* conn = _T("meb816");

	OCIServerAttach(
		srvhp,
		m_errhp,
		(text*)conn,
		( _tcsclen( conn ) ) * sizeof TCHAR,
		OCI_DEFAULT
	);

	OCIAttrSet(
		svchp,
		OCI_HTYPE_SVCCTX,
		srvhp,
		0,
		OCI_ATTR_SERVER,
		m_errhp
	);
	
	OCIHandleAlloc(
		envhp,
		(void**)&usrhp,
		OCI_HTYPE_SESSION,
		0,
		NULL
	);
	
	OCIAttrSet(
		usrhp,
		OCI_HTYPE_SESSION,
		(text*)user,
		( _tcsclen( user ) ) * sizeof TCHAR,
		OCI_ATTR_USERNAME,
		m_errhp
	);

	OCIAttrSet(
		usrhp,
		OCI_HTYPE_SESSION,
		(text*)pass,
		( _tcsclen( pass ) ) * sizeof TCHAR,
		OCI_ATTR_PASSWORD,
		m_errhp
	);
	
	OCISessionBegin(
		svchp,
		m_errhp,
		usrhp,
		OCI_CRED_RDBMS,
		OCI_DEFAULT
	);
	
	OCIAttrSet(
		svchp,
		OCI_HTYPE_SVCCTX,
		usrhp,
		0,
		OCI_ATTR_SESSION,
		m_errhp
	);
	// ------------------


	// ------------------
	// Get the count of records in a table.
	// ------------------
	int ret=0;
	OCIStmt* stmhp;
	OCIDefine* defhp;

	TCHAR* wsql = _T("SELECT count(*) FROM ADM_DATABASE");
	
	int n_wsql_len = _tcsclen( wsql );

	m_strOutput = ociErr(
		OCIHandleAlloc(
			envhp,
			(void**)&stmhp,
			OCI_HTYPE_STMT,
			0,
			NULL
		)
	);

	m_strOutput += ociErr(
		OCIStmtPrepare(
			stmhp,
			m_errhp,
			// wsql,																	// <- SAMPLE CODE IS BAD?!!?
			(text*)wsql,
			(ub4)n_wsql_len,
			(ub4)OCI_NTV_SYNTAX,
			(ub4)OCI_DEFAULT
		)
	);

	m_strOutput += ociErr(
		OCIDefineByPos(
			stmhp,
			&defhp,
			m_errhp,
			(ub4)1,
			&ret,
			sizeof(int),
			SQLT_INT,
			NULL,
			NULL,
			NULL,
			OCI_DEFAULT
		)
	);

	m_strOutput += ociErr(
		OCIStmtExecute(
			svchp,
			stmhp,
			m_errhp,
			0,
			0,
			NULL,
			NULL,
			OCI_DEFAULT
		)
	);

	m_strOutput += ociErr(
		OCIStmtFetch(
			stmhp,
			m_errhp,
			1,
			OCI_FETCH_NEXT,
			OCI_DEFAULT
		)
	);
	// ------------------


	// ------------------
	// Get all the records in a table.
	// ------------------

	wsql = _T("SELECT * FROM ADM_DATABASE");
	
	n_wsql_len = _tcsclen( wsql );

	ociErr(
		OCIHandleAlloc(
			envhp,
			(void**)&stmhp,
			OCI_HTYPE_STMT,
			0,
			NULL
		)
	);
	ociErr(
		OCIStmtPrepare(
			stmhp,
			errhp,
			(text*)sql,											// MDM I hate this cast, but it's required
			_tcsclen( sql ) * sizeof TCHAR,				// size in BYTES
			OCI_NTV_SYNTAX,
			OCI_DEFAULT
		)
	);


void Database::makeQuerySql(TableDef* td,TCHAR* buf)
{
	_tcscpy( buf, _T("SELECT ") );
	_tcscat( buf, td->selectClause() );


	TableDef* c = td->getColumn();
	while(c) {
                if(c != td->getColumn()) _tcscat( buf,_T(","));
                _tcscat(buf,_T("NVL("));
                _tcscat(buf,td->tableName());
                _tcscat(buf,_T("."));
                _tcscat(buf,c->columnName());
                _tcscat(buf,_T(","));
		switch(c->columnType()) {
			case TableDef::byte:
			case TableDef::ubyte:
			case TableDef::word:
			case TableDef::uword:
			case TableDef::uinteger:
			case TableDef::integer:
                                _tcscat(buf,_T("0"));
				break;
			case TableDef::real:
                                _tcscat(buf,_T("0.0"));
				break;
			case TableDef::dreal:
                                _tcscat(buf,_T("0.0"));
				break;
			case TableDef::string:
                                _tcscat(buf,_T("' '"));
				break;
			case TableDef::date:
                                _tcscat(buf,_T("''"));
			}

                _tcscat(buf,_T(")"));
		c = c->getColumn();
	}
        _tcscat(buf,_T(" FROM "));
        _tcscat(buf,td->tableName());
		if (_tcsclen(td->joinTableName()) > 0) {
			_tcscat(buf,_T(","));
			_tcscat(buf,td->joinTableName());
		}
		_tcscat(buf,_T(" "));
        _tcscat(buf,td->whereClause());
        _tcscat(buf,_T(" "));
        _tcscat(buf,td->orderClause());
}




	
	// ------------------


	// ------------------
	// Shut down connection.
	// ------------------
	OCISessionEnd(svchp,m_errhp,usrhp,OCI_DEFAULT);
	OCIServerDetach(srvhp,m_errhp, OCI_DEFAULT);			// This has failed if oracle is down.
	// ------------------

	
	// Copy out the results.
	CString strTemp = m_strOutput;
	m_strOutput.Format( _T("Errors = %s\r\nRecord count = %d"), strTemp, ret );

	//-------------------------------------------------------------------//
	*/


	/*
	//-------------------------------------------------------------------//
	// Pointer arithmetic test.
	//
	// Check this out.  The arithmatic result of pointer 
	// addition/subtraction is ALWAYS in units of the size of
	// the object that the pointer points to.  Check out the last
	// example, it makes things most clear.  Add 1 to a second pointer,
	// and the addresses of the pointers will be diff by the size
	// of the structure, of course.  But subtract one pointer from
	// the other, and you will get ONE, not the size of the struct.
	//-------------------------------------------------------------------//
	struct TS
	{
		int a;
		double b;
		char* p;
	};
	
	TS t;
	TS *p;

	p = &t;

	CString strTemp;
	// strTemp.Format( _T("p=%x  p+1=%x p+4=%x\r\n"), p, (&*(p+1)), p+4);	// Same
	strTemp.Format( _T("p=%x  p+1=%x p+4=%x\r\n"), p, p+1, p+4);
	m_strOutput = strTemp;

	strTemp.Format( _T("p=%x  p+1=%x p+4=%x\r\n"), p, p+1-p, (p+4)-p);
	// strTemp.Format( _T("p=%x  p+1=%x p+4=%x\r\n"), p, (&*(p+1))-p, (p+4)-p);	// Same
	m_strOutput += strTemp;

	p++;
	strTemp.Format( _T("p++=%x\r\n"), p);
	m_strOutput += strTemp;

	TS* q;
	q = p + 1;
	strTemp.Format( _T("p=%x  q=%x  q-p=%x\r\n"), p, q, q - p);
	m_strOutput += strTemp;

	//-------------------------------------------------------------------//
	*/


	/*
	//-------------------------------------------------------------------//
	// a + b = c;
	// Gamini asked what the heck this means.
	// Bill and I decided that (a+b) returns a temp object that gets
	// assigned a value of c, then is discarded.  Gamini pointed
	// out that it works only if you dont make the return from the
	// operator+() into a const value.
	//
	// I want to test that out.
	//
	// Yep.  Works if not const return, otherwise, can't assign a
	// value to a const.  C++ is COOL.  :>
	//
	// NOTE: Doesn't work with integers, etc.
	//-------------------------------------------------------------------//
	class test
	{
	public:
		test operator+( test& testAdd )
		{
			test testResult;
			testResult.m_nValue += testAdd.m_nValue;
			return testResult;
		}

		int m_nValue;
	};

	test a, b, c;

	( a + b ) = c;
	//-------------------------------------------------------------------//
	*/


	/*
	//-------------------------------------------------------------------//
	// Looking for better printf handling...
	// Unfortunately vsprintf() cannot provide the required size until
	// AFTER it returns - and you can't pass in NULL for the target
	// buffer... rrr... so the only other answer is to provide a huge
	// static string buffer, do the vsprintf() there, then copy out
	// the result (knowing how big it is going to be...).  Problem
	// is, how big is "huge"?  too big wastes, and too small crashes...
	// Best just leave it to the caller - she's the one who knows 
	// the detailed space requirements...
	//-------------------------------------------------------------------//
	ustring strTest;
	strTest.printf( 1000, _T("Testing %d, %d, %d..."), 1, 2, 3 );

	m_strOutput = strTest.c_str();
	//-------------------------------------------------------------------//
	*/


	/*
	//-------------------------------------------------------------------//
	// enum size (and general class size) test 
	//-------------------------------------------------------------------//
	class A
	{
		enum Enum1
		{
			A1,
			A2
		};
	};
	class B
	{
		typedef enum 
		{
			B1,
			B2
		} Enum2;
	};
	class C
	{
		enum 
		{
			C1,
			C2
		} Enum3;
	};
	class D
	{
	};
	struct E
	{
	};
	class F
	{
	public:
		virtual ~F() {}
	};
	class G
	{
		enum Enum1
		{
			A1,
			A2
		};
		typedef enum 
		{
			B1,
			B2
		} Enum2;
		enum 
		{
			C1,
			C2
		} m_enum3;

		Enum1 m_e1;
		Enum2 m_e2;

		G()
		{
			// All roughly equivalent...
			// except you can't make new enum3's easily...
			m_e1 = A1;
			m_e2 = B1;
			m_enum3 = C1;
		}

		void fe1( Enum1 e1 ) {}
		void fe2( Enum2 e2 ) {}
		void fe3() { m_enum3 = C1; }
	};

	ustring strTest;
	strTest.printf( 1000, _T("Sizes of A, B, C, D, E, F, G: %d, %d, %d, %d, %d, %d, %d..."), sizeof(A), sizeof(B), sizeof(C), sizeof(D), sizeof(E), sizeof(F), sizeof(G) );

	m_strOutput = strTest.c_str();
	//-------------------------------------------------------------------//
	*/


	/*
	//-------------------------------------------------------------------//
	// Bit shifted table
	//-------------------------------------------------------------------//
	ustring strTest;
	unsigned int nValue;
	for ( int nA = 0; nA < 32; nA++ )
	{
		nValue = 1 << nA;

		ustring strTemp;
		strTemp.printf( 1000, _T("Shift: %3d Value: %12d\r\n"), nA, nValue );
		strTest += strTemp;
	}
	
	m_strOutput = strTest.c_str();
	//-------------------------------------------------------------------//
	*/


	//-------------------------------------------------------------------//
	// Maps and their requisite < operators.
	//
	// You need a < operator for maps (etc.) to sort.
	// You can't have a < operator for non-class types, which includes
	// integers, pointers, etc. - I was trying to use the pointers 
	// to dig down and use the class info, you can't do it - use a 
	// reference instead.
	//-------------------------------------------------------------------//

	// See all the stuff defined in the CONSTANTS block at top...

	//-------------------------------------------------------------------//


	/*
	//-------------------------------------------------------------------//
	// Inheritance and const.
	//
	//-------------------------------------------------------------------//

	A a;
	B b;

	// Doesn't compile.  Crazy!
	// b.foo() = 3;

	// Sets s_nA!
	b.A::foo() = 3;

	// Doesn't compile.  Crazy!
	// A* pA = &b;
	// b.foo() = pA->foo();

	// B* pB = &b;
	// b.A::foo() = pB->foo();

	m_strOutput.Format( _T("A: %d B: %d"), a.foo(), b.foo() );

	//-------------------------------------------------------------------//
	*/


	/*
	//-------------------------------------------------------------------//
	// Needed a little app to peg the CPU
	//-------------------------------------------------------------------//
	while ( true )
	{
		double dPi = acos(-1);
		CString strTemp;
		strTemp.Format( _T("Pi = %f\n"), dPi );
		m_strOutput += strTemp;
		UpdateData( FALSE );
		UpdateWindow();
	}
	//-------------------------------------------------------------------//
	*/


	/*
	//-------------------------------------------------------------------//
	// How to get the time between two events.
	//-------------------------------------------------------------------//
	static _timeb timeStart;
	_ftime( &timeStart );

	Sleep( 2000 );

	static _timeb timeEnd;
	_ftime( &timeEnd );
	int nTimeMS = CalcTimeSpanMS( timeStart, timeEnd );
	ustring strMsg;
	strMsg.printf( 1000, _T("PROFILE: %10.2f seconds for this operation..."), nTimeMS/1000.0 );

	m_strOutput = strMsg.c_str();
	//-------------------------------------------------------------------//
	*/


	/*
	//-------------------------------------------------------------------//
	// Default constructor not created if you have a non-default constructor.
	//-------------------------------------------------------------------//
   class A
   {
      A( A& ) {}
   };

   // Illegal!
   // A a;

	//-------------------------------------------------------------------//
   */


	/*
	//-------------------------------------------------------------------//
   // CFileDialog uses a struct that got changed from WinNT to Win2000.
	// And M$ does some Irish schenanigans with it that causes a crash
	// if you compile with VC6 and use [_WIN32_WINNT=0x0500].
	//
	// Add this to StdAfx.h, above all else, to get it to work in VC6.
	// It always seems to work in VS .net 2003 - WOW GO FIGURE I WONDER WHY.
	//
		// MDM NEED THIS **HACK** TO GET CFileDialog TO WORK IN WinXP FROM A VC6 BUILD
		// Put this in stdafx.h...
		//
		//		#undef _WIN32_WINNT
		//		#define _WIN32_WINNT 0x0400
		//		#include <afxdlgs.h>
		//		#undef _WIN32_WINNT
		//		#define _WIN32_WINNT 0x0500
   //
	//-------------------------------------------------------------------//

   {
      CFileDialog fd(
         TRUE,
         _T(".dat"), 
         _T(""),
		      OFN_HIDEREADONLY 
         |  OFN_OVERWRITEPROMPT 
         |  OFN_FILEMUSTEXIST
         |  OFN_LONGNAMES,
         _T("Layermap Files(*.dat)|*.dat|All Files (*.*)|*.*||")
      );
      // fd.m_ofn.lStructSize = sizeof(OPENFILENAME);
      // fd.m_ofn.pvReserved = 0;
      // fd.m_ofn.dwReserved = 0;

      fd.DoModal();

      // delete pfd;

   }
	*/


	/*
	//-------------------------------------------------------------------//
	// Why is my TransferredQuestion class having trouble with 
	// serialization?  Got it.
	//
	//		====================
	//		to serialize a class
	//		====================
	//			a) add the serialize block to the top private section of the class interface
	//			b) implement the serialize function
	//			c) make sure the class has a default constructor
	//				haven't figured out if there's a way around this
	//				yet but it seems harmless enough a requirement
	//		====================
	//
	//-------------------------------------------------------------------//
	TQContainer tq;	
	//-------------------------------------------------------------------//
	*/


	// SortedVectorTest();


	// BrowserTest();


	/*
	//-------------------------------------------------------------------//
	// Testing Gamini's Intel Math Kernel Library (MKL).
	//-------------------------------------------------------------------//
	// 1) Installed MKL from here: 
	//			E:\Michael's Data\Software Development\Intel Math Libraries
	// 2) Added pardiso_unsym_c.c to project, removed precompiled header, compiled (OK).
	//	3) Added lib dir to project: C:\Program Files\Intel\MKL\8.1.1\ia32\lib
	//	4) Added libs to project: mkl_c.lib mkl_solver.lib libguide.lib
	// 5) main() will call the sample code 
	//			- need to add [extern "C" int main();] to top of this file
	//			- call main(), set breakpoint and step in to follow thru code
	//
	//	Outputs to std output via printf, so you won't see anything.
	// 
	// Cleanup: 
	//      remove file pardiso_unsym_c.c from project
	//      remove libs from being included in project
	//          (mkl_solver.lib libguide.lib mkl_c.lib)
	//-------------------------------------------------------------------//
	
	main();

	//-------------------------------------------------------------------//
	*/


	//-------------------------------------------------------------------//
	// Scrap project updated to VC8!
	//-------------------------------------------------------------------//


	//-------------------------------------------------------------------//
	// CSliderCtrl
	//-------------------------------------------------------------------//
	// What is the best way to handle slider notification?
	// I want to handle it in the slider class, if at all possible.
	// Right now, the silly "standard" handling in HTDJ is in the
	// dialog's OnHScroll() and OnVScroll() members, and for some
	// reason the RTTI check in the dialog class is failing.
	//
	// OK I found the simple solution: static-cast before doing a
	// dynamic_cast<>.  Here's an example:
	// 
	//		DOES NOT WORK in VC8:	
	//			pSlider = dynamic_cast< ListDataByteSlider* > ( pScrollBar )
	//		DOES WORK:					
	//			pSlider = dynamic_cast< ListDataByteSlider* > ( (CSliderCtrl*)pScrollBar )
	//
	// I would still like to fix the sliders so when you click the 
	// mark goes right to the click point.  Do I need a scrap test?
	//
	// I did this:
	//		added a slider control IDC_SLIDER1
	//		made it a 
   //-------------------------------------------------------------------//


	/*
	//-------------------------------------------------------------------//
	// Heather: "if i have an abstract, templated class in C++, 
	// and i want to stash them all in an STL collection, 
	// do they all have to have the same template type?"
	//
	// Me: you can use a base class.
	//-------------------------------------------------------------------//
	
	// Here, you'll need to specify the type explicitly.
	std::vector<AbstractTemplateBase<int>> vABC;
	
	// Here, we use a base class of the abstract class.
	std::vector<BaseOfTemplate> vBOT;
	
	AbstractTemplateBase<int> atbInt;
	AbstractTemplateBase<double> atbDouble;
	vBOT.push_back( atbInt );
	vBOT.push_back( atbDouble );	

   //-------------------------------------------------------------------//
   */

	//-------------------------------------------------------------------//
	// Managed-code contained in unmanaged project?
	//-------------------------------------------------------------------//
	// Paul DiLasia covered this, let's give it a try...
	// The goal is to use .net to convert from Oracle to Access.
	// But why am I using an unmanaged C++ app?  I think I will just
	// go for a full-blown managed C++ app.
   //-------------------------------------------------------------------//


    /*
	//-------------------------------------------------------------------//
	// Heather needs to pass a pointer to a string pointer, so she
	// can deallocate and reallocate.
	//-------------------------------------------------------------------//
	
	char* szFearForYourLifeString = new char[20];
   strcpy( szFearForYourLifeString, "Don't hurt me" );	

	trashMyString( &szFearForYourLifeString );
	m_strOutput = szFearForYourLifeString;	
	
   //-------------------------------------------------------------------//
   */


	//-------------------------------------------------------------------//
    // Redirecting std::cout to go to a file or a string...
	//-------------------------------------------------------------------//

    // Create the file.
    std::filebuf f;
    f.open("myfile.txt", std::ios::out);

    // Create the string stream.
    // Note that we don't even need a string, the stream contains its own buffer.
    // NOTE that we could use to_string<T> to convert any non-string to a string.
    // But we don't need to, we have a stream and [<<], which does that work for us, whoop!
    std::ostringstream stream;

    // Store the current [cout] buffer, so we can restore when done.
    std::streambuf* buf = std::cout.rdbuf();

    // Assign the file as the cout buffer.
    std::cout.rdbuf(&f);

    std::cout << "Hello File!" <<std::endl;

    // Assign the stringstream as the cout buffer.
    std::cout.rdbuf(stream.rdbuf());

    std::cout << "Hello String!" <<std::endl;

    // Restore the standard cout buffer.
    std::cout.rdbuf(buf);

    // Where this goes, nobody knows!  :P
    std::cout << "Hello cout!" <<std::endl;

    // Copy the string output.
    m_strOutput = stream.str().c_str();

	//-------------------------------------------------------------------//


	//-------------------------------------------------------------------//
	// Ready for next scrap code...
	//-------------------------------------------------------------------//
   //-------------------------------------------------------------------//

	// Post results.
	UpdateData( FALSE );
	
}


//-------------------------------------------------------------------//
// ForceRefresh()																		//
//-------------------------------------------------------------------//
void CScrapDlg::ForceRefresh()
{
	UpdateData( FALSE );

	// Scroll the edit window to the bottom.
	int nLength = m_OutputEdit.GetWindowTextLength();
	m_OutputEdit.SetSel( nLength, nLength );

	Invalidate();
	RedrawWindow();
}


//-------------------------------------------------------------------//
// ociErr()																				//
//-------------------------------------------------------------------//
CString CScrapDlg::ociErr(int rc)
{
	static TCHAR temp[32];
	_tcscpy(temp, _T(" table: "));
	static TCHAR buf[512];
	if(OCI_ERROR==rc || OCI_INVALID_HANDLE==rc) {
		sb4 errcd;

		// NOTE: THIS IS FAILING EVEN WHEN WE GET GOOD INFO.
		if( 
			OCI_SUCCESS == OCIErrorGet(
				m_errhp,
				1,
				NULL,
				&errcd,
				(text*)buf,
				sizeof(buf),
				OCI_HTYPE_ERROR
			)
		) {
			
			if ( 1405 == errcd ) 
				// null data returned
				return CString(_T("")); 

			/*
			if ( _tcsclen(tableName() ) ) 
			{
				_tcsncpy(&buf[_tcsclen(buf)],temp,_tcsclen(temp)+1);
				_tcsncpy(&buf[_tcsclen(buf)],tableName(),_tcsclen(tableName())+1);
			}
			*/

			buf[sizeof(buf)-1] = _T('\0');
			return CString( buf );
		
		} else 
		{
			return CString( _T("Unknown error") );
		}
	
	} else if (OCI_SUCCESS_WITH_INFO == rc) 
	{
		sb4 errcde;
		if(OCI_SUCCESS==OCIErrorGet(m_errhp,1,NULL,&errcde,(text*)buf,sizeof(buf),OCI_HTYPE_ERROR)) 
		{
			_tcsncpy(&buf[_tcsclen(buf)],temp,_tcsclen(temp)+1);
			// _tcsncpy(&buf[_tcsclen(buf)],tableName(),_tcsclen(tableName())+1);
			buf[sizeof(buf)-1] = _T('\0');
			return CString( buf );
		} else {
			return CString( _T("Unknown error") );
		}
	}

	// Weird, we didn't return anything here and the compiler didn't complain!
	return CString( _T("") );
}


void CScrapDlg::SortedVectorTest()
{
	//-------------------------------------------------------------------//
	// Test new sorted_vector class (see STLContainers.h).
	// Works like a charm!  Here's the result:
	//
	//		10,000 items
	//		------------
	//		SORTED INSERT:			0.3130 seconds
	//		UNSORTED INSERT:     0.0470 seconds
	//		SORT:						0.0150 seconds
	// 
	//		100,000 items
	//		-------------
	//		UNSORTED INSERT:     0.4220 seconds
	//		SORT:						0.0780 seconds
	//		SORTED INSERT:		  24.8900 seconds
	// 
	//-------------------------------------------------------------------//

	_timeb timeStart, timeEnd;
	ustring strMsg;
	int nA;
	int nTimeMS;

	const int cnArraySize = 100000;
	sorted_vector<uLong> test;
	test.reserve( cnArraySize );


	// =========================
	// METHOD 1 sorted insertion

	test.clear();

	_ftime( &timeStart );

	for ( nA = 0; nA < cnArraySize; nA++ )
	{
		test.push_sorted( get_random_uLong() );
	}

	_ftime( &timeEnd );
	nTimeMS = (int)CalcTimeSpanMS( timeStart, timeEnd );
	strMsg.printf( 1000, _T("SORTED INSERT: %10.4f seconds\r\n"), nTimeMS/1000.0 );
	m_strOutput += strMsg.c_str();

	TraceVector( test );
	// =========================


	// =========================
	// METHOD 2 batch insertion + sort

	test.clear();

	_ftime( &timeStart );

	for ( nA = 0; nA < cnArraySize; nA++ )
	{
		test.push_unsorted( get_random_uLong() );
	}

	_ftime( &timeEnd );
	nTimeMS = (int)CalcTimeSpanMS( timeStart, timeEnd );
	strMsg.printf( 1000, _T("UNSORTED INSERT: %10.4f seconds\r\n"), nTimeMS/1000.0 );
	m_strOutput += strMsg.c_str();

	TraceVector( test );

	_ftime( &timeStart );

	test.sort();

	_ftime( &timeEnd );
	nTimeMS = (int)CalcTimeSpanMS( timeStart, timeEnd );
	strMsg.printf( 1000, _T("SORT: %10.4f seconds\r\n"), nTimeMS/1000.0 );
	m_strOutput += strMsg.c_str();

	TraceVector( test );
	// =========================

}


//-------------------------------------------------------------------//
// Programmatic browsing
// Comparison of WinInet (more client-friendly) and WinHTTP (newer).
// NOTE: I NEED TO POST MULTIPART FORM DATA!
// See http://msdn.microsoft.com/library/en-us/winhttp/http/porting_wininet_applications_to_winhttp.asp?frame=true
// I'm starting with MSDN samples for WinInet and WinHTTP.
//-------------------------------------------------------------------//
void CScrapDlg::BrowserTest()
{
	// Actually at this point i am dropping in a web browser activex control, 
	// requested by Jason.  But I'm not finding any decent interface functions yet...


}



