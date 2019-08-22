# わんくま同盟 横浜勉強会 #12

## 同期処理を非同期化しよう！for C++

Github リポジトリ： https://github.com/tocchann/yokohama12

参考資料:[Codezine連載「Windowsアプリケーションで「処理中」を表現する」](https://codezine.jp/article/corner/384)

当初は処理の外側にダイアログを置こうかな？と思ったんですが、内容的に外側にループがもてないので、あきらめましたｗ

ということで、実務にありがちな超ベタ書きコードです。これくらいの実装なら普通にあるでしょ？といっても過言ではないレベル。

## アジェンダ

* 応答なしになる理由
* 応答なしにならないようにするには
* 同期処理を非同期にしよう！

## 応答なしになる理由

#### 前提条件

Windows は各プロセスがメッセージを処理しあいながら、協調して動作するように作られている。

#### 結論

メッセージが処理されないままの状態が一定時間経過すると応答していないと判断される。

## 応答なしにならないようにするには

メッセージを処理すればいい。

### メッセージを処理するには？

アプリケーションで、メッセージ処理関数を呼び出すことで実現

多くのフレームワークが、アプリケーションクラスの Run() メソッドがメッセージループを持つ。  
また、モーダルダイアログ(MFCならDoModal, .NET なら、ShowDialog)も内部で独自のメッセージループを回している(APIも同様)

#### 最もシンプルなメッセージループの例

~~~cpp
MSG msg;
while( GetMessage( &msg, nullptr, 0, 0 ) )
{
  TranslateMessage( &msg );
  DispatchMessage( &msg );
}
~~~

### アプリ内で一時的にメッセージループを回す場合は？

.NET でいえば、DoEvents() を呼び出すことで強制的にメッセージを処理することが可能。  
MFCにも類似の AfxPumpMessage というグローバル関数があるがちょっと癖があるので、ラップ関数を作るのが一般的

#### MFC 版 Appliation::DoEvents もどき

以下の特性があるのでラップ関数を作って対応する

MFCには実際のメッセージ処理を行ってくれる `AfxPumpMessage()` というまんまの名前のグローバル関数がある。  
ただし、`GetMessage()` でメッセージを待ってしまうため、メッセージがないときに呼び出すと帰ってこない。  
回避策として、呼び出し前にメッセージがあるかを確認することで呼び出し判定を行うことができる

~~~cpp
BOOL CSampleAsyncWorkApp::DoEvents()
{
  MSG msg;
  while( ::PeekMessage( &msg, nullptr, 0, 0, PM_NOREMOVE ) )
  {
    if( !AfxPumpMessage() )
    {
      return FALSE;
    }
  }
  return TRUE;
}
~~~

## 同期処理を非同期にしよう！

ここからは実際のコードを見たいと思います。

### 本質じゃない部分

* ダイアログアプリになっているので、WM_QUIT が処理されない＜ここ重要
* LVN_GETDISPINFO を使ってリストコントロールのメモリ消費量を低減
* 画像処理は GDI+ で対応
* 色数情報は、32bit カラーなので Gdiplus::ARGB を利用

### 管理してるデータ

画像の色とその出現数があればいい  
-> `std::map<Gdiplus::ARGB,size_t>` でデータ管理。

### 画面表示部分

~~~cpp
void CSampleAsyncWorkDlg::OnGetdispinfoListCount( NMHDR* pNMHDR, LRESULT* pResult )
{
  NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
  if( pDispInfo->item.mask & LVIF_TEXT )
  {
    Gdiplus::Color color( static_cast<Gdiplus::ARGB>(pDispInfo->item.lParam) );
    switch( pDispInfo->item.iSubItem )
    {
      case 0:
        swprintf_s( pDispInfo->item.pszText, pDispInfo->item.cchTextMax,
                    _T( "%02X-%02X-%02X-%02X" ),
                    color.GetA(), color.GetR(), color.GetG(), color.GetB() )
        break;
      case 1:
        swprintf_s( pDispInfo->item.pszText, pDispInfo->item.cchTextMax,
                    _T( "%zu" ), m_numColors[color.GetValue()] );
        break;
    }
  }
  *pResult = 0;
}
~~~
### 画像解析部分

CountColors というスタティック関数で実装

ここが時間のかかる処理のコア部分  
時間のかかる処理は大きく2つのループがある  
一つは画像の画素そのものを読み取るところ  
もう一つはリストにアイテムを追加するところ

##### プロトタイプ(CodeVer_Prototype)

シンプルな実装  
画素の読み取りと、リストへの追加をそれぞれ別々に行っている。  
(ちなみに初期パラメータには、CWnd* pParent はない)

~~~cpp
static void APIENTRY CountColors( CWnd* pParent, CListCtrl& lc, LPCTSTR imagePath, std::map<Gdiplus::ARGB, size_t>& numColors )
{
  // InsertItem するときに使う情報(コールバックでテキスト表示するのでデータはLPARAMだけ)
  LVITEM item{};
  item.mask = LVIF_PARAM|LVIF_TEXT;
  // テキストデータはその都度生成する(メモリイメージ省略のため)
  item.cchTextMax = 0;
  item.pszText = LPSTR_TEXTCALLBACK;

  // メッセージポンプが動かない版 WM_SETCURSOR されるとマウスカーソルが戻るためメッセージポンプが動く場合はセットしない
  CWaitCursor wait;
  {
    // 初期化処理
    lc.DeleteAllItems();
    numColors.clear();
    // ファイルから読み込む
    Gdiplus::Bitmap bmp( imagePath );
    Gdiplus::BitmapData bmpData;
    if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
    {
      const BYTE* imageTop = static_cast<const BYTE*>(bmpData.Scan0);
      // 画像から色情報をピックアップして、map<Gdiplus::ARGB,size_t> に格納する
      for( UINT yLine = 0 ; yLine < bmpData.Height ; yLine++ )
      {
        const Gdiplus::ARGB* lineTop = reinterpret_cast<const Gdiplus::ARGB*>(imageTop + bmpData.Stride * yLine);
        for( UINT xPos = 0 ; xPos < bmpData.Width ; xPos++ )
        {
          auto itr = numColors.find( lineTop[xPos] );
          if( itr != numColors.end() )
          {
            itr->second += 1;
          }
          else
          {
            numColors[lineTop[xPos]] = 1;
          }
        }
      }
      bmp.UnlockBits( &bmpData );
    }
  }
  // 取り込んだ色情報をリストコントロールに一括セットする
  lc.SetItemCount( static_cast<int>(numColors.size()) );
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
      break;  // 色が追加できなくなった時点であきらめる
    }
  }
  lc.SetRedraw( TRUE );
  lc.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
  lc.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
  lc.Invalidate( TRUE );
}
~~~

