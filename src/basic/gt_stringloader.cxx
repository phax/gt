#include "gt_stringloader.hxx"

const unsigned int BUF_SIZE = 2048;

static HINSTANCE g_hInst = NULL;

namespace GT {

/*! Get the pointer to the string with the ID nID.
    \param nID string resource ID
    \retval number of bytes copied to string
 */
//--------------------------------------------------------------------
GT_IMPL_BASIC (int) GetStringResBuf
                                        (resid_t nID,
                                         LPTSTR* pMsg)
//--------------------------------------------------------------------
{
  ASSERT (pMsg);
  static TCHAR g_sBuffer [BUF_SIZE + 1];
  *pMsg = g_sBuffer;
  return GetStringRes (nID, g_sBuffer, BUF_SIZE);
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (int) GetStringRes
                                        (resid_t nID,
                                         LPTSTR  sBuffer,
                                         size_t  nBufLen)
//--------------------------------------------------------------------
{
  // empty result buffer
  _tcscpy (sBuffer, _T (""));

  // special handling for ID 0
  if (nID == 0)
    return 0;

  // returns the number of loaded bytes
  int nCount = ::LoadString (g_hInst, (UINT) nID, sBuffer, int (nBufLen));
  if (nCount == 0)
  {
    // loadstring returned an empty string - okay?
    const DWORD nLastError = ::GetLastError ();
    if (nLastError)
      fprintf (stderr, "Error loading StringRes %d: Error %u\n", nID, nLastError);
    ASSERT (nLastError == 0);
  }
  return nCount;
}

}  // namespace

#ifdef _DLL

/*! Global DLLMain. Save hInstance!
    \retval true always
 */
//--------------------------------------------------------------------
extern "C" BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD, LPVOID)
//--------------------------------------------------------------------
{
//  fprintf (stderr, "Using DLL hInstance\n");
  g_hInst = hinstDLL;
  return true;
}

#else

//! initialization helper
//--------------------------------------------------------------------
class ModHlp
//--------------------------------------------------------------------
{
public:
  ModHlp ()
  {
    g_hInst = ::GetModuleHandle (NULL);
//    fprintf (stderr, "Using EXE hInstance\n");
  }
} g_aOneAndOnly;

#endif
