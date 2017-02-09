MAKE_ANALYZER (LIB);

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (LIB)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->CompareA (0, 8, "!<arch>\n");
}

//--------------------------------------------------------------------
MAKE_DISPLAY (LIB)
//--------------------------------------------------------------------
{
  LIB_Header aLH;
  string sName;
  pstring sConvName;
  size_t nSize;
  size_t n;
  char* pEnd;
  char* pLongNames = NULL;

  out_append (rc (HRC_GENLIB_LIB_HEADER));

  if (!m_bListMode)
  {
    // start reading at 8
    m_pBuffer->SetActFilePos (8);

    // create table
    Table aTable;
    aTable.AddColumn (rc (HRC_HENLIB_LIB_TABLE_OFFSET), HEX,       10, 8);
    aTable.AddColumn (rc (HRC_HENLIB_LIB_TABLE_SIZE),   NUM_SPACE, 10, 8);
    aTable.AddColumn (rc (HRC_HENLIB_LIB_TABLE_NAME),   STR,       4,  ALIGN_LEFT);

    // loop
    while (m_pBuffer->GetBuffer (&aLH, sizeof (LIB_Header)))
    {
      if (aLH.m_nEnd != 0x0a60)
      {
        out_error_append (rc (HRC_GENLIB_LIB_ERR_INVALID));
        break;
      }

      // get size of member
      nSize = strtol (aLH.m_sMemberSize, &pEnd, 10);

      // get name
      if (aLH.m_sName[0] == '/')
      {
        // special name!
        switch (aLH.m_sName[1])
        {
          case '/':
          {
            // the long name member -> read all long names
            // Each long name ends with a '\0'!
            pLongNames = new char [nSize + 1];
            pLongNames[nSize] = 0;
            m_pBuffer->GetBuffer (pLongNames, nSize);
            m_pBuffer->IncActFilePos (-file_t (nSize));
            sName.assign ("//");
            break;
          }
          case ' ':
          {
            // special linker member
            sName.assign ("/");
            break;
          }
          default:
          {
            // should be a digit
            ASSERT (isdigit (aLH.m_sName[1]));

            const long nLongNameOfs = strtol (aLH.m_sName + 1, &pEnd, 10);

            // for C:\cygwin\lib\libgcj.a we have the problem, that the long
            // name table does not consist of NULL-terminated string but
            // of '/' terminated strings.
            // The original code was simply:
            // sName.assign (pLongNames + nLongNameOfs);
            size_t nLength = 0;
            while (pLongNames[nLongNameOfs + nLength] != '\0' &&
                   pLongNames[nLongNameOfs + nLength] != '/')
              nLength++;
            sName.assign (pLongNames + nLongNameOfs, nLength);
          }
        }
      }
      else
      {
        // normal name
        sName.clear ();
        for (n = 0; n < 16; ++n)
        {
          if (aLH.m_sName[n] == '/')
            break;
          sName.push_back (aLH.m_sName[n]);
        }
      }

      // convert from char* to TCHAR*
      str_assign (sConvName, sName);

      // write offset, name and size
      aTable.AddInt (0, m_pBuffer->GetActFilePos () - sizeof (LIB_Header));
      aTable.AddInt (1, file_t (nSize));
      aTable.AddStr (2, sConvName);

      // skip member (PH: seems to be aligned to 2!)
      m_pBuffer->IncActFilePos (ALIGN_2 (nSize));
    }

    // write table!
    out_incindent ();
    out_table (&aTable);
    out_decindent ();
  }

  // free mem
  delete pLongNames;
}

