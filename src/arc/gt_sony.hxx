//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (SONY)
//--------------------------------------------------------------------
{
  SONY_Header aH;

  _SetArchiveFlag (ARC_NO_UNPACKED_SIZE);

  // read header
  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);
  m_pBuffer->GetBufferX (&aH, GT_SONY_H_SIZE);

  ArchiveFile *pArcFile = new ArchiveFile (m_pBuffer->GetFileName ());
  pArcFile->SetPackedSize (aH.nPackedSize);
  _AddFile (pArcFile);

  // check if archive is valid ....
  m_pBuffer->IncActFilePos (aH.nPackedSize);
}
