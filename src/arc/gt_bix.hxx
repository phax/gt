//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (BIX)
//--------------------------------------------------------------------
{
  BIX_GlobalHeader aGH;
  BIX_LocalHeader aLH;
  pstring sFilename;

  _SetArchiveFlag (ARC_DT_DOS);

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);
  m_pBuffer->GetBufferX (&aGH, GT_BIX_GH_SIZE);

  for (;;)
  {
    if (!m_pBuffer->GetBuffer (&aLH, GT_BIX_LH_SIZE))
      break;

    // read filename
    m_pBuffer->GetFixedSizeAString (sFilename, aLH.nFilenameLen);

    // add file
    ArchiveFile* pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (aLH.nPackedSize);
    pArcFile->SetUnpackedSize (aLH.nOriginalSize);
    pArcFile->SetDT (aLH.nDT);
    _AddFile (pArcFile);

    // skip any remaining header data (can be 0!)
    m_pBuffer->IncActFilePos (aLH.nHeaderSize - GT_BIX_LH_SIZE - aLH.nFilenameLen);

    // skip data
    m_pBuffer->IncActFilePos (aLH.nPackedSize);
  }
}
