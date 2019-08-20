
// SampleAsyncWorkDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "SampleAsyncWork.h"
#include "SampleAsyncWorkDlg.h"

#include "ProgressDlg.h"

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
	m_countListCtrl.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
	m_countListCtrl.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
	//
	//	デコーダが対応するフィルター一覧を作る
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
		CString filterStrings;
		m_imageFilter = _T( "All Image Files|" );
		bool firstTime = true;
		for( UINT index = 0 ; index < num ; ++index )
		{
			const auto& codec = pCodecInfo[index];
			//	ビットマップでロードするのでサポートしているものしか対応しない
			if( codec.Flags & Gdiplus::ImageCodecFlagsSupportBitmap )
			{
				filterStrings += _T( '|' );
				filterStrings += pCodecInfo[index].FormatDescription;
				filterStrings += _T( '|' );
				if( firstTime )
				{
					firstTime = false;
				}
				else
				{
					m_imageFilter += _T( ';' );
				}
				m_imageFilter += pCodecInfo[index].FilenameExtension;
				filterStrings += pCodecInfo[index].FilenameExtension;
			}
		}
		m_imageFilter += filterStrings;
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

//	サンプルの動作パターン

//	InsertItem を同時に行う
#define ExecMode_Sync_InsertItem	0b0000'0001
//	PumpMessage を自分で呼ぶ
#define ExecMode_Call_PumpMssage	0b0000'0010
//	ダイアログを出す
#define ExecMode_Disp_ProgressDlg	0b0000'0100
//	タスクを使う
#define ExecMode_Use_Task			0b0001'0000

// シングルスレッド：リストアップと同時に追加
#define CodeVer_Prototype ExecMode_Sync_InsertItem
// シングルスレッド：リストアップと追加を分離
#define CodeVer_SepInsert 0
// シングルスレッド：リストアップと同時に追加＆メッセージポンプ回し
#define CodeVer_SimplePump (ExecMode_Sync_InsertItem|ExecMode_Call_PumpMssage)
// シングルスレッド：モードレスでの処理中ダイアログ
#define CodeVer_ModelessDlg (ExecMode_Sync_InsertItem|ExecMode_Call_PumpMssage|ExecMode_Disp_ProgressDlg)
// マルチスレッド：モーダルでの処理中ダイアログ
#define CodeVer_ModalDlg (ExecMode_Sync_InsertItem|ExecMode_Disp_ProgressDlg|ExecMode_Use_Task)

//	実行バージョンのコード
//#define ExecVer CodeVer_Prototype
//#define ExecVer CodeVer_SepInsert
//#define ExecVer CodeVer_SimplePump
//#define ExecVer CodeVer_ModelessDlg
#define ExecVer CodeVer_ModalDlg


