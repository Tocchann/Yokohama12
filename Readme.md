# わんくま同盟 横浜勉強会 #12

## 同期処理を非同期化しよう！for C++

参考資料:[Codezine連載「Windowsアプリケーションで「処理中」を表現する」](https://codezine.jp/article/corner/384)

~~~cpp
// ファイルのバイトデータの出現数を数える関数
bool CountCharInFile(
    class CProgressDlg& dlg,
    LPCTSTR filePath,
    std::map<char,size_t>& numbers );
~~~
古のUIスレッドオンリーパターン(抜粋)
~~~cpp
// CString m_targetPath;
CFileDialog dlg( TRUE, nullptr, m_targetPath,
                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                _T("すべてのファイル|*.*||"), this );
if( dlg.DoModal() != IDOK )
{
    return;
}
m_targetPath = dlg.GetPathName();
CProgressDlg dlg;
dlg.Create();
// std::map<char,size_t> m_numbers;
if( CountCharInFile( dlg, m_targetPath, m_numbers ) )
{
    // 画面を更新
}
else
{
    // キャンセル処理
}
dlg.CloseWindow();
~~~
タスクを使った非同期処理
~~~cpp
// CString m_targetPath;
CFileDialog dlg( TRUE, nullptr, m_targetPath,
                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                _T("すべてのファイル|*.*||"), this );
if( dlg.DoModal() != IDOK )
{
    return;
}
m_targetPath = dlg.GetPathName();
CProgressDlg dlg;
auto task = concurrency::create_task( [&]()
{
    return CountCharInFile( dlg, m_targetPath, m_numbers );
} ).then( [&]( bool result )
{
    dlg.PostMessage( WM_CLOSE );
    return result;
} );
dlg.DoModal();
if( task.get() )
{
    // 画面更新
}
~~~
