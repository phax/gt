//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (PPZ)
//--------------------------------------------------------------------
{
  PPZ_GlobalHeader aGH;

  // read global header
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_PPZ_GH_SIZE);

  // print version info
  _SetArchiveVersion (aGH.nMajorVersion, aGH.nMinorVersion);

  // get original size
  aGH.nOriginalSize = LONG_SWAP_BYTE (aGH.nOriginalSize);

  /// no own name
  ArchiveFile *pArcFile = new ArchiveFile (m_pBuffer->GetFileName ());
  pArcFile->SetPackedSize (m_pBuffer->GetFileSize ());
  pArcFile->SetUnpackedSize (aGH.nOriginalSize);
  _AddFile (pArcFile);

  // done
  m_pBuffer->SetToEOF ();
}
