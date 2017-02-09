MAKE_ANALYZER (Crypt);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (Crypt)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return (aGI.m_pBuffer->GetInt32 (0) & 0xFFFFFF00) == 0x79724300;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (Crypt)
//--------------------------------------------------------------------
{
  const gtuint8 b0 = m_pBuffer->GetUint8 (0);
  const gtuint8 b4 = m_pBuffer->GetUint8 (4);

  out_format (rc (HRC_GENLIB_CRYPT),
              (b0 >> 5) & 7,
              (b0 >> 2) & 7);
  if (b4 & 1)
    out_append (rc (HRC_GENLIB_CRYPT_HSL));
  out_append (_T ("\n"));
}
