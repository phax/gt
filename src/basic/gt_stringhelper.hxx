#ifndef _GT_STRINGHELPER_HXX_
#define _GT_STRINGHELPER_HXX_

namespace GT {

//! check of p is a ptr into the resource file
//--------------------------------------------------------------------
inline bool GT_CALL str_isresptr (LPCTSTR p)
//--------------------------------------------------------------------
{
  return p < (LPCTSTR) 0x00010000;
}

//! if it is an resource ptr, auto-resolve it!
GT_DECL_BASIC (void) str_auto_resolve_resid (LPCTSTR& p);

GT_DECL_BASIC (void) str_convert (LPCSTR  pSrc, LPWSTR pDst, size_t nSrcLen);
GT_DECL_BASIC (void) str_convert (LPCWSTR pSrc, LPSTR  pDst, size_t nSrcLen);

GT_DECL_BASIC (void) str_append (wstring& sDst, LPCSTR  pSrc, size_t nSrcLen = size_t (-1));
GT_DECL_BASIC (void) str_append (wstring& sDst, LPCWSTR pSrc, size_t nSrcLen = size_t (-1));
GT_DECL_BASIC (void) str_append (string&  sDst, LPCSTR  pSrc, size_t nSrcLen = size_t (-1));
GT_DECL_BASIC (void) str_append (string&  sDst, LPCWSTR pSrc, size_t nSrcLen = size_t (-1));

GT_DECL_BASIC (void) str_assign (wstring& sDst, LPCSTR  pSrc, size_t nSrcLen = size_t (-1));
GT_DECL_BASIC (void) str_assign (wstring& sDst, LPCWSTR pSrc, size_t nSrcLen = size_t (-1));
GT_DECL_BASIC (void) str_assign (string&  sDst, LPCSTR  pSrc, size_t nSrcLen = size_t (-1));
GT_DECL_BASIC (void) str_assign (string&  sDst, LPCWSTR pSrc, size_t nSrcLen = size_t (-1));

GT_DECL_BASIC (void) str_assign (wstring& sDst, const string&  sSrc);
inline void GT_CALL  str_assign (wstring& sDst, const wstring& sSrc) { (void) sDst.assign (sSrc); }
GT_DECL_BASIC (void) str_assign (string&  sDst, const wstring& sSrc);
inline void GT_CALL  str_assign (string&  sDst, const string&  sSrc) { (void) sDst.assign (sSrc); }

// the next 2 accept resid_t!!!
GT_DECL_BASIC (void) str_append_from_res (pstring& sDst, resid_t nID);
GT_DECL_BASIC (void) str_assign_from_res (pstring& sDst, resid_t nID);

// formatting helpers
GT_DECL_BASIC (void)           str_append_format (pstring& sDst, LPCTSTR pFmt, ...);
GT_DECL_BASIC (void)           str_assign_format (pstring& sDst, LPCTSTR pFmt, ...);
GT_DECL_BASIC (const pstring&) str_get_formatted (LPCTSTR pFmt, ...);

GT_DECL_BASIC (void) str_erase_trailing (pstring& sDst, TCHAR c);
GT_DECL_BASIC (void) str_replace_all (pstring& sDst, TCHAR cOld, TCHAR cNew);
GT_DECL_BASIC (void) str_replace_all (pstring& sDst, TCHAR cOld, LPCTSTR sNew);
GT_DECL_BASIC (void) str_replace_all (pstring& sDst, LPCTSTR sOld, LPCTSTR sNew);

GT_DECL_BASIC (void) str_split (const pstring& sSrc, TCHAR cSep, pstring& sPart1, pstring& sPart2);

GT_DECL_BASIC (gtuint8) hexchar_to_gtuint8 (const char c);
GT_DECL_BASIC (gtuint8) hexchar_to_gtuint8 (const wchar_t c);

// Convert the passed string to an gtuint16. String needs to have at least 4 chars!
GT_DECL_BASIC (gtuint16) str_hex_to_gtuint16 (LPCSTR p);
GT_DECL_BASIC (gtuint16) str_hex_to_gtuint16 (LPCWSTR p);

// Convert the passed string to an gtint32. String needs to have at least 8 chars!
GT_DECL_BASIC (gtint32) str_hex_to_gtint32 (LPCSTR p);
GT_DECL_BASIC (gtint32) str_hex_to_gtint32 (LPCWSTR p);

#ifdef __GNUC__
// not supported
GT_DECL_BASIC (int) _vsctprintf (LPCTSTR pFmt, va_list args);
#endif

} // namespace GT

#endif
