MAKE_ANALYZER (PMWLite);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (PMWLite)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetUint16 (0) == 0x4D50;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (PMWLite)
//--------------------------------------------------------------------
{
  // get version number
  const size_t vh = m_pBuffer->GetUint8 (4);
  const size_t vl = m_pBuffer->GetUint8 (5);
  out_format (rc (HRC_GENLIB_PMWLITE_HEADER),
              vh, vl);
}
