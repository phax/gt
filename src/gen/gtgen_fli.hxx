MAKE_ANALYZER (FLI);

/*! Only binary
 */
//--------------------------------------------------------------------
MAKE_IS_A (FLI)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  FLI_Header aH;
  if (!aGI.m_pBuffer->GetBuffer (0, &aH, FLI_H_SIZE))
    return false;

  /* according to the graphics file format library
   * a FLI file can contain only 4000 frames
   */
  return (file_t (aH.nFileSize) == aGI.m_pBuffer->GetFileSize () &&
          (aH.nFileID == 0xAF11 || aH.nFileID == 0xAF12) &&
          aH.nNumberOfFrames <= 4000 &&
          aH.nPixelDepth == 8 &&
//          aH.nFlags == 0x03 &&
          aH._res1 == 0x00);
}

//--------------------------------------------------------------------
MAKE_DISPLAY (FLI)
//--------------------------------------------------------------------
{
  FLI_Header aH;
  m_pBuffer->GetBufferX (0, &aH, FLI_H_SIZE);

  // FLI or FLC
  const bool bIsFLI = (m_pBuffer->GetUint16 (4) == 0xAF11);

  out_format (rc (HRC_GENLIB_FLI_HEADER),
              bIsFLI
                ? _T ("FLI")
                : _T ("FLC"),
              aH.nWidth,
              aH.nHeight,
              1 << aH.nPixelDepth,
              aH.nNumberOfFrames);

  if (!m_bListMode)
  {
    out_incindent ();
    if (aH.nFrameDelay > 0)
    {
      double dDelay = aH.nFrameDelay;
      if (bIsFLI)
        dDelay /= 70;
      else
        dDelay /= 1000;

      DoublePair aHL;
      _splitdouble (dDelay, aHL, 2);

      out_format (rc (HRC_GENLIB_FLI_DELAY),
                  aHL.h, aHL.l);
    }
    out_decindent ();
  }
}

