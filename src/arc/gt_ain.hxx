//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (AIN)
//--------------------------------------------------------------------
{
  AIN_GlobalHeader aGH;

  // read global header
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_AIN_GH_SIZE);

  // print number of files in archive
  out_format (rc (HRC_ARCLIB_AIN_FILES_IN_ARCHIVE),
              aGH.nFilesInArchive);

  // continued there - but which format??
  m_pBuffer->SetActFilePos (192 + aGH.nDataSize);

  // print "don't know message"
  BasicArchiveLister::ListFiles ();
}
