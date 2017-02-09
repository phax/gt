MAKE_ANALYZER (PIF);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (PIF)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetUint8 (0) == 0 &&
         aGI.m_pBuffer->GetFileSize () > 0x200 &&
         aGI.m_pBuffer->CompareA (0x171, 15, "MICROSOFT PIFEX");
}

/*! Get information about a Windows PIF file.
    PIF = Program Information File.
 */
//--------------------------------------------------------------------
MAKE_DISPLAY (PIF)
//--------------------------------------------------------------------
{
  // get char buffer
  char sChar[32];
  m_pBuffer->GetBufferX (2, sChar, 30);
  sChar[30] = '\0';

  // erase trailing spaces
  int i = 29;
  while (i > 0 && sChar[i] == ' ')
    i--;
  sChar[i + 1] = '\0';

  // convert
  pstring sText;
  str_assign (sText, sChar);

  // start building result sting
  out_format (rc (HRC_GENLIB_PIF_HEADER),
              sText.c_str ());
}
