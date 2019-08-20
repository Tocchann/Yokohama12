
// SampleAsyncWork.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once
#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif
#include "resource.h"		// メイン シンボル

//	サンプルの動作パターン

// シングルスレッド：リストアップと同時に追加
#define CodeVer_Prototype 0
// シングルスレッド：リストアップと追加を分離
#define CodeVer_SepInsert 1
// シングルスレッド：リストアップと同時に追加＆メッセージポンプ回し
#define CodeVer_SimplePump 2
// シングルスレッド：モードレスでの処理中ダイアログ
#define CodeVer_ModelessDlg 3
// マルチスレッド：モーダルでの処理中ダイアログ
#define CodeVer_ModalDlg 4
// マルチスレッド：モーダルでの処理中ダイアログ＋計測処理の並列化
#define CodeVer_Parallel 5

//	実行バージョンのコード
#define ExecVer CodeVer_ModelessDlg

// CSampleAsyncWorkApp:
// このクラスの実装については、SampleAsyncWork.cpp を参照してください
//
class CSampleAsyncWorkApp : public CWinApp
{
public:
	CSampleAsyncWorkApp();
// オーバーライド
public:
	virtual BOOL InitInstance();
// 実装
	DECLARE_MESSAGE_MAP()
};
extern CSampleAsyncWorkApp theApp;
