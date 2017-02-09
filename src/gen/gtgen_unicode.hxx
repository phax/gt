MAKE_ANALYZER (UnicodeText);

/*! binary only (too many 0-bytes!)
 */
//--------------------------------------------------------------------
MAKE_IS_A (UnicodeText)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetUint16 (0) == 0xfeff ||
         aGI.m_pBuffer->GetUint16 (0) == 0xfffe ||
         aGI.m_pBuffer->GetInt32 (0) == 0xfffe0000 ||
         aGI.m_pBuffer->GetInt32 (0) == 0x0000feff;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (UnicodeText)
//--------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_UNICODE_TEXT_HEADER));
}
