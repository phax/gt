MAKE_ANALYZER (CCDLNG);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (CCDLNG)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x4835C9B8;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (CCDLNG)
//--------------------------------------------------------------------
{
  // the language is stored in plain text at offset 8
  pstring sLanguage;
  m_pBuffer->GetASCIIZ_AString (8, sLanguage, 28);

  out_format (rc (HRC_GENLIB_CCDLNG),
              sLanguage.c_str ());
}
