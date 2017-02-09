#include "gt_configfile.hxx"

#include "gt_errormsg.hxx"
#include "gt_linereader.hxx"
#include "gt_registry.hxx"
#include "gt_cmdline_params.hxx"
#include "gt_utils.hxx"       // _max

namespace GT {

//--------------------------------------------------------------------
static void _GetPathOfModule
                                        (      LPCTSTR pModule,
                                               LPTSTR  pPath,
                                         const int     nModuleLen)
//--------------------------------------------------------------------
{
  ASSERT (pModule);
  ASSERT (pPath);

  // copy the module to the path
  _tcsncpy (pPath, pModule, nModuleLen + 1);

  // go to the end of the path
  LPTSTR pLastSlash  = _tcsrchr (pPath, '/');
  LPTSTR pLastBSlash = _tcsrchr (pPath, '\\');
  LPTSTR p = NULL;

  if (pLastSlash && pLastBSlash)
  {
    // get the next char of the pointer which far right...
    p = _tcsinc (_max (pLastSlash, pLastBSlash));
  }
  else
  {
    if (pLastSlash)
      p = _tcsinc (pLastSlash);
    else
    if (pLastBSlash)
      p = _tcsinc (pLastBSlash);
  }

  if (p)
  {
    // p is somewhere in pPath -> cut at the desired position
    _tcscpy (p, _T (""));
  }
  else
  {
    // no '/' or '\' was found -> return empty string
    _tcscpy (pPath, _T (""));
  }
}

//--------------------------------------------------------------------
static void _GetProfilePath (TCHAR *pPath)
//--------------------------------------------------------------------
{
  pstring sRegPath, sRegKey;
  str_assign_from_res (sRegPath, HRC_GTLIB_CONFIGFILE_REGPATH);
  str_assign_from_res (sRegKey,  HRC_GTLIB_CONFIGFILE_REGKEY);

  // if an error occurs it is displayed inside!
  if (!Registry::GetCurrentUserString (sRegPath.c_str (),
                                       sRegKey.c_str (),
                                       512,
                                       pPath))
  {
    return;
  }

  LPCTSTR pLastSlash  = _tcsrchr (pPath, _T ('/'));
  LPCTSTR pLastBSlash = _tcsrchr (pPath, _T ('\\'));
  if ((!pLastSlash  || *_tcsinc (pLastSlash) != 0) &&
      (!pLastBSlash || *_tcsinc (pLastBSlash) != 0))
  {
    _tcscat (pPath, _T ("\\"));
  }
}

//--------------------------------------------------------------------
static void _GetEXEPath (LPTSTR pPath)
//--------------------------------------------------------------------
{
  // get the fully qualified path to the EXE
  TCHAR sModule[512];
  const int nLen = ::GetModuleFileName (NULL, sModule, 512);
  if (nLen == 0)  // <- error!
    ShowWin32Error (::GetLastError (), _T ("[_GetEXEPath]"));

  // get the path including the last '/' or '\'
  _GetPathOfModule (sModule, pPath, nLen);
}

//--------------------------------------------------------------------
ConfigFile::ConfigFile
                                        (      LPCTSTR sConfigFilename,
                                         const bool    bReadLocal,
                                         const bool    bReadGlobal)
//--------------------------------------------------------------------
  : m_pFile (NULL)
{
  // try to open the config file in the profile folder
  if (bReadLocal)
  {
    _GetProfilePath (m_sConfigFile);
    _tcscat (m_sConfigFile, sConfigFilename);

    m_pFile = new LineReader (m_sConfigFile, 2048);
    if (!m_pFile->IsValid ())
    {
      delete m_pFile;
      m_pFile = NULL;
    }
  }

  // try to open the file in the EXE folder
  if (!m_pFile && bReadGlobal)
  {
    _GetEXEPath (m_sConfigFile);
    _tcscat (m_sConfigFile, sConfigFilename);

    m_pFile = new LineReader (m_sConfigFile, 2048);
    if (!m_pFile->IsValid ())
    {
      delete m_pFile;
      m_pFile = NULL;
    }
  }
}

//--------------------------------------------------------------------
ConfigFile::~ConfigFile ()
//--------------------------------------------------------------------
{
  if (m_pFile)
  {
    delete m_pFile;
    m_pFile = NULL;
  }
}

//--------------------------------------------------------------------
void ConfigFile::_ShowConfigFileError
                                        (const resid_t nErrorID,
                                         const size_t  nLineNumber,
                                               void*   pAdditional /* = NULL */)
//--------------------------------------------------------------------
{
  pstring sText;

  // first error?
  if (m_nErrorCount++ == 0)
  {
    str_assign_from_res (sText, HRC_GTLIB_CONFIGFILE_ERROR);
    ShowErrorMessage (sText.c_str (), m_sConfigFile);
  }

  str_assign_from_res (sText, nErrorID);
  if (pAdditional)
    ShowErrorMessage (sText.c_str (), nLineNumber, pAdditional);
  else
    ShowErrorMessage (sText.c_str (), nLineNumber);
}

//--------------------------------------------------------------------
bool ConfigFile::Read ()
//--------------------------------------------------------------------
{
  m_nErrorCount = 0;

  // no config file was found -> everything is okay ;-)
  if (!m_pFile)
    return true;

  pstring sLine;

  size_t nLineNumber = 0;
  while (m_pFile->GetLine (sLine))
  {
    nLineNumber++;
    LPTSTR p = const_cast <LPTSTR> (sLine.c_str ());

    // search for any comments and destroy 'em ;-)
    LPTSTR pComment = _tcschr (p, _T (';'));
    if (pComment)
      *pComment = _T ('\0');

    // skip all leading spaces
    while (*p && _istspace (*p))
      p = _tcsinc (p);

    // was there end of string?
    if (!*p)
      continue;

    // or a flags?
    if (*p != _T ('/') && *p != _T ('-'))
    {
      _ShowConfigFileError (HRC_GTLIB_CONFIGFILE_SYNTAX_ERROR, nLineNumber);
      return false;
    }

    p = _tcsinc (p);  // skip the '/' or the '-'
    TCHAR *pStart = p;  // save the start position of the pointer

    while (_istalnum (*p))
    {
      p = _tcsinc (p);
    }

    LPTSTR pEnd = p;

    // check if any char until the end of the line is a whitespace!
    bool bIsValid = true;
    while (*p)
    {
      if (!_istspace (*p))
      {
        bIsValid = false;
        break;
      }
      p = _tcsinc (p);
    }

    if (bIsValid)
    {
      *pEnd = _T ('\0');
      if (!CmdlineParam_Apply (pStart))
      {
        _ShowConfigFileError (HRC_GTLIB_CONFIGFILE_UNKNOWN_SWITCH, nLineNumber, pStart);
      }
    }
    else
    {
      _ShowConfigFileError (HRC_GTLIB_CONFIGFILE_INVALID_CHAR, nLineNumber);
      return false;
    }
  }

  return true;
}

}  // namespace
