MAKE_ANALYZER (PGP);

/*! text only
 */
//--------------------------------------------------------------------
MAKE_IS_A (PGP)
//--------------------------------------------------------------------
{
  if (!aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->CompareA (0, 14, "-----BEGIN PGP");
}

//--------------------------------------------------------------------
MAKE_DISPLAY (PGP)
//--------------------------------------------------------------------
{
  pstring sVersion;
  file_t nStart = m_pBuffer->Find (':', 27, 50);
  if (nStart != GT_NOT_FOUND)
  {
    char sChar[128];
    m_pBuffer->GetUntil (nStart + 2, 13, sChar, 128 - 1);
    str_assign (sVersion, sChar);
  }
  else
  {
    sVersion = _T ("???");
  }

  out_format (rc (HRC_GENLIB_PGP_HEADER),
              sVersion.c_str ());
}
