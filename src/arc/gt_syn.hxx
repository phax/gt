//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (SYN)
//--------------------------------------------------------------------
{
  SYN_GlobalHeader aGH;
  SYN_LocalHeader aLH;
  pstring sFilename;

  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_SYN_GH_SIZE);

  file_t nRestSize = 0;

  for (gtuint32 i = 0; i < aGH.nNumberOfFiles; i++)
  {
    m_pBuffer->GetBufferX (&aLH, GT_SYN_LH_SIZE);

    str_assign (sFilename, aLH.sFilename);

    ArchiveFile *pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (aLH.nSize);
    pArcFile->SetUnpackedSize (aLH.nSize);
    _AddFile (pArcFile);

    nRestSize += aLH.nSize;
  }

  m_pBuffer->IncActFilePos (nRestSize);
}
