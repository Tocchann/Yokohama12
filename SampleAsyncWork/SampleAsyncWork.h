
// SampleAsyncWork.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once
#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif
#include "resource.h"		// メイン シンボル

//	サンプルの動作パターン

//	InsertItem を別にする
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
