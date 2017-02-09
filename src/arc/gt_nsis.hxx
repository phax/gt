//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (NSIS13)
//--------------------------------------------------------------------
{
  NSIS13_GlobalHeader aGH;
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_NSIS13_GH_SIZE);

  // check if everything is here!
  if (m_pBuffer->GetFileSize () < file_t (aGH.length_of_all_following_data))
  {
    SetErrorCode (ARCERR_MISSING_DATA);    return;
  }

/*
  out_format (_T ("  Length of header:  %d bytes\n"), aGH.length_of_header);
  out_format (_T ("  Header offset:     %08Xh/%d\n"), aGH.header_ptr, aGH.header_ptr);
  out_format (_T ("  Total data length: %d bytes\n"), aGH.length_of_all_following_data);
*/
  // and finally set pointer to total data length
  m_pBuffer->SetActFilePos (m_nLocalOffset + 0 + aGH.length_of_all_following_data);
}

//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (NSIS16)
//--------------------------------------------------------------------
{
  NSIS16_GlobalHeader aGH;
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_NSIS16_GH_SIZE);

  // check if everything is here!
  if (m_pBuffer->GetFileSize () < file_t (aGH.length_of_all_following_data))
  {
    SetErrorCode (ARCERR_MISSING_DATA);    return;
  }

/*
  out_format (_T ("  Length of header:  %d bytes\n"), aGH.length_of_header);
  out_format (_T ("  Total data length: %d bytes\n"), aGH.length_of_all_following_data);
*/
  // and finally set pointer to total data length
  m_pBuffer->SetActFilePos (m_nLocalOffset + 0 + aGH.length_of_all_following_data);
}
