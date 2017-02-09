MAKE_ANALYZER (FPK);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (FPK)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return (aGI.m_pBuffer->GetInt32 (0) & 0x00ffffff) == 0x00555050;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (FPK)
//--------------------------------------------------------------------
{
  const gtuint8 b3 = m_pBuffer->GetUint8 (3);
  const gtuint8 b4 = m_pBuffer->GetUint8 (4);
  const gtuint8 b5 = m_pBuffer->GetUint8 (5);

  out_format (rc (HRC_GENLIB_FPK_HEADER),
              b3, b4, b5);
}
