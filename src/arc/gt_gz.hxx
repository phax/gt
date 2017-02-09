//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (GZ)
//--------------------------------------------------------------------
{
  // filesize is stored as the last 4 bytes of the file
  const gtuint32 nOriginalSize = m_pBuffer->GetInt32 (m_pBuffer->GetFileSize () - 4);

  pstring sFilename;
  if (m_pBuffer->GetUint8 (3) == 8)
  {
    // heuristic:
    // we have a filename placed there!
    m_pBuffer->GetASCIIZ_AString (m_nLocalOffset + 10, sFilename, GT_MAX_SIZE);
  }
  else
  {
    // get filename and cut (last) extension
    DirEntry aDE (m_pBuffer->GetFileName ());
    aDE.GetFullNameWithoutExt (sFilename);
  }

  ArchiveFile* pArcFile = new ArchiveFile (sFilename);
  pArcFile->SetPackedSize (m_pBuffer->GetFileSize ());
  pArcFile->SetUnpackedSize (nOriginalSize);
  _AddFile (pArcFile);

  m_pBuffer->SetToEOF ();
}
