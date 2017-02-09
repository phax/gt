#ifndef _GT_PARAMHANDLER_HXX_
#define _GT_PARAMHANDLER_HXX_

namespace GT {

//! callback type
typedef void (GT_CALL *FileHandleProc) (LPCTSTR pFilename);

/* call for each non-switch parameter.
   It will resolve wildcards and use the driveinfo stuff.
 */
void GT_CALL __EvaluateCmdlineParameter
                                        (      LPCTSTR        pParam,
                                               FileHandleProc pCallback,
                                         const bool           bSubDirs);

}  // namespace

#endif
