//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ARX)
//--------------------------------------------------------------------
{
  ARX_Header aH;
  pstring sFileName;

  _SetArchiveFlag (ARC_DT_DOS);

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);

  for (;;)
  {
    m_pBuffer->GetBufferX (&aH, GT_ARX_H_SIZE);

    if (aH.nPackedSize == 0)
      aH.nPackedSize = aH.nOriginalSize;

    m_pBuffer->GetPascalString (sFileName);

    ArchiveFile *pArcFile = new ArchiveFile (sFileName);
    pArcFile->SetPackedSize (aH.nPackedSize);
    pArcFile->SetUnpackedSize (aH.nOriginalSize);
    pArcFile->SetDT (aH.nDateTime);
    _AddFile (pArcFile);

    m_pBuffer->IncActFilePos (aH.nPackedSize + 1);

    if (m_pBuffer->GetActFilePos () >= m_pBuffer->GetFileSize () - 1)
    {
      m_pBuffer->SetToEOF ();
      break;
    }
  }
}
