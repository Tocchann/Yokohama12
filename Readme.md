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

継続的にメッセージを処理すればいい。

#### 最もシンプルなメッセージループの例

このような形で継続的にメッセージ処理を呼び出すことで実現できる

~~~cpp
MSG msg;
while( GetMessage( &msg, nullptr, 0, 0 ) )
{
  TranslateMessage( &msg );
  DispatchMessage( &msg );
}
~~~

### メッセージを処理するには？

アプリケーションで、継続的にメッセージ処理関数を呼び出すことで実現

多くのフレームワークで、アプリケーションクラスの Run() メソッドにメッセージループと呼ばれるものがある。  
また、モーダルダイアログ(API のDialogBox, MFCのDoModal, .NET の ShowDialog など)も内部で独自のメッセージループがある。

### アプリ内で一時的にメッセージループを回す場合は？

.NET でいえば、DoEvents() を呼び出すことで強制的にメッセージを処理することが可能。  
MFCにも類似の AfxPumpMessage というグローバル関数があるがちょっと癖があるので、下記のようなラップ関数を作るのが一般的

#### MFC 版 Appliation::DoEvents もどき

MFCで実際にメッセージ処理を行う `AfxPumpMessage()` というまんまの名前のグローバル関数がある。  
ただし、この関数は `GetMessage()` でメッセージを待ってしまうため、メッセージがないときに呼び出すとメッセージが来るまで待機してしまう。  
回避策は、呼び出し前にメッセージがあるかを確認することで呼び出し判定を行うことができる

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

* ダイアログアプリになっているので、WM_QUIT での終了処理が存在しない
* LVN_GETDISPINFO を使ってリストコントロールのメモリ消費量を低減
* 画像処理は GDI+ で対応
* 色数情報は、32bit カラーなので Gdiplus::ARGB を利用

### 管理してるデータ

`std::map<Gdiplus::ARGB,size_t>` で色ごとの出現数をカウントできるようにしてある。  
それ以上でもないがそれ以下でもない(出現数は表示していない)。

### 画面表示部分(LVN_GETDISPINFOハンドラの内容)

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

時間のかかる処理そのものとなっている。

##### プロトタイプ(CodeVer_Prototype)

シンプルな実装  
画素の読み取りと、リストへの追加をそれぞれ別々に行っている。  

本物の初期実装と異なる点(ソース変遷定義のために共通化された部分)

1. 初期パラメータには CWnd* pParent はなかった
1. LVITEM の宣言位置は、リストコントロールに追加する箇所にあった
1. 一見するとよくわからない括弧がある

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

改良に至る外圧など。。。

1. ちょっとでかいと何やってるかよくわからない！
2. リストへの追加をリアルタイムに見せてほしい！

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

作ってみたのだが。。。

1. スクロールバーは出てくるだけで描画されない！
    * わんくま掲示板で質問だ！(再描画されないんですが。。。という質問の9割がこれ)
    * メッセージを処理すればいいらしい(前出の DoEvents に当たるものを用意)
2. メッセージループを時々読んでやれば解決だ！

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

バグに要望だと！？

1. 処理中にダイアログを操作できてしまう
   1. 画面終わるけどアプリが終了しない？
1. 画像がでかくなると時間がかかるばっかりで何をやってるのかよくわからない
1. あとどれくらいかかるのか表示してほしい！

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

1. プログレスダイアログを移動しようとしている間処理が止まる！
    * 掲示板で質問した時に、スレッドアウトすればいいとか言われたよな？

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

  // 別のスレッド(タスクの処理は別スレッドで行われる)で処理する部分
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
            LVFINDINFO findInfo;
            findInfo.flags = LVFI_PARAM;
            findInfo.lParam = lineTop[xPos];
            int findNum = lc.FindItem( &findInfo );
            _ASSERTE( findNum >= 0 );
            lc.Update( findNum );
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

1. インジケータで処理内容は見えるようになったけど遅くなってないか？
1. 並列化ってやつをすればいいんじゃないか？

もちろん実装してないだけで、並列化は可能です。  
ただし、このコードは初期のプロトタイプで版でも並列化が困難です。

## まとめ

応答なしにならない方法はこれ以外にもあります。  
今回の実装パターンは処理速度向上にはみじんも役に立っていません(計測するとわかりますが、遅くなっている)。  
処理をバックグラウンドで行うという方法もあります。

#### おまけ

GDI+ のオブジェクトはスレッドセーフではありませんので注意しましょう  
パフォーマンスを求める場合は計測しましょう
