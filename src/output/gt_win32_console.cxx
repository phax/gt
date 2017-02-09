#include "gt_win32_console.hxx"

#include "gt_cmdline_params.hxx"
#include "gt_errormsg.hxx"
#include "gt_utils.hxx"

#ifdef GT_DYNAMIC_CONSOLE
#include <io.h>
#include <fcntl.h>
#endif

namespace GT {

static CONSOLE_SCREEN_BUFFER_INFO g_aCSBI;

#define FGCOL1(c1)          (FOREGROUND_##c1)
#define FGCOL2(c1,c2)       (FGCOL1 (c1) | FGCOL1 (c2))
#define FGCOL3(c1,c2,c3)    (FGCOL1 (c1) | FGCOL1 (c2) | FGCOL1 (c3))
#define FGCOL4(c1,c2,c3,c4) (FGCOL1 (c1) | FGCOL1 (c2) | FGCOL1 (c3) | FGCOL1 (c4))

//--------------------------------------------------------------------
static void _SetConsoleTextAttributes
                                        (HANDLE  h,
                                         WORD    w,
                                         LPCTSTR sErrMsg)
//--------------------------------------------------------------------
{
  if (!::SetConsoleTextAttribute (h, w))
    if (::GetLastError () != ERROR_INVALID_HANDLE)
      ShowWin32Error (::GetLastError (), sErrMsg);
}

//--------------------------------------------------------------------
static void _UpdateCSBI
                                        (HANDLE                      h,
                                         CONSOLE_SCREEN_BUFFER_INFO* pCSBI,
                                         LPCTSTR                     sErrMsg)
//--------------------------------------------------------------------
{
  if (!::GetConsoleScreenBufferInfo (h, pCSBI))
    if (::GetLastError () != ERROR_INVALID_HANDLE)
      ShowWin32Error (::GetLastError (), sErrMsg);
}

/*! If the function handles the control signal, it should return TRUE.
      If it returns FALSE, the next handler function in the list of
      handlersfor this process is used.
    Important: this routine runs (at least) in another thread and the
      MessageBox flag MB_APPLMODAL does not work as expected!
 */
//--------------------------------------------------------------------
static BOOL WINAPI _HandlerRoutine

