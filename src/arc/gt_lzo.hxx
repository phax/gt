//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (LZO)
//--------------------------------------------------------------------
{
  gtint32 nTotalOriginalSize = 0;
  gtint32 nTotalPackedSize = 0;
  LZO_GlobalHeader aGH;

  _SetArchiveFlag (ARC_DT_UNIX);

  m_pBuffer->GetBuffer (m_nLocalOffset, &aGH, GT_LZO_GH_SIZE);

  // get the filename
  pstring sFilename;
  m_pBuffer->GetPascalString (sFilename);

  // skip one long
  m_pBuffer->IncActFilePos (4);

  for (;;)
  {
    // get the first token
    gtuint32 nOriginalSize = m_pBuffer->GetInt32 ();
    nOriginalSize = LONG_SWAP_BYTE (nOriginalSize);
    nTotalOriginalSize += nOriginalSize;

    if (nOriginalSize == 0)
      break;

    gtuint32 nPackedSize = m_pBuffer->GetInt32 ();
    nPackedSize = LONG_SWAP_BYTE (nPackedSize);
    nTotalPackedSize += nPackedSize;

    // skip one more long + packed size...
    m_pBuffer->IncActFilePos (4 + nPackedSize);
  }

  ArchiveFile* pArcFile = new ArchiveFile (sFilename);
  pArcFile->SetPackedSize (nTotalPackedSize);
  pArcFile->SetUnpackedSize (nTotalOriginalSize);
  pArcFile->SetDT (LONG_SWAP_BYTE (aGH.nDateTime));
  _AddFile (pArcFile);
}
