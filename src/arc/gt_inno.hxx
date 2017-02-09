struct INNO_DeflateHeader1
{
  gtuint32 nCRC32;
  gtuint32 nPackedSize;
  gtuint32 nOriginalSize;
};

//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (INNO)
//--------------------------------------------------------------------
{
  INNO_Header aH;
  INNO_EndHeader aEH;
  INNO_DeflateHeader1 aH1;

  // read header at start
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aH, GT_INNO_H_SIZE);

//  m_pBuffer->SetActFilePos (m_nLocalOffset + GT_INNO_H_SIZE);
  m_pBuffer->GetBufferX (&aH1, sizeof (INNO_DeflateHeader1));

/*
  const gtuint32 nCRC32 = CRC32::Get (&(aH1.nPackedSize), 8);

  // no check at the moment!
  if (nCRC32 != aH1.nCRC32)
  {
    out_error_format (_T ("  CRC32 check on header failed (is: %08Xh; should be: %08Xh)\n"), nCRC32, aH1.nCRC32);
    out_info_format (_T ("  GT calculation does not work at the moment!\n"));

    // done
    m_pBuffer->SetToEOF ();
    return;
  }
*/
  // get name of one and only object
  LPTSTR pName;
  GetStringResBuf (HRC_ARCLIB_INNO_SETUP_DATA, &pName);

  // create object
  ArchiveFile *pArcFile = new ArchiveFile (pName);
  pArcFile->SetPackedSize (aH1.nPackedSize);
  pArcFile->SetUnpackedSize (aH1.nOriginalSize);
  _AddFile (pArcFile);

  // read buffer at end of file
  m_pBuffer->GetBufferX (m_nLocalOffset + aH.nFileSize - GT_INNO_EH_SIZE, &aEH, GT_INNO_EH_SIZE);

  if (aEH.nSignatureOffset > m_pBuffer->GetTotalFileSize ())
  {
    // seems like the archive was ripped from an EXE...
    pstring sAddOnStr;
    str_assign_from_res (sAddOnStr, HRC_ARCLIB_INNO_RIPPED);
    _SetAddOnString (sAddOnStr);
  }

  // this is the size of the archive!!
  m_pBuffer->SetActFilePos (m_nLocalOffset + aH.nFileSize);
}
