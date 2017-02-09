#include "gt_exe_pe.hxx"

#include "gt_exception.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_table.hxx"
#include "gt_utils.hxx"

namespace GT {

//--------------------------------------------------------------------
void GT_CALL EXE_PE_ExportAnalyzer::List ()
//--------------------------------------------------------------------
{
  gtuint16* pOrdinals    = NULL;
  rva_t*    pFunctions   = NULL;
  rva_t*    pNameOffsets = NULL;
  pstring sName;
  EXE_PE_ExportHeader aExportHeader;
  pstring sForwardedTo;

  out_append (_T ("\n"));

  if (m_nOffset < 0 || m_nOffset >= m_pBuffer->GetFileSize ())
  {
    out_error_format (rc (HRC_EXELIB_PE_EXPORT_INVALID),
                      m_nOffset);
    return;
  }

  try
  {
    // read export header
    m_pBuffer->GetBufferX (m_nOffset, &aExportHeader, EXE_PE_EXPORTHEADER_SIZE);

    if (aExportHeader.nNumberOfNames > 0xffff)
    {
      out_error_format (rc (HRC_EXELIB_PE_EXPORT_TOO_MANY),
                        aExportHeader.nNumberOfNames);
    }
    else
      if (aExportHeader.nNumberOfNames > 0)
      {
        // get phys. ofs of DLL name RVA
        const gtuint32 nDLLNameOfs = m_pSectionTable->r2o (aExportHeader.nDLLNameRVA);

        // read string of original DLL name
        m_pBuffer->GetASCIIZ_AString (nDLLNameOfs, sName, GT_MAX_SIZE);

        out_format (rc (HRC_EXELIB_PE_EXPORT_DLL_NAME),
                    sName.c_str ());

        out_incindent ();

        // base ordinal number
        out_format (rc (HRC_EXELIB_PE_EXPORT_BASE_ORDINAL),
                    aExportHeader.nOrdinalBase, aExportHeader.nOrdinalBase);

        // number of exported names
        if (aExportHeader.nNumberOfNames == 1)
          out_append (rc (HRC_EXELIB_PE_EXPORT_1_NAME_EXPORTED));
        else
          out_format (rc (HRC_EXELIB_PE_EXPORT_N_NAMES_EXPORTED),
                      aExportHeader.nNumberOfNames);

        // number of exported functions
        if (aExportHeader.nNumberOfFunctions == 1)
          out_append (rc (HRC_EXELIB_PE_EXPORT_1_FUNC_EXPORTED));
        else
          out_format (rc (HRC_EXELIB_PE_EXPORT_N_FUNCS_EXPORTED),
                      aExportHeader.nNumberOfFunctions);

        // define a new table
        Table aTable;
        aTable.AddColumn (rc (HRC_EXELIB_PE_EXPORT_COLUMN_ORDINAL), NUM_SPACE, 8,  4);
        aTable.AddColumn (rc (HRC_EXELIB_PE_EXPORT_COLUMN_OFFSET),  HEX,       11, 8);
        aTable.AddColumn (rc (HRC_EXELIB_PE_EXPORT_COLUMN_NAME),    STR,       4,  ALIGN_LEFT);

        pOrdinals    = new gtuint16 [aExportHeader.nNumberOfNames];
        pFunctions   = new rva_t    [aExportHeader.nNumberOfNames];
        pNameOffsets = new rva_t    [aExportHeader.nNumberOfNames];

        // and fill the buffer (use the offsets!)
        m_pBuffer->GetBufferX (m_pSectionTable->r2o (aExportHeader.nNamesOrdinalRVA), pOrdinals,    aExportHeader.nNumberOfNames * 2);
        m_pBuffer->GetBufferX (m_pSectionTable->r2o (aExportHeader.nFunctionsRVA),    pFunctions,   aExportHeader.nNumberOfNames * 4);
        m_pBuffer->GetBufferX (m_pSectionTable->r2o (aExportHeader.nNamesRVA),        pNameOffsets, aExportHeader.nNumberOfNames * 4);

        for (gtuint32 i = 0; i < aExportHeader.nNumberOfNames; ++i)
        {
          // get exported string name
          m_pBuffer->GetASCIIZ_AString (m_pSectionTable->r2o (pNameOffsets[i]), sName, GT_MAX_SIZE);

          sForwardedTo.clear ();

          // check if it is an forwarded export
          // by checking whether the functions RVA is inside our export DD.
          // If the RVA points into our Export DD we read another string
          //   at this position and this is the function we forward to!
          if (pFunctions[i] >= m_pDataDir->nRVA &&
              pFunctions[i] <= (m_pDataDir->nRVA + m_pDataDir->nSize))
          {
            // forwarded export!
            m_pBuffer->GetASCIIZ_AString (m_pSectionTable->r2o (pFunctions[i]), sForwardedTo, GT_MAX_SIZE);

            // In compluslm.dll from VC8 this is not really true - dunno why... (length == 1)
            if (sForwardedTo.length () <= 1)
              sForwardedTo.clear ();
          }

          if (!sForwardedTo.empty ())
          {
            // append to string
            sName += _T (" -> ");
            sName += sForwardedTo;

            aTable.AddInt (0, aExportHeader.nOrdinalBase + gtuint32 (pOrdinals[i]));
            aTable.AddInt (1, gtuint32 (0));
            aTable.AddStr (2, sName.c_str ());
          }
          else
          {
            aTable.AddInt (0, aExportHeader.nOrdinalBase + gtuint32 (pOrdinals[i]));
            aTable.AddInt (1, m_pSectionTable->r2o (pFunctions[i]));
            aTable.AddStr (2, sName.c_str ());
          }
        }

        out_table (&aTable);

        out_decindent ();
      }
  }
  catch (const Excpt &e)
  {
    e.Display ();
  }

  delete [] pOrdinals;
  delete [] pFunctions;
  delete [] pNameOffsets;
}

}  // namespace
