//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (DPA)
//--------------------------------------------------------------------
{
  DPA_Header aH;
  file_t nOldOffset;
  pstring sFilename;

  // print archive version...
  _SetArchiveVersion (m_pBuffer->GetUint8 (m_nLocalOffset + 21),
                      m_pBuffer->GetUint8 (m_nLocalOffset + 20));

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);

  for (;;)
  {
    nOldOffset = m_pBuffer->GetActFilePos ();

    m_pBuffer->GetBufferX (&aH, GT_DPA_H_SIZE);

    str_assign (sFilename, aH.sFilename);

    ArchiveFile *pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (aH.nNextOffset - nOldOffset);
    pArcFile->SetUnpackedSize (aH.nOriginalSize);
    _AddFile (pArcFile);

    // to avoid endless loops ;-)
    if (aH.nNextOffset == nOldOffset)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    if (aH.nNextOffset >= m_pBuffer->GetFileSize ())
    {
      m_pBuffer->SetToEOF ();
      break;
    }

    m_pBuffer->SetActFilePos (aH.nNextOffset);
  }
}
