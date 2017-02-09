//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ARC)
//--------------------------------------------------------------------
{
  ARC_Header aH;
  pstring sFilename;

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);

  for (;;)
  {
    m_pBuffer->GetBufferX (&aH, GT_ARC_H_SIZE);

    if (aH.nCompressionMethod == 0)
      break;

    if (aH.nCompressionMethod >= 1 && aH.nCompressionMethod <= 19)
    {
      // check for invalid values
      if (aH.nOriginalSize < 0 || aH.nPackedSize < 0)
      {
        SetErrorCode (ARCERR_UNEXPECTED_VALUE);
        break;
      }

      // conversion from char* to TCHAR*
      str_assign (sFilename, aH.sFilename);

      ArchiveFile* pArcFile = new ArchiveFile (sFilename);
      pArcFile->SetPackedSize (aH.nPackedSize);
      pArcFile->SetUnpackedSize (aH.nOriginalSize);
      _AddFile (pArcFile);
    }
    else
    if (aH.nCompressionMethod >= 20 && aH.nCompressionMethod <= 29)
    {
      // comment...
      // -> ignore
    }
    else
    {
      // unknown compression method...
      SetErrorCode (ARCERR_UNKNOWN_METHOD);
      break;
    }

    m_pBuffer->IncActFilePos (aH.nPackedSize);
  }
}
