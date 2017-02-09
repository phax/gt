namespace _TAR {

/*! returns -1 on error
 */
//--------------------------------------------------------------------
static gtuint32 _otol (char *s, size_t nLength)
//--------------------------------------------------------------------
{
  const size_t P_MAX_OCT_TABLE = 11;
  static const gtuint32 P_OCT_TABLE [P_MAX_OCT_TABLE] =
          { 1,             // \000000000001
            8,             // \000000000010
            64,            // \000000000100
            512,           // \000000001000
            4096,          // \000000010000
            32768,         // \000000100000
            262144,        // \000001000000
            2097152,       // \000010000000
            16777216,      // \000100000000
            134217728,     // \001000000000
            1073741827 };  // \010000000000

  size_t i;
  gtuint8 c;
  gtuint32 nResult = 0;

  ASSERT (s);
  ASSERT (nLength > 0);

  // replace all ' ' with '0'
  for (i = 0; i < nLength; i++)
    if (s[i] == ' ')
      s[i] = '0';
  s[nLength] = '\0';

  for (i = 0; i < P_MAX_OCT_TABLE; i++)
  {
    c = s[i];
    if (!isdigit (c))
      return gtuint32 (-1);
    nResult += (c - '0') * P_OCT_TABLE [P_MAX_OCT_TABLE - 1 - i];
  }

  return nResult;
}

}  // namespace _TAR

//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (TAR)
//--------------------------------------------------------------------
{
  TAR_Header aH;
  pstring sFilename;

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);

  for (;;)
  {
    m_pBuffer->GetBufferX (&aH, GT_TAR_H_SIZE);

    // end of file???
    // eg. the source of cvs 1.12.4 has no ustar in the header :(
    if (0 && strncmp (aH.sMagic, "ustar", 5) != 0)
      break;

    // cannot be const because of p_Align
    gtuint32 nSize = _TAR::_otol (aH.sSize, 11);

    if (nSize > m_pBuffer->GetFileSize ())
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    // check for directory
    str_assign (sFilename, aH.sName);

    ArchiveFile* pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (nSize);
    pArcFile->SetUnpackedSize (nSize);
    _AddFile (pArcFile);

    // continue on the next 512 byte boundary
    _align (nSize, 512);
    m_pBuffer->IncActFilePos (nSize);
  }
}
