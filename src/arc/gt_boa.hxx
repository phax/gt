//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (BOA)
//--------------------------------------------------------------------
{
  BOA_GlobalHeader aGH;
  BOA_LocalHeader aLH;
  pstring sFilename;

  _SetArchiveFlag (ARC_DT_UNIX);

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);
  m_pBuffer->GetBufferX (&aGH, GT_BOA_GH_SIZE);

  // set the archive version
  _SetArchiveVersion (aGH.nMajorVersion, aGH.nMinorVersion);

  for (;;)
  {
    // read filename
    m_pBuffer->GetASCIIZ_AString (sFilename, GT_MAX_SIZE);

    // after the last file, an empty is put
    if (sFilename.empty ())
    {
      m_pBuffer->SetToEOF ();
      break;
    }

    // read local header
    m_pBuffer->GetBufferX (&aLH, GT_BOA_LH_SIZE);

    // add file
    ArchiveFile* pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (aLH.nPackedSize);
    pArcFile->SetUnpackedSize (aLH.nOriginalSize);
    pArcFile->SetDT (aLH.nDT);
    _AddFile (pArcFile);

    // skip data
    m_pBuffer->IncActFilePos (aLH.nPackedSize);
  }
}
