#ifndef _GTLIB_EXCEPTION_HXX_
#define _GTLIB_EXCEPTION_HXX_

#ifdef _MSC_VER
#pragma once
#endif

namespace GT {

//! use ONLY for internal errors (like release assertions)
GT_DECL_BASIC (void) InternalError
                                        (      LPCTSTR  sFilename,
                                               LPCTSTR  sSourceFilename,
                                         const gtuint32 nLineNumber);

//--------------------------------------------------------------------
class GT_EXPORT_BASIC Excpt
//--------------------------------------------------------------------
{
protected:
  pstring  m_sMessage;
  pstring  m_sFilename;
  gtuint32 m_nLine;

public:

#ifdef _MSC_VER
#pragma warning (disable:4096)  // disable warning, that cdecl is missing
#endif

  Excpt (      LPCTSTR  sFilename,
         const gtuint32 nLine,
               LPCTSTR  sMessage,
         ...);

#ifdef _MSC_VER
#pragma warning (default:4096)
#endif

  const pstring& GetText () const { return m_sMessage; }
  void           AddText (LPCTSTR pText);
  void           Display () const;
};

}  // namespace

#endif
