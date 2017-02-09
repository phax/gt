MAKE_ANALYZER (APE);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (APE)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x2043414D;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (APE)
//--------------------------------------------------------------------
{
  APE_Header aH;
  m_pBuffer->GetBufferX (0, &aH, APE_H_SIZE);

  out_format (_T ("Monkeys Audio music file %d.%03d\n"),
              aH.nVersion / 1000,
              aH.nVersion % 1000);

  if (!m_bListMode)
  {
    Table aTable;
    aTable.EnableHeadline (false);
    aTable.AddColumn (_T (""), STR,       22, ALIGN_RIGHT);
    aTable.AddColumn (_T (""), NUM_SPACE,  8, 8);

    aTable.AddStr (0, _T ("Compression level:"));
    aTable.AddInt (1, aH.nCompressionLevel);

    aTable.AddStr (0, _T ("Channels:"));
    aTable.AddInt (1, aH.nChannels);

    aTable.AddStr (0, _T ("Sample rate:"));
    aTable.AddInt (1, aH.nSampleRate);

    aTable.AddStr (0, _T ("Header bytes:"));
    aTable.AddInt (1, aH.nHeaderBytes);

    aTable.AddStr (0, _T ("Terminating bytes:"));
    aTable.AddInt (1, aH.nTerminatingBytes);

    aTable.AddStr (0, _T ("Total frames:"));
    aTable.AddInt (1, aH.nTotalFrames);

    aTable.AddStr (0, _T ("Final frame samples:"));
    aTable.AddInt (1, aH.nFinalFrameBlocks);

    out_incindent ();
    out_table (&aTable);
    out_decindent ();
  }
}
