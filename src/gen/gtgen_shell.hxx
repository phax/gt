MAKE_ANALYZER (Shell);

/*! text only
 */
//--------------------------------------------------------------------
MAKE_IS_A (Shell)
//--------------------------------------------------------------------
{
  if (!aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->CompareA (0, 2, "#!");
}

/*! Try to determine whether a file is a shell script.
    The algorithm is quite trivial:
      If the first 2 chars of a file are "#!" then we
      copy the rest of the line (except leading spaces)
      and put it to the output.
 */
//--------------------------------------------------------------------
MAKE_DISPLAY (Shell)
//--------------------------------------------------------------------
{
  // read first line (excluding the leading <#!> chars -> ofs 2)
  // (e.g. </usr/local/bin/perl -w>)
  char sChar[GT_MAX_SIZE];
  m_pBuffer->GetUntil (2, '\n', sChar, GT_MAX_SIZE);

  // skip leading spaces
  size_t nIdx = 0;
  while (nIdx < GT_MAX_SIZE && isspace (sChar [nIdx]))
    nIdx++;

  // make appropriate string
  pstring sTemp;
  str_assign (sTemp, sChar + nIdx);

  out_format (rc (HRC_GENLIB_SHELL_HEADER),
              sTemp.c_str ());
}
