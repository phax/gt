MAKE_ANALYZER (ICO);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (ICO)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x00010000 &&
         aGI.m_pBuffer->GetUint16 (4) > 0;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (ICO)
//--------------------------------------------------------------------
{
  ICO_Header aH;
  m_pBuffer->GetBufferX (0, &aH, ICO_H_SIZE);

  out_format (rc (HRC_GENLIB_ICO_HEADER),
              aH.nIconCount);
}
