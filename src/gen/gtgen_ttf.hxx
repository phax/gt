MAKE_ANALYZER (TTF);

namespace TTF {

//--------------------------------------------------------------------
static void _ProcessEntry
                                        (      FileBuffer* pBuffer,
                                               TTF_Name*   pName,
                                         const file_t      nLocalOffset,
                                         const size_t      nPos)
//--------------------------------------------------------------------
{
  ASSERT (pName);
  ASSERT (nPos > 0);

  if (nPos > pName->nCount)
  {
    out_error_append (rc (HRC_GENLIB_TTF_INVALID_IDX));
    return;
  }

  // read string table entry [nPos - 1]
  TTF_Entry aEntry;
  pBuffer->GetBuffer (nLocalOffset + TTF_NAME_SIZE + (nPos - 1) * TTF_ENTRY_SIZE,
                      &aEntry,
                      TTF_ENTRY_SIZE);

  aEntry.nPlatformID   = WORD_SWAP_BYTE (aEntry.nPlatformID);
  aEntry.nEncodingID   = WORD_SWAP_BYTE (aEntry.nEncodingID);
  aEntry.nLanguageID   = WORD_SWAP_BYTE (aEntry.nLanguageID);
  aEntry.nNameID       = WORD_SWAP_BYTE (aEntry.nNameID);
  aEntry.nStringLength = WORD_SWAP_BYTE (aEntry.nStringLength);
  aEntry.nRelOffset    = WORD_SWAP_BYTE (aEntry.nRelOffset);

  if (0) out_format (_T ("[dbg] Currently at %I64d/%08I64Xh (%d) (%d)\n"),
                     pBuffer->GetActFilePos (), pBuffer->GetActFilePos (),
                     aEntry.nPlatformID, aEntry.nEncodingID);
  if (0) out_format (_T ("[dbg] Reading name at %08I64Xh\n"), nLocalOffset + pName->nOffset + aEntry.nRelOffset);

  pstring sName;
  if (aEntry.nPlatformID == 0 || aEntry.nPlatformID == 3)
  {
    // string is Unicode in the file
    // string table ofs + name table offset + relative offset (e.g. 0x51b0e + 0x13e = 51c4c)
    pBuffer->GetFixedSizeWStringBigEndian (nLocalOffset + pName->nOffset + aEntry.nRelOffset,
                                           sName,
                                           aEntry.nStringLength / 2);
  }
  else
  {
    // string is ANSI in the file
    // string table ofs + name table offset + relative offset (e.g. 0x51b0e + 0x13e = 51c4c)
    pBuffer->GetFixedSizeAString (nLocalOffset + pName->nOffset + aEntry.nRelOffset,
                                  sName,
                                  aEntry.nStringLength);
  }

  out_format (rc (HRC_GENLIB_TTF_ENTRY),
              sName.c_str ());
}

}  // namespace TTF

/*! Binary files only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (TTF)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  const gtuint32 l0  = aGI.m_pBuffer->GetInt32 (0);
  const gtuint16 w4  = aGI.m_pBuffer->GetUint16 (4);
  const gtuint16 w6  = aGI.m_pBuffer->GetUint16 (6);
  const gtuint16 w10 = aGI.m_pBuffer->GetUint16 (10);

  gtuint32 nID            = LONG_SWAP_BYTE (l0);
  gtuint16 nTables        = WORD_SWAP_BYTE (w4);
  gtuint16 nSearchRange   = WORD_SWAP_BYTE (w6);
  gtuint16 nRangeShift    = WORD_SWAP_BYTE (w10);

  return (nID == 0x00010000) &&
         (nRangeShift == (gtint32 (nTables) << 4) - nSearchRange);
}

//--------------------------------------------------------------------
MAKE_DISPLAY (TTF)
//--------------------------------------------------------------------
{
  const gtuint32 l0  = m_pBuffer->GetInt32 (0);
  const gtuint16 w4  = m_pBuffer->GetUint16 (4);
  const gtuint16 w6  = m_pBuffer->GetUint16 (6);
  const gtuint16 w8  = m_pBuffer->GetUint16 (8);
  const gtuint16 w10 = m_pBuffer->GetUint16 (10);

  TTF_GlobalHeader aGH;
  aGH.nID            = LONG_SWAP_BYTE (l0);
  aGH.nTables        = WORD_SWAP_BYTE (w4);
  aGH.nSearchRange   = WORD_SWAP_BYTE (w6);
  aGH.nEntrySelector = WORD_SWAP_BYTE (w8);
  aGH.nRangeShift    = WORD_SWAP_BYTE (w10);

  pstring sFontName;
  pstring sType;
  TTF_LocalHeader aLH;
  TTF_Name aName;

  // init the current position
  file_t nPos = TTF_GH_SIZE;

  out_append (rc (HRC_GENLIB_TTF_HEADER));

  // go through all tables
  for (gtuint16 i = 0; i < aGH.nTables; i++)
  {
    nPos += 16;

    m_pBuffer->GetBufferX (nPos, &aLH, TTF_LH_SIZE);
    aLH.nCheckSum = LONG_SWAP_BYTE (aLH.nCheckSum);
    aLH.nOffset   = LONG_SWAP_BYTE (aLH.nOffset);
    aLH.nLength   = LONG_SWAP_BYTE (aLH.nLength);

    if (aLH.nID == 0x656D616E)  // no swap! ("name")
    {
      // read the name info buffer at the offset specified
      // in the nOffset member (e.g. 0x000517c0)
      m_pBuffer->GetBufferX (aLH.nOffset, &aName, TTF_NAME_SIZE);

      // swap these bytes too
      aName.nFormat = WORD_SWAP_BYTE (aName.nFormat);
      aName.nCount  = WORD_SWAP_BYTE (aName.nCount);
      aName.nOffset = WORD_SWAP_BYTE (aName.nOffset);

      // aName.nCount specifies the number of entries in the string-table
      TTF::_ProcessEntry (m_pBuffer, &aName, aLH.nOffset, 2);
      TTF::_ProcessEntry (m_pBuffer, &aName, aLH.nOffset, 3);

      // exit the "for" loop - we only want the name
      break;
    }
  }

  // finishes the first info line!
  out_append (_T ("\n"));
}
