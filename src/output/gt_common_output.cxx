#include "gt_common_output.hxx"

#include "gt_errormsg.hxx"

namespace GT {

//--------------------------------------------------------------------
void GT_CALL _CheckForEndlessLoop
                                        (pstring* pStr)
//--------------------------------------------------------------------
{
  // Automatically try to determine possible endless loops
  //   by checking the maximum string length
  // Largest so far: mozilla-source-1.3.rar: ~4.53 MB
  // Largest so far: mozilla-source-1.4a.rar: ~4.59 MB
  // Largest so far: an avi: ~12.73 MB
  // -> round up
  const size_t _MAX_BUFFERED_LEN = 13 * 1024 * 1024;
  static size_t nCurMax = _MAX_BUFFERED_LEN;

  // did we show the error message for the current string??
  static bool bShowedMsg = false;

  // larger than max -> error!
  if (pStr->length () > nCurMax)
  {
    if (debug)
      _ftprintf (stderr, _T ("ERROR: New [MAXLEN] found: %lu\n"), pStr->length ());

    if (!bShowedMsg)
    {
      ShowErrorMessage (_T ("Seems like we hang in an endless loop :("));
      bShowedMsg = true;
    }

    // remember max
    nCurMax = pStr->length ();
  }
  else
  {
    // reset error flag
    bShowedMsg = false;
  }
}

}
