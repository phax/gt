MAKE_ANALYZER (MAC);

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (MAC)
//--------------------------------------------------------------------
{
  // binary only
  if (aGI.m_pBuffer->IsText ())
    return false;

  // the file must have at least 128 bytes
  if (aGI.m_pBuffer->GetFileSize () < 128)
    return false;

  // at position 0 must be a 0
  if (aGI.m_pBuffer->GetUint8 (0) != 0)
    return false;

  // byte 1 contains the length of the internal filename: 1-63
  gtuint8 nFilenameLength = aGI.m_pBuffer->GetUint8 (1);
  if (nFilenameLength < 1 || nFilenameLength > 63)
    return false;

  // long @ 83 and long @ 87 must be 0x00800000
  gtuint32 n83 = aGI.m_pBuffer->GetInt32 (83);
  if (LONG_SWAP_BYTE (n83) >= 0x00800000)
    return false;

  gtuint32 n87 = aGI.m_pBuffer->GetInt32 (87);
  if (LONG_SWAP_BYTE (n87) >= 0x00800000)
    return false;

  // 25 bytes from 101 on must be 0
  gtuint8 sDummy[25];
  aGI.m_pBuffer->GetBuffer (101, sDummy, 25);
  for (int i = 0; i < 25; i++)
    if (sDummy[i] != 0)
      return false;

  // lets say it's okay...
  return true;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (MAC)
//--------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_MAC_HEADER));

  /*#f# todo */

  MAC_Header aH;
  m_pBuffer->GetBufferX (0, &aH, MAC_H_SIZE);
}

