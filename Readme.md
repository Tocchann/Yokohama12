# ��񂭂ܓ��� ���l�׋��� #12

## ����������񓯊������悤�Ifor C++

�Q�l����:[Codezine�A�ځuWindows�A�v���P�[�V�����Łu�������v��\������v](https://codezine.jp/article/corner/384)

~~~cpp
// �t�@�C���̃o�C�g�f�[�^�̏o�����𐔂���֐�
bool CountCharInFile(
    class CProgressDlg& dlg,
    LPCTSTR filePath,
    std::map<char,size_t>& numbers );
~~~
�Â�UI�X���b�h�I�����[�p�^�[��(����)
~~~cpp
// CString m_targetPath;
CFileDialog dlg( TRUE, nullptr, m_targetPath,
                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                _T("���ׂẴt�@�C��|*.*||"), this );
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
    // ��ʂ��X�V
}
else
{
    // �L�����Z������
}
dlg.CloseWindow();
~~~
�^�X�N���g�����񓯊�����
~~~cpp
// CString m_targetPath;
CFileDialog dlg( TRUE, nullptr, m_targetPath,
                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                _T("���ׂẴt�@�C��|*.*||"), this );
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
    // ��ʍX�V
}
~~~
