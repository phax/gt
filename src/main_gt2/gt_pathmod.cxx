/*! Install path
    May require only static libs
 */

#include "gt_cmdline_args.hxx"
#include <shlobj.h>

// contained in Platform SDK -> GCC doesn't know that
#ifndef SMTO_NOTIMEOUTIFNOTHUNG
#define SMTO_NOTIMEOUTIFNOTHUNG 0x0008
#endif

/*! Return the index of pPathTosearch into pPathVar.
    In case it is not contained, -1 is returned.
 */
//--------------------------------------------------------------------
size_t _stristr (LPCTSTR pPathVar, LPCTSTR pPathToSearch)
//--------------------------------------------------------------------
{
  // create UC copy of first
  DWORD n1 = (DWORD) _tcslen (pPathVar);
  LPTSTR p1 = new TCHAR [n1 + 1];
  _tcscpy (p1, pPathVar);
  CharUpperBuff (p1, n1);

  // create UC copy of second
  DWORD n2 = (DWORD) _tcslen (pPathToSearch);
  LPTSTR p2 = new TCHAR [n2 + 1];
  _tcscpy (p2, pPathToSearch);
  CharUpperBuff (p2, n2);

  // do main search in uppercased strings
  LPCTSTR pPos = _tcsstr (p1, p2);
  size_t nPos = pPos ? pPos - p1 : -1;

  // cleanup
  delete [] p1;
  delete [] p2;

  // return
  return nPos;
}