#### リストへの追加も同時に処理するように変更(CodeVer_SyncInsertItem)

ちょっとでかいと何やってるかよくわからないから改良しよう！  
リストに追加を同時にやれば動きが見えるからわかりやすいんじゃないか？

~~~cpp
static void APIENTRY CountColors( CWnd* pParent, CListCtrl& lc, LPCTSTR imagePath, std::map<Gdiplus::ARGB, size_t>& numColors )
{
  // InsertItem するときに使う情報(コールバックでテキスト表示するのでデータはLPARAMだけ)
  LVITEM item{};
  item.mask = LVIF_PARAM|LVIF_TEXT;
  item.cchTextMax = 0;
  item.pszText = LPSTR_TEXTCALLBACK;

  // メッセージポンプが動かない版 WM_SETCURSOR されるとマウスカーソルが戻るためメッセージポンプが動く場合はセットしない
  CWaitCursor wait;
  {
    // 前のデータを破棄
    lc.DeleteAllItems();
    numColors.clear();
    // 画像を読み込んで、ビットイメージを取り出す
    Gdiplus::Bitmap bmp( imagePath );
    Gdiplus::BitmapData bmpData;
    // 色データを個別計算するのは面倒なのでフルカラー画像で取り込むことにする
    if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
    {
      //const Gdiplus::ARGB* imageTop = static_cast<const Gdiplus::ARGB*>( bmpData.Scan0 ); // 位置がずれるのでここでこの形はだめ
      const BYTE* imageTop = static_cast<const BYTE*>(bmpData.Scan0);
      for( UINT yLine = 0 ; yLine < bmpData.Height ; yLine++ )
      {
        const Gdiplus::ARGB* lineTop = reinterpret_cast<const Gdiplus::ARGB*>(imageTop + bmpData.Stride * yLine);
        CWaitCursor wait;
        for( UINT xPos = 0 ; xPos < bmpData.Width ; xPos++ )
        {
          auto itr = numColors.find( lineTop[xPos] );
          // 色数が増えたら更新する
          if( itr != numColors.end() )
          {
            itr->second += 1;
            {
              LVFINDINFO findInfo;
              findInfo.flags = LVFI_PARAM;
              findInfo.lParam = lineTop[xPos];
              int findNum = lc.FindItem( &findInfo );
              _ASSERTE( findNum >= 0 );
              lc.Update( findNum );
            }
          }
          // 新色を追加
          else
          {
            numColors[lineTop[xPos]] = 1;
            item.lParam = lineTop[xPos];
            int index = lc.InsertItem( &item );
            if( index >= 0 )
            {
              item.iItem = index+1;
            }
          }
        }
      }
      bmp.UnlockBits( &bmpData );
    }
  }
  // 幅を調整して文字が見えるようにする
  lc.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
  lc.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
  lc.Invalidate( TRUE );
}
~~~

