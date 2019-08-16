# ��񂭂ܓ��� ���l�׋��� #12

## ����������񓯊������悤�Ifor C++

�Q�l����:[Codezine�A�ځuWindows�A�v���P�[�V�����Łu�������v��\������v](https://codezine.jp/article/corner/384)

~~~cpp
// �t�@�C���̃o�C�g�f�[�^�̏o�����𐔂���֐�(���e�͑S���ς��܂���)
// �摜�̐F���𐔂��邱�Ƃɂ���B�摜��GDI+�œǂݎ��
bool CountCharInFile(
    class CProgressDlg& dlg,
    LPCTSTR filePath,
    std::map<char,size_t>& numbers );
~~~
�Â�UI�X���b�h�I�����[�p�^�[��(����)
~~~cpp
void CSampleDlg::OnOK()
{
    // ���낢��O����
    CProgressDlg dlg;
    dlg.Create();
    if( CountCharInFile( dlg, m_targetPath, m_numColors ) )
    {
        // ��ʂ��X�V
    }
    dlg.CloseWindow();
}
~~~
�^�X�N���g�����񓯊�����
~~~cpp
void CSampleDlg::OnOK()
{
    // ���낢��O����
    CProgressDlg dlg;
    auto task = concurrency::create_task( [&]()
    {
        return CountCharInFile( dlg, m_targetPath, m_numColors );
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
}
~~~
�ڍs�̃L���� CProgressDlg �̎����ɂ������肵�܂��B�����͓����̂��y���݁H��
