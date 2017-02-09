MAKE_ANALYZER (PNG);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (PNG)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x474E5089 &&
         aGI.m_pBuffer->GetInt32 (4) == 0x0A1A0A0D;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (PNG)
//--------------------------------------------------------------------
{
  // read header
  PNG_Header aH;
  m_pBuffer->GetBufferX (0, &aH, PNG_H_SIZE);

  pstring sInterlaced;
  if (aH.nFlags > 0)
    str_assign_from_res (sInterlaced, HRC_GENLIB_PNG_INTERLACED);

  out_format (rc (HRC_GENLIB_PNG_HEADER),
              sInterlaced.c_str (),
              LONG_SWAP_BYTE (aH.nWidth),
              LONG_SWAP_BYTE (aH.nHeight),
              aH.nColorDepth == 8 && aH.nColorType == 2
                ? (gtuint32 (1) << 24)  // 16777216
                : (gtuint32 (1) << aH.nColorDepth));
}
