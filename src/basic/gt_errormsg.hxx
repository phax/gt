#ifndef _GT_ERRORMSG_HXX_
#define _GT_ERRORMSG_HXX_

namespace GT {

//! Enable error messages on the commandline (instead of the MsgBox)
GT_DECL_BASIC (void) EnableInlineErrorMessages (const bool bEnable);

//! generic show error message function
GT_DECL_BASIC (void) ShowErrorMessage (LPCTSTR sFormat, ...);

//! show a Win32 error message (sUserMessage can be NULL; else prepended to the msg)
GT_DECL_BASIC (void) ShowWin32Error (DWORD nMessageID, LPCTSTR sUserMessage);

}  // namespace

#endif
