#include "gt_cmdline_params.hxx"

namespace GT {

// one and only instance
FlagSeq g_aFlags;

//--------------------------------------------------------------------
GT_IMPL_BASIC (bool) CmdlineParam_Register (CmdlineParam *pFlag)
//--------------------------------------------------------------------
{
  ASSERT (pFlag);

  // check that there are no duplicates in the list!
  if (g_aFlags.GetFlagOfFullName (pFlag->m_sName))
    return false;

  g_aFlags.push_back (pFlag);
  return true;
}

/*! Set all flags to default values
 */
//--------------------------------------------------------------------
GT_IMPL_BASIC (void) CmdlineParam_SetToDefault ()
//--------------------------------------------------------------------
{
  g_aFlags.SetToDefault ();
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (pstring) CmdlineParam_GetAsString ()
//--------------------------------------------------------------------
{
  pstring sResult;
  g_aFlags.IfSetAppendToString (sResult);
  return sResult;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (bool) CmdlineParam_GetBool (LPCTSTR sName)
//--------------------------------------------------------------------
{
  ASSERT (sName);
  CmdlineParam *pFlag = g_aFlags.GetFlagOfFullName (sName);
  ASSERT (pFlag && pFlag->m_eType == GT_FLAGTYPE_BOOL);

  return pFlag && pFlag->m_eType == GT_FLAGTYPE_BOOL
           ? dynamic_cast <CmdlineParamBool*> (pFlag)->m_aValue
           : false;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (gtint32) CmdlineParam_GetLong (LPCTSTR sName)
//--------------------------------------------------------------------
{
  ASSERT (sName);
  CmdlineParam *pFlag = g_aFlags.GetFlagOfFullName (sName);
  ASSERT (pFlag && pFlag->m_eType == GT_FLAGTYPE_LONG);

  return pFlag && pFlag->m_eType == GT_FLAGTYPE_LONG
           ? dynamic_cast <CmdlineParamLong*> (pFlag)->m_aValue
           : 0;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (pstring) CmdlineParam_GetString (LPCTSTR sName)
//--------------------------------------------------------------------
{
  ASSERT (sName);
  CmdlineParam *pFlag = g_aFlags.GetFlagOfFullName (sName);
  ASSERT (pFlag && pFlag->m_eType == GT_FLAGTYPE_STRING);

  return pFlag && pFlag->m_eType == GT_FLAGTYPE_STRING
           ? dynamic_cast <CmdlineParamString*> (pFlag)->m_aValue
           : pstring ();
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (bool) CmdlineParam_Apply (LPCTSTR s)
//--------------------------------------------------------------------
{
  ASSERT (s);

//  _tprintf (_T ("Trying to apply '%s'"), s);

  // already contained?
  // -> use partial match because of the "/s" and "/status$P" problem!
  CmdlineParam* pFlag = g_aFlags.GetFlagOfPartialName (s);
  if (!pFlag)
    return false;

  // determine type
  switch (pFlag->m_eType)
  {
    case GT_FLAGTYPE_BOOL:
    {
      CmdlineParamBool *pFlagBool = dynamic_cast <CmdlineParamBool*> (pFlag);
      pFlagBool->m_aValue = !pFlagBool->m_aDefault;  // invert bool!
      break;
    }
    case GT_FLAGTYPE_LONG:
    {
      CmdlineParamLong *pFlagLong = dynamic_cast <CmdlineParamLong*> (pFlag);
      pFlagLong->m_aValue = _ttol (s + pFlag->m_nNameLen);  // get position of value
      break;
    }
    case GT_FLAGTYPE_STRING:
    {
      CmdlineParamString *pFlagString = dynamic_cast <CmdlineParamString*> (pFlag);
      pFlagString->m_aValue = s + pFlag->m_nNameLen;  // get position of value
      break;
    }
    default:
      ASSERT (0);
      break;
  }

  return true;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (bool) CmdlineParam_ApplyBool
                                        (      LPCTSTR sName,
                                         const bool    bValue)
//--------------------------------------------------------------------
{
  ASSERT (sName);
  CmdlineParam *pFlag = g_aFlags.GetFlagOfFullName (sName);
  ASSERT (pFlag && pFlag->m_eType == GT_FLAGTYPE_BOOL);

  if (pFlag && pFlag->m_eType == GT_FLAGTYPE_BOOL)
  {
    dynamic_cast <CmdlineParamBool*> (pFlag)->m_aValue = bValue;
    return true;
  }

  return false;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (bool) CmdlineParam_ApplyLong
                                        (      LPCTSTR sName,
                                         const gtint32 nValue)
//--------------------------------------------------------------------
{
  ASSERT (sName);
  CmdlineParam *pFlag = g_aFlags.GetFlagOfFullName (sName);
  ASSERT (pFlag && pFlag->m_eType == GT_FLAGTYPE_LONG);

  if (pFlag && pFlag->m_eType == GT_FLAGTYPE_LONG)
  {
    dynamic_cast <CmdlineParamLong*> (pFlag)->m_aValue = nValue;
    return true;
  }

  return false;
}

/*! Get access to the list of registeres cmdline parameters.
 */
//--------------------------------------------------------------------
GT_IMPL_BASIC (FlagSeq const*) CmdlineParam_GetList ()
//--------------------------------------------------------------------
{
  return &g_aFlags;
}

}  // namespace
