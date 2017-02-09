MAKE_ANALYZER (ImpulseTracker);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (ImpulseTracker)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x4D504D49;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (ImpulseTracker)
//--------------------------------------------------------------------
{
  pstring s;
  m_pBuffer->GetASCIIZ_AString (3, s, GT_MAX_SIZE);

  out_format (rc (HRC_GENLIB_IT_HEADER),
              s.c_str ());
}
