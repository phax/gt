#include "gt_analyzer.hxx"

#include "gt_cmdline_params.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_utils.hxx"

namespace GT {

/*! Thats the start of each analyzing.
    The bases classes for
      EXEs
      Archives
      Generics
    have an overloaded _ShowResults method!
 */
//--------------------------------------------------------------------
void GT_CALL BasicAnalyzer::AnalyzerShow ()
//--------------------------------------------------------------------
{
  // little check to ensure consistency
  ASSERT (this->AnalyzerMatch ());

  // list the file? Not if /ni was used
  if (!CmdlineParam_GetBool (GT_FLAG_NOTIDENTIFIED) ||
      m_pBuffer->GetScanOffset () > 0)
  {
    // write filename
    Output_FilenameStruct aFNStruct;
    m_pBuffer->FillStruct (&aFNStruct);
    out_filestart (&aFNStruct);

    // call virtual method
    this->_ShowResults ();

    // check the heap after each analyzation
    heapchk;

    // done file
    out_filedone ();

    if (m_nOverlayOffset < 0)
    {
      // Data is missing in the file!
      out_error_format (rc (HRC_GTLIB_ANALYZER_BYTES_MISSING),
                        -m_nOverlayOffset);
    }
    else
    if (m_nOverlayOffset > 0 && m_nOverlayOffset < m_pBuffer->GetFileSize ())
    {
      // only handled, if overlay offset < filesize, because EXE lister are
      // setting the overlay offset even if there is no overlay (just by
      // calculating the desired EXE size)
      m_pBuffer->CallListProc (m_nOverlayOffset);
    }

    // end file
    out_fileend ();
  }
}

}  // namespace
