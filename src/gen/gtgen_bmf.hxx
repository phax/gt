MAKE_ANALYZER (BMF);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (BMF)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetUint16 (0) == 0x8A81;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (BMF)
//--------------------------------------------------------------------
{
  BMF_Header aH;
  m_pBuffer->GetBufferX (0, &aH, BMF_H_SIZE);

  out_format (rc (HRC_GENLIB_BMF),
              aH.sVersion[0],
              aH.sVersion[1],
              aH.nWidth,
              aH.nHeight);
}
