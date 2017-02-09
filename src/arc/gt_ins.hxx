//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (INS)
//--------------------------------------------------------------------
{
  INS_LocalHeader aLH;
  pstring sName1, sName2;
  int nLen1, nLen2;

  gtint32 nFileCount = m_pBuffer->GetInt32 (m_nLocalOffset + 78);

  while (nFileCount-- > 0)
  {
    m_pBuffer->GetBuffer (&aLH, GT_INS_LH_SIZE);

    // read name 1
    nLen1 = m_pBuffer->GetUint16 ();
    m_pBuffer->GetFixedSizeAString (sName1, nLen1);

    // read name 2
    nLen2 = m_pBuffer->GetUint16 ();
    m_pBuffer->GetFixedSizeAString (sName2, nLen2);

    // build final string
    sName1 += _T (" -> ");
    sName1 += sName2;

    ArchiveFile* pArcFile = new ArchiveFile (sName1);
    pArcFile->SetPackedSize (aLH.nPackedSize);
    pArcFile->SetUnpackedSize (aLH.nOriginalSize);
    _AddFile (pArcFile);
  }

  // set file ptr to the end -> no overlay detection
  m_pBuffer->SetToEOF ();
}
