//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (AKT7)
//--------------------------------------------------------------------
{
  AKT7_GlobalHeader aGH;
  AKT7_LocalHeader aLH;
  pstring sPath, sOldPath, sFilename, sTemp;

  _SetArchiveFlag (ARC_NO_PACKED_SIZE | ARC_DT_DOS);

  // read global header
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_AKT7_GH_SIZE);

  // and skip the comment...
  m_pBuffer->IncActFilePos (aGH.nCommentLength);

  for (int i = 1; i <= aGH.nFilesInArchive; i++)
  {
    // read pascal string at current pos
    m_pBuffer->GetPascalString (sPath);

    // first check length, than check index
    // if the first is 0 it means reuse the previous path
    if (sPath.length () == 1 && sPath [0] == 1)
      sPath = sOldPath;
    else
      sOldPath = sPath;

    m_pBuffer->GetBufferX (&aLH, GT_AKT7_LH_SIZE);

    // build filename (with path)
    sFilename = sPath;
    str_append (sFilename, aLH.sFilename, 12);

    ArchiveFile* pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetUnpackedSize (aLH.nEndPos - aLH.nStartPos);
    pArcFile->SetDT (aLH.nDateTime);
    _AddFile (pArcFile);
  }

  m_pBuffer->SetToEOF ();
}
