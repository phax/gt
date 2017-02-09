#ifndef _GT_WIN32_CONSOLE_HXX_
#define _GT_WIN32_CONSOLE_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gt_output.hxx"

namespace GT {

// assign this member to m_nPauserAfterLines to disable pause mode
const gtuint32 NO_PAUSE = 0;

//--------------------------------------------------------------------
class WindowsConsole
//--------------------------------------------------------------------
{
private:
  WindowsConsole (const WindowsConsole&);              //!< declared only
  WindowsConsole& operator = (const WindowsConsole&);  //!< declared only

  enum
  {
    WC_INITED       = 0x0001,
    WC_REDIR_STDOUT = 0x0002,
    WC_REDIR_STDERR = 0x0004,
  };

  gtuint32 m_nFlags;
  HANDLE   m_hStdout;
  HANDLE   m_hStdin;
  HANDLE   m_hStderr;
  WORD     m_aOriginalAttributes;
  pstring  m_sSavedTitle;
  size_t   m_nCurLineNum;
  gtuint32 m_nPauserAfterLines;
  bool     m_bPrintLineNumbers;

  void _WriteToStdout (      LPCSTR pStr,
                       const size_t nLen);

  void _StartNewLine ();

  void _DoWrite (      LPCSTR pStr,
                 const size_t nLen);

  void _WritePortion (      LPCTSTR pStr,
                      const size_t  nLen,
                            LPSTR   pConvBuf);
  void _Init ();

  void _WaitForKeyPress ();

public:
  explicit WindowsConsole ();
  ~WindowsConsole ();

  // is a gtint32 because CmdLinesHdl gives it to us as long
  void SetPauseAfterXLines (const gtint32 nPauserAfterLines);
  void EnablePrintLineNumbers (const bool bEnable);

  // handle colors
  void SetTextColor (const EColor eColor);
  void RestoreOriginalColor ();

  // change title text
  void SetTitle (LPCTSTR sTitle);

  // show text in black area
  void DisplayText (LPCTSTR sText, size_t nLen);

  // internal stuff
  long GetBufferWidth () const;
  long GetBufferHeight () const;
  long GetVisibleX () const;
  long GetVisibleY () const;
  long GetVisibleWidth () const;
  long GetVisibleHeight () const;
};

}  // namespace

#endif
