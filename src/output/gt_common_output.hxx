#ifndef _GT_COMMON_OUTPUT_HXX_
#define _GT_COMMON_OUTPUT_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include <list>

#include "gt_output.hxx"

namespace GT {

//--------------------------------------------------------------------
enum EOutType
//--------------------------------------------------------------------
{
  OUT_STRING = 8,
  OUT_PLAIN,
  OUT_COLOR,
  OUT_DEFCOLOR,
};

//--------------------------------------------------------------------
class OutputMember
//--------------------------------------------------------------------
{
private:
  EOutType m_eType;
  union
  {
    mutable void* m_pData;
    EColor        m_eColor;
  };

  OutputMember ();  //!< declared only

public:
  //! ctor: type without data
  explicit OutputMember (const EOutType eType)
    : m_eType (eType),
      m_pData (NULL)
  {}

  //! copy ctor - change data ownership
  OutputMember (const OutputMember &r)
    : m_eType (r.m_eType),
      m_pData (r.m_pData)
  {
    // change ownership
    r.m_pData = NULL;
  }

  //! ctor: string
  OutputMember (const pstring &s, const EOutType eType)
    : m_eType (eType),
      m_pData (new pstring (s))
  {}

  //! ctor: string
  OutputMember (LPCTSTR s, const EOutType eType)
    : m_eType (eType),
      m_pData (new pstring (s))
  {}

  //! ctor: string
  OutputMember (LPCTSTR s, size_t len, const EOutType eType)
    : m_eType (eType),
      m_pData (new pstring (s, len))
  {}

  //! ctor: color
  explicit OutputMember (const EColor e)
    : m_eType  (OUT_COLOR),
      m_eColor (e)
  {}

  // memory is deleted manually after consumption!

  EOutType GetType () const
  {
    return m_eType;
  }

  EColor AsColor () const
  {
    // cast pointer to enum :,|
    ASSERT (m_eType == OUT_COLOR);
    return m_eColor;
  }

  pstring* AsString ()
  {
    ASSERT (m_eType != OUT_COLOR);
    return reinterpret_cast <pstring*> (m_pData);
  }
};

typedef std::list <OutputMember> OutputMemberList;

//! common for stdout and HTML
void GT_CALL _CheckForEndlessLoop (pstring* pStr);

}  // namespace

#endif
