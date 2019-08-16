
// SampleAsyncWork.cpp : アプリケーションのクラス動作を定義します。
//

#include "pch.h"
#include "framework.h"
#include "SampleAsyncWork.h"
#include "SampleAsyncWorkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSampleAsyncWorkApp

BEGIN_MESSAGE_MAP(CSampleAsyncWorkApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSampleAsyncWorkApp の構築

CSampleAsyncWorkApp::CSampleAsyncWorkApp()
{
	// 再起動マネージャーをサポートします
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}


// 唯一の CSampleAsyncWorkApp オブジェクト

CSampleAsyncWorkApp theApp;

static ULONG_PTR APIENTRY InitGDIPlus()
{
	ULONG_PTR	gdipToken = 0;
	Gdiplus::GdiplusStartupInput input;
	Gdiplus::GdiplusStartupOutput output;
	if( Gdiplus::GdiplusStartup( &gdipToken, &input, &output ) != Gdiplus::Ok )
	{
		gdipToken = 0;
	}
	return gdipToken;
}
static void APIENTRY ReleaseGDIPlus( ULONG_PTR gdipToken )
{
	if( gdipToken != 0 )
	{
		Gdiplus::GdiplusShutdown( gdipToken );
	}
}

// CSampleAsyncWorkApp の初期化

BOOL CSampleAsyncWorkApp::InitInstance()
{
	// アプリケーション マニフェストが visual スタイルを有効にするために、
	// ComCtl32.dll Version 6 以降の使用を指定する場合は、
	// Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
	// これを設定します。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	SetRegistryKey( _T("Wankuma") );
	auto gdipToken = InitGDIPlus();
	{
		CSampleAsyncWorkDlg dlg;
		m_pMainWnd = &dlg;
		dlg.DoModal();
		m_pMainWnd = nullptr;
	}
	ReleaseGDIPlus( gdipToken );

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// ダイアログは閉じられました。アプリケーションのメッセージ ポンプを開始しないで
	//  アプリケーションを終了するために FALSE を返してください。
	return FALSE;
}