                                        (DWORD dwCtrlType)
//--------------------------------------------------------------------
{
  switch (dwCtrlType)
  {
    // Handle the CTRL+C signal.
    case CTRL_C_EVENT:
      // return TRUE to indicate "handled"
      return FALSE;

    // CTRL+CLOSE: confirm that the user wants to exit.
    case CTRL_CLOSE_EVENT:
    {
      pstring sText, sHeader;
      str_assign_from_res (sText, HRC_APP_CONSOLE_REALLY_QUIT);
      str_assign_from_res (sHeader, HRC_APP_CONSOLE_REALLY_QUIT_HDR);
      if (::MessageBox (NULL,
                        sText.c_str (),
                        sHeader.c_str (),
                        MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND) == IDYES)
      {
        // FALSE means we did not handle it -> the default is quit the program
        return FALSE;
      }
      // do nothing, we handled it
      return TRUE;
    }
    // Pass other signals to the next handler.
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    default:
      return FALSE;
  }
}

//--------------------------------------------------------------------
void WindowsConsole::_Init ()
//--------------------------------------------------------------------
{
  TCHAR sTitle[1024];

  if (!(m_nFlags & WC_INITED))
  {
#ifdef GT_DYNAMIC_CONSOLE
    // create a new console!!
    if (!::AllocConsole ())
      FatalAppExit (0xdead, _T ("AllocConsole failed"));
#endif

    // check all three default handles
    m_hStdout = ::GetStdHandle (STD_OUTPUT_HANDLE);
    if (m_hStdout == INVALID_HANDLE_VALUE || !m_hStdout)
      FatalAppExit (0xdead, _T ("Failed to get STDOUT handle"));

    m_hStdin = ::GetStdHandle (STD_INPUT_HANDLE);
    if (m_hStdin == INVALID_HANDLE_VALUE || !m_hStdin)
      FatalAppExit (0xdead, _T ("Failed to get STDIN handle"));

    m_hStderr = ::GetStdHandle (STD_ERROR_HANDLE);
    if (m_hStderr == INVALID_HANDLE_VALUE || !m_hStderr)
      FatalAppExit (0xdead, _T ("Failed to get STDERR handle"));

#ifdef GT_DYNAMIC_CONSOLE
    int h;
    FILE *f;

    h = _open_osfhandle ((long) m_hStdout, _O_TEXT);
    f = _fdopen (h, "w");
    *stdout = *f;
    setvbuf (stdout, NULL, _IONBF, 0);

    h = _open_osfhandle ((long) m_hStdin, _O_TEXT);
    f = _fdopen (h, "r");
    *stdin = *f;
    setvbuf (stdin, NULL, _IONBF, 0);

    h = _open_osfhandle ((long) m_hStderr, _O_TEXT);
    f = _fdopen (h, "w");
    *stderr = *f;
    setvbuf (stderr, NULL, _IONBF, 0);
#endif

    // retrieve info about stdout
    _UpdateCSBI (m_hStdout, &g_aCSBI, _T ("Initing"));
    m_aOriginalAttributes = g_aCSBI.wAttributes;

    // get current title
    int len = ::GetConsoleTitle (sTitle, 1023);
    if (len > 0)
      m_sSavedTitle = sTitle;

    // check redirection
    if (::GetFileType (m_hStdout) == FILE_TYPE_DISK)
      m_nFlags |= WC_REDIR_STDOUT;

    if (::GetFileType (m_hStderr) == FILE_TYPE_DISK)
      m_nFlags |= WC_REDIR_STDERR;

    // add handler (TRUE: Add)
    if (!::SetConsoleCtrlHandler (_HandlerRoutine, TRUE))
      FatalAppExit (0xdead, _T ("Error registering console ctrl handler"));

    // inited!
    m_nFlags |= WC_INITED;
  }
}

/*! Wait until any key is pressed!
 */
//--------------------------------------------------------------------
void WindowsConsole::_WaitForKeyPress ()
//--------------------------------------------------------------------
{
  INPUT_RECORD aConInputRecord;
  DWORD nOldMode;
  DWORD nNumRead;

  // Switch to raw mode (no line input, no echo input)
  ::GetConsoleMode (m_hStdin, &nOldMode);
  ::SetConsoleMode (m_hStdin, 0L);

  for (;;)
  {
    // get input event
    if (!::ReadConsoleInput (m_hStdin, &aConInputRecord, 1L, &nNumRead) || nNumRead == 0)
    {
      // error :(
      break;
    }

    // Was it a keyevent?
    if (aConInputRecord.EventType == KEY_EVENT &&
        aConInputRecord.Event.KeyEvent.bKeyDown)
    {
      // success
      break;
    }
  }

  // Restore previous console mode.
  ::SetConsoleMode (m_hStdin, nOldMode);
}

//--------------------------------------------------------------------
WindowsConsole::WindowsConsole ()
//--------------------------------------------------------------------
  : m_nFlags            (0),
    m_nCurLineNum       (0),
    m_nPauserAfterLines (NO_PAUSE),
    m_bPrintLineNumbers (false)
{}

//--------------------------------------------------------------------
WindowsConsole::~WindowsConsole ()
//--------------------------------------------------------------------
{
  // restore original attributes
  RestoreOriginalColor ();

  // shall we wait for a key?
  if (CmdlineParam_GetBool (GT_FLAG_WAITFORKEY))
  {
    out_status (rc (HRC_APP_PRESS_ANY_KEY));
    out_append (rc (HRC_APP_PRESS_ANY_KEY));  // print text
    out_flush ();                             // otherwise the text wont be visible!!
    _WaitForKeyPress ();                      // wait for a key
    out_append (_T ("\n"));                   // end line
  }

  // remove handler (FALSE: remove)
  ::SetConsoleCtrlHandler (_HandlerRoutine, FALSE);

  // restore original title
  SetTitle (m_sSavedTitle.c_str ());

#ifdef GT_DYNAMIC_CONSOLE
  // finally free the console again!
  ::FreeConsole ();
#endif
}

//--------------------------------------------------------------------
void WindowsConsole::SetPauseAfterXLines
                                        (const gtint32 nPauserAfterLines)
//--------------------------------------------------------------------
{
  if (nPauserAfterLines > 0)
    m_nPauserAfterLines = gtuint32 (nPauserAfterLines);
}

//--------------------------------------------------------------------
void WindowsConsole::EnablePrintLineNumbers
                                        (const bool bEnable)
//--------------------------------------------------------------------
{
  m_bPrintLineNumbers = bEnable;
}

//--------------------------------------------------------------------
void WindowsConsole::SetTextColor
                                        (const EColor eColor)
//--------------------------------------------------------------------
{
  _Init ();

  // if stdout is redirected do nothing
  if (m_hStdout == INVALID_HANDLE_VALUE)
    return;

  WORD wAttribute;

  switch (eColor)
  {
    case GT_COLOR_BLACK:        wAttribute = 0; break;
    case GT_COLOR_BLUE:         wAttribute = FGCOL1 (BLUE); break;
    case GT_COLOR_GREEN:        wAttribute = FGCOL1 (GREEN); break;
    case GT_COLOR_CYAN:         wAttribute = FGCOL2 (GREEN, BLUE); break;
    case GT_COLOR_RED:          wAttribute = FGCOL1 (RED); break;
    case GT_COLOR_MAGENTA:      wAttribute = FGCOL2 (RED, BLUE); break;
    case GT_COLOR_OLIVE:        wAttribute = FGCOL2 (RED, GREEN); break;
    case GT_COLOR_LIGHTGRAY:    wAttribute = FGCOL3 (RED, GREEN, BLUE); break;

    case GT_COLOR_DARKGRAY:     wAttribute = FGCOL1 (INTENSITY); break;
    case GT_COLOR_LIGHTBLUE:    wAttribute = FGCOL2 (BLUE, INTENSITY); break;
    case GT_COLOR_LIGHTGREEN:   wAttribute = FGCOL2 (GREEN, INTENSITY); break;
    case GT_COLOR_LIGHTCYAN:    wAttribute = FGCOL3 (GREEN, BLUE, INTENSITY); break;
    case GT_COLOR_LIGHTRED:     wAttribute = FGCOL2 (RED, INTENSITY); break;
    case GT_COLOR_LIGHTMAGENTA: wAttribute = FGCOL3 (RED, BLUE, INTENSITY); break;
    case GT_COLOR_YELLOW:       wAttribute = FGCOL3 (RED, GREEN, INTENSITY); break;
    case GT_COLOR_WHITE:        wAttribute = FGCOL4 (RED, GREEN, BLUE, INTENSITY); break;

    default:
    {
      ASSERT (0);
      wAttribute = FGCOL3 (RED, GREEN, BLUE);
      break;
    }
  }

  _SetConsoleTextAttributes (m_hStdout, wAttribute, _T ("SetTextColor"));
}

//--------------------------------------------------------------------
void WindowsConsole::RestoreOriginalColor ()
//--------------------------------------------------------------------
{
  _Init ();

  // restore info about stdout
  _SetConsoleTextAttributes (m_hStdout, m_aOriginalAttributes, _T ("RestoreOriginalColor"));
}

//--------------------------------------------------------------------
void WindowsConsole::SetTitle
                                        (LPCTSTR sTitle)
//--------------------------------------------------------------------
{
  ASSERT (sTitle);

  // resolve it if it is a res ID
  str_auto_resolve_resid (sTitle);

  _Init ();

#if 1
  ::SetConsoleTitle (sTitle);
#else
  // convert ANSI to OEM (result always char*)
  const DWORD nLen = _tcslen (sTitle);
  char* pConvBuf = (char*) alloca (nLen + 1);
  pConvBuf[nLen] = '\0';
  ::CharToOemBuff (sTitle, pConvBuf, nLen);

  ::SetConsoleTitleA (pConvBuf);
#endif
}

//--------------------------------------------------------------------
void WindowsConsole::_WriteToStdout
                                        (      LPCSTR pStr,
                                         const size_t nLen)
//--------------------------------------------------------------------
{
  DWORD nBytesWritten;

  // check that no weird character is contained in the output
  // Happens quite often for files having an asian encoding
  if (false) ASSERT (memchr (pStr, '\x07', nLen) == NULL);

  // and send to stdout
  if (!::WriteFile (m_hStdout, pStr, DWORD (nLen), &nBytesWritten, NULL))
    ::FatalAppExit (0xdead, _T ("Failed to write to stdout"));

  // and send to stdout
  if (nBytesWritten != nLen)
    ::FatalAppExit (0xdead, _T ("Not all bytes could be written to stdout"));
}

//--------------------------------------------------------------------
void WindowsConsole::_StartNewLine ()
//--------------------------------------------------------------------
{
  // save memory :)
  union
  {
    TCHAR sOldTitle[1024];
    char  sBuf[64];
  };

  // wait for a keypress?
  // Not at the first call!
  if (m_nPauserAfterLines != NO_PAUSE &&
      m_nCurLineNum > 0 &&
      !(m_nCurLineNum % m_nPauserAfterLines))
  {
    ::GetConsoleTitle (sOldTitle, 1023);

    // set "press any key" title
    SetTitle (rc (HRC_APP_CONSOLE_PRESS_KEY));
    _WaitForKeyPress ();

    // restore title
    SetTitle (sOldTitle);
  }

  // inc line counter
  ++m_nCurLineNum;

  // global flag to disable this nifty feature :)
  if (m_bPrintLineNumbers)
  {
    int nBufLen = sprintf (sBuf, "%u: ", m_nCurLineNum);
    _WriteToStdout (sBuf, size_t (nBufLen));
  }
}

/*! This is the last encapsulation for the writing process.
 */
//--------------------------------------------------------------------
void WindowsConsole::_DoWrite
                                        (      LPCSTR pStr,
                                         const size_t nLen)
//--------------------------------------------------------------------
{
  if (m_bPrintLineNumbers || m_nPauserAfterLines != NO_PAUSE)
  {
    LPCSTR pStart = pStr, p = pStr;

    // very first line???
    if (m_nCurLineNum == 0)
      _StartNewLine ();

    // scan for '\n' chars
    for (size_t i = 0; i < nLen; ++i, ++p)
      if (*p == '\n')
      {
        // +1 to print the newline char too
        _WriteToStdout (pStart, (p + 1) - pStart);
        _StartNewLine ();
        // remember new line beginning
        pStart = p + 1;
      }

    // write end of current line
    if (p > pStr)
      _WriteToStdout (pStart, p - pStart);
  }
  else
  {
    // just write content to file
    _WriteToStdout (pStr, nLen);
  }
}

/*! Write the LPCTSTR pointed to by pStr for nLen bytes. If a
     special conversion is needed, use pConvBuf as a char buffer!
 */
//--------------------------------------------------------------------
void WindowsConsole::_WritePortion
                                        (      LPCTSTR pStr,
                                         const size_t  nLen,
                                               LPSTR   pConvBuf)
//--------------------------------------------------------------------
{
  ASSERT (nLen > 0);

  // redirected STDOUT?
  if (m_nFlags & WC_REDIR_STDOUT)
  {
#ifdef _UNICODE
    // convert from wchar_t* to char*
    str_convert (pStr, pConvBuf, nLen);

    // and send to stdout
    _DoWrite (pConvBuf, nLen);
#else
    // and send to stdout
    _DoWrite (pStr, nLen);
#endif
  }
  else
  {
    // convert ANSI to OEM (different for char and wchar_t)
    ::CharToOemBuff (pStr, pConvBuf, DWORD (nLen));

    // and send to stdout
    _DoWrite (pConvBuf, nLen);
  }
}

/*! This is the public function to be called.
 */
//--------------------------------------------------------------------
void WindowsConsole::DisplayText (LPCTSTR sText, size_t nLen)
//--------------------------------------------------------------------
{
  ASSERT (m_hStdout);

  const size_t PRINT_PORTION = _max (long (2000), GetVisibleWidth () * GetVisibleHeight ());  // (e.g. 80x25 == 2000 chars)

  // directly access the STDOUT handle from windows
  size_t nOfs = 0;

  // always char!!
  char *sCharBuf = (char*) alloca (PRINT_PORTION + 1);

  // if not inited...
  _Init ();

  // don't write everything at a time, write it in small portions....
  while (nLen >= PRINT_PORTION)
  {
    _WritePortion (_tcsninc (sText, nOfs), PRINT_PORTION, sCharBuf);

    nLen -= PRINT_PORTION;
    nOfs += PRINT_PORTION;
  }

  // print remaining chars (if any)
  if (nLen > 0)
    _WritePortion (_tcsninc (sText, nOfs), nLen, sCharBuf);
}

//--------------------------------------------------------------------
long WindowsConsole::GetBufferWidth () const
//--------------------------------------------------------------------
{
  _UpdateCSBI (m_hStdout, &g_aCSBI, _T ("GetBufferWidth"));
  return g_aCSBI.dwSize.X;
}

//--------------------------------------------------------------------
long WindowsConsole::GetBufferHeight () const
//--------------------------------------------------------------------
{
  _UpdateCSBI (m_hStdout, &g_aCSBI, _T ("GetBufferHeight"));
  return g_aCSBI.dwSize.Y;
}

//--------------------------------------------------------------------
long WindowsConsole::GetVisibleX () const
//--------------------------------------------------------------------
{
  _UpdateCSBI (m_hStdout, &g_aCSBI, _T ("GetVisibleX"));
  return g_aCSBI.srWindow.Left;
}

//--------------------------------------------------------------------
long WindowsConsole::GetVisibleY () const
//--------------------------------------------------------------------
{
  _UpdateCSBI (m_hStdout, &g_aCSBI, _T ("GetVisibleY"));
  return g_aCSBI.srWindow.Top;
}

//--------------------------------------------------------------------
long WindowsConsole::GetVisibleWidth () const
//--------------------------------------------------------------------
{
  _UpdateCSBI (m_hStdout, &g_aCSBI, _T ("GetVisibleWidth"));
  return g_aCSBI.srWindow.Right - g_aCSBI.srWindow.Left;
}

//--------------------------------------------------------------------
long WindowsConsole::GetVisibleHeight () const
//--------------------------------------------------------------------
{
  _UpdateCSBI (m_hStdout, &g_aCSBI, _T ("GetVisibleHeight"));
  return g_aCSBI.srWindow.Bottom - g_aCSBI.srWindow.Top;
}

}  // namespace
