//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ZOO)
//--------------------------------------------------------------------
{
  ZOO_GlobalHeader aGH;
  ZOO_LocalHeader aLH;
  pstring sFilename;

  // read global header...
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_ZOO_GH_SIZE);

  // archive version...
  _SetArchiveVersion (aGH.nPackVersion, 0);

  // goto analyzing offset...
  m_pBuffer->SetActFilePos (aGH.nOffsetOfFirstFile);

  for (;;)
  {
    m_pBuffer->GetBufferX (&aLH, GT_ZOO_LH_SIZE);

    // found end of archive??
    if (aLH.nOffsetNextDir == 0)
      break;

    // check for correct values....
    if ((gtint32) aLH.nOriginalSize < 0 || (gtint32) aLH.nPackedSize < 0)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    // read until next '\0'
    m_pBuffer->GetASCIIZ_AString (sFilename, GT_MAX_SIZE);

    ArchiveFile *pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (aLH.nPackedSize);
    pArcFile->SetUnpackedSize (aLH.nOriginalSize);
// #f# buggy
//    pArcFile->SetDT (aLH.nDateTime);
    _AddFile (pArcFile);

    m_pBuffer->SetActFilePos (aLH.nOffsetNextDir);
  }

  m_pBuffer->SetToEOF ();
}
