#ifndef _GT_DRIVEINFO_HXX_
#define _GT_DRIVEINFO_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include <list>

namespace GT {

const gtuint32 GT_DRIVEINFO_ALL     = BITS[0];
const gtuint32 GT_DRIVEINFO_FIXED   = BITS[1];
const gtuint32 GT_DRIVEINFO_NETWORK = BITS[2];

//--------------------------------------------------------------------
struct SingleDriveInfo
//--------------------------------------------------------------------
{
  LPCTSTR m_sRoot;       //!< drive root name
  UINT    m_nDriveType;  //!< Windows drive type
};

//--------------------------------------------------------------------
class DriveInfo : public std::list <SingleDriveInfo>
//--------------------------------------------------------------------
{
public:
  static DriveInfo& Instance ();

  //! init the list of drives - needs to be called manually
  void GT_CALL Init (const gtuint32 eSearchType);
};

}  // namespace

#endif
