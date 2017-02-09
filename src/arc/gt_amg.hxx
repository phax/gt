//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (AMG)
//--------------------------------------------------------------------
{
  AMG_GlobalHeader aGH;
  AMG_LocalHeader aLH;
  pstring sFilename;

  _SetArchiveFlag (ARC_DT_DOS);

  // read global header
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_AMG_GH_SIZE);

  // print archive version...
  _SetArchiveVersion (aGH.nVersion >> 4, aGH.nVersion & 0x0F);

  if (aGH.nCommentLen > 0)
  {
    // skip archive comment
    _SetArchiveFlag (ARC_COMMENT);
    m_pBuffer->IncActFilePos (aGH.nCommentLen);
  }

  for (;;)
  {
    m_pBuffer->GetBufferX (&aLH, GT_AMG_LH_SIZE);

    // check for suspicious values...
    if (aLH.nPackedSize < 0 || aLH.nOriginalSize < 0)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    // read the path directly after the local header
    m_pBuffer->GetFixedSizeAString (sFilename, aLH.nPathLen);

    // if the 8th char is set, use a fixed length, else let it determine!
    if (aLH.sFilename[7] != 0)
      str_append (sFilename, aLH.sFilename, 8);
    else
      str_append (sFilename, aLH.sFilename);

    sFilename += _T ('.');

    // if the 3rd char is set, use a fixed length, else let it determine!
    if (aLH.sExt[2] != 0)
      str_append (sFilename, aLH.sExt, 3);
    else
      str_append (sFilename, aLH.sExt);

    ArchiveFile* pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (aLH.nPackedSize);
    pArcFile->SetUnpackedSize (aLH.nOriginalSize);
    pArcFile->SetDT (aLH.nDateTime);
    pArcFile->SetPWProtected (aLH.nFlags & 1);
    _AddFile (pArcFile);

    // now skip the comment - if any
    m_pBuffer->IncActFilePos (aLH.nFileCommentLen);

    // and now skip the packed data
    m_pBuffer->IncActFilePos (aLH.nPackedSize -
                              GT_AMG_LH_SIZE -
                              aLH.nPathLen -
                              aLH.nFileCommentLen);

    // found EOF?
    if (m_pBuffer->IsEOF ())
      break;
  }
}
