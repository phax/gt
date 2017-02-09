//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ZET)
//--------------------------------------------------------------------
{
  pstring sCurrentPath;
  pstring sFilename;
  pstring sResult;
  ZET_GlobalHeader aGH;
  ZET_LocalHeader aLH;

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);

  for (;;)
  {
    const file_t nOldFilePos = m_pBuffer->GetActFilePos ();

    if (!m_pBuffer->GetBuffer (&aGH, GT_ZET_GH_SIZE))
      break;

    if (aGH.nID != 0x5A4F)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    if (aGH.nSubType == 0xDD)
    {
      // type == global header
      m_pBuffer->IncActFilePos (aGH.nHeaderSize - GT_ZET_GH_SIZE);
    }
    else
    if (aGH.nSubType == 0xDE)
    {
      // to avoid an 'overlay detection'
      m_pBuffer->IncActFilePos (aGH.nHeaderSize - GT_ZET_GH_SIZE);
      break;
    }
    else
    if (aGH.nSubType == 0xE0)
    {
      // directory name
      m_pBuffer->IncActFilePos (6);
      m_pBuffer->GetASCIIZ_AString (sCurrentPath, GT_MAX_SIZE);
      m_pBuffer->SetActFilePos (nOldFilePos + aGH.nHeaderSize);
    }
    else
    if (aGH.nSubType == 0xE1)  // data part
    {
      m_pBuffer->IncActFilePos (4);
      const gtuint16 nDataLen = m_pBuffer->GetUint16 ();
      m_pBuffer->SetActFilePos (nOldFilePos + aGH.nHeaderSize + nDataLen);
    }
    else
    if (aGH.nSubType == 0xE2)
    {
      /* file */
      if (!m_pBuffer->GetBuffer (&aLH, GT_ZET_LH_SIZE))
        break;

      if (aLH.nOriginalSize < 0 || aLH.nPackedSize < 0)
      {
        SetErrorCode (ARCERR_UNEXPECTED_VALUE);
        break;
      }

      m_pBuffer->GetASCIIZ_AString (sFilename, GT_MAX_SIZE);

      sResult = sCurrentPath;
      sResult += sFilename;

      ArchiveFile *pArcFile = new ArchiveFile (sResult);
      pArcFile->SetPackedSize (aLH.nPackedSize);
      pArcFile->SetUnpackedSize (aLH.nOriginalSize);
      _AddFile (pArcFile);

      m_pBuffer->SetActFilePos (nOldFilePos + aGH.nHeaderSize);
    }
    else
    if (aGH.nSubType == 0xE3)
    {
      // must be kind of read next data block
      m_pBuffer->IncActFilePos (4);
    }
    else
    {
      if (debug)
        out_error_format (_T ("[dbg] Found unknown subtype %02Xh\n"), aGH.nSubType);
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }
  }
}
