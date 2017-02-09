#include "gt_filesearch.hxx"

namespace GT {

//--------------------------------------------------------------------
struct FileSearchData
//--------------------------------------------------------------------
{
  HANDLE          nHandle;
  WIN32_FIND_DATA sr;

  FileSearchData ()
    : nHandle (0L)
  {}
};

//--------------------------------------------------------------------
FileSearch::FileSearch ()
//--------------------------------------------------------------------
  : m_bError  (false),
    m_bOpened (false),
    m_pData   (new FileSearchData)
{}

//--------------------------------------------------------------------
FileSearch::~FileSearch ()
//--------------------------------------------------------------------
{
  FindClose ();
  delete m_pData;
}

//--------------------------------------------------------------------
void GT_CALL FileSearch::FindFirst (LPCTSTR sFilename)
//--------------------------------------------------------------------
{
  ASSERT (sFilename);

  FindClose ();

  if (!m_bError)
  {
    m_bOpened = true;
    m_pData->nHandle = ::FindFirstFile (sFilename, &m_pData->sr);
    m_bError = (m_pData->nHandle == INVALID_HANDLE_VALUE);
  }
}

//--------------------------------------------------------------------
void GT_CALL FileSearch::FindNext ()
//--------------------------------------------------------------------
{
  if (!m_bError)
  {
    m_bError = !::FindNextFile (m_pData->nHandle, &m_pData->sr);
  }
}

//--------------------------------------------------------------------
void GT_CALL FileSearch::FindClose ()
//--------------------------------------------------------------------
{
  if (!m_bOpened)
    return;

  if (m_pData->nHandle != INVALID_HANDLE_VALUE)
    ::FindClose (m_pData->nHandle);

  m_bOpened = false;
  m_bError  = false;
}

//--------------------------------------------------------------------
LPCTSTR GT_CALL FileSearch::GetFilename () const
//--------------------------------------------------------------------
{
  static TCHAR sFilename[1024];

  if (m_bOpened)
  {
    _tcscpy (sFilename, m_pData->sr.cFileName);
  }
  else
  {
    _tcscpy (sFilename, _T (""));
  }

  return sFilename;
}

//--------------------------------------------------------------------
file_t GT_CALL FileSearch::GetFilesize () const
//--------------------------------------------------------------------
{
  return m_bOpened
           ? (file_t (m_pData->sr.nFileSizeHigh) << 32) + m_pData->sr.nFileSizeLow
           : FILESEARCH_ERROR;
}

//--------------------------------------------------------------------
gtuint32 GT_CALL FileSearch::GetAttr () const
//--------------------------------------------------------------------
{
  return (m_bOpened ? m_pData->sr.dwFileAttributes : gtuint32 (FILESEARCH_ERROR));
}

//--------------------------------------------------------------------
bool GT_CALL FileSearch::IsDirectory () const
//--------------------------------------------------------------------
{
  return (m_bOpened && (m_pData->sr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

}  // namespace GT
