#ifndef _GT_REGISTRY_HXX_
#define _GT_REGISTRY_HXX_

namespace GT {

//--------------------------------------------------------------------
class GT_EXPORT_BASIC Registry
//--------------------------------------------------------------------
{
public:
  static bool GetCurrentUserString (LPCTSTR sRegistryPath,
                                    LPCTSTR sRegistryValue,
                                    LONG    nBufferLen,
                                    LPTSTR  sBuffer);

  static bool GetClassesRootString (LPCTSTR sRegistryPath,
                                    LPCTSTR sRegistryValue,
                                    LONG    nBufferLen,
                                    LPTSTR  sBuffer);
};

} // namespace

#endif
