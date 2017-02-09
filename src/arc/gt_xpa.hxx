//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (XPA)
//--------------------------------------------------------------------
{
  XPA_LocalHeader aLH;
  pstring sFilename;

  m_pBuffer->SetActFilePos (m_nLocalOffset + GT_XPA_GH_SIZE);

  for (;;)
  {
    m_pBuffer->GetPascalString (sFilename);
    m_pBuffer->GetBuffer (&aLH, GT_XPA_LH_SIZE);

    ArchiveFile *pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (aLH.nNewSize);
    pArcFile->SetUnpackedSize (aLH.nOldSize);
    _AddFile (pArcFile);

    // because of signed/unsigned mismatch :)
    if (m_pBuffer->GetActFilePos () + aLH.nNewSize >= m_pBuffer->GetFileSize () - 1)
    {
      m_pBuffer->SetToEOF ();
      break;
    }
    m_pBuffer->IncActFilePos (aLH.nNewSize);
  }
}
