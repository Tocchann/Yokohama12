#pragma once


// CProgressDlg ダイアログ
#include <atomic>
#include <ppl.h>
#include <concurrent_queue.h>

class CProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CProgressDlg)

public:

	CProgressDlg( _In_ CWnd* pParent = nullptr );   // 標準コンストラクター
	virtual ~CProgressDlg();
// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROGRESS };
#endif
	BOOL Create( _In_ CWnd* pParent );
	virtual INT_PTR DoModal();
	void ExitWork();
	//	キャンセルチェック
	bool IsCancel();

	concurrency::cancellation_token GetCancelToken() const
	{
		return m_cts.get_token();
	}

	//	プログレスバーのラップ
	void SetMarquee( _In_ bool fMarqueeMode );
	void SetRange( _In_ int nLower, _In_ int nUpper );
	int SetPos( _In_ int nPos );
	int OffsetPos( _In_ int nPos );
	int SetStep( _In_ int nStep );
	int StepIt();
private:
	bool m_modalMode;
	int m_lower;
	int m_upper;
	int m_step;
	int m_pos;
	bool m_marqueeMode;
	concurrency::cancellation_token_source m_cts;
	concurrency::event m_exitWork;
	//	モードレスダイアログ作成で参照されるオーナーウィンドウのハンドル
	CWnd* m_pOwnerWnd;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_progress;
	virtual BOOL OnInitDialog();
	afx_msg void OnClickedButtonCancel();
};
