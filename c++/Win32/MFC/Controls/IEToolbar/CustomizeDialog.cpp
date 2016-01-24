//-------------------------------------------------------------------//
// CCustomizeDialog class
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#include "stdafx.h"
// #include "Resource.h"
#include "CustomizeDialog.h"
#include "..\..\GlobalData.h"
#include "ToolBarEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDialog dialog

COptionsDialog::COptionsDialog( ETextOptions eTextOptions,
                                EIconOptions eIconOptions )
:
	// Call base class.
	inherited( COptionsDialog::IDD )
{
    //{{AFX_DATA_INIT(COptionsDialog)
    //}}AFX_DATA_INIT

    m_eTextOptions = eTextOptions;
    m_eIconOptions = eIconOptions;
}


void COptionsDialog::DoDataExchange( CDataExchange* pDX )
{
    inherited::DoDataExchange( pDX );
    //{{AFX_DATA_MAP(COptionsDialog)
    DDX_Control(pDX, IDC_CB_TEXTOPTIONS, m_cbTextOptions);
    DDX_Control(pDX, IDC_CB_ICONOPTIONS, m_cbIconOptions);
    //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// Operations

bool COptionsDialog::SelectTextOption( ETextOptions eTextOptions )
{
    for ( int nIndex = 0; nIndex < m_cbTextOptions.GetCount(); nIndex++ )
    {
        if ( eTextOptions == ( ETextOptions )m_cbTextOptions.GetItemData( nIndex ) )
        {
            m_cbTextOptions.SetCurSel( nIndex );
            m_eTextOptions = eTextOptions;
            return true;
        }
    }

    return false;
}

bool COptionsDialog::SelectIconOption( EIconOptions eIconOptions )
{
    for ( int nIndex = 0; nIndex < m_cbIconOptions.GetCount(); nIndex++ )
    {
        if ( eIconOptions == ( EIconOptions )m_cbIconOptions.GetItemData( nIndex ) )
        {
            m_cbIconOptions.SetCurSel( nIndex );
            m_eIconOptions = eIconOptions;
            return true;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
// Implementation

CCustomizeDialog* COptionsDialog::GetCustomizeDialog() const
{
    return STATIC_DOWNCAST( CCustomizeDialog, GetParent() );
}

/////////////////////////////////////////////////////////////////////////////
// COptionsDialog message handlers

BEGIN_MESSAGE_MAP(COptionsDialog, CDialog)
    //{{AFX_MSG_MAP(COptionsDialog)
    ON_CBN_SELENDOK(IDC_CB_TEXTOPTIONS, OnTextOptions)
    ON_CBN_SELENDOK(IDC_CB_ICONOPTIONS, OnIconOptions)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL COptionsDialog::OnInitDialog() 
{
    CDialog::OnInitDialog();

    CCustomizeDialog* pCustomizeDialog = GetCustomizeDialog();
    pCustomizeDialog->AddTextOption( m_cbTextOptions, toTextLabels,   IDS_TO_TEXTLABELS );
    pCustomizeDialog->AddTextOption( m_cbTextOptions, toTextOnRight,  IDS_TO_TEXTONRIGHT );
    pCustomizeDialog->AddTextOption( m_cbTextOptions, toNoTextLabels, IDS_TO_NOTEXTLABELS );
    VERIFY( SelectTextOption( m_eTextOptions ) );

    pCustomizeDialog->AddIconOption( m_cbIconOptions, ioSmallIcons, IDS_IO_SMALLICONS );
    pCustomizeDialog->AddIconOption( m_cbIconOptions, ioLargeIcons, IDS_IO_LARGEICONS );
    VERIFY( SelectIconOption( m_eIconOptions ) );

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsDialog::OnTextOptions()
{
    int nSel = m_cbTextOptions.GetCurSel();
    ASSERT( nSel != CB_ERR );
    m_eTextOptions = ( ETextOptions )m_cbTextOptions.GetItemData( nSel );
    GetCustomizeDialog()->SetTextOptions( m_eTextOptions, false );
}

void COptionsDialog::OnIconOptions()
{
    int nSel = m_cbIconOptions.GetCurSel();
    ASSERT( nSel != CB_ERR );
    m_eIconOptions = ( EIconOptions )m_cbIconOptions.GetItemData( nSel );
    GetCustomizeDialog()->SetIconOptions( m_eIconOptions, false );
}

/////////////////////////////////////////////////////////////////////////////
// CCustomizeDialog dialog

static const int cxPadding = 3;
static const int cyPadding = 3;

IMPLEMENT_DYNAMIC( CCustomizeDialog, CWnd )

CCustomizeDialog::CCustomizeDialog( CToolBarEx* pToolBar )
    : m_dlgOptions( pToolBar->m_eTextOptions, pToolBar->m_eIconOptions )
{
    m_pToolBar = pToolBar;
}

/////////////////////////////////////////////////////////////////////////////
// Operations

void CCustomizeDialog::SetTextOptions( ETextOptions eTextOptions, bool bInDialog )
{
    if ( bInDialog )
    {
        VERIFY( m_dlgOptions.SelectTextOption( eTextOptions ) );
    }
    else
    {
        m_pToolBar->SetTextOptions( eTextOptions );
    }
}

void CCustomizeDialog::SetIconOptions( EIconOptions eIconOptions, bool bInDialog )
{
    if ( bInDialog )
    {
        VERIFY( m_dlgOptions.SelectIconOption( eIconOptions ) );
    }
    else
    {
        m_pToolBar->SetIconOptions( eIconOptions );
    }

    int nHeight = GetButtonSize().cy;

    CWnd* pWnd = GetDlgItem( IDC_LB_AVAILABLE );
    if ( pWnd != 0 )
    {
        pWnd->SendMessage( LB_SETITEMHEIGHT, 0, nHeight );
        pWnd->Invalidate();
    }

    pWnd = GetDlgItem( IDC_LB_CURRENT );
    if ( pWnd != 0 )
    {
        pWnd->SendMessage( LB_SETITEMHEIGHT, 0, nHeight );
        pWnd->Invalidate();
    }
}

/////////////////////////////////////////////////////////////////////////////
// Overrides

void CCustomizeDialog::PostNcDestroy()
{
    delete this;
}

/////////////////////////////////////////////////////////////////////////////
// Implementation

void CCustomizeDialog::AddTextOption( CComboBox& cbTextOptions, ETextOptions eTextOptions,
                                      UINT nStringID )
{
    if ( m_pToolBar->IsTextOptionAvailable( eTextOptions ) )
    {
        CString strText;
        VERIFY( strText.LoadString( nStringID ) );
        int nItem = cbTextOptions.AddString( strText );
        ASSERT( nItem >= 0 );
        cbTextOptions.SetItemData( nItem, ( DWORD )eTextOptions );
    }
}

void CCustomizeDialog::AddIconOption( CComboBox& cbIconOptions, EIconOptions eIconOptions,
                                      UINT nStringID )
{
    if ( m_pToolBar->IsIconOptionAvailable( eIconOptions ) )
    {
        CString strText;
        VERIFY( strText.LoadString( nStringID ) );
        int nItem = cbIconOptions.AddString( strText );
        ASSERT( nItem >= 0 );
        cbIconOptions.SetItemData( nItem, ( DWORD )eIconOptions );
    }
}

CSize CCustomizeDialog::GetButtonSize() const
{
    CSize szImage = 
			( m_pToolBar->GetIconOptions() == ioSmallIcons ) 
		?	m_pToolBar->GetSmallImageSize() 
		:	m_pToolBar->GetLargeImageSize();

    return szImage + CSize( cxPadding * 2, cyPadding * 2 );
}

/////////////////////////////////////////////////////////////////////////////
// CCustomizeDialog message handlers

BEGIN_MESSAGE_MAP(CCustomizeDialog, CWnd)
    //{{AFX_MSG_MAP(CCustomizeDialog)
    ON_WM_DRAWITEM()
    ON_WM_MEASUREITEM()
    ON_WM_CTLCOLOR()
    //}}AFX_MSG_MAP
    ON_MESSAGE( WM_INITDIALOG, OnInitDialog )
END_MESSAGE_MAP()

LRESULT CCustomizeDialog::OnInitDialog( WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
    CRect rcClient;
    GetClientRect( rcClient );
    CRect rcWindow;
    GetWindowRect( rcWindow );

    // Create Options dialog as a child window of Customize Toolbar dialog
    VERIFY( 
		m_dlgOptions.Create( 
			// COptionsDialog::IDD,			// MDM NOT USED for FontDlg
			this 
		) 
	);

    CRect rcDlg;
    m_dlgOptions.GetWindowRect( rcDlg );

    // Place Options dialog at the bottom
    VERIFY( m_dlgOptions.SetWindowPos( 0, 0, rcClient.Height(), rcClient.Width(), rcDlg.Height(),
        SWP_NOZORDER | SWP_SHOWWINDOW ) );

    // Increase height of Customize Toolbar dialog accordingly
    rcWindow.bottom += rcDlg.Height();
    VERIFY( SetWindowPos( 0, 0, 0, rcWindow.Width(), rcWindow.Height(),
        SWP_NOZORDER | SWP_NOMOVE ) );

    return Default();
}

void CCustomizeDialog::OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct )
{
    if ( lpDrawItemStruct->CtlType == ODT_LISTBOX )
    {
        if ( ( nIDCtl == IDC_LB_AVAILABLE ) || ( nIDCtl == IDC_LB_CURRENT ) )
        {
            CDC* pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
            int nSavedDC = pDC->SaveDC();
            ASSERT( nSavedDC );

            // Get all needed info about the item being drawn
            CRect rcItem( &lpDrawItemStruct->rcItem );
            int nItem  = ( SHORT )HIWORD( lpDrawItemStruct->itemData );
            int nImage = ( SHORT )LOWORD( lpDrawItemStruct->itemData );
            int nIndex = ( int )lpDrawItemStruct->itemID;
            bool bSelected  = ( lpDrawItemStruct->itemState & ODS_SELECTED ) != 0;
            bool bFocused   = ( lpDrawItemStruct->itemState & ODS_FOCUS    ) != 0;
            bool bSeparator = ( ( nItem < 0 ) || ( nItem > m_pToolBar->m_aButtons.GetUpperBound() ) );

            // Calculate rectangles for image, text and focus frame
            CSize szButton = GetButtonSize();
            CRect rcImage( rcItem );
            rcImage.right = rcImage.left + szButton.cx;
            CRect rcText( rcItem );
            rcText.left  += szButton.cx + 5;
            rcText.right -= 2;
            CRect rcLabel( rcItem );
            rcLabel.left += szButton.cx + 1;

            // Cache system colors
            COLORREF clrText     = ::GetSysColor( bSelected ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT );
            COLORREF clrBack     = ::GetSysColor( bSelected ? COLOR_HIGHLIGHT : COLOR_BTNFACE );
            COLORREF clrGrayText = ::GetSysColor( COLOR_GRAYTEXT );
            COLORREF clrBtnFace  = ::GetSysColor( COLOR_BTNFACE );
            COLORREF clr3dShadow = ::GetSysColor( COLOR_3DSHADOW );
            COLORREF clr3dHilite = ::GetSysColor( COLOR_3DHILIGHT );

            pDC->FillSolidRect( rcImage, clrBtnFace );
            pDC->FillSolidRect( rcLabel, clrBack );

            // Draw focus rect, if needed
            if ( bFocused )
            {
                CBrush brush( RGB(
                    255 - GetRValue( clrBack ),
                    255 - GetGValue( clrBack ),
                    255 - GetBValue( clrBack ) ) );
                rcLabel.DeflateRect( 2, 2 );
                pDC->FrameRect( rcLabel, &brush );
                VERIFY( brush.DeleteObject() );
            }

            // Get item text to be drawn
            CString strText;
            if ( !bSeparator )
            {
                UINT nID = m_pToolBar->m_aButtons[ nItem ].tbinfo.idCommand;
                m_pToolBar->GetButtonText( nID, strText );
            }
            else
            {
                VERIFY( strText.LoadString( IDS_SEPARATOR ) );
            }

            // Prepare to draw item text
            int nFormat = DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS;
            pDC->SetBkMode( TRANSPARENT );
            pDC->SetTextColor( clrText );

            if ( !bSeparator )
            {
                // Draw item image, if any
                if ( nImage >= 0 )
                {
                    if ( bSelected )
                    {
                        pDC->Draw3dRect( rcImage, clr3dHilite, clr3dShadow );
                    }

                    CToolBarCtrl& tbCtrl = m_pToolBar->GetToolBarCtrl();
                    CImageList* pImageList = tbCtrl.GetHotImageList();
                    if ( !bSelected || !pImageList )
                    {
                        pImageList = tbCtrl.GetImageList();
                    }

                    rcImage.DeflateRect( cxPadding, cyPadding );
                    
                    // MDM This used to be wrapped in a VERIFY(), which
                    // would cause endless assertions in debug mode following
                    // increases in the number of toolbar buttons.
                    pImageList->Draw( pDC, nImage, rcImage.TopLeft(), ILD_NORMAL | ILD_TRANSPARENT );
                }

                if ( m_pToolBar->m_aButtons[ nItem ].tbinfo.fsStyle & TBSTYLE_DROPDOWN )
                {
                    // This button is drop-down button.  To indicate this simple fact
                    // we draw an arrow on right like one that submenu items have.

                    CRect rcArrow( rcText );
                    rcArrow.left = rcArrow.right - ::GetSystemMetrics( SM_CXMENUCHECK );
                    rcText.right -= rcArrow.Width();

                    CBrush brush( clrText );
                    CBrush* pOldBrush = pDC->SelectObject( &brush );

                    CDC dc;
                    VERIFY( dc.CreateCompatibleDC( pDC ) );

                    CBitmap bmpArrow;
                    VERIFY( bmpArrow.CreateCompatibleBitmap( pDC, rcArrow.Width(), rcArrow.Height() ) );

                    CBitmap* pOldBitmap = dc.SelectObject( &bmpArrow );
                    CRect rc( 0, 0, rcArrow.Width(), rcArrow.Height() );
                    VERIFY( dc.DrawFrameControl( rc, DFC_MENU, DFCS_MENUARROW ) );
                    VERIFY( pDC->BitBlt( rcArrow.left, rcArrow.top, rcArrow.Width(), rcArrow.Height(),
                        &dc, 0, 0, ROP_PSDPxax ) );

                    dc.SelectObject( pOldBitmap );
                    VERIFY( bmpArrow.DeleteObject() );

                    pDC->SelectObject( pOldBrush );
                    VERIFY( brush.DeleteObject() );
                }
            }
            else
            {
                if ( ( nIDCtl == IDC_LB_CURRENT ) &&
                     ( nIndex == SendDlgItemMessage( nIDCtl, LB_GETCOUNT ) - 1 ) )
                {
                    // The last item in the list of current buttons is a separator
                    // which cannot be removed (guess what is the purpose of it?).
                    // To indicate this fact we draw it as disabled item.

                    if ( bSelected )
                    {
                        pDC->SetTextColor( clrGrayText );
                    }
                    else
                    {
                        CRect rcTemp( rcText );
                        rcTemp.OffsetRect( 1, 1 );
                        pDC->SetTextColor( clr3dHilite );
                        pDC->DrawText( strText, rcTemp, nFormat );
                        pDC->SetTextColor( clr3dShadow );
                    }
                }
                else
                {
                    // Draw the separator bar in the middle
                    rcImage.DeflateRect( cxPadding, cyPadding );
                    int y = ( rcImage.top + rcImage.bottom ) / 2;
                    rcImage.top    = y - 1;
                    rcImage.bottom = y + 1;

                    pDC->Draw3dRect( rcImage, clr3dShadow, clr3dHilite );
                }
            }

            pDC->DrawText( strText, rcText, nFormat );

            VERIFY( pDC->RestoreDC( nSavedDC ) );
            return;
        }
    }

    CWnd::OnDrawItem( nIDCtl, lpDrawItemStruct );
}

void CCustomizeDialog::OnMeasureItem( int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
    if ( lpMeasureItemStruct->CtlType == ODT_LISTBOX )
    {
        if ( ( nIDCtl == IDC_LB_AVAILABLE ) ||
             ( nIDCtl == IDC_LB_CURRENT ) )
        {
            lpMeasureItemStruct->itemHeight = GetButtonSize().cy;
            return;
        }
    }

    CWnd::OnMeasureItem( nIDCtl, lpMeasureItemStruct );
}

HBRUSH CCustomizeDialog::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
    HBRUSH hbr = CWnd::OnCtlColor( pDC, pWnd, nCtlColor );

    if ( nCtlColor == CTLCOLOR_LISTBOX )
    {
        UINT nIDCtl = ( UINT )pWnd->GetDlgCtrlID();
        if ( ( nIDCtl == IDC_LB_AVAILABLE ) ||
             ( nIDCtl == IDC_LB_CURRENT ) )
        {
            hbr = ::GetSysColorBrush( COLOR_BTNFACE );
        }
    }

    return hbr;
}
