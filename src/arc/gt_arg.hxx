//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ARG)
//--------------------------------------------------------------------
{
  ARG_Header aH;
  pstring sFilename;

  _SetArchiveFlag (ARC_DT_DOS);

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);

  for (;;)
  {
    m_pBuffer->GetBufferX (&aH, GT_ARG_H_SIZE);

    // check for invalid values
    if (aH.nOriginalSize < 0 || aH.nPackedSize < 0)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    // convert from char* to TCHAR*
    str_assign (sFilename, aH.sFilename);

    ArchiveFile* pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (aH.nPackedSize);
    pArcFile->SetUnpackedSize (aH.nOriginalSize);
    pArcFile->SetDT (aH.nDateTime);
    _AddFile (pArcFile);

    m_pBuffer->IncActFilePos (aH.nPackedSize);
  }
}
