//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (BSA)
//--------------------------------------------------------------------
{
  BSA_Header aH;
  pstring sFilename;

  _SetArchiveFlag (ARC_DT_DOS);

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);

  for (;;)
  {
    m_pBuffer->GetBufferX (&aH, GT_BSA_H_SIZE);

    // read filename
    m_pBuffer->GetFixedSizeAString (sFilename, aH.nFilenameLen);

    // add file
    ArchiveFile* pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (aH.nPackedSize);
    pArcFile->SetUnpackedSize (aH.nOriginalSize);
    pArcFile->SetDT (aH.nDT);
    _AddFile (pArcFile);

    // skip data (one additional byte....)
    m_pBuffer->IncActFilePos (1 + aH.nPackedSize);

    // eof?
    if (m_pBuffer->GetActFilePos () + 1 == m_pBuffer->GetFileSize ())
    {
      m_pBuffer->SetToEOF ();
      break;
    }
  }
}
