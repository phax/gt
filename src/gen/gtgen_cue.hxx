MAKE_ANALYZER (CUE);

/*! Text only
 */
//--------------------------------------------------------------------
MAKE_IS_A (CUE)
//--------------------------------------------------------------------
{
  if (!aGI.m_pBuffer->IsText ())
    return false;

  char sBuffer[MAX_PATH];
  size_t nBufferSize;
  return aGI.m_pBuffer->CompareA (0, 6, "FILE \"") &&
         aGI.m_pBuffer->GetUntil (6, '"', sBuffer, sizeof (sBuffer) - 1, nBufferSize) &&
         aGI.m_pBuffer->GetUint8 (6 + nBufferSize) == '"';
}

//--------------------------------------------------------------------
MAKE_DISPLAY (CUE)
//--------------------------------------------------------------------
{
  char sBuffer[MAX_PATH];
  m_pBuffer->GetUntil (6, '"', sBuffer, sizeof (sBuffer) - 1);
  out_format (rc (HRC_GENLIB_CUE_HEADER), sBuffer);
}
