#include "gt_locale.hxx"

#include "gt_errormsg.hxx"
#include "gt_stringhelper.hxx"

namespace GT {

// #f# doesn't work and I don't know why

static void _getString (LCID nnLCID, LCTYPE aLCType, pstring& ret)
{
  // Determine size of locale information
  LCID nLCID = LANGIDFROMLCID (nnLCID & 0xFFFF);

  int nSize = GetLocaleInfo (nLCID, aLCType, NULL, 0);
  DWORD nLastError = GetLastError ();
  if (nLastError != ERROR_SUCCESS)
    ShowWin32Error (nLastError, _T ("GetLocaleInfo failed (1)"));

  // allocate on stack
  LPTSTR buf = (LPTSTR) _alloca (nSize * sizeof (TCHAR));
  VERIFY (GetLocaleInfo (nLCID, aLCType, buf, nSize) == nSize);
  nLastError = GetLastError ();
  if (nLastError != ERROR_SUCCESS)
    ShowWin32Error (nLastError, _T ("GetLocaleInfo failed (2)"));

  // and paste in result string
  str_assign (ret, buf);
}

GT_IMPL_BASIC (void) GetEnglishCountryName (const LCID nLCID, pstring &ret)
{
  _getString (nLCID, LOCALE_SENGCOUNTRY, ret);
}

GT_IMPL_BASIC (void) GetEnglishLanguageName (const LCID nLCID, pstring &ret)
{
  _getString (nLCID, LOCALE_SENGLANGUAGE, ret);
}

}  // namespace
