
// SampleAsyncWorkDlg.h : ヘッダー ファイル
//

#pragma once


// CSampleAsyncWorkDlg ダイアログ
class CSampleAsyncWorkDlg : public CDialog
{
// コンストラクション
public:
	CSampleAsyncWorkDlg(CWnd* pParent = nullptr);	// 標準コンストラクター

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SAMPLEASYNCWORK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


// 実装
protected:
	HICON m_hIcon;
	std::map<Gdiplus::ARGB, size_t> m_numColors;
	CString m_imageFilter;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	CString m_targetPath;
	CListCtrl m_countListCtrl;
	virtual void OnOK();
	afx_msg void OnClickedButtonSelTargetpath();
	afx_msg void OnGetdispinfoListCount( NMHDR* pNMHDR, LRESULT* pResult );
};
