#include "gt_stringhelper.hxx"
#include "gt_stringloader.hxx"

namespace GT {

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_auto_resolve_resid (LPCTSTR& p)
//--------------------------------------------------------------------
{
  if (str_isresptr (p))
  {
    // it's an ID!
    LPTSTR pReal;
    VERIFY (GetStringResBuf ((resid_t) p, &pReal) > 0);
    p = pReal;
  }
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_convert
                                        (LPCSTR pSrc,
                                         LPWSTR pDst,
                                         size_t nSrcLen)
//--------------------------------------------------------------------
{
  if (nSrcLen > 0)
  {
    // convert ANSI to Unicode
    VERIFY (::MultiByteToWideChar (CP_ACP, 0,
                                   pSrc, int (nSrcLen),
                                   pDst, int (nSrcLen)) != 0);
  }
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_convert
                                        (LPCWSTR pSrc,
                                         LPSTR   pDst,
                                         size_t  nSrcLen)
//--------------------------------------------------------------------
{
  if (nSrcLen > 0)
  {
    // convert Unicode to ANSI
    VERIFY (::WideCharToMultiByte (CP_ACP, 0,
                                   pSrc, int (nSrcLen),
                                   pDst, int (nSrcLen),
                                   NULL, NULL) != 0);
  }
}

namespace Internal {

//--------------------------------------------------------------------
static LPWSTR __a2w (LPCSTR pStr, const size_t nSrcLen)
//--------------------------------------------------------------------
{
  LPWSTR pw = new wchar_t[nSrcLen + 1];
  pw[nSrcLen] = 0;
  str_convert (pStr, pw, nSrcLen);
  return pw;
}

//--------------------------------------------------------------------
static LPSTR __w2a (LPCWSTR pStr, const size_t nSrcLen)
//--------------------------------------------------------------------
{
  LPSTR pw = new char[nSrcLen + 1];
  pw[nSrcLen] = 0;
  str_convert (pStr, pw, nSrcLen);
  return pw;
}

}  // namespace Internal

/*! append char* to wstring
 */
//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_append
                                        (wstring& sDst,
                                         LPCSTR   pSrc,
                                         size_t   nSrcLen /* = -1 */)
//--------------------------------------------------------------------
{
  if (nSrcLen == size_t (-1))
    nSrcLen = strlen (pSrc);

  LPWSTR pBuf = Internal::__a2w (pSrc, nSrcLen);
  sDst.append (pBuf, nSrcLen);
  delete [] pBuf;
}

/*! append wchar_t* to wstring
 */
//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_append
                                        (wstring& sDst,
                                         LPCWSTR  pSrc,
                                         size_t   nSrcLen /* = -1 */)
//--------------------------------------------------------------------
{
  if (nSrcLen == size_t (-1))
    sDst.append (pSrc);
  else
    sDst.append (pSrc, nSrcLen);
}

/*! append char* to string
 */
//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_append
                                        (string& sDst,
                                         LPCSTR  pSrc,
                                         size_t  nSrcLen /* = -1 */)
//--------------------------------------------------------------------
{
  if (nSrcLen == size_t (-1))
    sDst.append (pSrc);
  else
    sDst.append (pSrc, nSrcLen);
}

/*! append wchar_t* to string
 */
//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_append
                                        (string& sDst,
                                         LPCWSTR pSrc,
                                         size_t  nSrcLen /* = -1 */)
//--------------------------------------------------------------------
{
  if (nSrcLen == size_t (-1))
    nSrcLen = (int) wcslen (pSrc);

  LPSTR pBuf = Internal::__w2a (pSrc, nSrcLen);
  sDst.append (pBuf, nSrcLen);
  delete [] pBuf;
}

/*! assign char* to wstring
 */
//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_assign
                                        (wstring& sDst,
                                         LPCSTR   pSrc,
                                         size_t   nSrcLen /* = -1 */)
//--------------------------------------------------------------------
{
  if (nSrcLen == size_t (-1))
    nSrcLen = (int) strlen (pSrc);

  LPWSTR pBuf = Internal::__a2w (pSrc, nSrcLen);
  sDst.assign (pBuf, nSrcLen);
  delete [] pBuf;
}

/*! assign wchar_t* to wstring
 */
//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_assign
                                        (wstring& sDst,
                                         LPCWSTR  pSrc,
                                         size_t   nSrcLen /* = -1 */)
//--------------------------------------------------------------------
{
  if (nSrcLen == size_t (-1))
    sDst.assign (pSrc);
  else
    sDst.assign (pSrc, nSrcLen);
}

/*! assign char* to string
 */
//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_assign
                                        (string& sDst,
                                         LPCSTR  pSrc,
                                         size_t  nSrcLen /* = -1 */)
//--------------------------------------------------------------------
{
  if (nSrcLen == size_t (-1))
    sDst.assign (pSrc);
  else
    sDst.assign (pSrc, nSrcLen);
}

/*! assign wchar_t* to string
 */
//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_assign
                                        (string& sDst,
                                         LPCWSTR pSrc,
                                         size_t  nSrcLen /* = -1 */)
//--------------------------------------------------------------------
{
  if (nSrcLen == size_t (-1))
    nSrcLen = (int) wcslen (pSrc);

  LPSTR pBuf = Internal::__w2a (pSrc, nSrcLen);
  sDst.assign (pBuf, nSrcLen);
  delete [] pBuf;
}
//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_assign
                                        (      wstring& sDst,
                                         const string&  sSrc)
//--------------------------------------------------------------------
{
  const int nSrcLen = (int) sSrc.length ();
  LPWSTR pBuf = Internal::__a2w (sSrc.c_str (), nSrcLen);
  sDst.assign (pBuf, nSrcLen);
  delete [] pBuf;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_assign
                                        (      string&  sDst,
                                         const wstring& sSrc)
//--------------------------------------------------------------------
{
  const int nSrcLen = (int) sSrc.length ();
  LPSTR pBuf = Internal::__w2a (sSrc.c_str (), nSrcLen);
  sDst.assign (pBuf, nSrcLen);
  delete [] pBuf;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_append_from_res (pstring& sDst, resid_t nID)
//--------------------------------------------------------------------
{
  LPTSTR pBuf;
  int nCount = GetStringResBuf (nID, &pBuf);
  if (nCount > 0)
    sDst.append (pBuf, nCount);
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_assign_from_res (pstring& sDst, resid_t nID)
//--------------------------------------------------------------------
{
  LPTSTR pBuf;
  int nCount = GetStringResBuf (nID, &pBuf);
  if (nCount > 0)
    sDst.assign (pBuf, nCount);
  else
    sDst.clear ();
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_append_format (pstring& sDst, LPCTSTR pFmt, ...)
//--------------------------------------------------------------------
{
  va_list args;
  va_start (args, pFmt);

  // resolve resid after va_start!
  str_auto_resolve_resid (pFmt);

  // get length of result string (VC7 special)
  int nLen = _vsctprintf (pFmt, args);
  // alloc
  LPTSTR pBuf = (LPTSTR) _alloca ((nLen + 1) * sizeof (TCHAR));
  // do it
  _vstprintf (pBuf, pFmt, args);
  // append to result string
  sDst.append (pBuf, nLen);
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_assign_format (pstring& sDst, LPCTSTR pFmt, ...)
//--------------------------------------------------------------------
{
  va_list args;
  va_start (args, pFmt);

  // resolve resid after va_start!
  str_auto_resolve_resid (pFmt);

  // get length of result string
  int nLen = _vsctprintf (pFmt, args);
  // alloc
  LPTSTR pBuf = (LPTSTR) _alloca ((nLen + 1) * sizeof (TCHAR));
  // do it
  _vstprintf (pBuf, pFmt, args);

  // assign to result string
  sDst.assign (pBuf, nLen);
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (const pstring&) str_get_formatted (LPCTSTR pFmt, ...)
//--------------------------------------------------------------------
{
  static pstring s_sTemp;

  va_list args;
  va_start (args, pFmt);

  // resolve resid after va_start!
  str_auto_resolve_resid (pFmt);

  // get length of result string
  int nLen = _vsctprintf (pFmt, args);
  // alloc
  LPTSTR pBuf = (LPTSTR) _alloca ((nLen + 1) * sizeof (TCHAR));
  // do it
  _vstprintf (pBuf, pFmt, args);

  // assign to result string
  s_sTemp.assign (pBuf, nLen);

  return s_sTemp;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_erase_trailing (pstring& sDst, TCHAR c)
//--------------------------------------------------------------------
{
  size_t i = 0;
  while (i < sDst.length ())
  {
    if (*(sDst.end () - i - 1) != c)
      break;
    i++;
  }

  if (i > 0)
    sDst.erase (sDst.end () - i, sDst.end ());
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_replace_all (pstring& sDst, TCHAR cOld, TCHAR cNew)
//--------------------------------------------------------------------
{
  pstring::iterator it = sDst.begin ();
  for (; !(it == sDst.end ()); ++it)
    if (*it == cOld)
      *it = cNew;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_replace_all (pstring& sDst, TCHAR cOld, LPCTSTR sNew)
//--------------------------------------------------------------------
{
  ASSERT (cOld != 0);
  ASSERT (sNew);
  if (cOld == 0 || !sNew)
    return;

  LPCTSTR pCurrent;
  LPCTSTR pLast = sDst.c_str ();

  // clear the static string
  pstring sTemp;

  while ((pCurrent = _tcschr (pLast, cOld)) != NULL)
  {
    // add all from the last start until the found position
    sTemp.append (pLast, pCurrent - pLast);

    // add the new string
    sTemp.append (sNew);

    // save the old position + 1 (=1 char)
    pLast = _tcsinc (pCurrent);
  }

  // any change?
  if (pLast > sDst.c_str ())
  {
    // and finally add the rest
    sTemp += pLast;

    // replace the original string
    sDst = sTemp;
  }
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_replace_all (pstring& sDst, LPCTSTR sOld, LPCTSTR sNew)
//--------------------------------------------------------------------
{
  ASSERT (sOld);
  ASSERT (sNew);
  if (!sOld || !sNew)
    return;

  LPCTSTR pCurrent;
  LPCTSTR pLast = sDst.c_str ();
  const size_t nOldLen = _tcslen (sOld);

  // clear the static string
  pstring sTemp;

  while ((pCurrent = _tcsstr (pLast, sOld)) != NULL)
  {
    // add all from the last start until the found position
    sTemp.append (pLast, pCurrent - pLast);

    // add the new string
    sTemp.append (sNew);

    // save the old position + old len
    pLast = _tcsninc (pCurrent, nOldLen);
  }

  // any change?
  if (pLast > sDst.c_str ())
  {
    // and finally add the rest
    sTemp += pLast;

    // replace the original string
    sDst = sTemp;
  }
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) str_split (const pstring& sSrc, TCHAR cSep, pstring& sPart1, pstring& sPart2)
//--------------------------------------------------------------------
{
  LPCTSTR pSrc = sSrc.c_str ();
  LPCTSTR p = _tcschr (pSrc, cSep);
  if (p != NULL)
  {
    str_assign (sPart1, pSrc, p - pSrc);
    str_assign (sPart2, p + 1);
  }
  else
  {
    sPart1.clear ();
    sPart2.clear ();
  }
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (gtuint8) hexchar_to_gtuint8 (const char c)
//--------------------------------------------------------------------
{
  switch (c)
  {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return c - '0';
    case 'a':
    case 'A':
      return 10;
    case 'b':
    case 'B':
      return 11;
    case 'c':
    case 'C':
      return 12;
    case 'd':
    case 'D':
      return 13;
    case 'e':
    case 'E':
      return 14;
    case 'f':
    case 'F':
      return 15;
    default:
      ASSERT (false);
      return 0;
  }
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (gtuint8) hexchar_to_gtuint8 (const wchar_t c)
//--------------------------------------------------------------------
{
  switch (c)
  {
    case L'0':
    case L'1':
    case L'2':
    case L'3':
    case L'4':
    case L'5':
    case L'6':
    case L'7':
    case L'8':
    case L'9':
      return c - L'0';
    case L'a':
    case L'A':
      return 10;
    case L'b':
    case L'B':
      return 11;
    case L'c':
    case L'C':
      return 12;
    case L'd':
    case L'D':
      return 13;
    case L'e':
    case L'E':
      return 14;
    case L'f':
    case L'F':
      return 15;
    default:
      ASSERT (false);
      return 0;
  }
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (gtuint16) str_hex_to_gtuint16 (LPCSTR p)
//--------------------------------------------------------------------
{
  ASSERT (strlen (p) >= 4);
  gtuint16 ret = 0;
  for (int i = 0; i < 4; ++i)
  {
    ret <<= 4;
    ret += hexchar_to_gtuint8 (p[i]);
  }
  return ret;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (gtuint16) str_hex_to_gtuint16 (LPCWSTR p)
//--------------------------------------------------------------------
{
  ASSERT (wcslen (p) >= 4);
  gtuint16 ret = 0;
  for (int i = 0; i < 4; ++i)
  {
    ret <<= 4;
    ret += hexchar_to_gtuint8 (p[i]);
  }
  return ret;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (gtint32) str_hex_to_gtint32 (LPCSTR p)
//--------------------------------------------------------------------
{
  ASSERT (strlen (p) >= 8);
  gtint32 ret = 0;
  for (int i = 0; i < 8; ++i)
  {
    ret <<= 4;
    ret += hexchar_to_gtuint8 (p[i]);
  }
  return ret;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (gtint32) str_hex_to_gtint32 (LPCWSTR p)
//--------------------------------------------------------------------
{
  ASSERT (wcslen (p) >= 8);
  gtint32 ret = 0;
  for (int i = 0; i < 8; ++i)
  {
    ret <<= 4;
    ret += hexchar_to_gtuint8 (p[i]);
  }
  return ret;
}

#ifdef __GNUC__
// #f# HACK alert
GT_IMPL_BASIC (int) _vsctprintf (LPCTSTR pFmt, va_list args)
{
  return _tcslen (pFmt) * 4;
}
#endif

}  // namespace GT
