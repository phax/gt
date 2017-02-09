MAKE_ANALYZER (WMF);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (WMF)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == gtint32 (0x9AC6CDD7);
}

//--------------------------------------------------------------------
MAKE_DISPLAY (WMF)
//--------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_WMF_HEADER));

  if (!m_bListMode)
  {
    out_incindent ();

    // read WMF header
    WMF_Header aH;
    m_pBuffer->GetBufferX (0, &aH, WMF_H_SIZE);

    // and append result
    out_format (rc (HRC_GENLIB_WMF_COORDS),
                aH.nLeft,
                aH.nTop,
                aH.nRight,
                aH.nBottom);

    out_format (rc (HRC_GENLIB_WMF_SIZE),
                aH.nRight - aH.nLeft + 1,
                aH.nTop - aH.nBottom + 1);

    out_decindent ();
  }
}
