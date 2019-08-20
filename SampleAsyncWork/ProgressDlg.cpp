// ProgressDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "SampleAsyncWork.h"
#include "ProgressDlg.h"


// MFCの機能を100%活用するメッセージポンプ
BOOL CProgressDlg::PumpMessage()
{
	MSG msg;
	while( ::PeekMessage( &msg, nullptr, 0, 0, PM_NOREMOVE ) )
	{
		// AfxPumpMessage() は、内部で GetMessage でメッセージを待ってしまうためそのまま使うと都合が悪い
		if( !AfxPumpMessage() )
		{
			return FALSE;	// WM_QUITが来た(やばいんだけど。。。ｗ)
		}
	}
	return TRUE;
}

// CProgressDlg ダイアログ
IMPLEMENT_DYNAMIC(CProgressDlg, CDialog)

CProgressDlg::CProgressDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_PROGRESS, pParent)
	, m_modalMode( false )
	, m_lower( -1 )
	, m_upper( -1 )
	, m_step( 0 )
	, m_marqueeMode( true )
	, m_pos( 0 )
{
	_ASSERTE( m_pParentWnd != nullptr );	//	親ウィンドウは強制で指定してもらう
	m_exitWork.reset();
}
CProgressDlg::~CProgressDlg()
{
	if( !m_modalMode )
	{
		PumpMessage();
		m_pParentWnd->EnableWindow( TRUE );
	}
}
void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_PROGRESS, m_progress );
}
BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	ON_BN_CLICKED( IDC_BUTTON_CANCEL, &CProgressDlg::OnClickedButtonCancel )
END_MESSAGE_MAP()

BOOL CProgressDlg::Create()
{
	_ASSERTE( m_pParentWnd != nullptr && m_pParentWnd->m_hWnd != nullptr && ::IsWindow( *m_pParentWnd ) );
	m_pParentWnd->EnableWindow( FALSE );
	auto result = __super::Create( IDD_PROGRESS, m_pParentWnd );
	if( result )
	{
		ShowWindow( SW_SHOW );
		UpdateWindow();
	}
	else
	{
		m_pParentWnd->EnableWindow( TRUE );
	}
	m_modalMode = false;
	return result;
}
INT_PTR CProgressDlg::DoModal()
{
	m_modalMode = true;

	return __super::DoModal();
}
void CProgressDlg::ExitWork()
{
	m_exitWork.set();
	if( m_progress.m_hWnd != nullptr )
	{
		PostMessage( WM_CLOSE );
	}
}
bool CProgressDlg::IsCancel()
{
	if( !m_modalMode )
	{
		if( m_progress.m_hWnd != nullptr )
		{
			//	ポンプが FALSE を返す == WM_QUIT 何だが。。。
			if( !PumpMessage() )
			{
				return true;
			}
		}
	}
	//	その場で使い捨てしてもトークン生成コストは高くない
	return m_cts.get_token().is_canceled();
}
void CProgressDlg::SetMarquee( _In_ bool fMarqueeMode )
{
	m_marqueeMode = fMarqueeMode;
	if( m_progress.m_hWnd != nullptr )
	{
		if( m_marqueeMode )
		{
			m_progress.ModifyStyle( 0, PBS_MARQUEE );
			m_progress.SetMarquee( fMarqueeMode, 0 );
		}
		else
		{
			m_progress.SetMarquee( fMarqueeMode, 0 );
			m_progress.ModifyStyle( PBS_MARQUEE, 0 );
		}
	}
}
void CProgressDlg::SetRange( _In_ int nLower, _In_ int nUpper )
{
	if( nLower < 0 || nUpper < 0 )
	{
		return;	//	マイナス設定なら何もしない
	}
	m_lower = nLower;
	m_upper = nUpper;
	if( m_progress.m_hWnd != nullptr )
	{
		if( m_progress.GetStyle() & PBS_MARQUEE )
		{
			SetMarquee( false );	//	マーキーモードの解除
		}
		m_progress.SetRange32( nLower, nUpper );
	}
}
int CProgressDlg::SetPos( _In_ int nPos )
{
	int result = m_pos;
	m_pos = nPos;
	if( m_progress.m_hWnd != nullptr )
	{
		return m_progress.SetPos( nPos );
	}
	return result;
}
int CProgressDlg::OffsetPos( _In_ int nPos )
{
	int result = m_pos;
	m_pos += nPos;
	if( m_progress.m_hWnd != nullptr )
	{
		return m_progress.OffsetPos( nPos );
	}
	return result;
}
int CProgressDlg::SetStep( _In_ int nStep )
{
	if( nStep == 0 )
	{
		return m_step;
	}
	int result = m_step;
	m_step = nStep;
	if( m_progress.m_hWnd != nullptr )
	{
		return m_progress.SetStep( nStep );
	}
	return result;
}
int CProgressDlg::StepIt()
{
	int result = m_pos;
	m_pos += m_step;
	if( m_progress.m_hWnd != nullptr )
	{
		return m_progress.StepIt();
	}
	return result;
}

// CProgressDlg メッセージ ハンドラー
BOOL CProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//	マーキーモードまたはインジケータの範囲あ未設定ならマーキーモード
	if( m_marqueeMode || (m_lower < 0 && m_upper < 0) )
	{
		SetMarquee( true );
	}
	else
	{
		SetRange( m_lower, m_upper );
		SetStep( m_step );
		SetPos( m_pos );
	}
	return TRUE;
}
void CProgressDlg::OnClickedButtonCancel()
{
	GetDlgItem( IDC_BUTTON_CANCEL )->EnableWindow( FALSE );	//	状態遷移したことを見せておく(本当はこの後ダイアログ内はWaitCursor にするといい感じになる)
	//	キャンセルを行うので通知をセット
	m_cts.cancel();
}
