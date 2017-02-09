MAKE_ANALYZER (PCX);

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (PCX)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  if (aGI.m_pBuffer->GetFileSize () < PCX_H_SIZE)
    return false;

  PCX_Header aH;
  aGI.m_pBuffer->GetBuffer (0, &aH, PCX_H_SIZE);

  return (aH.nID == 0x0A &&
          aH._res1 == 0x00 &&
          aH.nXMax > aH.nXMin &&
          aH.nYMax > aH.nYMin);
}

//--------------------------------------------------------------------
MAKE_DISPLAY (PCX)
//--------------------------------------------------------------------
{
  // read header
  PCX_Header aH;
  m_pBuffer->GetBufferX (0, &aH, PCX_H_SIZE);

  // build string
  out_format (rc (HRC_GENLIB_PCX_HEADER),
              aH.nXMax - aH.nXMin + 1,
              aH.nYMax - aH.nYMin + 1,
              gtint32 (1) << (aH.nBitsPerPixel * aH.nNumberOfBitPlanes));
}

