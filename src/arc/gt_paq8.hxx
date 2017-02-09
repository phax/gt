//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (PAQ8)
//--------------------------------------------------------------------
{
  pstring sLine, sUncompressed, sFileName;

  _SetArchiveFlag (ARC_NO_PACKED_SIZE);

  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);

  // read header
  m_pBuffer->GetLine (DELIMITER_CRLF, sLine);

  do
  {
    m_pBuffer->GetLine (DELIMITER_CRLF, sLine);

    // split at \x09
    str_split (sLine, _T ('\x09'), sUncompressed, sFileName);

    ArchiveFile* pArcFile = new ArchiveFile (sFileName);
    pArcFile->SetUnpackedSize (_ttol (sUncompressed.c_str ()));
    _AddFile (pArcFile);
  } while (m_pBuffer->LookAheadUint8 () != 0x1a);

  m_pBuffer->SetToEOF ();
}
