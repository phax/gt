#include "gt_registry.hxx"

namespace GT {

namespace Internal {

//--------------------------------------------------------------------
static bool _GetRegistryString (HKEY    hKey,
                                LPCTSTR sRegistryPath,
                                LPCTSTR sRegistryValue,
                                LONG    nBufferLen,
                                LPTSTR  sBuffer)
//--------------------------------------------------------------------
{
  ASSERT (sRegistryPath);
  ASSERT (sRegistryValue);
  ASSERT (nBufferLen > 0);
  ASSERT (sBuffer);

  *sBuffer = 0;

  HKEY hMyKey;
  LONG nResult;

  nResult = ::RegOpenKeyEx (hKey,
                            sRegistryPath,
                            0,
                            KEY_QUERY_VALUE,
                            &hMyKey);
  if (nResult != ERROR_SUCCESS)
    return false;

  DWORD dwBufferLen = nBufferLen;
  nResult = ::RegQueryValueEx (hMyKey,
                               sRegistryValue,
                               NULL,
                               NULL,
                               (LPBYTE) sBuffer,
                               &dwBufferLen);

  // don't forget to close!
  ::RegCloseKey (hMyKey);

  // check for error after closing!
  if (nResult != ERROR_SUCCESS)
    return false;

  return true;
}

}  // namespace Internal

//--------------------------------------------------------------------
bool Registry::GetCurrentUserString
                                        (LPCTSTR sRegistryPath,
                                         LPCTSTR sRegistryValue,
                                         LONG    nBufferLen,
                                         LPTSTR  sBuffer)
//--------------------------------------------------------------------
{
  ASSERT (sRegistryPath);
  ASSERT (sRegistryValue);
  ASSERT (nBufferLen > 0);
  ASSERT (sBuffer);

  return Internal::_GetRegistryString (HKEY_CURRENT_USER,
                                       sRegistryPath,
                                       sRegistryValue,
                                       nBufferLen,
                                       sBuffer);
}

//--------------------------------------------------------------------
bool Registry::GetClassesRootString
                                        (LPCTSTR sRegistryPath,
                                         LPCTSTR sRegistryValue,
                                         LONG    nBufferLen,
                                         LPTSTR  sBuffer)
//--------------------------------------------------------------------
{
  ASSERT (sRegistryPath);
  ASSERT (sRegistryValue);
  ASSERT (nBufferLen > 0);
  ASSERT (sBuffer);

  return Internal::_GetRegistryString (HKEY_CLASSES_ROOT,
                                       sRegistryPath,
                                       sRegistryValue,
                                       nBufferLen,
                                       sBuffer);
}

}  // namespace  GT
