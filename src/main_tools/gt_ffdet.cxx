#include "gt_cmdline_params.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"

namespace GT {

#define GT_FFDET_FLAG_CVSOUT   _T ("cvsout")
#define GT_FFDET_FLAG_BINONLY  _T ("binonly")
#define GT_FFDET_FLAG_TEXTONLY _T ("textonly")

tParamBool const* GT_CALL GetBoolParam   (const size_t nIndex)
{
  static const tParamBool _BOOL_FLAGS [] = {
    { GT_FFDET_FLAG_CVSOUT,   0, false },
    { GT_FFDET_FLAG_BINONLY,  0, false },
    { GT_FFDET_FLAG_TEXTONLY, 0, false },
  };

  return nIndex < GT_ARRAY_SIZE (_BOOL_FLAGS)
           ? &_BOOL_FLAGS[nIndex]
           : NULL;
}

tParamLong const*   GT_CALL GetLongParam   (const size_t nIndex) { return NULL; }
tParamString const* GT_CALL GetStringParam (const size_t nIndex) { return NULL; }
LPCTSTR GT_CALL GetProgramName () { return _T ("gt_ffdet"); }
LPCTSTR GT_CALL GetProgramVersion () { return _T ("0.02"); }
LPCTSTR GT_CALL GetConfigfileName () { return _T ("gt_ffdet.cfg"); }

void GT_CALL HandleCommandlineDependencies () {}
bool GT_CALL CanStartSearching () { return true; }

/*! This is the callback function called for each files passed
    on the commandline!
 */
//--------------------------------------------------------------------
void GT_CALL HandleFilename (LPCTSTR pFilename)
//--------------------------------------------------------------------
{
  FileBuffer aBuffer (pFilename, 0, NULL, NULL);

  if (aBuffer.Init () != OPEN_SUCCESS)
  {
    out_error_format (_T ("Failed to open '%s'\n"), pFilename);
  }
  else
  {
    // IsUnixText is only set of IsText is true
    const bool bText = aBuffer.IsText ();

    if ((CmdlineParam_GetBool (GT_FFDET_FLAG_BINONLY)  && bText) ||
        (CmdlineParam_GetBool (GT_FFDET_FLAG_TEXTONLY) && !bText))
    {
      // ignore file
    }
    else
    {
      // write CVS output?
      if (CmdlineParam_GetBool (GT_FFDET_FLAG_CVSOUT))
      {
        // CVS uses only slashes
        pstring sFile (pFilename);
        str_replace_all (sFile, _T ('\\'), _T ('/'));

        // binaries have the parameter "-kb"
        if (bText)
          out_format (_T ("cvs add \"%s\"\n"), sFile.c_str ());
        else
          out_format (_T ("cvs add -kb \"%s\"\n"), sFile.c_str ());
      }
      else
      {
        // default output
        out_format (_T ("[%s] %s\n"),
                    bText
                      ? aBuffer.IsUnixText ()
                        ? _T ("text [*ix]")
                        : aBuffer.IsMacText ()
                          ? _T ("text [Mac]")
                          : _T ("text [DOS]")
                      : _T ("binary"),
                    pFilename);
      }
    }
  }

  out_flush ();
}

void GT_CALL Run () {}
void GT_CALL HandleCleanup (const int nTotalNonFlagsParameters) {}


}  // Namespace
