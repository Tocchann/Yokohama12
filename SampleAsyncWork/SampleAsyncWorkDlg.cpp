
// SampleAsyncWorkDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "SampleAsyncWork.h"
#include "SampleAsyncWorkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif
};
CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}


// CSampleAsyncWorkDlg ダイアログ



CSampleAsyncWorkDlg::CSampleAsyncWorkDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SAMPLEASYNCWORK_DIALOG, pParent)
	, m_targetPath( _T( "" ) )
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
void CSampleAsyncWorkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT_TARGETPATH, m_targetPath );
	DDX_Control( pDX, IDC_LIST_COUNT, m_countListCtrl );
}

BEGIN_MESSAGE_MAP(CSampleAsyncWorkDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED( IDC_BUTTON_SEL_TARGETPATH, &CSampleAsyncWorkDlg::OnClickedButtonSelTargetpath )
	ON_NOTIFY( LVN_GETDISPINFO, IDC_LIST_COUNT, &CSampleAsyncWorkDlg::OnGetdispinfoListCount )
END_MESSAGE_MAP()

// CSampleAsyncWorkDlg メッセージ ハンドラー
void CSampleAsyncWorkDlg::OnSysCommand( UINT nID, LPARAM lParam )
{
	if( (nID & 0xFFF0) == IDM_ABOUTBOX )
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand( nID, lParam );
	}
}
BOOL CSampleAsyncWorkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	//	エディットにオートコンプリートをつける
	SHAutoComplete( *GetDlgItem( IDC_EDIT_TARGETPATH ), SHACF_FILESYS_ONLY );
	//	リストのカラムをセットする
	m_countListCtrl.SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_AUTOSIZECOLUMNS );
	m_countListCtrl.InsertColumn( 0, _T( "色" ), LVCFMT_RIGHT );
	m_countListCtrl.InsertColumn( 1, _T( "出現数" ), LVCFMT_RIGHT );
	//
	//	デコーダが対応するフィルター一覧を作る(GDI+が対応するものは全部...なんだけど、なぜメタ系まで取り出せてしまうのか。。。)
	//
	{
		m_imageFilter.Empty();
		UINT	num, size;
		auto result = Gdiplus::GetImageDecodersSize( &num, &size );
		if( result != Gdiplus::Ok )
		{
			CString msg;
			msg.Format( _T( "GDI+ のエラー:%d" ), result );
			AfxMessageBox( msg );
			EndDialog( IDABORT );
			return TRUE;
		}
		auto pCodecInfo = static_cast<Gdiplus::ImageCodecInfo*>( malloc( size ) );
		if( pCodecInfo == nullptr )
		{
			EndDialog( IDABORT );
			AfxThrowMemoryException();
		}
		Gdiplus::GetImageDecoders( num, size, pCodecInfo );
		CString allExts;
		for( UINT index = 0 ; index < num ; ++index )
		{
			const auto& codec = pCodecInfo[index];
			//	ビットマップでロードするのでサポートしているものしか対応しない
			if( codec.Flags & Gdiplus::ImageCodecFlagsSupportBitmap )
			{
				m_imageFilter += pCodecInfo[index].FormatDescription;
				m_imageFilter += _T( '|' );
				if( !allExts.IsEmpty() )
				{
					allExts += _T( ';' );
				}
				allExts += pCodecInfo[index].FilenameExtension;
				m_imageFilter += pCodecInfo[index].FilenameExtension;
				m_imageFilter += _T( '|' );
			}
		}
		m_imageFilter += _T( "All Image Files|" );
		m_imageFilter += allExts;
		m_imageFilter += _T( "||" );
		free( pCodecInfo );
	}
	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}
void CSampleAsyncWorkDlg::OnClickedButtonSelTargetpath()
{
	if( !UpdateData() )
	{
		return;
	}
	CFileDialog dlg( TRUE, nullptr, m_targetPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, m_imageFilter );
	if( dlg.DoModal() == IDOK )
	{
		m_targetPath = dlg.GetPathName();
		UpdateData( FALSE );
	}
}
static void APIENTRY CountColors( CListCtrl& lc, LPCTSTR imagePath, std::map<COLORREF, size_t>& numColors )
{
	CWaitCursor wait;
	//	初期化処理
	lc.DeleteAllItems();
	numColors.clear();

	LVITEM item{};
	item.mask = LVIF_PARAM|LVIF_TEXT;
	item.cchTextMax = 0;
	item.pszText = LPSTR_TEXTCALLBACK;	//	テキストデータはその都度生成する(メモリイメージ省略のため)

	//	ファイルから読み込む(できればフルカラーがいいけどどうなってるかわかんねーんだよなぁ。。。)
	Gdiplus::Bitmap bmp( imagePath );

	Gdiplus::BitmapData bmpData;
	if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
	{
		//const COLORREF* imageTop = static_cast<const COLORREF*>( bmpData.Scan0 );
		const BYTE* imageTop = static_cast<const BYTE*>(bmpData.Scan0);
		//	色データを個別計算するのは面倒なのでフルカラー画像で取り込むことにする
		//	色数の取得処理そのものは並列化可能ただしカウンタを並列化するといろいろ面倒なことになるので今回はやらない
		for( UINT yLine = 0 ; yLine < bmpData.Height ; yLine++ )
		{
			const COLORREF* lineTop = reinterpret_cast<const COLORREF*>( imageTop + bmpData.Stride * yLine );
			for( UINT xPos = 0 ; xPos < bmpData.Width ; xPos++ )
			{
				auto itr = numColors.find( lineTop[xPos] );
				int index = -1;
				if( itr != numColors.end() )
				{
					itr->second += 1;
				}
				else
				{
					numColors[lineTop[xPos]] = 1;
					item.lParam = lineTop[xPos];
					lc.InsertItem( &item );	//	積極的に更新はしない
				}
			}
		}
		bmp.UnlockBits( &bmpData );
	}
	lc.SetColumnWidth( 0, LVSCW_AUTOSIZE );
	lc.SetColumnWidth( 1, LVSCW_AUTOSIZE );
	lc.Invalidate( TRUE );
}
void CSampleAsyncWorkDlg::OnOK()
{
	if( !UpdateData() )
	{
		return;
	}
	if( m_targetPath.IsEmpty() )
	{
		AfxMessageBox( _T("調査対象ファイルを指定してください") );
		return;
	}
	if( PathFileExists( m_targetPath ) == FALSE )
	{
		AfxMessageBox( m_targetPath + _T( "\n\nファイルが見つかりません。" ) );
		return;
	}
	CountColors( m_countListCtrl, m_targetPath, m_numColors );
}
void CSampleAsyncWorkDlg::OnGetdispinfoListCount( NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if( pDispInfo->item.mask & LVIF_TEXT )
	{
		switch( pDispInfo->item.iSubItem )
		{
		case 0:	wsprintf( pDispInfo->item.pszText, _T( "0x%08X" ), pDispInfo->item.lParam );	break;
		case 1:	wsprintf( pDispInfo->item.pszText, _T( "0x%08X" ), m_numColors[pDispInfo->item.lParam] );	break;
		}
	}
	*pResult = 0;
}
