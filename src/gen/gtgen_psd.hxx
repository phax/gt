MAKE_ANALYZER (PSD);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (PSD)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x53504238;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (PSD)
//--------------------------------------------------------------------
{
  // read PSD header
  PSD_Header aH;
  m_pBuffer->GetBufferX (0, &aH, PSD_H_SIZE);

  out_format (rc (HRC_GENLIB_PSD_HEADER),
              LONG_SWAP_BYTE (aH.nWidth),
              LONG_SWAP_BYTE (aH.nHeight));
}
