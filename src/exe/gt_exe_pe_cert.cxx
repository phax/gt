#include "gt_exe_pe.hxx"

#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_utils.hxx"

namespace GT {

//--------------------------------------------------------------------
void GT_CALL EXE_PE_CertificateAnalyzer::List ()
//--------------------------------------------------------------------
{
  // header
  out_append (_T ("\n"));

  if (m_nOffset < 0 || m_nOffset >= m_pBuffer->GetFileSize ())
  {
    out_error_format (rc (HRC_EXELIB_PE_CERT_INVALID),
                      m_nOffset);
    return;
  }

  // header
  out_format (rc (HRC_EXELIB_PE_CERT_AT_OFFSET),
              m_nOffset, RVA_VAL (m_pDataDir->nRVA));

  out_incindent ();

  // #f# todo

  out_decindent ();
}

}  // namespace
