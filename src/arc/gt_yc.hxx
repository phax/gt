//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (YC)
//--------------------------------------------------------------------
{
  pstring sFilename;
  file_t nOldFilePos = 0;
  YC_GlobalHeader aGH;
  YC_LocalHeader aLH;

  _SetArchiveFlag (ARC_DT_DOS | ARC_NO_PACKED_SIZE);

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);

  for (;;)
  {
    // read the global header
    m_pBuffer->GetBufferX (&aGH, GT_YC_GH_SIZE);

    // ID is unique!
    if (aGH.nID != 0x43590100)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    // loop over all files in this section
    for (int i = 0; i < aGH.nFilesInSection; i++)
    {
      // read the file header
      m_pBuffer->GetBufferX (&aLH, GT_YC_LH_SIZE);

      // invalid value!!!
      if (aLH.nOriginalSize < 0)
      {
        SetErrorCode (ARCERR_UNEXPECTED_VALUE);
        break;
      }

      // now read a fixed length string
      m_pBuffer->GetFixedSizeAString (sFilename, aLH.nFilenameLen);

      ArchiveFile *pArcFile = new ArchiveFile (sFilename);
      pArcFile->SetUnpackedSize (aLH.nOriginalSize);
      pArcFile->SetDT (aLH.nDateTime);
      _AddFile (pArcFile);

      // and skip one mor byte
      m_pBuffer->IncActFilePos (1);
    }

    // increase by section size
    nOldFilePos += aGH.nSectionSize;
    m_pBuffer->SetActFilePos (nOldFilePos);

    if (m_pBuffer->IsEOF ())
      break;
  }
}
