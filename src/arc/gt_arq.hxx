//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ARQ)
//--------------------------------------------------------------------
{
  ARQ_Header H;
  ARQ_Body B;
  pstring sFilename;

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);

  for (;;)
  {
    m_pBuffer->GetBufferX (&H, GT_ARQ_H_SIZE);

    if (H.nID != 0x5767 || H.nFilenameLen == 0)
      break;

    if (H.nType == 1)
    {
      m_pBuffer->GetFixedSizeAString (sFilename, H.nFilenameLen);

      m_pBuffer->GetBufferX (&B, GT_ARQ_B_SIZE);

      ArchiveFile* pArcFile = new ArchiveFile (sFilename);
      pArcFile->SetPackedSize (B.nPackedSize);
      pArcFile->SetUnpackedSize (B.nOriginalSize);
      pArcFile->SetPWProtected (B.nFlags & 4);
      _AddFile (pArcFile);

      m_pBuffer->IncActFilePos (B.nPackedSize);
    }
    else
      if (H.nType == 2)
      {
        m_pBuffer->IncActFilePos (4);
      }
      else
      {
        SetErrorCode (ARCERR_UNKNOWN_METHOD);
        break;
      }
  }

  m_pBuffer->SetToEOF ();
}
