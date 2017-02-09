//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ASD)
//--------------------------------------------------------------------
{
  ASD_GlobalHeader aGH;
  ASD_LocalHeader aLH;
  pstring sFilename;

  _SetArchiveFlag (ARC_NO_PACKED_SIZE | ARC_DT_DOS);

  m_pBuffer->GetBuffer (m_nLocalOffset + 0, &aGH, GT_ASD_GH_SIZE);

  for (int i = 0; i < aGH.nFilesInArchive; i++)
  {
    m_pBuffer->GetPascalString (sFilename);

    m_pBuffer->GetBufferX (&aLH, GT_ASD_LH_SIZE);

    // check for correct values
    if ((gtint32) aLH.nOriginalSize < 0)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    ArchiveFile* pFile = new ArchiveFile (sFilename);
    pFile->SetUnpackedSize (aLH.nOriginalSize);
    pFile->SetDT (aLH.nDateTime);
    _AddFile (pFile);
  }

  m_pBuffer->SetToEOF ();
}
