#include "gt_exe_pe.hxx"

#include "gt_cmdline_params.hxx"
#include "gt_exception.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_utils.hxx"

namespace GT {

//--------------------------------------------------------------------
void GT_CALL EXE_PE_ImportAnalyzer::_ListImportedNames
                                        (const gtuint32 nLookupTableOffset)
//--------------------------------------------------------------------
{
  gtuint32 nCurrentOffset = nLookupTableOffset;
  pstring sName;

  try
  {
    for (;;)
    {
      const rva_t nCurrentRVA (m_pBuffer->GetInt32 (nCurrentOffset));
      if (nCurrentRVA == rva_t (0))
        break;

      // inc offset 4 bytes (== sizeof (gtuint32))
      nCurrentOffset += 4;

      const rva_t aRealRVA (GT_UPPER_31_BITS (nCurrentRVA));

      if (GT_ISSET_BIT_0 (nCurrentRVA))
      {
        // its only a number
        out_format (rc (HRC_EXELIB_PE_IMPORT_LIST_ORDINAL),
                    RVA_VAL (aRealRVA));
      }
      else
      {
        // we have a name!
        const gtuint32 nNameOffset = m_pSectionTable->r2o (aRealRVA);

        // read function name there
        m_pBuffer->GetASCIIZ_AString (nNameOffset + 2, sName, GT_MAX_SIZE);

        out_format (rc (HRC_EXELIB_PE_IMPORT_LIST_NAME),
                    sName.c_str ());
      }
    }
  }
  catch (const Excpt &e)
  {
    e.Display ();
  }
}

//--------------------------------------------------------------------
void GT_CALL EXE_PE_ImportAnalyzer::List ()
//--------------------------------------------------------------------
{
  EXE_PE_ImportTableEntry aImportTableEntry;
  size_t nDLLIndex = 0;
  file_t nInfoOffset = m_nOffset;
  pstring sDLLName;

  out_append (_T ("\n"));

  if (m_nOffset < 0 || m_nOffset >= m_pBuffer->GetFileSize ())
  {
    out_error_format (rc (HRC_EXELIB_PE_IMPORT_INVALID),
                      m_nOffset);
    return;
  }

  out_append (rc (HRC_EXELIB_PE_IMPORT_LIST_HEADER));

  out_incindent ();

  try
  {
    for (;;++nDLLIndex)
    {
      // read cur entry
      m_pBuffer->GetBuffer (nInfoOffset,
                            &aImportTableEntry,
                            EXE_PE_IMPORTTABLEENTRY_SIZE);

      nInfoOffset += EXE_PE_IMPORTTABLEENTRY_SIZE;

      // last entry?
      if (aImportTableEntry.nNameRVA == rva_t (0))
        break;

      const gtuint32 nDLLNameOffset = m_pSectionTable->r2o (aImportTableEntry.nNameRVA);

      // read DLL name
      m_pBuffer->GetASCIIZ_AString (nDLLNameOffset, sDLLName, GT_MAX_SIZE);

      out_format (rc (HRC_EXELIB_PE_IMPORT_LIST_DLL),
                  nDLLIndex,
                  sDLLName.c_str ());

      // is it a forwarded import
      if (aImportTableEntry.nForwarderChain != 0 &&
          aImportTableEntry.nForwarderChain != 0xFFFFFFFF)
        out_append (rc (HRC_EXELIB_PE_IMPORT_LIST_FORWARDED));

      out_append (_T ("\n"));

      // if commandline switch /peimp2 was used
      // -> list the imported functions too
      if (CmdlineParam_GetBool (GT_FLAG_PEIMP2))
      {
        gtuint32 nLookupTableOffset;
        if (aImportTableEntry.nImportLookupTableRVA != rva_t (0))
          nLookupTableOffset = m_pSectionTable->r2o (aImportTableEntry.nImportLookupTableRVA);
        else
          nLookupTableOffset = m_pSectionTable->r2o (aImportTableEntry.nImportAddressTableRVA);

        out_incindent ();
        _ListImportedNames (nLookupTableOffset);
        out_decindent ();
      }
    }
  }
  catch (const Excpt &e)
  {
    e.Display ();
  }

  out_decindent ();
}

}  // namespace
