MAKE_ANALYZER (DBF);

namespace DBF {

//--------------------------------------------------------------------
static bool _InitBasicBuffer
                                        (FileBuffer* pBuffer,
                                         DBF_GlobalHeader* pGH,
                                         gtint32& nFieldCount)
//--------------------------------------------------------------------
{
  if (!pBuffer->GetBuffer (0, pGH, DBF_GH_SIZE))
    return false;

  // this must be the number of fields
  nFieldCount = (gtint32 (pGH->nHeaderSize) - DBF_GH_SIZE - 1) / DBF_LH_SIZE;

  // check for strange or unexpected values
  if (pGH->nHeaderSize == 0 ||
      nFieldCount <= 0 ||
      pGH->bBreakOnOperation ||
      pGH->bEncrypted ||
//        pGH->bMDXFile ||
      (gtuint32) (DBF_GH_SIZE + 1 + pGH->nRecordCount * pGH->nRecordSize + 1) > (gtuint32) pBuffer->GetFileSize ())
  {
    return false;
  }

  return true;
}

}  // namespace DBF

/*! Text or binary
 */
//------------------------------------------------------------------
MAKE_IS_A (DBF)
//------------------------------------------------------------------
{
  const gtuint8 DBF_END_OF_RECORDS = 13;

  // avoid too many method calls
  FileBuffer* pBuffer = aGI.m_pBuffer;
  const file_t nFileSize = pBuffer->GetFileSize ();

  // read header values
  DBF_GlobalHeader aGH;
  gtint32 nFieldCount;
  if (!DBF::_InitBasicBuffer (pBuffer, &aGH, nFieldCount))
    return false;

  // local vars
  file_t nEndPos = DBF_GH_SIZE;   // start pos
  gtint32 nLocalFieldCount = 0;
  gtint32 nRecordSize = 0;
  gtuint8 nEndPosByte = pBuffer->GetUint8 (nEndPos);
  DBF_LocalHeader aLH;

  // read header (as long we're in the cached area)
  while ((nEndPos + DBF_LH_SIZE) < nFileSize &&
         nEndPosByte != DBF_END_OF_RECORDS &&
         nLocalFieldCount < nFieldCount)
  {
    // try to read another local header
    if (!pBuffer->GetBuffer (nEndPos, &aLH, DBF_LH_SIZE))
      return false;

    // increase the total record size
    nRecordSize += aLH.nLength;
    nLocalFieldCount++;
    nEndPos += DBF_LH_SIZE;

    if (nEndPos >= nFileSize)
      return false;

    nEndPosByte = pBuffer->GetUint8 (nEndPos);
  }

  // needed because couting starts at 0 (don't know exactly why)
//    if (_abs (gtint32 (aGH.nHeaderSize - nEndPos)) <= 2)
  {
    nEndPos = aGH.nHeaderSize;
  }

  const size_t nCalculatedFilesSize = aGH.nHeaderSize +
                                      aGH.nRecordCount * aGH.nRecordSize +
                                      1;

  bool bIsDBF;

  if (aGH.bMDXFile)
  {
    // no record size can be calculated in MDX file!
    bIsDBF = (aGH.nHeaderSize == nEndPos) &&
             (nFieldCount     == nLocalFieldCount) &&
             (0               == nEndPosByte) &&
             (nFileSize       == nCalculatedFilesSize);  // +2: adr.dbf
  }
  else
  {
    // one finalization byte
    nRecordSize++;
    bIsDBF = (aGH.nHeaderSize    == nEndPos) &&
             (aGH.nRecordSize    == nRecordSize) &&
             (nFieldCount        == nLocalFieldCount) &&
             (DBF_END_OF_RECORDS == nEndPosByte) &&
             (nFileSize          == nCalculatedFilesSize);  // +2: adr.dbf
  }

  // just stupid debug output!
  if (0 && !bIsDBF)
  {
    out_format (_T ("[dbg] [%s] (%d == %I64d) (%d == %d) (%d == %d) (%d == %d) (%I64d == %d)\n"),
                aGH.bMDXFile ? _T ("DBF MDX") : _T ("DBF"),

                aGH.nHeaderSize,
                nEndPos,

                aGH.nRecordSize,
                aGH.bMDXFile ? aGH.nRecordSize : nRecordSize,

                nFieldCount,
                nLocalFieldCount,

                aGH.bMDXFile ? 0 : DBF_END_OF_RECORDS,
                nEndPosByte,

                nFileSize,
                nCalculatedFilesSize);
  }

  return bIsDBF;
}

//------------------------------------------------------------------
MAKE_DISPLAY (DBF)
//------------------------------------------------------------------
{
  // read header values
  DBF_GlobalHeader aGH;
  gtint32          nFieldCount;
  DBF::_InitBasicBuffer (m_pBuffer, &aGH, nFieldCount);

  // write info (first 2 bits contain the version)
  out_format (rc (HRC_GENLIB_DBF_VERSION),
              aGH.nFlag & 7);

  // not in listmode?
  if (!m_bListMode)
  {
    DBF_LocalHeader aLH;
    pstring sName;

    out_incindent ();

    if (aGH.bMDXFile)
      out_append (rc (HRC_GENLIB_DBF_INDEX_FILE));

    out_format (rc (HRC_GENLIB_DBF_OVERVIEW),
                aGH.nRecordCount,
                nFieldCount,
                aGH.nRecordSize);

    if (aGH.bMDXFile)
    {
      out_append (_T ("\n"));
    }
    else
    {
      // skip main header
      m_pBuffer->SetActFilePos (DBF_GH_SIZE);

      // init table
      Table aTable;
      aTable.AddColumn (rc (HRC_GENLIB_DBF_COLUMN_NAME), STR,       13, ALIGN_RIGHT);
      aTable.AddColumn (rc (HRC_GENLIB_DBF_COLUMN_LEN),  NUM_SPACE,  5, 3);
      aTable.AddColumn (rc (HRC_GENLIB_DBF_COLUMN_TYPE), STR,       11, ALIGN_LEFT);

      for (gtint32 i = 0; i < nFieldCount; i++)
      {
        // read column data
        m_pBuffer->GetBufferX (&aLH, DBF_LH_SIZE);

        // convert name to an ASCIIZ string and add to table
        str_assign (sName, aLH.sName, sizeof (aLH.sName));
        aTable.AddStr (0, sName);

        // len and type not present in MDX file
        aTable.AddInt (1, gtuint32 (aLH.nLength));

        switch (aLH.nType)
        {
          case 'B': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_BINARY);    break;
          case 'C': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_CHARACTER); break;
          case 'D': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_DATE);      break;
          case 'F': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_FLOAT);     break;
          case 'G': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_GENERAL);   break;
          case 'L': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_LOGICAL);   break;
          case 'M': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_MEMO);      break;
          case 'N': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_NUMERICAL); break;
          case 'P': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_PICTURE);   break;
          case 'V': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_VARIABLE);  break;
          case '2': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_SHORT_INT); break;
          case '4': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_LONG_INT);  break;
          case '8': aTable.AddStr (2, HRC_GENLIB_DBF_COLUMN_DOUBLE);    break;
          default:
          {
            // reuse sName var :-|
            str_assign_format (sName, rc (HRC_GENLIB_DBF_COLUMN_UNKNOWN), (char) aLH.nType);
            aTable.AddStr (2, sName);
            break;
          }
        }
      }

      // show table
      out_incindent ();
      out_table (&aTable);
      out_decindent ();
    }

    out_decindent ();
  }
}
