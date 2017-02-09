#include "gt_exe_pe.hxx"

#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_utils.hxx"

namespace GT {

//--------------------------------------------------------------------
void GT_CALL EXE_PE_TLSAnalyzer::List ()
//--------------------------------------------------------------------
{
  out_append (_T ("\n"));

  if (m_nOffset < 0 || m_nOffset >= m_pBuffer->GetFileSize ())
  {
    out_error_format (rc (HRC_EXELIB_PE_TLS_INVALID),
                      m_nOffset);
    return;
  }

  // header
  out_format (rc (HRC_EXELIB_PE_TLS_AT_OFFSET),
              m_nOffset,
              RVA_VAL (m_pDataDir->nRVA),
              m_pDataDir->nSize);

  out_incindent ();

  // nothing more...
  if ((m_pDataDir->nSize / 24) == 1)
    out_append (rc (HRC_EXELIB_PE_1_TLS_ENTRY));
  else
    out_format (rc (HRC_EXELIB_PE_N_TLS_ENTRIES),
                m_pDataDir->nSize / 24);

  out_decindent ();
}

}  // namespace
