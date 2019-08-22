# ��񂭂ܓ��� ���l�׋��� #12

## ����������񓯊������悤�Ifor C++

Github ���|�W�g���Fhttps://github.com/tocchann/yokohama12


�Q�l����:[Codezine�A�ځuWindows�A�v���P�[�V�����Łu�������v��\������v](https://codezine.jp/article/corner/384)

�����͏����̊O���Ƀ_�C�A���O��u�������ȁH�Ǝv������ł����A���e�I�ɊO���Ƀ��[�v�����ĂȂ��̂ŁA������߂܂�����

�Ƃ������ƂŁA�����ɂ��肪���Ȓ��x�^�����R�[�h�ł��B���ꂭ�炢�̎����Ȃ畁�ʂɂ���ł���H�Ƃ����Ă��ߌ��ł͂Ȃ����x���B

## �A�W�F���_

* �����Ȃ��ɂȂ闝�R
* �����Ȃ��ɂȂ�Ȃ��悤�ɂ���ɂ�
* ����������񓯊��ɂ��悤�I

## �����Ȃ��ɂȂ闝�R

#### �O�����

Windows �͊e�v���Z�X�����b�Z�[�W�������������Ȃ���A�������ē��삷��悤�ɍ���Ă���B

#### ���_

���b�Z�[�W����������Ȃ��܂܂̏�Ԃ���莞�Ԍo�߂���Ɖ������Ă��Ȃ��Ɣ��f�����B

## �����Ȃ��ɂȂ�Ȃ��悤�ɂ���ɂ�

���b�Z�[�W����������΂����B

### ���b�Z�[�W����������ɂ́H

�A�v���P�[�V�����ŁA���b�Z�[�W�����֐����Ăяo�����ƂŎ���

�����̃t���[�����[�N���A�A�v���P�[�V�����N���X�� Run() ���\�b�h�����b�Z�[�W���[�v�����B  
�܂��A���[�_���_�C�A���O(MFC�Ȃ�DoModal, .NET �Ȃ�AShowDialog)�������œƎ��̃��b�Z�[�W���[�v���񂵂Ă���(API�����l)

#### �ł��V���v���ȃ��b�Z�[�W���[�v�̗�

~~~cpp
MSG msg;
while( GetMessage( &msg, nullptr, 0, 0 ) )
{
  TranslateMessage( &msg );
  DispatchMessage( &msg );
}
~~~

### �A�v�����ňꎞ�I�Ƀ��b�Z�[�W���[�v���񂷏ꍇ�́H

.NET �ł����΁ADoEvents() ���Ăяo�����Ƃŋ����I�Ƀ��b�Z�[�W���������邱�Ƃ��\�B  
MFC�ɂ��ގ��� AfxPumpMessage �Ƃ����O���[�o���֐������邪������ƕȂ�����̂ŁA���b�v�֐������̂���ʓI

#### MFC �� Appliation::DoEvents ���ǂ�

�ȉ��̓���������̂Ń��b�v�֐�������đΉ�����

MFC�ɂ͎��ۂ̃��b�Z�[�W�������s���Ă���� `AfxPumpMessage()` �Ƃ����܂�܂̖��O�̃O���[�o���֐�������B  
�������A`GetMessage()` �Ń��b�Z�[�W��҂��Ă��܂����߁A���b�Z�[�W���Ȃ��Ƃ��ɌĂяo���ƋA���Ă��Ȃ��B  
�����Ƃ��āA�Ăяo���O�Ƀ��b�Z�[�W�����邩���m�F���邱�ƂŌĂяo��������s�����Ƃ��ł���

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

## ����������񓯊��ɂ��悤�I

��������͎��ۂ̃R�[�h���������Ǝv���܂��B

### �{������Ȃ�����

* �_�C�A���O�A�v���ɂȂ��Ă���̂ŁAWM_QUIT ����������Ȃ��������d�v
* LVN_GETDISPINFO ���g���ă��X�g�R���g���[���̃���������ʂ�ጸ
* �摜������ GDI+ �őΉ�
* �F�����́A32bit �J���[�Ȃ̂� Gdiplus::ARGB �𗘗p

### �Ǘ����Ă�f�[�^

�摜�̐F�Ƃ��̏o����������΂���  
-> `std::map<Gdiplus::ARGB,size_t>` �Ńf�[�^�Ǘ��B

### ��ʕ\������

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
### �摜��͕���

CountColors �Ƃ����X�^�e�B�b�N�֐��Ŏ���

���������Ԃ̂����鏈���̃R�A����  
���Ԃ̂����鏈���͑傫��2�̃��[�v������  
��͉摜�̉�f���̂��̂�ǂݎ��Ƃ���  
������̓��X�g�ɃA�C�e����ǉ�����Ƃ���

##### �v���g�^�C�v(CodeVer_Prototype)

�V���v���Ȏ���  
��f�̓ǂݎ��ƁA���X�g�ւ̒ǉ������ꂼ��ʁX�ɍs���Ă���B  
(���Ȃ݂ɏ����p�����[�^�ɂ́ACWnd* pParent �͂Ȃ�)

~~~cpp
static void APIENTRY CountColors( CWnd* pParent, CListCtrl& lc, LPCTSTR imagePath, std::map<Gdiplus::ARGB, size_t>& numColors )
{
  // InsertItem ����Ƃ��Ɏg�����(�R�[���o�b�N�Ńe�L�X�g�\������̂Ńf�[�^��LPARAM����)
  LVITEM item{};
  item.mask = LVIF_PARAM|LVIF_TEXT;
  // �e�L�X�g�f�[�^�͂��̓s�x��������(�������C���[�W�ȗ��̂���)
  item.cchTextMax = 0;
  item.pszText = LPSTR_TEXTCALLBACK;

  // ���b�Z�[�W�|���v�������Ȃ��� WM_SETCURSOR �����ƃ}�E�X�J�[�\�����߂邽�߃��b�Z�[�W�|���v�������ꍇ�̓Z�b�g���Ȃ�
  CWaitCursor wait;
  {
    // ����������
    lc.DeleteAllItems();
    numColors.clear();
    // �t�@�C������ǂݍ���
    Gdiplus::Bitmap bmp( imagePath );
    Gdiplus::BitmapData bmpData;
    if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
    {
      const BYTE* imageTop = static_cast<const BYTE*>(bmpData.Scan0);
      // �摜����F�����s�b�N�A�b�v���āAmap<Gdiplus::ARGB,size_t> �Ɋi�[����
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
  // ��荞�񂾐F�������X�g�R���g���[���Ɉꊇ�Z�b�g����
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
      break;  // �F���ǉ��ł��Ȃ��Ȃ������_�ł�����߂�
    }
  }
  lc.SetRedraw( TRUE );
  lc.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
  lc.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
  lc.Invalidate( TRUE );
}
~~~

#### ���X�g�ւ̒ǉ��������ɏ�������悤�ɕύX(CodeVer_SyncInsertItem)

������Ƃł����Ɖ�����Ă邩�悭�킩��Ȃ�������ǂ��悤�I  
���X�g�ɒǉ��𓯎��ɂ��Γ����������邩��킩��₷���񂶂�Ȃ����H

~~~cpp
static void APIENTRY CountColors( CWnd* pParent, CListCtrl& lc, LPCTSTR imagePath, std::map<Gdiplus::ARGB, size_t>& numColors )
{
  // InsertItem ����Ƃ��Ɏg�����(�R�[���o�b�N�Ńe�L�X�g�\������̂Ńf�[�^��LPARAM����)
  LVITEM item{};
  item.mask = LVIF_PARAM|LVIF_TEXT;
  item.cchTextMax = 0;
  item.pszText = LPSTR_TEXTCALLBACK;

  // ���b�Z�[�W�|���v�������Ȃ��� WM_SETCURSOR �����ƃ}�E�X�J�[�\�����߂邽�߃��b�Z�[�W�|���v�������ꍇ�̓Z�b�g���Ȃ�
  CWaitCursor wait;
  {
    // �O�̃f�[�^��j��
    lc.DeleteAllItems();
    numColors.clear();
    // �摜��ǂݍ���ŁA�r�b�g�C���[�W�����o��
    Gdiplus::Bitmap bmp( imagePath );
    Gdiplus::BitmapData bmpData;
    // �F�f�[�^���ʌv�Z����͖̂ʓ|�Ȃ̂Ńt���J���[�摜�Ŏ�荞�ނ��Ƃɂ���
    if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
    {
      //const Gdiplus::ARGB* imageTop = static_cast<const Gdiplus::ARGB*>( bmpData.Scan0 ); // �ʒu�������̂ł����ł��̌`�͂���
      const BYTE* imageTop = static_cast<const BYTE*>(bmpData.Scan0);
      for( UINT yLine = 0 ; yLine < bmpData.Height ; yLine++ )
      {
        const Gdiplus::ARGB* lineTop = reinterpret_cast<const Gdiplus::ARGB*>(imageTop + bmpData.Stride * yLine);
        CWaitCursor wait;
        for( UINT xPos = 0 ; xPos < bmpData.Width ; xPos++ )
        {
          auto itr = numColors.find( lineTop[xPos] );
          // �F������������X�V����
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
          // �V�F��ǉ�
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
  // ���𒲐����ĕ�����������悤�ɂ���
  lc.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
  lc.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
  lc.Invalidate( TRUE );
}
~~~

#### ���b�Z�[�W�|���v���K�v��(CodeVer_SimplePump)

�X�N���[���o�[�͏o�Ă��邯�ǉ�����Ă邩�킩��Ȃ��I  
��񂭂܌f���Ŏ��₾�I(�ĕ`�悳��Ȃ���ł����B�B�B�Ƃ��������9��������)  
���b�Z�[�W����������΂����̂��I(DoEvents��p�ӂ����I)

~~~cpp
static void APIENTRY CountColors( CWnd* pParent, CListCtrl& lc, LPCTSTR imagePath, std::map<Gdiplus::ARGB, size_t>& numColors )
{
  // InsertItem ����Ƃ��Ɏg�����(�R�[���o�b�N�Ńe�L�X�g�\������̂Ńf�[�^��LPARAM����)
  LVITEM item{};
  item.mask = LVIF_PARAM|LVIF_TEXT;
  item.cchTextMax = 0;
  item.pszText = LPSTR_TEXTCALLBACK;
  {
    // �O�̃f�[�^��j��
    lc.DeleteAllItems();
    numColors.clear();
    // �摜��ǂݍ���ŁA�r�b�g�C���[�W�����o��
    Gdiplus::Bitmap bmp( imagePath );
    Gdiplus::BitmapData bmpData;
    CSampleAsyncWorkApp::DoEvents();
    // �F�f�[�^���ʌv�Z����͖̂ʓ|�Ȃ̂Ńt���J���[�摜�Ŏ�荞�ނ��Ƃɂ���
    if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
    {
      //const Gdiplus::ARGB* imageTop = static_cast<const Gdiplus::ARGB*>( bmpData.Scan0 ); // �ʒu�������̂ł����ł��̌`�͂���
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
  // ���𒲐����ĕ�����������悤�ɂ���
  lc.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
  lc.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
  lc.Invalidate( TRUE );
}
~~~

#### �i����Ԃ��o����(CodeVer_ModelessDlg)

�摜���ł����Ȃ�Ǝ��Ԃ�������΂�����ŉ�������Ă�̂��悭�킩��Ȃ�  
�������Ɂ~�{�^���������Ă��I���Ȃ�(���_�C�A���O��WM_QUIT�̏����������Ă��Ȃ����ߗ����Ȃ�����)
�i���_�C�A���O���o���΂�������Ȃ��I

~~~cpp
static void APIENTRY CountColors( CWnd* pParent, CListCtrl& lc, LPCTSTR imagePath, std::map<Gdiplus::ARGB, size_t>& numColors )
{
  // InsertItem ����Ƃ��Ɏg�����(�R�[���o�b�N�Ńe�L�X�g�\������̂Ńf�[�^��LPARAM����)
  LVITEM item{};
  item.mask = LVIF_PARAM|LVIF_TEXT;
  item.cchTextMax = 0;
  item.pszText = LPSTR_TEXTCALLBACK;
  // �C���W�P�[�^�t���_�C�A���O
  CProgressDlg dlg;
  if( !dlg.Create( pParent ) )	// ������
  {
    AfxThrowResourceException();	// ���\�[�X����܂���G���[�ł����ł��傤
  }
  pParent->EnableWindow( FALSE );
  {
    // �O�̃f�[�^��j��
    lc.DeleteAllItems();
    numColors.clear();
    // �摜��ǂݍ���ŁA�r�b�g�C���[�W�����o��
    Gdiplus::Bitmap bmp( imagePath );
    Gdiplus::BitmapData bmpData;
    // �F�f�[�^���ʌv�Z����͖̂ʓ|�Ȃ̂Ńt���J���[�摜�Ŏ�荞�ނ��Ƃɂ���
    if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
    {
      //const Gdiplus::ARGB* imageTop = static_cast<const Gdiplus::ARGB*>( bmpData.Scan0 ); // �ʒu�������̂ł����ł��̌`�͂���
      const BYTE* imageTop = static_cast<const BYTE*>(bmpData.Scan0);
      // �C���W�P�[�^�����C�����ŃZ�b�g�B�X�e�b�v�J�E���g�͂P
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
  // ���𒲐����ĕ�����������悤�ɂ���
  lc.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
  lc.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
  lc.Invalidate( TRUE );
  dlg.ExitWork();
}
~~~

#### �񓯊������ɂ��悤(CodeVer_AsyncWork)

�i���_�C�A���O�������Ă���ƁA�������~�܂�I
�����݂̓��͉��P����(�����������d���Ȃ����C������)
�i���o���悤�ɂ�����̂�肩�Ȃ莞�Ԃ�������悤�ɂȂ��ĂȂ����H(���R�Ƃ������o)

~~~cpp
static void APIENTRY CountColors( CWnd* pParent, CListCtrl& lc, LPCTSTR imagePath, std::map<Gdiplus::ARGB, size_t>& numColors )
{
  // InsertItem ����Ƃ��Ɏg�����(�R�[���o�b�N�Ńe�L�X�g�\������̂Ńf�[�^��LPARAM����)
  LVITEM item{};
  item.mask = LVIF_PARAM|LVIF_TEXT;
  item.cchTextMax = 0;
  item.pszText = LPSTR_TEXTCALLBACK;
  // �C���W�P�[�^�t���_�C�A���O
  CProgressDlg dlg;
  auto task = concurrency::create_task( [&]()
  {
    // �O�̃f�[�^��j��
    lc.DeleteAllItems();
    numColors.clear();
    // �摜��ǂݍ���ŁA�r�b�g�C���[�W�����o��
    Gdiplus::Bitmap bmp( imagePath );
    Gdiplus::BitmapData bmpData;
    // �F�f�[�^���ʌv�Z����͖̂ʓ|�Ȃ̂Ńt���J���[�摜�Ŏ�荞�ނ��Ƃɂ���
    if( bmp.LockBits( nullptr, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmpData ) == Gdiplus::Ok )
    {
      //const Gdiplus::ARGB* imageTop = static_cast<const Gdiplus::ARGB*>( bmpData.Scan0 ); // �ʒu�������̂ł����ł��̌`�͂���
      const BYTE* imageTop = static_cast<const BYTE*>(bmpData.Scan0);
      // �C���W�P�[�^�����C�����ŃZ�b�g�B�X�e�b�v�J�E���g�͂P
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
  // �����œ��������ďI���ҋ@���ĂȂ��Ɣj�]����
  task.wait();
  // ���𒲐����ĕ�����������悤�ɂ���
  lc.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER );
  lc.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
  lc.Invalidate( TRUE );
}
~~~

### ���񖢎���(CodeVer_Parallels)

�񓯊��ɂ��邾���ł͎��Ԃ̒Z�k�ɂ͂Ȃ�Ȃ�  
���񉻂��K�v  
���́A�v���g�^�C�v�̍l�����͎��͊Ԉ���Ă��Ȃ�����!

�ǂ�����񉻂���̂������̂��H

���e��܂ł́A�h��ɂ������Ǝv���܂��B

