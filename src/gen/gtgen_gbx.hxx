MAKE_ANALYZER (GBX);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (GBX)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x06584247;
}

static void _gbx_read_simple (FileBuffer* pBuffer, gtuint32 nCount)
{
  pstring sName;
  for (gtuint32 i = 0; i < nCount; ++i)
  {
    gtuint32 nLen = pBuffer->GetInt32 ();
    pBuffer->GetFixedSizeAString (sName, nLen);
    gtuint32 nNext = pBuffer->GetInt32 ();
    out_format (_T ("%s\n"), sName.c_str ());

    out_incindent ();
    _gbx_read_simple (pBuffer, nNext);
    out_decindent ();
  }
}

//--------------------------------------------------------------------
MAKE_DISPLAY (GBX)
//--------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_GBX_HEADER));
  const bool bCanList = (m_pBuffer->GetInt32 (0x05) == 0x52435542)
//                         && ((m_pBuffer->GetUint8 (0x0a) == 0x90) || (m_pBuffer->GetUint8 (0x0a) == 0x60))
                         && (m_pBuffer->GetInt32 (0x15) > 0)
                         ;

  if (!m_bListMode && bCanList)
  {
    out_incindent ();
    gtuint32 nNormal = m_pBuffer->GetInt32 (0x1d);
    gtuint32 nForward = m_pBuffer->GetInt32 (0x15);
    m_pBuffer->SetActFilePos (0x21);
    pstring sName;
    gtuint32 nLen, i;
    _gbx_read_simple (m_pBuffer, nNormal);

    for (i = 0; i < nForward; ++i)
    {
      m_pBuffer->GetInt32 ();
      nLen = m_pBuffer->GetInt32 ();
      if (nLen > 0xff)
      {
        out_error_append (_T ("Reader function doesn't work proper :(\n"));
        break;
      }
      m_pBuffer->GetFixedSizeAString (sName, nLen);
      m_pBuffer->GetInt32 ();
      m_pBuffer->GetInt32 ();
      m_pBuffer->GetInt32 ();
      out_format (_T ("-> %s\n"), sName.c_str ());
    }

    out_decindent ();
  }
}
