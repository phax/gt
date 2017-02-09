MAKE_ANALYZER (CLASS);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (CLASS)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0xBEBAFECA;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (CLASS)
//--------------------------------------------------------------------
{
  const gtuint16 nMinor = WORD_SWAP_BYTE (m_pBuffer->GetUint16 (4));
  const gtuint16 nMajor = WORD_SWAP_BYTE (m_pBuffer->GetUint16 (6));
  // Source: http://java.sun.com/docs/books/jvms/second_edition/html/ClassFile.doc.html#75883
  const pstring sVersion = nMajor == 45 ? _T ("1.0.2/1.1") :
                           nMajor == 46 ? _T ("1.2") :
                           nMajor == 47 ? _T ("1.3") :
                           nMajor == 48 ? _T ("1.4") :
                           nMajor == 49 ? _T ("1.5") :
                           nMajor == 50 ? _T ("1.6") : _T ("unknown");
  out_format (rc (HRC_GENLIB_CLASS_HEADER), nMajor, nMinor, sVersion.c_str ());
}
