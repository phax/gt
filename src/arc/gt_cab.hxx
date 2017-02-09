/*
 Can be found at offset 201 in an EXE overlay archive!
 */

//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (CAB)
//--------------------------------------------------------------------
{
  CAB_GlobalHeader aGH;
  CAB_LocalHeader aLH;
  pstring sFilename;

  _SetArchiveFlag (ARC_NO_PACKED_SIZE | ARC_DT_DOS);

  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_CAB_GH_SIZE);

  m_pBuffer->SetActFilePos (m_nLocalOffset + aGH.nDataStart);

  for (int i = 0; i < aGH.nFileCount; i++)
  {
    // read the header
    m_pBuffer->GetBufferX (&aLH, GT_CAB_LH_SIZE);

    // check consistency
    if (aLH.nOriginalSize < 0)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    // read the filename
    m_pBuffer->GetASCIIZ_AString (sFilename, GT_MAX_SIZE);

    ArchiveFile *pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetUnpackedSize (aLH.nOriginalSize);
    pArcFile->SetDT (aLH.nDateTime);
    _AddFile (pArcFile);
  }

  // to allow overlay listing in gtpack.cxx
  if (m_nLocalOffset + aGH.nFileSize > m_pBuffer->GetFileSize ())
    SetErrorCode (ARCERR_MISSING_DATA);  else
    m_pBuffer->SetActFilePos (m_nLocalOffset + aGH.nFileSize);
}
