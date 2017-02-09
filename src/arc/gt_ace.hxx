#define GT_ACE_NOTFIRST    BITS[12]
#define GT_ACE_NOTLAST     BITS[13]
#define GT_ACE_PASSWORD    BITS[14]
#define GT_ACE_FILE        BITS[15]

//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ACE)
//--------------------------------------------------------------------
{
  ACE_GlobalHeader aGH;
  ACE_LocalHeaderFile aLH;
  pstring sFilename;
  file_t nGlobalHeaderOfs = m_nLocalOffset;

  _SetArchiveFlag (ARC_DT_DOS);

  if (m_pBuffer->CompareA (m_nLocalOffset + 0, 5, "!sfx!"))
  {
    // its an SFX archive -> 6 bytes + ofs2hdr
    nGlobalHeaderOfs += m_pBuffer->GetInt32 (m_nLocalOffset + 6);
  }

  // read global archive header
  m_pBuffer->GetBufferX (nGlobalHeaderOfs, &aGH, GT_ACE_GH_SIZE);

  // set the archive version
  _SetArchiveVersion (aGH.nCompressVersion / 10, aGH.nCompressVersion % 10);

  // goto start of first local entry
  m_pBuffer->SetActFilePos (nGlobalHeaderOfs + 4 + aGH.nDataOffset);

  for (;;)
  {
    const file_t nOldFilePos = m_pBuffer->GetActFilePos ();

//    out_format ("Currently at %08Xh\n", nOldFilePos);

    if (!m_pBuffer->GetBuffer (&aLH, GT_ACE_FH_SIZE))
      break;

    // nType == 1 - file entry
    // nType == 2 - security envelope ???
    if (aLH.nType == 1)
    {
      if (!m_pBuffer->GetFixedSizeAString (sFilename, aLH.nFilenameLen))
      {
        // failed to read filename...
        SetErrorCode (ARCERR_UNEXPECTED_EOF);
        break;
      }

      ArchiveFile *pArcFile = new ArchiveFile (sFilename);
      pArcFile->SetPackedSize (aLH.nPackedSize);
      pArcFile->SetUnpackedSize (aLH.nOriginalSize);
      pArcFile->SetDT (aLH.nDateTime);
      pArcFile->SetPWProtected (aLH.nFlags & GT_ACE_PASSWORD);
      pArcFile->SetDirectory (!(aLH.nFlags & GT_ACE_FILE));
      pArcFile->SetContinuedFromLast (aLH.nFlags & GT_ACE_NOTFIRST);
      pArcFile->SetContinuedOnNext (aLH.nFlags & GT_ACE_NOTLAST);
      _AddFile (pArcFile);
    }
    else
    if (debug)
    {
      out_format (_T ("Debug: unknown type %d\n"), aLH.nType);
    }

    m_pBuffer->SetActFilePos (nOldFilePos + 4 + aLH.nHeaderSize + aLH.nPackedSize);
  }

//  out_format ("Done at %08Xh/%ld\n", m_pBuffer->GetActFilePos ());
}
