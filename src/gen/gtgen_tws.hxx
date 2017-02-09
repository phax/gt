MAKE_ANALYZER (TWS);

//--------------------------------------------------------------------
MAKE_IS_A (TWS)
//--------------------------------------------------------------------
{
  return aGI.m_pBuffer->GetUint16 (0) == 0xFFFF &&
         aGI.m_pBuffer->GetUint16 (4) == 7 && // length of following string
         aGI.m_pBuffer->CompareA (6, 7, "TextPad");
}

//--------------------------------------------------------------------
MAKE_DISPLAY (TWS)
//--------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_TWS_HEADER));

  if (!m_bListMode)
  {
    out_incindent ();

    gtuint16 nVersion = m_pBuffer->GetUint16 (2);
    out_format (rc (HRC_GENLIB_TWS_VERSION), nVersion);

    out_decindent ();
  }
}