#### メッセージポンプが必要だ(CodeVer_SimplePump)

スクロールバーは出てくるけど何やってるかわからない！  
わんくま掲示板で質問だ！(再描画されないんですが。。。という質問の9割がこれ)  
メッセージを処理すればいいのか！(DoEventsを用意した！)

~~~cpp
static void APIENTRY CountColors( CWnd* pParent, CListCtrl& lc, LPCTSTR imagePath, std::map<Gdiplus::ARGB, size_t>& numColors )
{
  // InsertItem するときに使う情報(コールバックでテキスト表示するのでデータはLPARAMだけ)
  LVITEM item{};
  item.mask = LVIF_PARAM|LVIF_TEXT;
  item.cchTextMax = 0;
  item.pszText = LPSTR_TEXTCALLBACK;
  {
    // 前のデータを破棄
    lc.DeleteAllItems();
    numColors.clear();
    // 画像を読み込んで、ビットイメージを取り出す
    Gdiplus::Bitmap bmp( imagePath );
    Gdiplus::BitmapData bmpData;
    CSampleAsyncWorkApp::DoEvents();
    // 色データを個別計算するのは面倒なのでフルカラー画像で取り込むことにする
    if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
    {
      //const Gdiplus::ARGB* imageTop = static_cast<const Gdiplus::ARGB*>( bmpData.Scan0 ); // 位置がずれるのでここでこの形はだめ
      const BYTE* imageTop = static_cast<const BYTE*>(bmpData.Scan0);
      for( UINT yLine = 0 ; yLine < bmpData.Height ; yLine++ )
      {
        const Gdiplus::ARGB* lineTop = reinterpret_cast<const Gdiplus::ARGB*>(imageTop + bmpData.Stride * yLine);
        if( !CSampleAsyncWorkApp::DoEvents() )
        {
          break;
        }
        for( UINT xPos = 0 ; xPos < bmpData.Width ; xPos++ )
        {
          auto itr = numColors.find( lineTop[xPos] );
          if( itr != numColors.end() )
          {
            itr->second += 1;
            {
              LVFINDINFO findInfo;
              findInfo.flags = LVFI_PARAM;
              findInfo.lParam = lineTop[xPos];
              int findNum = lc.FindItem( &findInfo );
              _ASSERTE( findNum >= 0 );
              lc.Update( findNum );
            }
          }
          else
          {
            numColors[lineTop[xPos]] = 1;
            item.lParam = lineTop[xPos];
            int index = lc.InsertItem( &item );
            if( index >= 0 )
            {
              item.iItem = index+1;
            }
          }
        }
      }
      bmp.UnlockBits( &bmpData );
    }
  }
  // 幅を調整して文字が見えるようにする
  lc.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
  lc.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
  lc.Invalidate( TRUE );
}
~~~

#### 進捗状態を出そう(CodeVer_ModelessDlg)

画像がでかくなると時間がかかるばっかりで何をやってるのかよくわからない  
処理中に×ボタンを押しても終わらない(※ダイアログはWM_QUITの処理を持っていないため落ちないだけ)
進捗ダイアログを出せばいいじゃない！

~~~cpp
static void APIENTRY CountColors( CWnd* pParent, CListCtrl& lc, LPCTSTR imagePath, std::map<Gdiplus::ARGB, size_t>& numColors )
{
  // InsertItem するときに使う情報(コールバックでテキスト表示するのでデータはLPARAMだけ)
  LVITEM item{};
  item.mask = LVIF_PARAM|LVIF_TEXT;
  item.cchTextMax = 0;
  item.pszText = LPSTR_TEXTCALLBACK;
  // インジケータ付きダイアログ
  CProgressDlg dlg;
  if( !dlg.Create( pParent ) )	// 無効化
  {
    AfxThrowResourceException();	// リソースありませんエラーでいいでしょう
  }
  pParent->EnableWindow( FALSE );
  {
    // 前のデータを破棄
    lc.DeleteAllItems();
    numColors.clear();
    // 画像を読み込んで、ビットイメージを取り出す
    Gdiplus::Bitmap bmp( imagePath );
    Gdiplus::BitmapData bmpData;
    // 色データを個別計算するのは面倒なのでフルカラー画像で取り込むことにする
    if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
    {
      //const Gdiplus::ARGB* imageTop = static_cast<const Gdiplus::ARGB*>( bmpData.Scan0 ); // 位置がずれるのでここでこの形はだめ
      const BYTE* imageTop = static_cast<const BYTE*>(bmpData.Scan0);
      // インジケータをライン数でセット。ステップカウントは１
      dlg.SetRange( 0, bmpData.Height );
      dlg.SetStep( 1 );
      for( UINT yLine = 0 ; yLine < bmpData.Height ; yLine++ )
      {
        const Gdiplus::ARGB* lineTop = reinterpret_cast<const Gdiplus::ARGB*>(imageTop + bmpData.Stride * yLine);
        dlg.StepIt();
        if( dlg.IsCancel() )
        {
          break;
        }
        CWaitCursor wait;
        for( UINT xPos = 0 ; xPos < bmpData.Width ; xPos++ )
        {
          auto itr = numColors.find( lineTop[xPos] );
          if( itr != numColors.end() )
          {
            itr->second += 1;
            {
              LVFINDINFO findInfo;
              findInfo.flags = LVFI_PARAM;
              findInfo.lParam = lineTop[xPos];
              int findNum = lc.FindItem( &findInfo );
              _ASSERTE( findNum >= 0 );
              lc.Update( findNum );
            }
          }
          else
          {
            numColors[lineTop[xPos]] = 1;
            item.lParam = lineTop[xPos];
            int index = lc.InsertItem( &item );
            if( index >= 0 )
            {
              item.iItem = index+1;
            }
          }
        }
      }
      bmp.UnlockBits( &bmpData );
    }
  }
  // 幅を調整して文字が見えるようにする
  lc.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
  lc.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
  lc.Invalidate( TRUE );
  dlg.ExitWork();
}
~~~

