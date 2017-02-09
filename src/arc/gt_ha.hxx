//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (HA)
//--------------------------------------------------------------------
{
  HA_LocalHeader aLH;
  pstring sDirectory, sFilename;
  pstring::iterator it;

  m_pBuffer->SetActFilePos (m_nLocalOffset + GT_HA_GH_SIZE);  // skip the header

  for (;;)
  {
    if (!m_pBuffer->GetBuffer (&aLH, GT_HA_LH_SIZE))
      break;

    if (aLH.nOriginalSize < 0 || aLH.nPackedSize < 0)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    // first get the directory...
    m_pBuffer->GetASCIIZ_AString (sDirectory, GT_MAX_SIZE);

    // unfortunately HA saves '\\' as #0xFF ...
    str_replace_all (sDirectory, TCHAR (0xff), _T ('\\'));

    // ... and then the filename
    m_pBuffer->GetASCIIZ_AString (sFilename, GT_MAX_SIZE);

    sDirectory += sFilename;

    ArchiveFile *pArcFile = new ArchiveFile (sDirectory);
    pArcFile->SetPackedSize (aLH.nPackedSize);
    pArcFile->SetUnpackedSize (aLH.nOriginalSize);
    _AddFile (pArcFile);

    const gtuint8 nSpecial = m_pBuffer->GetUint8 ();
    m_pBuffer->IncActFilePos (nSpecial + aLH.nPackedSize);
  }
}
