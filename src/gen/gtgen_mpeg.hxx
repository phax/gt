MAKE_ANALYZER (MPEG);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (MPEG)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == gtint32 (0xB3010000);
}

//--------------------------------------------------------------------
MAKE_DISPLAY (MPEG)
//--------------------------------------------------------------------
{
  /*
  what    bits   bytes
  ID      32     4
  width   12     1.5
  height  12     1.5
  */

  const gtuint16 w4 = m_pBuffer->GetUint16 (4);
  const gtuint16 w5 = m_pBuffer->GetUint16 (5);

  out_format (rc (HRC_GENLIB_MPEG_HEADER),
              WORD_SWAP_BYTE (w4) >> 4,
              WORD_SWAP_BYTE (w5) & 0x0FFF);
}
