#include "gt_output.hxx"
#include "gt_stringhelper.hxx"
#include "gt_stringloader.hxx"

namespace GT {

static bool g_bShowMsgBox = true;

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) EnableInlineErrorMessages
                                        (const bool bEnable)
//--------------------------------------------------------------------
{
  g_bShowMsgBox = !bEnable;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) ShowErrorMessage (LPCTSTR sFormat, ...)
//--------------------------------------------------------------------
{
  ASSERT (sFormat);

  va_list args;
  va_start (args, sFormat);

  // get length of result string
  size_t nResLen = _vsctprintf (sFormat, args) + 1;

  LPCTSTR pDbg = _T ("\nPress Cancel to enter the debugger");

  if (g_bShowMsgBox)
  {
    // append debug string?
    if (debug)
      nResLen += _tcslen (pDbg);
  }

  // allocate on stack (+1 for trailing \n in non-MsgBox mode)
  LPTSTR pBuffer = (LPTSTR) _alloca (sizeof (TCHAR) * (nResLen + 1));

  // fill buffer
  _vstprintf (pBuffer, sFormat, args);

  if (g_bShowMsgBox)
  {
    // append debug info
    if (debug)
      _tcscat (pBuffer, pDbg);

    // and show message
    if (::MessageBox (NULL,
                      pBuffer,
                      _T ("Error"),
                      (debug ? MB_OKCANCEL : MB_OK) | MB_ICONERROR) != IDOK)
    {
      // only reached in debug version!
      DebugBreak ();
    }
  }
  else
  {
    // append newline manually!
    _tcscat (pBuffer, _T ("\n"));

    // write to output stream
    out_error_append (pBuffer);
  }
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) ShowWin32Error
                                        (DWORD   nMessageID,
                                         LPCTSTR sUserMessage)
//--------------------------------------------------------------------
{
  LPVOID lpMsgBuf = NULL;
  LPTSTR sText = NULL;

  ASSERT (nMessageID != 0);

  /*
  FormatMessage (
  DWORD     source and processing options
  LPCVOID   message source
  DWORD     message ID
  DWORD     language ID
  LPTSTR    message buffer
  DWORD     maximum size of message buffer
            minimum size if flag "FORMAT_MESSAGE_ALLOCATE_BUFFER" is used
  va_list*  array of message inserts
  )
  */
  FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL,
                 nMessageID,
                 MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                 (LPTSTR) &lpMsgBuf,
                 0,
                 NULL);

  if (sUserMessage && *sUserMessage)
    sText = (LPTSTR) sUserMessage;
  else
    VERIFY (GetStringResBuf (HRC_WIN32_ERROR, &sText) > 0);

  ShowErrorMessage (_T ("%s (%ld):\n")
                    _T ("%s\n"),
                    sText,
                    nMessageID,
                    lpMsgBuf);

  // free again
  LocalFree (lpMsgBuf);
}

}  // namespace GT
