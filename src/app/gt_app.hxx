#ifndef _GT_APP_HXX_
#define _GT_APP_HXX_

// the following methods need to be implemented by each application
// that uses this application framework:

namespace GT {

struct tParamBool;
struct tParamLong;
struct tParamString;

/* for commandline parameter registration:
   A bool flag that is just applied and changes something
   A long flag is a flag that directly needs an integer value
   A string flag is a flag that directly needs an string value
 */
tParamBool const*   GT_CALL GetBoolParam   (const size_t nIndex);
tParamLong const*   GT_CALL GetLongParam   (const size_t nIndex);
tParamString const* GT_CALL GetStringParam (const size_t nIndex);

/* Used in the programs headline. The name should only be
     the name and the version should only be the version
     (but inlcuding any suffix like beta etc.)
 */
LPCTSTR GT_CALL GetProgramName ();
LPCTSTR GT_CALL GetProgramVersion ();
LPCTSTR GT_CALL GetConfigfileName ();

/* Used after all commandline switches are applied.
   Use it to handle dependencies between commandline flags.
   Important: the output functions are by default on stdout!
 */
void GT_CALL HandleCommandlineDependencies ();

/*! This function is called after all initialization stuff
    was successfully done. Here you can stop the execution
    if e.g. a commandline parameter is missing.
    By returning true, file searching starts. When returning
    false, the execution is aborted!
 */
bool GT_CALL CanStartSearching ();

/*! This is the callback function called for each files passed
    on the commandline!
 */
void GT_CALL HandleFilename (LPCTSTR pFilename);

/*! This function is called after all parameters have been
      evaluated. No need to use it....
 */
void GT_CALL Run ();

/*! This function is called in the cleanup code.
 */
void GT_CALL HandleCleanup (const int nTotalNonFlagsParameters);

}  // Namespace

#endif
