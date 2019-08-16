# わんくま同盟 横浜勉強会 #12

## 同期処理を非同期化しよう！for C++

参考資料:[Codezine連載「Windowsアプリケーションで「処理中」を表現する」](https://codezine.jp/article/corner/384)

~~~cpp
// ファイルのバイトデータの出現数を数える関数(内容は全く変わりません)
bool CountCharInFile(
    class CProgressDlg& dlg,
    LPCTSTR filePath,
    std::map<char,size_t>& numbers );
~~~
古のUIスレッドオンリーパターン(抜粋)
~~~cpp
void CSampleDlg::OnOK()
{
    // いろいろ前処理
    CProgressDlg dlg;
    dlg.Create();
    // std::map<char,size_t> m_numbers;
    if( CountCharInFile( dlg, m_targetPath, m_numbers ) )
    {
        // 画面を更新
    }
    dlg.CloseWindow();
}
~~~
タスクを使った非同期処理
~~~cpp
void CSampleDlg::OnOK()
{
    // いろいろ前処理
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
}
~~~
移行のキモは CProgressDlg の実装にあったりします。そこは当日のお楽しみ？ｗ
