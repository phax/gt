#ifndef _GT_FIXEDNAME_HXX_
#define _GT_FIXEDNAME_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include <vector>
#include "gtc_datatypes.hxx"

namespace GT {

typedef std::vector <const FixedExtensionEntry*> SpecialExtensionList;

//! check if the filename sFilename in the directory sDirectory is special
GT_EXPORT_GEN bool GT_CALL GetSpecialNameDescription
                                        (LPCTSTR  sDirectory,
                                         LPCTSTR  sFilename,
                                         pstring& sDescription);

//! check if the extension sExtension is special
GT_EXPORT_GEN bool GT_CALL GetSpecialExtensionDescription
                                        (LPCTSTR               sExtension,
                                         SpecialExtensionList& aMatchingEntries);

//! check if the extension sExtension is known in the registry
GT_EXPORT_GEN bool GT_CALL GetSpecialRegistryDescription
                                        (LPCTSTR  sExtension,
                                         pstring& sRegDescription,
                                         pstring& sRegClass,
                                         pstring& sProgramPath);

}

#endif