static void APIENTRY CountColors( CWnd* pParent, CListCtrl& lc, LPCTSTR imagePath, std::map<COLORREF, size_t>& numColors )
{
	//	InsertItem するときに使う情報(コールバックでテキスト表示するのでデータはLPARAMだけ)
	LVITEM item{};
	item.mask = LVIF_PARAM|LVIF_TEXT;
	//	テキストデータはその都度生成する(メモリイメージ省略のため)
	item.cchTextMax = 0;
	item.pszText = LPSTR_TEXTCALLBACK;

	//	メッセージポンプが動かない版
#if (ExecVer & (ExecMode_Call_PumpMssage|ExecMode_Disp_ProgressDlg)) == 0
	CWaitCursor wait;
#endif
#if ExecVer & ExecMode_Disp_ProgressDlg
	CProgressDlg dlg( pParent );
#if (ExecVer & ExecMode_Use_Task) == 0
	if( !dlg.Create() )	//	無効化
	{
		AfxThrowResourceException();	//	リソースありませんエラーでいいでしょう
	}
#endif
#endif

#if ExecVer & ExecMode_Use_Task
	auto task = concurrency::create_task( [&]()
#endif
	{
		//	初期化処理
		lc.DeleteAllItems();
		numColors.clear();
#if ExecVer & ExecMode_Call_PumpMssage
		CProgressDlg::PumpMessage();
#endif
		//	ファイルから読み込む
		Gdiplus::Bitmap bmp( imagePath );
		Gdiplus::BitmapData bmpData;
		if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
		{
			//const COLORREF* imageTop = static_cast<const COLORREF*>( bmpData.Scan0 );
			const BYTE* imageTop = static_cast<const BYTE*>(bmpData.Scan0);
			//	色データを個別計算するのは面倒なのでフルカラー画像で取り込むことにする
#if ExecVer & ExecMode_Disp_ProgressDlg
			dlg.SetRange( 0, bmpData.Height );
			dlg.SetStep( 1 );
#endif
			for( UINT yLine = 0 ; yLine < bmpData.Height ; yLine++ )
			{
				const COLORREF* lineTop = reinterpret_cast<const COLORREF*>(imageTop + bmpData.Stride * yLine);
#if ExecVer & ExecMode_Disp_ProgressDlg
				dlg.StepIt();
				if( dlg.IsCancel() )
				{
					break;
				}
#elif ExecVer & ExecMode_Call_PumpMssage
				if( !CProgressDlg::PumpMessage() )
				{
					break;	//	ここでリターンとかシャレにならないんだけど。。。
				}
#endif
#if (ExecVer & ExecMode_Use_Task) == 0
				CWaitCursor wait;
#endif
				for( UINT xPos = 0 ; xPos < bmpData.Width ; xPos++ )
				{
					auto itr = numColors.find( lineTop[xPos] );
					if( itr != numColors.end() )
					{
						itr->second += 1;
#if ExecVer & ExecMode_Sync_InsertItem
						LVFINDINFO findInfo;
						findInfo.flags = LVFI_PARAM;
						findInfo.lParam = lineTop[xPos];
						int findNum = lc.FindItem( &findInfo );
						_ASSERTE( findNum >= 0 );
						lc.Update( findNum );	//	更新する
#endif
					}
					else
					{
						numColors[lineTop[xPos]] = 1;
#if ExecVer & ExecMode_Sync_InsertItem
						item.lParam = lineTop[xPos];
						int index = lc.InsertItem( &item );	//	積極的に更新はしない
						if( index >= 0 )
						{
							item.iItem = index+1;
						}
#endif
					}
				}
			}
			bmp.UnlockBits( &bmpData );
		}
	}
#if ExecVer & ExecMode_Use_Task
	).then( [&]( )
	{
		dlg.ExitWork();
	} );
	dlg.DoModal();
	task.wait();	//	ここで同期化して終了待機してないと破綻する
#endif
#if (ExecVer & ExecMode_Sync_InsertItem) == 0
	lc.SetItemCount( static_cast<int>( numColors.size() ) );
	lc.SetRedraw( FALSE );
	for( const auto& numCol : numColors )
	{
		item.lParam = numCol.first;
		item.iItem = lc.InsertItem( &item );
		if( item.iItem >= 0 )
		{
			item.iItem++;
		}
		else
		{
			break;	//	色が追加できなくなった時点であきらめる
		}
	}
	lc.SetRedraw( TRUE );
#endif
	lc.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
	lc.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
	lc.Invalidate( TRUE );
#if (ExecVer & (ExecMode_Disp_ProgressDlg|ExecMode_Call_PumpMssage)) == (ExecMode_Disp_ProgressDlg|ExecMode_Call_PumpMssage)
	dlg.ExitWork();
#endif
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
	CountColors( this, m_countListCtrl, m_targetPath, m_numColors );
}
void CSampleAsyncWorkDlg::OnGetdispinfoListCount( NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if( pDispInfo->item.mask & LVIF_TEXT )
	{
		switch( pDispInfo->item.iSubItem )
		{
		case 0:	wsprintf( pDispInfo->item.pszText, _T( "0x%08X" ), pDispInfo->item.lParam );	break;
		case 1:	wsprintf( pDispInfo->item.pszText, _T( "%u64" ), m_numColors[static_cast<COLORREF>( pDispInfo->item.lParam) ] );	break;
		}
	}
	*pResult = 0;
}
