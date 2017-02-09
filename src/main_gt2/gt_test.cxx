#include "gt_app.hxx"
#include "gt_direntry.hxx"
#include "gt_output.hxx"

namespace GT {

tParamBool const*   GT_CALL GetBoolParam   (const size_t nIndex) { return NULL; }
tParamLong const*   GT_CALL GetLongParam   (const size_t nIndex) { return NULL; }
tParamString const* GT_CALL GetStringParam (const size_t nIndex) { return NULL; }
LPCTSTR GT_CALL GetProgramName () { return _T ("gt_test"); }
LPCTSTR GT_CALL GetProgramVersion () { return _T ("0.01"); }
LPCTSTR GT_CALL GetConfigfileName () { return _T ("gt_test.cfg"); }
void GT_CALL HandleCommandlineDependencies () {}
bool GT_CALL CanStartSearching () { return true; }

/*! This is the callback function called for each files passed
    on the commandline!
 */
//--------------------------------------------------------------------
void GT_CALL HandleFilename (LPCTSTR pFilename)
//--------------------------------------------------------------------
{
  pstring sDir, sBase, sExt;

  pstring sFilename (pFilename);
  DirEntry aDE (sFilename);

  aDE.GetDirectoryName (sDir, true);
  aDE.GetBaseName (sBase);
  aDE.GetFileNameExtension (sExt);
  out_format (_T ("'%s' '%s' '%s'\n"), sDir.c_str (), sBase.c_str (), sExt.c_str ());
}

void GT_CALL Run () {}
void GT_CALL HandleCleanup (const int nTotalNonFlagsParameters) {}

}  // Namespace
