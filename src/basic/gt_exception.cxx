#include "gt_exception.hxx"

#include "gt_errormsg.hxx"
#include "gt_utils.hxx"

namespace GT {

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) InternalError
                                        (      LPCTSTR  sFilename,
                                               LPCTSTR  sSourceFilename,
                                         const gtuint32 nLineNumber)
//--------------------------------------------------------------------
{
  ASSERT (sFilename);
  ASSERT (sSourceFilename);
  ASSERT (nLineNumber > 0);

  pstring sText;
  str_assign_from_res (sText, HRC_GTLIB_ERROR_INTERNAL);
  ShowErrorMessage (sText.c_str (),
                    sSourceFilename,
                    nLineNumber,
                    sFilename);
}

//--------------------------------------------------------------------
Excpt::Excpt
                                        (      LPCTSTR  sFilename,
                                         const gtuint32 nLine,
                                               LPCTSTR  sMessage,
                                         ...)
//--------------------------------------------------------------------
  : m_sFilename (sFilename),
    m_nLine     (nLine)
{
  TCHAR sText[1024];

  // resolve dynamic parts
  va_list args;
  va_start (args, sMessage);
  _vstprintf (sText, sMessage, args);
  va_end (args);

  // assign to member var
  m_sMessage = sText;

  // [ph] 2003/11/17 never show this msg - annoying!
  if (debug && false)
  {
#ifdef _MSC_VER
    // BC++ 5.5 is not feeling too well when using this code
    // DebugBreak ();
#endif

    ShowErrorMessage (_T ("DebugMsg: Exception is thrown (%s: %ld)\n  %s\n"),
                      m_sFilename.c_str (),
                      m_nLine,
                      m_sMessage.c_str ());
  }
}

//--------------------------------------------------------------------
void Excpt::AddText (LPCTSTR pText)
//--------------------------------------------------------------------
{
  // resolve ResId
  str_auto_resolve_resid (pText);

  // and append
  m_sMessage += pText;
}

//--------------------------------------------------------------------
void Excpt::Display () const
//--------------------------------------------------------------------
{
  pstring sText;
  str_assign_from_res (sText, HRC_GTLIB_ERROR_EXCEPTION);
  ShowErrorMessage (sText.c_str (),
                    m_sFilename.c_str (),
                    m_nLine,
                    m_sMessage.c_str ());
}

}  // namespace
