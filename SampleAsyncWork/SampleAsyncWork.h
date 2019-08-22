
// SampleAsyncWork.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once
#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif
#include "resource.h"		// メイン シンボル

// CSampleAsyncWorkApp:
// このクラスの実装については、SampleAsyncWork.cpp を参照してください
//
class CSampleAsyncWorkApp : public CWinApp
{
public:
	static BOOL DoEvents();


	CSampleAsyncWorkApp();
// オーバーライド
public:
	virtual BOOL InitInstance();
// 実装
	DECLARE_MESSAGE_MAP()
};
extern CSampleAsyncWorkApp theApp;
