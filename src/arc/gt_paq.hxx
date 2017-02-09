//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (PAQ)
//--------------------------------------------------------------------
{
  PAQ_LocalHeader aLH;
  pstring sRealFilename;

  m_pBuffer->SetActFilePos (m_nLocalOffset + GT_PAQ_GH_SIZE + 3);

  for (;;)
  {
    m_pBuffer->GetBufferX (&aLH, GT_PAQ_LH_SIZE);

    // check for consistency....
    if (aLH.nOriginalSize < 0 ||
        aLH.nPackedSize < 0 ||
        aLH.nFilenameLength >= 13)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    str_assign (sRealFilename, aLH.sFilename, aLH.nFilenameLength);

    ArchiveFile *pArcFile = new ArchiveFile (sRealFilename);
    pArcFile->SetPackedSize (aLH.nPackedSize);
    pArcFile->SetUnpackedSize (aLH.nOriginalSize);
    _AddFile (pArcFile);

    m_pBuffer->IncActFilePos (aLH.nPackedSize + 3);

    if (m_pBuffer->IsEOF ())
      break;
  }
}
