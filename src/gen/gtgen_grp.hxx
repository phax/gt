MAKE_ANALYZER (GRP);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (GRP)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x43434D50;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (GRP)
//--------------------------------------------------------------------
{
  pstring s;
  m_pBuffer->GetASCIIZ_AString (40, s, GT_MAX_SIZE);

  out_format (rc (HRC_GENLIB_GRP_HEADER),
              s.c_str ());
}

