const gtuint8 GT_777_METHOD_SOLID = 0x01;
const gtuint8 GT_777_FLAG_PASSWORD = 0x01;

//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (_777)
//--------------------------------------------------------------------
{
  _777_GlobalHeader aGH;
  _777_SolidGlobalHeader aSGH;
  _777_SolidLocalHeader aSLH;
  _777_LocalHeader aLH;
  pstring sFilename;

  m_pBuffer->GetBuffer (m_nLocalOffset + 0, &aGH, GT_777_GH_SIZE);

  if (aGH.nMethod == GT_777_METHOD_SOLID)
  {
    // solid archive
    _SetArchiveFlag (ARC_SOLID |
                     ARC_NO_PACKED_SIZE |
                     ARC_NO_UNPACKED_SIZE);

    if (m_pBuffer->GetBuffer (&aSGH, GT_777_SGH_SIZE))
    {
      m_pBuffer->IncActFilePos (aSGH.nDirectoryOffset);

      // calc files in archive count
      const size_t nFilesInArc = size_t (m_pBuffer->GetFileSize () - m_pBuffer->GetActFilePos ()) / GT_777_SLH_SIZE;

      for (size_t i = 0; i < nFilesInArc; i++)
      {
        if (!m_pBuffer->GetBuffer (&aSLH, GT_777_SLH_SIZE))
          break;  // exit for loop

        m_pBuffer->GetFixedSizeAString (sFilename, aSLH.nFilenameLen);

        // new file without anything
        _AddFile (new ArchiveFile (sFilename));
      }
    }
  }
  else
  {
    // not solid archive
    _SetArchiveFlag (ARC_DT_WIN);

    for (;;)
    {
      if (!m_pBuffer->GetBuffer (&aLH, GT_777_LH_SIZE))
        break;  // exit for loop

      // check for feasible values
      if (aLH.nOriginalSize < 0 ||
          aLH.nPackedSize < 0)
      {
        SetErrorCode (ARCERR_UNEXPECTED_VALUE);
        break;
      }

      // get filename
      m_pBuffer->GetFixedSizeAString (sFilename, aLH.nFilenameLen);

      // show file
      ArchiveFile *pArcFile = new ArchiveFile (sFilename);
      pArcFile->SetPackedSize (aLH.nPackedSize);
      pArcFile->SetUnpackedSize (aLH.nOriginalSize);
      pArcFile->SetDT (aLH.nModificationDT);
      pArcFile->SetPWProtected (aLH.nPassword & GT_777_FLAG_PASSWORD);
      _AddFile (pArcFile);

      // skip packed data
      m_pBuffer->IncActFilePos (aLH.nPackedSize);
    }
  }
}
