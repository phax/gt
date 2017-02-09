#include "gt_linereader.hxx"

namespace GT  {

const int TEXTBUF_MAX_LEN = 4096;

//--------------------------------------------------------------------
LineReader::LineReader (      LPCTSTR sFilename,
                        const size_t  nMaxSize)
//--------------------------------------------------------------------
  : m_pBuffer        (NULL),
    m_pCurrentBuffer (NULL),
    m_pBufferEnd     (NULL),
    m_nMaxSize       (nMaxSize),
    m_nSize          (0)
{
  ASSERT (m_nMaxSize >= 0);

  m_h = CreateFile (sFilename,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL, // security
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);  // templatefile

  if (IsValid ())
  {
    m_pBuffer = new char [m_nMaxSize];
    _read ();
  }
}

//--------------------------------------------------------------------
void LineReader::_read ()
//--------------------------------------------------------------------
{
  ASSERT (IsValid ());

  // read new data from the file
  DWORD nBytesRead;
  ReadFile (m_h,
            m_pBuffer,
            static_cast <DWORD> (m_nMaxSize),
            &nBytesRead,
            NULL);
  m_nSize          = static_cast <size_t> (nBytesRead);
  m_pCurrentBuffer = m_pBuffer;
  m_pBufferEnd     = m_pBuffer + m_nSize;
}

//--------------------------------------------------------------------
bool LineReader::GetLine (pstring& line)
//--------------------------------------------------------------------
{
  ASSERT (m_pCurrentBuffer);
  ASSERT (m_pBufferEnd);
  ASSERT (IsValid ());

  // a automatic member is faster than a dynamic part!
  static TCHAR pLineBuffer [TEXTBUF_MAX_LEN + 1];
  TCHAR *pCurrentLineChar = pLineBuffer;
  int nLineLength = 0;

// get until EOL
ContinueReading:
  while (*m_pCurrentBuffer != _T ('\r') &&
         *m_pCurrentBuffer != _T ('\n') &&
         m_pCurrentBuffer < m_pBufferEnd)
  {
    *pCurrentLineChar = *m_pCurrentBuffer;
    pCurrentLineChar = _tcsinc (pCurrentLineChar);
    m_pCurrentBuffer++;

    // internal overflow??
    if (++nLineLength >= TEXTBUF_MAX_LEN)
    {
      FatalAppExit (0xdead, _T ("Error: the maximum line length is exceeded"));
      return false;
    }
  }

  // check whether the buffer has to be refreshed...
  if (m_pCurrentBuffer >= m_pBufferEnd && m_nSize > 0)
  {
    _read ();
    goto ContinueReading;
  }

  // skip EOL chars of the current line (else the number counter fails)
  if (*m_pCurrentBuffer == _T ('\r'))
    m_pCurrentBuffer++;

  if (*m_pCurrentBuffer == _T ('\n'))
    m_pCurrentBuffer++;

  // and add the trailing \n anyway
  *pCurrentLineChar = _T ('\n');

  // fill the result buffer
  line.assign (pLineBuffer, nLineLength + 1);  // + 1 for the trailing '\n'

  return (m_nSize > 0);
}

}  // namespace
