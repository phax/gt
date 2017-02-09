//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (BLI)
//--------------------------------------------------------------------
{
  bool bFirst = true;
  file_t nPrevPos;
  file_t nHeadPos = 0;
  file_t nPackedSize;
  gtuint32 nOriginalSize = 0, nDateTime = 0;
  pstring sFilename;
  BLI_GlobalHeader aGH;
  BLI_LocalHeader250 aLH;

  _SetArchiveFlag (ARC_DT_DOS);

  // read global header...
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_BLI_GH_SIZE);

  _SetArchiveVersion (aGH.nVersion / 100, aGH.nVersion % 100);

  if (aGH.nVersion != 250 && aGH.nVersion != 251)
  {
    // only 250 and 251 are supported!
    SetErrorCode (ARCERR_UNKNOWN_METHOD);    return;
  }

  for (;;)
  {
    if (!m_pBuffer->GetBuffer (&aLH, GT_BLI_LH_SIZE))
      break;

    BLI_SkipHeader *pSH = (BLI_SkipHeader*) &aLH;

    if (pSH->nMethod <= 1)
    {
      m_pBuffer->IncActFilePos (GT_BLI_SH_SIZE + pSH->nBytesToSkip - GT_BLI_LH_SIZE);
      continue;
    }

    // list file
    nPrevPos = nHeadPos;
    nHeadPos = m_pBuffer->GetActFilePos () - GT_BLI_LH_SIZE;
    nPackedSize = nHeadPos - nPrevPos;

    if (!bFirst)
    {
      ArchiveFile *pArcFile = new ArchiveFile (sFilename);
      pArcFile->SetPackedSize (nPackedSize);
      pArcFile->SetUnpackedSize (nOriginalSize);
      pArcFile->SetDT (nDateTime);
      _AddFile (pArcFile);
    }
    else
    {
      bFirst = false;
      // done list file
    }

    if (aGH.nVersion == 250)
    {
      BLI_LocalHeader250 *pLH = (BLI_LocalHeader250*) &aLH;
      nOriginalSize = pLH->nOriginalSize;
      nDateTime = (pLH->nDate << 16) + pLH->nTime;

      str_assign (sFilename, pLH->sFilename, 13);
    }
    else
      if (aGH.nVersion == 251)
      {
        BLI_LocalHeader251 *pLH = (BLI_LocalHeader251*) &aLH;
        nOriginalSize = pLH->nOriginalSize;
        nDateTime = (pLH->nDate << 16) + pLH->nTime;

        str_assign (sFilename, pLH->sFilename, 13);
      }
  }

  // list file
  nPrevPos = nHeadPos;
  nHeadPos = m_pBuffer->GetActFilePos () - GT_BLI_LH_SIZE;
  nPackedSize = nHeadPos - nPrevPos;

  if (!bFirst)
  {
    ArchiveFile *pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (nPackedSize);
    pArcFile->SetUnpackedSize (nOriginalSize);
    pArcFile->SetDT (nDateTime);
    _AddFile (pArcFile);
  }
}
