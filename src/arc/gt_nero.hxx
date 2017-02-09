//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (NERO)
//--------------------------------------------------------------------
{
  NERO_Block aBlock;
  TCHAR s5[5];

  _SetArchiveFlag (ARC_NO_UNPACKED_SIZE);

  // the start offset is the last 4 bytes in the file!
  gtuint32 nStartOffset = m_pBuffer->GetInt32 (m_pBuffer->GetFileSize () - 4);
  nStartOffset = LONG_SWAP_BYTE (nStartOffset);
  m_pBuffer->SetActFilePos (nStartOffset);

  for (;;)
  {
    m_pBuffer->GetBufferX (&aBlock, GT_NERO_BLOCK_SIZE);
    aBlock.nSize = LONG_SWAP_BYTE (aBlock.nSize);

    // convert DWORD to char[5]
    _ltoc4 (aBlock.nID, s5);

    ArchiveFile *pArcFile = new ArchiveFile (s5);
    pArcFile->SetPackedSize (aBlock.nSize);
    _AddFile (pArcFile);

    // we're done
    if (aBlock.nID == 0x21444E45)
      break;

    // skip block
    m_pBuffer->IncActFilePos (aBlock.nSize);
  }

  m_pBuffer->SetToEOF ();
}