#### 非同期処理にしよう(CodeVer_AsyncWork)

進捗ダイアログを握っていると、処理が止まる！
反応の鈍さは改善した(ただし少し重くなった気がする)
進捗出すようにしたら昔よりかなり時間がかかるようになってないか？(漠然とした感覚)

~~~cpp
static void APIENTRY CountColors( CWnd* pParent, CListCtrl& lc, LPCTSTR imagePath, std::map<Gdiplus::ARGB, size_t>& numColors )
{
  // InsertItem するときに使う情報(コールバックでテキスト表示するのでデータはLPARAMだけ)
  LVITEM item{};
  item.mask = LVIF_PARAM|LVIF_TEXT;
  item.cchTextMax = 0;
  item.pszText = LPSTR_TEXTCALLBACK;
  // インジケータ付きダイアログ
  CProgressDlg dlg;
  auto task = concurrency::create_task( [&]()
  {
    // 前のデータを破棄
    lc.DeleteAllItems();
    numColors.clear();
    // 画像を読み込んで、ビットイメージを取り出す
    Gdiplus::Bitmap bmp( imagePath );
    Gdiplus::BitmapData bmpData;
    // 色データを個別計算するのは面倒なのでフルカラー画像で取り込むことにする
    if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
    {
      //const Gdiplus::ARGB* imageTop = static_cast<const Gdiplus::ARGB*>( bmpData.Scan0 ); // 位置がずれるのでここでこの形はだめ
      const BYTE* imageTop = static_cast<const BYTE*>(bmpData.Scan0);
      // インジケータをライン数でセット。ステップカウントは１
      dlg.SetRange( 0, bmpData.Height );
      dlg.SetStep( 1 );
      for( UINT yLine = 0 ; yLine < bmpData.Height ; yLine++ )
      {
        const Gdiplus::ARGB* lineTop = reinterpret_cast<const Gdiplus::ARGB*>(imageTop + bmpData.Stride * yLine);
        dlg.StepIt();
        if( dlg.IsCancel() )
        {
          break;
        }
        for( UINT xPos = 0 ; xPos < bmpData.Width ; xPos++ )
        {
          auto itr = numColors.find( lineTop[xPos] );
          if( itr != numColors.end() )
          {
            itr->second += 1;
            if( itr->second % 100 == 0 )
            {
              LVFINDINFO findInfo;
              findInfo.flags = LVFI_PARAM;
              findInfo.lParam = lineTop[xPos];
              int findNum = lc.FindItem( &findInfo );
              _ASSERTE( findNum >= 0 );
              lc.Update( findNum );
            }
          }
          else
          {
            numColors[lineTop[xPos]] = 1;
            item.lParam = lineTop[xPos];
            int index = lc.InsertItem( &item );
            if( index >= 0 )
            {
              item.iItem = index+1;
            }
          }
        }
      }
      bmp.UnlockBits( &bmpData );
    }
  } ).then( [&]()
  {
    dlg.ExitWork();
  } );
  dlg.DoModal();
  // ここで同期化して終了待機してないと破綻する
  task.wait();
  // 幅を調整して文字が見えるようにする
  lc.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
  lc.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
  lc.Invalidate( TRUE );
}
~~~

### 今回未実装(CodeVer_Parallels)

非同期にするだけでは時間の短縮にはならない  
並列化が必要  
実は、プロトタイプの考え方は実は間違っていなかった!

どこを並列化するのがいいのか？

懇親会までの、宿題にしたいと思います。

