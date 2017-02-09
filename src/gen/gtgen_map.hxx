MAKE_ANALYZER (MAP);

/*! text only
 */
//--------------------------------------------------------------------
MAKE_IS_A (MAP)
//--------------------------------------------------------------------
{
  if (!aGI.m_pBuffer->IsText ())
    return false;

  file_t nPos = aGI.m_pBuffer->Find ("\xD\xA\xD\xA", 0, 64);
  if (nPos == GT_NOT_FOUND)
    return false;

  return aGI.m_pBuffer->CompareA (nPos + 4, 10, " Timestamp");
}

//--------------------------------------------------------------------
MAKE_DISPLAY (MAP)
//--------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_MAP_MS_HEADER));
}