//--------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int nCmdShow)
//--------------------------------------------------------------------
{
  int argc = 0;
  TCHAR **argv = NULL;
  GT::GetCommandLineArgs (argc, argv);

  bool bAdd = false;
  bool bRemove = false;
  bool bQuiet = false;
  LPTSTR pPathToHandle = NULL;
  bool bChanged = false;
  bool bForHKCL = false;

  if (GetVersion () >= 0x80000000)
  {
    MessageBox (NULL, _T ("This tool works only on Windows NT platforms"), _T ("Information"), MB_OK | MB_ICONINFORMATION);
    return 1;
  }

  for (int i = 1; i < argc; ++i)
  {
    LPTSTR pArg = argv[i];
    if (*pArg == _T ('-'))
    {
      ++pArg;
      if (*pArg == _T ('a'))
        bAdd = true;
      else
      if (*pArg == _T ('r'))
        bRemove = true;
      else
      if (*pArg == _T ('q'))
        bQuiet = true;
      else
      if (*pArg == _T ('p'))
      {
        pPathToHandle = new TCHAR [_tcslen (pArg + 1) + 1];
        _tcscpy (pPathToHandle, pArg + 1);
      }
      else
      if (*pArg == _T ('l'))
      {
        // using local maschine
        bForHKCL = true;
      }
      else
      if (*pArg == _T ('?'))
      {
        MessageBox (NULL, _T ("Parameters:\n")
                          _T ("\t-a\tadd to PATH\n")
                          _T ("\t-r\tremove from PATH\n")
                          _T ("\t-q\tquiet mode\n")
                          _T ("\t-pDIR\tthe path to add\n")
                          _T ("\t-l\tapply to local machine\n"),
                          _T ("Usage"), MB_OK);
        return 1;
      }
    }
  }

#ifdef _DEBUG
  // if no directory is set, use an automatic path
  if (!pPathToHandle)
  {
    TCHAR szPath[MAX_PATH];

    // SHGetFolderPath does not work on Std Win98 :(
    if (SUCCEEDED (SHGetFolderPath (NULL,
                                    CSIDL_PROGRAM_FILES,
                                    NULL,
                                    0,
                                    szPath)))
    {
      LPCTSTR pConstAppend = _T ("\\gt2");
      pPathToHandle = new TCHAR [_tcslen (szPath) + _tcslen (pConstAppend) + 1];
      _tcscpy (pPathToHandle, szPath);
      _tcscat (pPathToHandle, pConstAppend);
    }
    else
    {
      // error - why ever
      MessageBox (NULL, _T ("Error: the program files directory could not be retrieved."), _T ("Error"), MB_OK | MB_ICONERROR);
      return 1;
    }
  }
#endif

  if (!pPathToHandle)
  {
    MessageBox (NULL, _T ("Error: no path was specified. Please check the commandline."), _T ("Error"), MB_OK | MB_ICONERROR);
    return 1;
  }

  // add more than one path at once is not allowed
  if (_tcschr (pPathToHandle, _T (';')) != NULL)
  {
    MessageBox (NULL, _T ("Error: Don't try to add pathes that contain a semicolon ';'.\n")
                      _T ("Call separately for each path."), _T ("Error"), MB_OK | MB_ICONERROR);
    delete [] pPathToHandle;
    return 1;
  }

  LONG n;

  // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment
  // HKEY_CURRENT_USER\Environment
  HKEY hKey;
  n = RegOpenKeyEx (bForHKCL
                      ? HKEY_LOCAL_MACHINE
                      : HKEY_CURRENT_USER,
                    bForHKCL
                      ? _T ("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment")
                      : _T ("Environment"),
                    0,
                    KEY_QUERY_VALUE | (bAdd || bRemove ? KEY_SET_VALUE : 0),
                    &hKey);
  _ASSERTE (n == ERROR_SUCCESS);
  if (n == ERROR_SUCCESS)
  {
    LPTSTR pPath = NULL;
    size_t nFoundPos = size_t (-1);

    // query value length
    // Fails if the value is not present
    DWORD nType, nBufSize;
    n = RegQueryValueEx (hKey, _T ("Path"), NULL, &nType, NULL, &nBufSize);
    if (n == ERROR_SUCCESS)
    {
      _ASSERTE (nType == REG_SZ || nType == REG_EXPAND_SZ);
      _ASSERTE (nBufSize > 0);

      // query value
      pPath = new TCHAR [(nBufSize / sizeof (TCHAR)) + 1];
      n = RegQueryValueEx (hKey, _T ("Path"), NULL, &nType, (LPBYTE) pPath, &nBufSize);
      _ASSERTE (n == ERROR_SUCCESS);
      // If the data has the REG_SZ, REG_MULTI_SZ or REG_EXPAND_SZ type, the string
      //   may not have been stored with the proper null-terminating characters.
      // Applications should ensure that the string is properly terminated before
      //   using it, otherwise, the application may fail by overwriting a buffer.
      _ASSERTE ((nBufSize / sizeof (TCHAR)) == (_tcslen (pPath) + 1));
      pPath[nBufSize / sizeof (TCHAR)] = _T ('\0');

      // find position
      nFoundPos = _stristr (pPath, pPathToHandle);
    }
    else
    {
      // workaround
      pPath = new TCHAR[1];
      *pPath = _T ('\0');
    }

    if (bAdd)
    {
      // add path
      if (nFoundPos != size_t (-1))
      {
        if (!bQuiet)
          MessageBox (NULL, _T ("GT2 is already in the path"), _T ("Warning"), MB_OK | MB_ICONWARNING);
      }
      else
      {
        // append to path
        LPTSTR pNewPath = new TCHAR [_tcslen (pPath) + 1 + _tcslen (pPathToHandle) + 1];
        _tcscpy (pNewPath, pPath);
        if (*pNewPath)
          _tcscat (pNewPath, _T (";"));
        _tcscat (pNewPath, pPathToHandle);

        // set new value
        n = RegSetValueEx (hKey, _T ("Path"), 0, REG_EXPAND_SZ, (LPBYTE) pNewPath, DWORD (_tcslen (pNewPath) + 1) * sizeof (TCHAR));
        _ASSERTE (n == ERROR_SUCCESS);
        if (n == ERROR_SUCCESS)
        {
          bChanged = true;
          if (!bQuiet)
            MessageBox (NULL, _T ("GT2 was successfully added to the path"), _T ("Information"), MB_OK | MB_ICONINFORMATION);
        }
        else
          MessageBox (NULL, _T ("Failed to add GT2 to the path"), _T ("Error"), MB_OK | MB_ICONERROR);

        // cleanup
        delete [] pNewPath;
      }
    }
    else
    if (bRemove)
    {
      // remove path
      if (nFoundPos != size_t (-1))
      {
        // remove from path
        LPTSTR pNewPath = new TCHAR [_tcslen (pPath) + 1];
        _tcscpy (pNewPath, pPath);

        // skip the previous ';' too?
        size_t nLen = _tcslen (pPathToHandle);
        if (nFoundPos > 0 && pNewPath[nFoundPos - 1] == _T (';'))
        {
          // was append -> remove ';'
          --nFoundPos;
          ++nLen;
        }
        else
        {
          // first element! an ';' at the back?
          if (pNewPath [nFoundPos + nLen] == _T (';'))
            ++nLen;
        }
        // move everything from right to left
        memmove (pNewPath + nFoundPos, pNewPath + nFoundPos + nLen, (_tcslen (pNewPath + nFoundPos + nLen) + 1) * sizeof (TCHAR));

        // set new value
        n = RegSetValueEx (hKey, _T ("Path"), 0, REG_EXPAND_SZ, (LPBYTE) pNewPath, DWORD (_tcslen (pNewPath) + 1) * sizeof (TCHAR));
        _ASSERTE (n == ERROR_SUCCESS);
        if (n == ERROR_SUCCESS)
        {
          bChanged = true;
          if (!bQuiet)
            MessageBox (NULL, _T ("GT2 was successfully removed from the path"), _T ("Information"), MB_OK | MB_ICONINFORMATION);
        }
        else
          MessageBox (NULL, _T ("Failed to remove GT2 from the path"), _T ("Error"), MB_OK | MB_ICONERROR);

        // cleanup
        delete [] pNewPath;
      }
      else
      {
        if (!bQuiet)
          MessageBox (NULL, _T ("GT2 is not in the path"), _T ("Warning"), MB_OK | MB_ICONWARNING);
      }
    }
    else
    {
      // info only
      MessageBox (NULL,
                  nFoundPos != size_t (-1)
                    ? _T ("GT2 is already in the path")
                    : _T ("GT2 is not in the path"),
                  _T ("Information"),
                  MB_OK | MB_ICONINFORMATION);
    }

    delete [] pPath;

    // close again
    RegCloseKey (hKey);
  }
  else
  {
    MessageBox (NULL,
                _T ("Your environment settings could not be found."),
                _T ("Error"),
                MB_OK | MB_ICONERROR);
  }

  delete [] pPathToHandle;

  if (bChanged)
  {
//    BroadcastSystemMessage (BSF_POSTMESSAGE | BSF_IGNORECURRENTTASK,NULL,WM_SETTINGCHANGE,0,(int)_T ("Environment"));
    DWORD_PTR dwReturnValue;
    LRESULT nRet = SendMessageTimeout (HWND_BROADCAST,
                                       WM_SETTINGCHANGE,
                                       0,
                                       (LPARAM) _T ("Environment"),
                                       SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG,
                                       5000,
                                       &dwReturnValue);
    _ASSERTE (nRet != 0);
  }

  return 0;
}
