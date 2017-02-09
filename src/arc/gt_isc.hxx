//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ISC)
//--------------------------------------------------------------------
{
  ISC_GlobalHeader aGH;
  ISC_LocalHeader aLH;
  pstring sName;

  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_ISC_GH_SIZE);

  if (aGH.nAnalyzeOffset == 0x200 && aGH.nDataOffset == 0x200)
  {
    SetErrorCode (ARCERR_UNKNOWN_METHOD);
    return;
  }

  const file_t nAnalyzeOffset   = m_nLocalOffset + aGH.nAnalyzeOffset;
  const file_t nFilenamesOffset = nAnalyzeOffset + m_pBuffer->GetInt32 (nAnalyzeOffset + 0x0C);
  const gtint32 nFilenamesCount = m_pBuffer->GetInt32 (nAnalyzeOffset + 0x28);

//    out_format ("%d files\n", nFilenamesCount);

  if (nFilenamesCount <= 0)
  {
    SetErrorCode (ARCERR_UNKNOWN_METHOD);
    return;
  }

  // now read the size of this section (nSectionSize = )
  m_pBuffer->GetInt32 (nFilenamesOffset);
  gtint32 i;

  file_t nReadOffset = nFilenamesOffset + 4;

  // read all offsets and determine the minimum
  // so we know where to start...
  gtuint32 *pOffsets = new gtuint32 [nFilenamesCount];
  m_pBuffer->GetBufferX (nReadOffset, pOffsets, nFilenamesCount * 4);
  gtuint32 nMin = pOffsets[0];
  for (i = 1; i < nFilenamesCount; i++)
    if (pOffsets[i] < nMin)
      nMin = pOffsets[i];
  delete [] pOffsets;

  const gtuint32 nRelativeNameOffset = m_pBuffer->GetInt32 (nFilenamesOffset + nMin);

//    out_format ("nRelativeNameOffset = %08Xh\n", nRelativeNameOffset);

  // for all files
  for (i = 0; i < nFilenamesCount; i++)
  {
    const gtuint32 nRelativeTargetOffset = m_pBuffer->GetInt32 (nReadOffset);
    const file_t nAbsoluteTargetOffset = nFilenamesOffset + nRelativeTargetOffset;

    nReadOffset += sizeof (gtint32);

    if (nAbsoluteTargetOffset >= m_pBuffer->GetFileSize ())
    {
      // found an invalid offset...
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

/*
    if (nRelativeTargetOffset < nFilenamesOffset || nRelativeTargetOffset > m_pBuffer->GetFileSize ())
      continue;
*/

    // does the link point directly into the filename table?
    if (nRelativeTargetOffset > nRelativeNameOffset)
    {
      m_pBuffer->GetASCIIZ_AString (nAbsoluteTargetOffset, sName, GT_MAX_SIZE);

      ArchiveFile* pArcFile = new ArchiveFile (sName);
      pArcFile->SetDirectory (true);
      _AddFile (pArcFile);
    }
    else
    {
      m_pBuffer->GetBufferX (nAbsoluteTargetOffset, &aLH, GT_ISC_LH_SIZE);

      // deleted???
      if (aLH.nFlags != 0x0008 && aLH.nFlags != 0x000C)
      {
        m_pBuffer->GetASCIIZ_AString (nFilenamesOffset + aLH.nNameOffset, sName, GT_MAX_SIZE);

        ArchiveFile* pArcFile = new ArchiveFile (sName);
        pArcFile->SetPackedSize (aLH.nPackedSize);
        pArcFile->SetUnpackedSize (aLH.nOriginalSize);
        _AddFile (pArcFile);
      }
    }
  }

  // there can't be an overlay
  m_pBuffer->SetToEOF ();
}
