MAKE_ANALYZER (MIDI);

/*! Both text and binary!!!
 */
//--------------------------------------------------------------------
MAKE_IS_A (MIDI)
//--------------------------------------------------------------------
{
  return aGI.m_pBuffer->GetInt32 (0) == 0x6468544D;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (MIDI)
//--------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_MIDI_HEADER));

  if (!m_bListMode)
  {
    out_incindent ();

    MIDI_Header aH;
    m_pBuffer->GetBufferX (0, &aH, MIDI_H_SIZE);

    const int nFormat = aH.nFormat & 0xFF;

    if (nFormat == 0)
      out_append (rc (HRC_GENLIB_MIDI_MULTI_CHANNEL));
    else
    if (nFormat == 1)
      out_append (rc (HRC_GENLIB_MIDI_SIMULTEANOUS));
    else
    if (nFormat == 2)
      out_append (rc (HRC_GENLIB_MIDI_SINGLE));
    else
      out_append (rc (HRC_GENLIB_MIDI_UNKNOWN));

    out_append (rc (HRC_GENLIB_MIDI_BLOCKS));

    out_decindent ();
  }
}
