MAKE_ANALYZER (HLP);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (HLP)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x00035F3F;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (HLP)
//--------------------------------------------------------------------
{
  const bool bIsWin9x = (file_t (m_pBuffer->GetInt32 (12)) == m_pBuffer->GetFileSize ());

  out_format (rc (HRC_GENLIB_HLP_HEADER),
              bIsWin9x
                ? _T ("95/98")
                : _T ("3.x"));
}
