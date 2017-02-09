//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (LG)
//--------------------------------------------------------------------
{
  LG_Header aH;
  pstring sDirectory;
  pstring sFilename;

  _SetArchiveFlag (ARC_DT_DOS);

  m_pBuffer->SetActFilePos (m_nLocalOffset + 4);

  for (;;)
  {
    if (!m_pBuffer->GetBuffer (&aH, GT_LG_H_SIZE))
      break;

    // check if the values are okay...
    if (aH.nOriginalSize < 0 || aH.nPackedSize < 0)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    // 1. get the directory
    m_pBuffer->GetASCIIZ_AString (sDirectory, GT_MAX_SIZE);

    // 2. get the filename
    m_pBuffer->GetASCIIZ_AString (sFilename, GT_MAX_SIZE);

    sDirectory += sFilename;

    ArchiveFile *pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (aH.nPackedSize);
    pArcFile->SetUnpackedSize (aH.nOriginalSize);
    pArcFile->SetDT (aH.nDateTime);
    _AddFile (pArcFile);

    m_pBuffer->IncActFilePos (2);

    if ((aH.nFlags & 0xF0) > 0 && aH.nFlags != 81)
      m_pBuffer->IncActFilePos (3);

    m_pBuffer->IncActFilePos (aH.nPackedSize);
  }
}
