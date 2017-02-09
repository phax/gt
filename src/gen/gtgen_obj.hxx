MAKE_ANALYZER (OBJ);

struct COFF_FPO_DATA
{
  gtuint32 ulOffStart;            // offset 1st byte of function code
  gtuint32 cbProcSize;            // # bytes in function
  gtuint32 cdwLocals;             // # bytes in locals/4
  gtuint16 cdwParams;             // # bytes in params/4

  gtuint16 cbProlog : 8;          // # bytes in prolog
  gtuint16 cbRegs   : 3;          // # regs saved
  gtuint16 fHasSEH  : 1;          // TRUE if SEH in func
  gtuint16 fUseBP   : 1;          // TRUE if EBP has been allocated
  gtuint16 reserved : 1;          // reserved for future use
  gtuint16 cbFrame  : 2;          // frame type
};
GT_COMPILER_ASSERT (sizeof (COFF_FPO_DATA) == 16);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (OBJ)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetUint16 (0) == 0x014C;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (OBJ)
//--------------------------------------------------------------------
{
  COFF_Header aH;
  m_pBuffer->GetBufferX (0, &aH, COFF_H_SIZE);

  out_format (rc (HRC_GENLIB_OBJ_HEADER),
              aH.nNumberOfSections);

  if (!m_bListMode)
  {
    out_incindent ();

    pstring s;
    gtuint32 i;

    EXE_PE_SectionTableAnalyzer aSTA (m_pBuffer,
                                      COFF_H_SIZE,
                                      aH.nNumberOfSections,
                                      false);  // not EXE mode
    EXE_PE_Section *aSections = aSTA.GetSectionPtr ();
    const size_t nSectionCount = aSTA.GetNumberOfSections ();

    // symbol table at
    out_format (rc (HRC_GENLIB_OBJ_SYMTAB_AT),
                aH.nPtrToSymbolTable, aH.nPtrToSymbolTable);

    // symbol table count
    out_format (rc (HRC_GENLIB_OBJ_SYMTAB_COUNT),
                aH.nNumberOfSymbols);

    // Any flags?
    if (aH.nCharacteristics != 0)
    {
      out_format (rc (HRC_GENLIB_OBJ_CHARACTERISTICS),
                  aH.nCharacteristics);
      out_incindent ();
      if (aH.nCharacteristics & 0x0001) out_append (rc (HRC_GENLIB_OBJ_FLAG_RELOCS_STRP));
      if (aH.nCharacteristics & 0x0002) out_append (rc (HRC_GENLIB_OBJ_FLAG_EXEC));
      if (aH.nCharacteristics & 0x0004) out_append (rc (HRC_GENLIB_OBJ_FLAG_NO_LINES));
      if (aH.nCharacteristics & 0x0008) out_append (rc (HRC_GENLIB_OBJ_FLAG_NO_SYMTAB));
      if (aH.nCharacteristics & 0x0010) out_append (rc (HRC_GENLIB_OBJ_FLAG_AGGR_TRIM));
      if (aH.nCharacteristics & 0x0020) out_append (rc (HRC_GENLIB_OBJ_FLAG_2GB));
      if (aH.nCharacteristics & 0x0040) out_append (rc (HRC_GENLIB_OBJ_FLAG_16BIT_WORD));
      if (aH.nCharacteristics & 0x0080) out_append (rc (HRC_GENLIB_OBJ_FLAG_LITTLE_END));
      if (aH.nCharacteristics & 0x0100) out_append (rc (HRC_GENLIB_OBJ_FLAG_32BIT_WORD));
      if (aH.nCharacteristics & 0x0200) out_append (rc (HRC_GENLIB_OBJ_FLAG_NO_DEBUG));
      if (aH.nCharacteristics & 0x0400) out_append (rc (HRC_GENLIB_OBJ_FLAG_REMOVEABLE));
  //  if (aH.nCharacteristics & 0x0800) undefined
      if (aH.nCharacteristics & 0x1000) out_append (rc (HRC_GENLIB_OBJ_FLAG_SYSTEM_FILE));
      if (aH.nCharacteristics & 0x2000) out_append (rc (HRC_GENLIB_OBJ_FLAG_DLL));
      if (aH.nCharacteristics & 0x4000) out_append (rc (HRC_GENLIB_OBJ_FLAG_RUN_ON_UP));
      if (aH.nCharacteristics & 0x8000) out_append (rc (HRC_GENLIB_OBJ_FLAG_BIG_END));
      out_decindent ();
    }

    out_format (rc (HRC_GENLIB_OBJ_SIZEOF_OPT_HDR),
                aH.nSizeOfOptionalHeader,
                aH.nSizeOfOptionalHeader);

    // list all sections
    {
      out_append (rc (HRC_GENLIB_OBJ_SECTION_LIST));

      Table aTable;
      aTable.AddColumn (rc (HRC_GENLIB_OBJ_LIST_NR),       NUM_SPACE, 6,  4);
      aTable.AddColumn (rc (HRC_GENLIB_OBJ_LIST_NAME),     STR,       10, ALIGN_LEFT);
      aTable.AddColumn (rc (HRC_GENLIB_OBJ_LIST_REALNAME), STR,       10, ALIGN_LEFT);
      aTable.AddColumn (rc (HRC_GENLIB_OBJ_LIST_OFFSET),   HEX,       10, 8);
      aTable.AddColumn (rc (HRC_GENLIB_OBJ_LIST_SIZE),     HEX,       10, 8);

      for (i = 0; i < nSectionCount; i++)
      {
        EXE_PE_Section *pCurrentSection = &aSections[i];

        // get readonly ptr
        LPCTSTR sSectionName = aSTA.GetSectionName (i, true);

        // get real name (skip everything after '$')
        TCHAR sRealName[9];
        _tcsncpy (sRealName, sSectionName, 9);
        LPTSTR pDollar = _tcschr (sRealName, _T ('$'));
        if (pDollar)
          while (*pDollar)
            *pDollar++ = _T (' ');

        aTable.AddInt (0, i);
        aTable.AddStr (1, sSectionName);
        aTable.AddStr (2, sRealName);
        aTable.AddInt (3, pCurrentSection->nPhysicalOffset);
        aTable.AddInt (4, pCurrentSection->nPhysicalSize);
      }

      out_incindent ();
      out_table (&aTable);
      out_decindent ();
    }

    // and show some info about the sections
    size_t nKnownSections = 0;
    for (i = 0; i < nSectionCount; i++)
    {
      EXE_PE_Section *pCurrentSection = &aSections[i];

      LPCTSTR sSectionName = aSTA.GetSectionName (i, true);
      ++nKnownSections;

      if (_tcscmp (sSectionName, _T (".arch   ")) == 0)
      {
        out_append (rc (HRC_GENLIB_OBJ_SECTION_ARCH));
      }
      else if (_tcscmp (sSectionName, _T (".bss    ")) == 0)
      {
        out_append (rc (HRC_GENLIB_OBJ_SECTION_BSS));
      }
      else if (_tcscmp (sSectionName, _T (".data   ")) == 0)
      {
        out_append (rc (HRC_GENLIB_OBJ_SECTION_DATA));
      }
      else if (_tcscmp (sSectionName, _T (".debug$F")) == 0)
      {
        out_format (rc (HRC_GENLIB_OBJ_SECTION_DEBUG_F),
                    pCurrentSection->nPhysicalSize / sizeof (COFF_FPO_DATA));

        // check alignment
        if ((pCurrentSection->nPhysicalSize % sizeof (COFF_FPO_DATA)) != 0)
          out_error_append (rc (HRC_GENLIB_OBJ_SECTION_FPO_ERR));
      }
      else if (_tcscmp (sSectionName, _T (".debug$S")) == 0 ||
               _tcscmp (sSectionName, _T (".debug$T")) == 0)
      {
        out_append (rc (HRC_GENLIB_OBJ_SECTION_DEBUG));

        const gtuint32 nVersion = m_pBuffer->GetInt32 (pCurrentSection->nPhysicalOffset);
        switch (nVersion)
        {
          case 0x00000000:
            out_append (rc (HRC_GENLIB_OBJ_SECTION_DEBUG_CV3));
            break;
          case 0x00000001:
            out_append (rc (HRC_GENLIB_OBJ_SECTION_DEBUG_CV4));
            break;
          case 0x00000002:
            out_append (rc (HRC_GENLIB_OBJ_SECTION_DEBUG_CV5));
            break;
          default:
            out_format (rc (HRC_GENLIB_OBJ_SECTION_DEBUG_UNK),
                        nVersion);
            break;
        }
        out_append (_T ("\n"));
      }
      else if (_tcscmp (sSectionName, _T (".drectve")) == 0)
      {
        // read linker directive
        m_pBuffer->GetFixedSizeAString (pCurrentSection->nPhysicalOffset,
                                        s,
                                        pCurrentSection->nPhysicalSize);

        // and print it
        out_format (rc (HRC_GENLIB_OBJ_SECTION_DRECTVE),
                    s.c_str ());
      }
      else if (_tcscmp (sSectionName, _T (".edata  ")) == 0)
      {
        out_append (rc (HRC_GENLIB_OBJ_SECTION_EDATA));
      }
      else if (_tcscmp (sSectionName, _T (".idata  ")) == 0)
      {
        out_append (rc (HRC_GENLIB_OBJ_SECTION_IDATA));
      }
      else if (_tcscmp (sSectionName, _T (".pdata  ")) == 0)
      {
        out_append (rc (HRC_GENLIB_OBJ_SECTION_PDATA));
      }
      else if (_tcsncmp (sSectionName, _T (".rdata"), 6) == 0)
      {
        // seen: .rdata$r
        out_append (rc (HRC_GENLIB_OBJ_SECTION_RDATA));
      }
      else if (_tcscmp (sSectionName, _T (".reloc  ")) == 0)
      {
        out_append (rc (HRC_GENLIB_OBJ_SECTION_RELOC));
      }
      else if (_tcscmp (sSectionName, _T (".rsrc   ")) == 0)
      {
        out_append (rc (HRC_GENLIB_OBJ_SECTION_RSRC));
      }
      else if (_tcscmp (sSectionName, _T (".sxdata ")) == 0)
      {
        out_append (rc (HRC_GENLIB_OBJ_SECTION_SXDATA));
      }
      else if (_tcsncmp (sSectionName, _T (".text"), 5) == 0)
      {
        // seen: .text$yd
        out_append (rc (HRC_GENLIB_OBJ_SECTION_TEXT));
      }
      else if (_tcscmp (sSectionName, _T (".tls    ")) == 0)
      {
        out_append (rc (HRC_GENLIB_OBJ_SECTION_TLS));
      }
      else if (_tcsncmp (sSectionName, _T (".xdata"), 6) == 0)
      {
        // seen: .xdata$x
        out_append (rc (HRC_GENLIB_OBJ_SECTION_XDATA));
      }
      else
      {
        --nKnownSections;
        out_format (rc (HRC_GENLIB_OBJ_SECTION_UNKNOWN),
                    sSectionName);
      }
    }

    if (nKnownSections != nSectionCount)
    {
      out_incindent ();
      out_info_format (rc (HRC_GENLIB_OBJ_IDENTIFIED),
                       nKnownSections,
                       nSectionCount);
      out_decindent ();
    }

    out_decindent ();
  }
}
