MAKE_ANALYZER (APETAG);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (APETAG)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  // compare first three bytes
  return aGI.m_pBuffer->CompareA (0, 8, "APETAGEX");
}

//--------------------------------------------------------------------
MAKE_DISPLAY (APETAG)
//--------------------------------------------------------------------
{
  APETAG_Header aApeTag;
  m_pBuffer->GetBufferX (0, &aApeTag, APETAG_H_SIZE);

  out_format (_T ("APETag %d.%03d for %d bytes\n"),
              aApeTag.nVersion / 1000,
              aApeTag.nVersion % 1000,
              aApeTag.nLength);

  if (!m_bListMode)
  {
    out_incindent ();

    // read tag into mem
    char* pBuf = (char*) _alloca (aApeTag.nLength - APETAG_H_SIZE);
    m_pBuffer->GetBufferX (pBuf, aApeTag.nLength - APETAG_H_SIZE);

    // read all tags
    char *p = pBuf;
    char *pEnd = pBuf + aApeTag.nLength - APETAG_H_SIZE;
    pstring sName, sValue;
    for (gtuint32 i = 0; i < aApeTag.nTagCount && p < pEnd; ++i)
    {
      gtuint32 nLen = *((gtuint32*)p);
      p += 4;
      // skip flags    gtuint32 nFlags = *((gtuint32*)p);
      p += 4;

      // tag name is stored with trailing '\0'!!
      str_assign (sName, p);
      p += strlen (p) + 1;

      // value does not contain the trailing '\0'
      str_assign (sValue, p, nLen);
      p += nLen;

      out_format (rc (HRC_GENLIB_APETAG_FIELD),
                  sName.c_str (),
                  sValue.c_str ());
    }

    out_decindent ();
  }
}
