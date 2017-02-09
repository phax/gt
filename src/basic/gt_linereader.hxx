#ifndef _GT_LINEREADER_HXX_
#define _GT_LINEREADER_HXX_

namespace GT {

//--------------------------------------------------------------------
class LineReader
//--------------------------------------------------------------------
{
private:
  LineReader ();
  LineReader (LineReader const&);

protected:
  HANDLE m_h;
  char*  m_pBuffer;
  char*  m_pCurrentBuffer;
  char*  m_pBufferEnd;
  size_t m_nMaxSize;
  size_t m_nSize;

  void _read ();

public:
  LineReader (      LPCTSTR sFilename,
              const size_t  nMaxSize);

  //------------------------------------------------------------------
  virtual ~LineReader ()
  //------------------------------------------------------------------
  {
    // if handle is open - close it
    if (m_h != INVALID_HANDLE_VALUE)
      VERIFY (::CloseHandle (m_h));
    delete m_pBuffer;
  }

  //------------------------------------------------------------------
  bool IsValid () const
  //------------------------------------------------------------------
  {
    return (m_h != INVALID_HANDLE_VALUE);
  }

  bool GetLine (pstring &line);
};

} // namespace

#endif
