//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (AKT9)
//--------------------------------------------------------------------
{
  AKT9_GlobalHeader aGH;
  AKT9_LocalHeader aLH;
  pstring sPath;
  gtuint8 nIncCount;

  _SetArchiveFlag (ARC_DT_DOS);

  // read global header
  m_pBuffer->GetBuffer (m_nLocalOffset + 0, &aGH, GT_AKT9_GH_SIZE);

  // adn skip comment bytes...
  m_pBuffer->IncActFilePos (aGH.nCommentSize);

  for (;;)
  {
    // read the path
    m_pBuffer->GetPascalString (sPath);

    m_pBuffer->GetBufferX (&aLH, GT_AKT9_LH_SIZE);

    // check consistency
    if (aLH.nOriginalSize < 0 || aLH.nPackedSize < 0)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    str_append (sPath, aLH.sFilename, 12);

    ArchiveFile *pArcFile = new ArchiveFile (sPath);
    pArcFile->SetPackedSize (aLH.nPackedSize);
    pArcFile->SetUnpackedSize (aLH.nOriginalSize);
    pArcFile->SetDT (aLH.nDateTime);
    _AddFile (pArcFile);

    if (aLH.nCompressionMethod != 2)
    {
      nIncCount = m_pBuffer->GetUint8 ();
      m_pBuffer->IncActFilePos (nIncCount);
    }
    else
    {
      m_pBuffer->IncActFilePos (1);
    }

    m_pBuffer->IncActFilePos (aLH.nPackedSize + 1);

    if (m_pBuffer->GetActFilePos () >= m_pBuffer->GetFileSize () - 1)
    {
      m_pBuffer->SetToEOF ();
      break;
    }
  }
}
