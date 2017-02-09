#include "gt_driveinfo.hxx"

namespace GT {

const size_t DRIVEINFO_BUFLEN = 32 * 4 + 1;

//--------------------------------------------------------------------
DriveInfo& DriveInfo::Instance ()
//--------------------------------------------------------------------
{
  static DriveInfo aInstance;
  return aInstance;
}

//--------------------------------------------------------------------
void GT_CALL DriveInfo::Init (const gtuint32 eSearchType)
//--------------------------------------------------------------------
{
  // if no param was passed, do nothing ..
  if (eSearchType == 0)
    return;

  // returns a string like this: "c:\<null>d:\<null><null>"
  static TCHAR sDriveInfoBuffer[DRIVEINFO_BUFLEN];
  ::GetLogicalDriveStrings (DRIVEINFO_BUFLEN, sDriveInfoBuffer);

  LPTSTR pDriveInfo = sDriveInfoBuffer;
  UINT nCurrentDriveType;

  for (;;)
  {
    const size_t nDriveInfoLen = _tcslen (pDriveInfo);

    // okay, we found the last '\0'
    if (nDriveInfoLen == 0)
      break;

    // determine the type of the drive - via the "root" string
    nCurrentDriveType = ::GetDriveType (pDriveInfo);

    // if only fixed drives should be scanned, but the drive is not fixed, continue
    if (((eSearchType & GT_DRIVEINFO_ALL) && (nCurrentDriveType != DRIVE_UNKNOWN)) ||
        ((eSearchType & GT_DRIVEINFO_FIXED) && (nCurrentDriveType == DRIVE_FIXED)) ||
        ((eSearchType & GT_DRIVEINFO_NETWORK) && (nCurrentDriveType == DRIVE_REMOTE)))
    {
      // save the drive info
      SingleDriveInfo aInfo = { pDriveInfo, nCurrentDriveType };
      push_back (aInfo);
    }

    // go to the next root
    pDriveInfo = _tcsninc (pDriveInfo, nDriveInfoLen + 1);
  }
}

}  // namespace
