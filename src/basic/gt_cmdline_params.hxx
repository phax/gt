#ifndef _GT_CMDLINE_PARAMS_HXX_
#define _GT_CMDLINE_PARAMS_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gt_stringhelper.hxx"
#include <vector>

namespace GT {

// common flag
#define GT_FLAG_LISTMODE        _T ("l")

// GT specific commandline flags
#define GT_FLAG_EXEMOD          _T ("exemod")
#define GT_FLAG_FTE             _T ("fte")
#define GT_FLAG_FTN             _T ("ftn")
#define GT_FLAG_FTP             _T ("ftp")
#define GT_FLAG_FTS             _T ("fts")
#define GT_FLAG_NOTIDENTIFIED   _T ("ni")
#define GT_FLAG_NOARCS          _T ("noarcs")
#define GT_FLAG_NOFOOTER        _T ("nofooter")
#define GT_FLAG_NOOVR           _T ("noovr")
#define GT_FLAG_NOSCANEXT       _T ("noscanext")
#define GT_FLAG_NOSCANNAME      _T ("noscanname")
#define GT_FLAG_NOTUNKNOWN      _T ("nu")
#define GT_FLAG_PEALL           _T ("peall")
#define GT_FLAG_PEDD            _T ("pedd")
#define GT_FLAG_PEEXP           _T ("peexp")
#define GT_FLAG_PEFIXUP         _T ("pefixup")
#define GT_FLAG_PEIMP           _T ("peimp")
#define GT_FLAG_PEIMP2          _T ("peimp2")
#define GT_FLAG_PERES           _T ("peres")
#define GT_FLAG_PERES2          _T ("peres2")
#define GT_FLAG_PESECT          _T ("pesect")
#define GT_FLAG_PEVER           _T ("pever")
#define GT_FLAG_STATUS          _T ("status")
#define GT_FLAG_WAITFORKEY      _T ("wf")

// for GTUI2
#define GT_FLAG_GTUI2           _T ("gtui2")
#define GT_FLAG_ZHWND           _T ("zhwnd")

//--------------------------------------------------------------------
enum ECmdlineParamType
//--------------------------------------------------------------------
{
  GT_FLAGTYPE_INVALID = 160,
  GT_FLAGTYPE_BOOL,
  GT_FLAGTYPE_LONG,
  GT_FLAGTYPE_STRING,
  GT_FLAGTYPE_LAST
};

//--------------------------------------------------------------------
struct tParamBool
//--------------------------------------------------------------------
{
  LPCTSTR m_sName;
  resid_t m_nHelpResID;
  bool    m_aDefault;
};

//--------------------------------------------------------------------
struct tParamLong
//--------------------------------------------------------------------
{
  LPCTSTR m_sName;
  resid_t m_nHelpResID;
  gtint32 m_aDefault;
};

//--------------------------------------------------------------------
struct tParamString
//--------------------------------------------------------------------
{
  LPCTSTR m_sName;
  resid_t m_nHelpResID;
  LPCTSTR m_aDefault;
};

//--------------------------------------------------------------------
class CmdlineParam
//--------------------------------------------------------------------
{
public:
  const ECmdlineParamType m_eType;
        LPCTSTR           m_sName;
  const size_t            m_nNameLen;
  const resid_t           m_nHelpResID;

  CmdlineParam (const ECmdlineParamType eType,
                      LPCTSTR           sName,
                const resid_t           nHelpResID)
    : m_eType      (eType),
      m_sName      (sName),
      m_nNameLen   (_tcslen (sName)),
      m_nHelpResID (nHelpResID)
  {}

  virtual ~CmdlineParam ()
  {
    /* purposly left empty */
  }

  // pure virtual methods
  virtual void SetToDefault () = 0;
  virtual void AppendToString (pstring& s) = 0;
  virtual bool IsSet () const = 0;
};

//--------------------------------------------------------------------
class CmdlineParamBool : public CmdlineParam
//--------------------------------------------------------------------
{
public:
  const bool m_aDefault;
        bool m_aValue;

  explicit CmdlineParamBool (const tParamBool& aFlag)
    : CmdlineParam (GT_FLAGTYPE_BOOL, aFlag.m_sName, aFlag.m_nHelpResID),
      m_aDefault   (aFlag.m_aDefault ? true : false),
      m_aValue     (aFlag.m_aDefault ? true : false)
  {}

  virtual void SetToDefault ()             { m_aValue = m_aDefault; }
  virtual void AppendToString (pstring& s) { str_append_format (s, _T (" -%s"), m_sName); }
  virtual bool IsSet () const              { return !(m_aValue == m_aDefault); }
};

//--------------------------------------------------------------------
class CmdlineParamLong : public CmdlineParam
//--------------------------------------------------------------------
{
public:
  const gtint32 m_aDefault;
        gtint32 m_aValue;

  explicit CmdlineParamLong (const tParamLong& aFlag)
    : CmdlineParam (GT_FLAGTYPE_LONG, aFlag.m_sName, aFlag.m_nHelpResID),
      m_aDefault   (aFlag.m_aDefault),
      m_aValue     (aFlag.m_aDefault)
  {}

  virtual void SetToDefault ()             { m_aValue = m_aDefault; }
  virtual void AppendToString (pstring& s) { str_append_format (s, _T (" -%s%d"), m_sName, m_aValue); }
  virtual bool IsSet () const              { return !(m_aValue == m_aDefault); }
};

//--------------------------------------------------------------------
class CmdlineParamString : public CmdlineParam
//--------------------------------------------------------------------
{
public:
  const pstring m_aDefault;
        pstring m_aValue;

  explicit CmdlineParamString (const tParamString& aFlag)
    : CmdlineParam (GT_FLAGTYPE_STRING, aFlag.m_sName, aFlag.m_nHelpResID),
      m_aDefault   (aFlag.m_aDefault),
      m_aValue     (aFlag.m_aDefault)
  {}

  virtual void SetToDefault ()             { m_aValue = m_aDefault; }
  virtual void AppendToString (pstring& s) { str_append_format (s, _T (" \"-%s%s\""), m_sName, m_aValue.c_str ()); }
  virtual bool IsSet () const              { return !(m_aValue == m_aDefault); }
};

/* must be a 'vector' because we cannot 'sort' a 'list' since
   a 'list::iterator' has no overloaded '-' operator!
 */
typedef std::vector <CmdlineParam*> CmdlineParamList;

//--------------------------------------------------------------------
class FlagSeq : public CmdlineParamList
//--------------------------------------------------------------------
{
public:
  /*! Destructor - delete all elements.
   */
  //------------------------------------------------------------------
  virtual ~FlagSeq ()
  //------------------------------------------------------------------
  {
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
      delete (*cit);
  }

  /*! Get the flag with the given name.
      \param sSwitch commandline switch to search
      \retval NULL not found
      \return pointer to the CmdlineParam object
   */
  //------------------------------------------------------------------
  CmdlineParam* GetFlagOfFullName (LPCTSTR sSwitch) const
  //------------------------------------------------------------------
  {
    ASSERT (sSwitch);
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
      if (_tcscmp (sSwitch, (*cit)->m_sName) == 0)
        return *cit;
    return NULL;
  }

  /*! Get the flag that partially matches the given name.
      The problem here is:
        Both /s and /statusX are valid command.
      The solution is:
        Bool parameters need a full match
      \param sSwitch commandline switch (including and param)
      \retval NULL not found
      \return pointer to the CmdlineParam object
   */
  //------------------------------------------------------------------
  CmdlineParam* GetFlagOfPartialName (LPCTSTR sSwitch) const
  //------------------------------------------------------------------
  {
    ASSERT (sSwitch);
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
    {
      if ((*cit)->m_eType == GT_FLAGTYPE_BOOL)
      {
        // full match required!
        if (_tcscmp (sSwitch, (*cit)->m_sName) == 0)
          return *cit;
      }
      else
      {
        // partial match is okay!
        if (_tcsncmp (sSwitch, (*cit)->m_sName, (*cit)->m_nNameLen) == 0)
          return *cit;
      }
    }
    return NULL;
  }

  //------------------------------------------------------------------
  void SetToDefault ()
  //------------------------------------------------------------------
  {
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
      (*cit)->SetToDefault ();
  }

  //------------------------------------------------------------------
  void IfSetAppendToString (pstring& sResult) const
  //------------------------------------------------------------------
  {
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
      if ((*cit)->IsSet ())
        (*cit)->AppendToString (sResult);
  }
};

GT_DECL_BASIC (bool)    CmdlineParam_Register (CmdlineParam* pFlag);
GT_DECL_BASIC (void)    CmdlineParam_SetToDefault ();
GT_DECL_BASIC (pstring) CmdlineParam_GetAsString ();

GT_DECL_BASIC (bool)    CmdlineParam_GetBool (LPCTSTR sName);
GT_DECL_BASIC (gtint32) CmdlineParam_GetLong (LPCTSTR sName);
GT_DECL_BASIC (pstring) CmdlineParam_GetString (LPCTSTR sName);

GT_DECL_BASIC (bool) CmdlineParam_Apply (LPCTSTR s);
GT_DECL_BASIC (bool) CmdlineParam_ApplyBool (LPCTSTR sName, const bool bValue);
GT_DECL_BASIC (bool) CmdlineParam_ApplyLong (LPCTSTR sName, const gtint32 nValue);

GT_DECL_BASIC (FlagSeq const*) CmdlineParam_GetList ();

}  // namespace

#endif
