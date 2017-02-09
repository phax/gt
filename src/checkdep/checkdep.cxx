/*
 * @file:   checkdep.cxx
 * @author: PH
 */

#include "checkdep.h"
#include <list>
#include <map>
using namespace std;

// global flags - handled on the commandline
bool g_bWriteMakefile = false;
bool g_bWriteMakefileTargets = true;
bool g_bScanSystemIncludes = false;
bool g_bBeVerbose = false;
bool g_bNoLogo = false;
bool g_bInsertEnv = false;
bool g_bCountingOutput = false;
bool g_bCheckIfPathExists = false;
bool g_bUseCmdHeadline = true;
bool g_bIgnoreSelfFile = false;

#ifdef _WIN32
HANDLE g_hFoundFile = INVALID_HANDLE_VALUE;
#else
FILE* g_f = NULL;
#endif

DWORD g_read_bytes;
char  g_read_buffer[MAX_CHAR];
char *g_read_pos = g_read_buffer + MAX_CHAR;
char *g_read_end = g_read_buffer + MAX_CHAR - 1;

char* g_pUserDefMakeFilePrefix = NULL;
char* g_pUserDefMakeFileSuffix = NULL;

char   g_pSearchPath[PATH_BUFFER] = "";
char   g_sActPath_s[MAX_PATH];
DWORD  g_sActPath_len;

DWORD g_nCommandlineFileSpecCounter = 0;

// some files that are never present for us
const char* IGNORE_INCLUDE_FILES[] = {
  "macsocket.h",
  "protypes.h",
};

// some files are known for having weird includes...
const char* FILES_WITH_WEIRD_INCLUDES [] = {
};

//--------------------------------------------------------------------
void __cdecl __ErrorMsg (const char* pFmt, ...)
//--------------------------------------------------------------------
{
  char sMsg[MAX_CHAR];
  va_list args;
  va_start (args, pFmt);
  vsprintf (sMsg, pFmt, args);
  va_end (args);

  if (g_bWriteMakefile)
  {
    fprintf (stderr, "  %s\n", sMsg);
    fprintf (stdout, "# %s\n", sMsg);
  }
  else
    fprintf (stderr, "%s\n", sMsg);
}

/*! This is an assertion function that is called if a severe
      error occurs.
 */
//--------------------------------------------------------------------
static void __CheckdepAssert (bool b,
                              const char* s,
                              const char* pSource,
                              const size_t nLine)
//--------------------------------------------------------------------
{
  if (!b)
  {
    __ErrorMsg ("Checkdep assertion failed:\n  File %s (Line %u)\n  Expr: %s", pSource, nLine, s);
    exit (1);
  }
}

#ifndef _WIN32

// global var needed :(
static const char* g_pFindMask = NULL;

/* Special callback function that needs to return true
     if we want the file and false if not.
 */
//--------------------------------------------------------------------
static int _LinuxSelectFile (const struct dirent* entry)
//--------------------------------------------------------------------
{
  // fnmatch (pattern, name, flags)
  // -> returns 0 upon success
  return (fnmatch (g_pFindMask, entry->d_name, FNM_PATHNAME) == 0);
}
#endif

/*! Single list entry
 */
//--------------------------------------------------------------------
class FileName
//--------------------------------------------------------------------
{
private:
  FileName ();
  FileName (const FileName&);
  FileName& operator = (const FileName&);

public:
  DWORD nNameLen;
  char  sName[MAX_PATH];

  explicit FileName (const char* s, DWORD len = -1)
    : nNameLen (len == -1 ? strlen (s) : len)
  {
    strncpy (sName, s, nNameLen);
    sName [nNameLen] = 0;
  }
};

namespace std {
/*! special overloading for FileName*
 */
//--------------------------------------------------------------------
template <> struct less<FileName*>
//--------------------------------------------------------------------
{
  //------------------------------------------------------------------
  bool operator () (FileName* p1, FileName* p2) const
  //------------------------------------------------------------------
  {
    return (p1->nNameLen < p2->nNameLen) ||
           (p1->nNameLen == p2->nNameLen && strncmp (p1->sName, p2->sName, p1->nNameLen) < 0);
  }
};

}  // namespace

//--------------------------------------------------------------------
class FileList : public list <FileName*>
//--------------------------------------------------------------------
{
public:
  //------------------------------------------------------------------
  virtual ~FileList ()
  //------------------------------------------------------------------
  {
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
      delete *cit;
  }

  //------------------------------------------------------------------
  const_iterator cfind (const char* s, DWORD len = -1)
  //------------------------------------------------------------------
  {
    if (len == -1)
      len = strlen (s);
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
      if (len == (*cit)->nNameLen && _strnicmp ((*cit)->sName, s, (*cit)->nNameLen) == 0)
        return cit;
    return end ();
  }

  //------------------------------------------------------------------
  void cadd (const char* s, DWORD len)
  //------------------------------------------------------------------
  {
    push_back (new FileName (s, len));
  }

  //------------------------------------------------------------------
  bool contains (const char* s, DWORD len)
  //------------------------------------------------------------------
  {
    return !(cfind (s, len) == end ());
  }
};


// map filename to dir
//--------------------------------------------------------------------
class DirCache : public map <FileName*, FileName*>
//--------------------------------------------------------------------
{
public:
  //------------------------------------------------------------------
  virtual ~DirCache ()
  //------------------------------------------------------------------
  {
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
    {
      delete (*cit).first;
      delete (*cit).second;
    }
  }

  //------------------------------------------------------------------
  void cadd (const char* sFilename, const char* sPath, DWORD nPathLen)
  //------------------------------------------------------------------
  {
    insert (make_pair (new FileName (sFilename), new FileName (sPath, nPathLen)));
  }

  //------------------------------------------------------------------
  const_iterator cfind (const char* sFilename)
  //------------------------------------------------------------------
  {
    FileName e (sFilename);
    return find (&e);
  }
};

/*! This functions concatenates the given 3 strings and returns one
    large buffer allocated with malloc.
 */
//--------------------------------------------------------------------
char* _Str_Concat3 (const char* sPrefix,
                    const char* sFilename,
                    const char* sSuffix)
//--------------------------------------------------------------------
{
  char* p;

  // alloc length of all 3 strings and fill buffer
  p = (char*) malloc (strlen (sPrefix) +
                       strlen (sFilename) +
                       strlen (sSuffix) +
                       1);
  strcpy (p, sPrefix);
  strcat (p, sFilename);
  strcat (p, sSuffix);
  return p;
}

/*! Use this function to unify a path were necessary.
    -> always lowercase the pathes on windows because
       the Windows is not case sensitive
 */
//--------------------------------------------------------------------
static char* _OS_UnifyPath (char* p)
//--------------------------------------------------------------------
{
#ifdef _WIN32
  // no case sensitive file system on Windows!
  return _strlwr (p);
#else
  // no changes on *x because the file system is case sensitive!
  return p;
#endif
}

/*! Ensure that the string passed in p has the correct
      path separator -> on Windows it should be '\'
      whereas on *x it should be '/'
 */
//--------------------------------------------------------------------
static void _OS_EnsureCorrectPathSeparators (char* p)
//--------------------------------------------------------------------
{
  // ensure correct path separators are given
  while (*p)
  {
    if (*p == '\\')
      *p = PATH_SEP_CHAR;
    ++p;
  }
}

/*! This function checks if a path really exists.
 */
//--------------------------------------------------------------------
static bool _OS_PathExists (char* s)
//--------------------------------------------------------------------
{
  int nResult;

  // check path seps!
  _OS_EnsureCorrectPathSeparators (s);

#ifdef _WIN32
  // On Windows, check file attributes
  nResult = GetFileAttributes (s);
  if (nResult == -1)
    return false;

  return (nResult & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
#else
  // on Linux, check file access
  nResult = access (s, R_OK);
  return (nResult == 0) ? true : false;
#endif
}

/*! Try to open the given file.
    \retval true Files was opened. Handle in global var.
 */
//--------------------------------------------------------------------
bool _OS_FileExists (char* s)
//--------------------------------------------------------------------
{
  // check path seps!
  _OS_EnsureCorrectPathSeparators (s);

#ifdef _WIN32
  // open file for reading - share for read
  g_hFoundFile = CreateFile (s,
                             GENERIC_READ,
                             0, // FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                             NULL);
  if (g_hFoundFile != INVALID_HANDLE_VALUE)
    return true;

  // no access :(
  if (GetLastError () == ERROR_ACCESS_DENIED)
    __ErrorMsg ("ERROR: access denied to '%s'", s);

#else
  g_f = fopen (s, "rt");
  if (g_f != NULL)
    return true;
#endif

  return false;
}

/*! Check if the given path is contained in the search path
    The parameter nLen is just to avoid unnecessary strlen calls
 */
//--------------------------------------------------------------------
static bool _SP_IsContained (char* sPath, DWORD nLen)
//--------------------------------------------------------------------
{
  char* p;
  char c;

  if (!sPath || !*sPath || !nLen)
    return false;

  // needs to be done manually!
  _OS_EnsureCorrectPathSeparators (sPath);

  // search in string!
  p = strstr (g_pSearchPath, sPath);

  // not found -> return
  if (!p)
    return false;

  // found on the beginning?
  //
  // _SP_IsContained ("abc", 3)
  //
  // abc;def;ghi
  // |
  //
  if (p == g_pSearchPath)
  {
    c = *_strninc (p, nLen);
    return (c == '\0' || c == ENV_SEP_CHAR);
  }

  // found at the end?
  //
  // _SP_IsContained ("ghi", 3)
  //
  // abc;def;ghi
  //         |
  //

  if ((p - g_pSearchPath) + nLen == strlen (g_pSearchPath))
  {
    // pred of p must be ENV_SEP_CHAR
    return (*_strdec (g_pSearchPath, p) == ENV_SEP_CHAR);
  }

  // found somewhere
  //
  // _SP_IsContained ("def", 3)
  //
  // abc;def;ghi
  //     |
  //
  return (*_strdec (g_pSearchPath, p) == ENV_SEP_CHAR &&
          *_strninc (p, nLen) == ENV_SEP_CHAR);
}

/*! Use this function to directly append pPath to the search path.
    The search path is check for overflows.
 */
//--------------------------------------------------------------------
static void _SP_Append (const char* pPath)
//--------------------------------------------------------------------
{
  if (*g_pSearchPath)
  {
    CHECKDEP_ASSERT (strlen (g_pSearchPath) + strlen (ENV_SEP_STR) < PATH_BUFFER);
    strcat (g_pSearchPath, ENV_SEP_STR);
  }

  // don't forget to unify!
  CHECKDEP_ASSERT (strlen (g_pSearchPath) + strlen (pPath) < PATH_BUFFER);
  strcat (g_pSearchPath, pPath);
}

/*! Add the path specified by pPathToAdd to the global list of all
      search pathes.
    The path is automatically converted to the correct
      path separators.
 */
//--------------------------------------------------------------------
static bool _SP_AddPath (char* pPathToAdd)
//--------------------------------------------------------------------
{
  // get length
  const size_t len = strlen (pPathToAdd);

  // downcase on Windows
  pPathToAdd = _OS_UnifyPath (pPathToAdd);

  // should never be contained!
  if (strchr (pPathToAdd, ENV_SEP_CHAR) != NULL)
  {
    __ErrorMsg ("Checkdep: The path '%s' contains invalid characters", pPathToAdd);
    return false;
  }

  // is the path already contained?
  if (_SP_IsContained (pPathToAdd, len))
    return false;

  // path itself is invalid!
  if (!_OS_PathExists (pPathToAdd))
    return false;

  // alright, add it....
  _SP_Append (pPathToAdd);
  return true;
}

/*! Add the include pathes contained in the environment variable
      INCLUDE to our search pathes.
    Note: they are added after the ones manually specified by the user!
 */
//--------------------------------------------------------------------
static void _SP_AddEnvPathes ()
//--------------------------------------------------------------------
{
#define MAKE_COPY_ON_STACK(src,dst) char* __##src=src,*dst;\
                                    size_t n=strlen(__##src)+1;\
                                    dst=(char*)_alloca(n);\
                                    strcpy(dst,__##src)

  // get system environment variable "INCLUDE"
  char* pEnv = getenv ("INCLUDE");
  if (pEnv && *pEnv)
  {
    // we need a copy because _OS_UnifyPath my alter it!
    MAKE_COPY_ON_STACK(pEnv, pCopy);

    // unify path separators
    // append as a whole unified string and do not check its contents!!!!
    _SP_Append (_OS_UnifyPath (pCopy));
  }
}

/*! Remove the last path from the search path list!
 */
//--------------------------------------------------------------------
static void _SP_RemoveLast ()
//--------------------------------------------------------------------
{
  // find last path separator
  char* pEnd = strrchr (g_pSearchPath, ENV_SEP_CHAR);

  if (!pEnd)
  {
    __ErrorMsg ("Checkdep: Could not remove last path because no path is present");
  }
  else
  {
    // replace ENV_SEP_CHAR with '\0'
    *pEnd = '\0';
  }
}

/*! Check if the given file exists in any of the search pathes
      specified.
 */
//--------------------------------------------------------------------
static bool _SP_ContainsFile (const char* sFilename)
//--------------------------------------------------------------------
{
  /* Using a global filename/directory cache that should
       speed up file searching, since not so many calls to
       ::CreateFile are required.
   */
  static DirCache aDirCache;
  DirCache::const_iterator cit = aDirCache.cfind (sFilename);
  if (!(cit == aDirCache.end ()))
  {
    // get length of filename
    g_sActPath_len = (*cit).second->nNameLen;
    strncpy (g_sActPath_s, (*cit).second->sName, g_sActPath_len);
    g_sActPath_s[g_sActPath_len] = '\0';

    bool bOkay = _OS_FileExists (g_sActPath_s);
    CHECKDEP_ASSERT (bOkay);
    return bOkay;
  }

  /* The Windows API SearchPath is too slow!
   */
  char* p, *dst, *pStart = g_pSearchPath;

  for (;;)
  {
    // this is our current destination ptr
    dst = g_sActPath_s;

    // find end of current search string (ENV_SEP_CHAR) or end of string
    p = pStart;
    while (*p && *p != ENV_SEP_CHAR)
    {
      *dst = *p;
      dst = _strinc (dst);
      p   = _strinc (p);
    }

    // the next start is one char after the current p (either '\0' or ENV_SEP_CHAR)
    pStart = _strinc (p);

    // add path separator '\' (only if at least one directory is present)
    // -> if no dir is here, g_sActPath == ""
    if (dst != g_sActPath_s)
    {
      *dst = PATH_SEP_CHAR;
      dst = _strinc (dst);
    }

    // and add filename
    p = (char*) sFilename;
    while (*p)
    {
      *dst = *p;
      dst = _strinc (dst);
      p   = _strinc (p);
    }

    // finalize string
    *dst = '\0';

// [ph]
CHECKDEP_ASSERT (strlen (g_sActPath_s) < MAX_PATH);

    // get length of filename
    g_sActPath_len = dst - g_sActPath_s;

    // and try to open it!
    if (_OS_FileExists (g_sActPath_s))
    {
      // find and not in cache -> add to cache
      aDirCache.cadd (sFilename, g_sActPath_s, g_sActPath_len);
      return true;
    }

    // last path?
    if (*_strdec (g_pSearchPath, pStart) == '\0')
      break;
  }

  return false;
}

//--------------------------------------------------------------------
static void _File_FillBuffer ()
//--------------------------------------------------------------------
{
#ifdef _WIN32
  ReadFile (g_hFoundFile, g_read_buffer, MAX_CHAR, &g_read_bytes, NULL);
#else
  g_read_bytes = fread (g_read_buffer, 1, MAX_CHAR, g_f);
#endif
  if (g_read_bytes <= 0)
  {
    g_read_pos = g_read_end = NULL;
  }
  else
  {
    g_read_pos = g_read_buffer;
    g_read_end = g_read_buffer + g_read_bytes;
  }
}

//--------------------------------------------------------------------
static bool _File_ReadLine (char* pDest, const int nMaxLen)
//--------------------------------------------------------------------
{
  int nBytesCopiedToBuffer = 0;

  for (;;)
  {
    while (g_read_pos < g_read_end)
    {
      if (*g_read_pos == '\r' ||
          *g_read_pos == '\n' ||
          nBytesCopiedToBuffer == nMaxLen - 1)
      {
        // skip a \r char if any and then the \n (if any)
        // skip only one because empty lines are also counted!
        if (*g_read_pos == '\r' && g_read_pos < g_read_end)
        {
          // we read a byte
          g_read_pos++;
        }

        if (*g_read_pos == '\n' && g_read_pos < g_read_end)
        {
          // add to dest line
          *pDest = '\n';

          // goto next char
          pDest = _strinc (pDest);

          // we read a byte
          ++g_read_pos;
        }

        // add terminating NULL
        *pDest = '\0';
        pDest = _strinc (pDest);
        return true;
      }

      // add current char
      *pDest = *g_read_pos;
      pDest = _strinc (pDest);

      // we read a bytes!
      ++g_read_pos;

      // we copied a byte to the buffer
      ++nBytesCopiedToBuffer;
    }

    _File_FillBuffer ();

    if (!g_read_pos || !g_read_end)
      return false;
  }
}

/*! Check if the given file end with "_gen.hxx".
    If yes, look if a corresponding IDL file exists.

    [FlB] This method makes some assumtions about file naming.
          The heuristic used, assumes that generated files
          are named "<xy>_gen.hxx" and that there usually are
           corresponding IDL files with name "<xy>.idl".

    [FlB] For more info why this method was introduced see FMS 14702.
*/
//--------------------------------------------------------------------
static bool _IsGeneratedFromIDL (const char* sFileName)
//--------------------------------------------------------------------
{
  // Look for "_gen.hxx" at the end of the filename
  const int nGenLen = strlen (GEN_FILE_SUFFIX);
  const int nInLen = strlen (sFileName);
  if (nInLen >= nGenLen && strcmp (sFileName + nInLen - nGenLen, GEN_FILE_SUFFIX) == 0)
  {
    // Yes, it ends with it.

    // Determine the corresopnding IDL file name:
    char *pIDL = (char*) _alloca (nInLen - nGenLen + 4 + 1);
    strncpy (pIDL, sFileName, nInLen - nGenLen);
    strcpy (pIDL + nInLen - nGenLen, ".idl");

    // Now look, if the IDL file is there:
    if (_SP_ContainsFile (pIDL))
    {
      return true;
    }
  }

  return false;
}

/*! Recursive function to scan the file specified by sName.
    \param pReferencingFile is NULL for the first iteration
 */
//--------------------------------------------------------------------
static bool _Main_ScanFile (char*  sName,
                            DWORD*  pTotalLineNumbers,
                            const char* pReferencingFile,
                            FileList*   pAllScannedFiles)
//--------------------------------------------------------------------
{
  static int g_nLevel = 0;

  char sLine[MAX_CHAR];
  FileList aLocalIncludes;  //!< list of all includes of this file.
  char* p, *pStart;
  DWORD nLineNumber = 0;
  bool bRemoveLastPath = false;

  // check if current file exists on disk!
  if (!_SP_ContainsFile (sName))
  {
    //  [ph] check flag
    // Check if the file really does not exist or if it is probably a dir
    if (!g_bCheckIfPathExists || !_OS_PathExists (sName))
    {
      // happens quite often :(
      __ErrorMsg ("%s -> Cannot find '%s' in the path",
                  pReferencingFile
                    ? pReferencingFile
                    : "Commandline",
                  sName);

      // [FlB] This might be a generated file.
      //       Try a heuristic: If the filename is *_gen.hxx, look for the corresponding *.idl file.
      //       If its there, continue as usual.
      //       See also FMS 14702.
      if (!_IsGeneratedFromIDL (sName))
      {
        return false;
      }
    }
    else
    {
      __ErrorMsg ("Info: %s is a dir and not a file", sName);
      return false;
    }
  }

  // -> sPathBuffer contains now the full qualified path!
  if (pAllScannedFiles->contains (g_sActPath_s, g_sActPath_len))
  {
    if (g_bBeVerbose)
    {
      __ErrorMsg ("Info: %s already scanned", g_sActPath_s);
    }

    // close it because it was opened in _OS_FileExists
#ifdef _WIN32
    CloseHandle (g_hFoundFile);
#else
    fclose (g_f);
#endif
    return true;
  }

  if (g_bBeVerbose)
  {
    // write
    __ErrorMsg ("[Level %d] %s", ++g_nLevel, sName);
  }


  // handle special case: if an include file is referenced with a
  //   relative path (e.g. "bla\foo.h") we need to add this path
  //   to the search path too because this file may include another
  //   file in the same directory with "foo_include.h" so that
  //   checkdep may not recognize it because it does not search for
  //   "bla\foo_include.h" - it would only search for "foo_include.h".
  // -> add the current path including "bla" to the search path
  // This temporary search path is removed from the list after
  //   the file was scanned!
  pStart = strchr (sName, PATH_SEP_CHAR);
  if (pStart != NULL)
  {
    DWORD nPathLen;
    char* pLocalPath;

    // okay, we have a file with a relative path -> find last path sep!
    pStart = strrchr (g_sActPath_s, PATH_SEP_CHAR);

    // get length of path part
    nPathLen = pStart - g_sActPath_s;

    // get memory on stack!! (_alloca)
    pLocalPath = (char*) _alloca ((nPathLen + 1) * sizeof (char));

    // set path
    strncpy (pLocalPath, g_sActPath_s, nPathLen);
    pLocalPath[nPathLen] = '\0';

    // try to add the path (the string is concatenated to the existing path,
    // so it is no problem to use _alloca!)
    if (_SP_AddPath (pLocalPath))
    {
      // we need to remove the path aftewrwards!
      bRemoveLastPath = true;

      // small info!
      if (g_bBeVerbose)
      {
        __ErrorMsg ("%s: added '%s' temporarily to the search path list", sName, pLocalPath);
      }
    }
  }

  // change console title :)
  if (g_bUseCmdHeadline)
  {
    _stprintf (sLine, "[%3ldK lines] - [%s]", (*pTotalLineNumbers) / 1024, g_sActPath_s);
    SetConsoleTitle (sLine);
  }

  // print lowercase output
  if (g_bCountingOutput)
  {
    fprintf (stdout, "%s\n", _OS_UnifyPath (g_sActPath_s));
  }

  while (_File_ReadLine (sLine, MAX_CHAR))
  {
    ++nLineNumber;

    // reinitialize pointer to the line
    p = sLine;

    // skip leading whitespaces
    while (isspace (*p))
      p = _strinc (p);

    // count line numbers
    // -> no matter whether they are empty or not
    ++(*pTotalLineNumbers);

    // if the line is empty -> skip
    if (!*p)
    {
      continue;
    }

    // if line contains "#include" as the first characters -> okay
    if (strncmp (p, "#include", 8) == 0)
    {
      p = _strninc (p, 8);  // length of "#include"

      // skip whitespaces
      while (isspace (*p))
        p = _strinc (p);

      // get filename
      if (*p == '"')
      {
        // goto first char after the '"'
        p = _strinc (p);
        pStart = p;

        // find the trailing '"'
        while (*p && *p != '"')
        {
          p = _strinc (p);
        }

        if (*p)
        {
          bool bIgnore = false;
          int i;

          // end current string before comparing!!
          *p = '\0';

          for (i = 0; i < CHECKDEP_SIZEOF (IGNORE_INCLUDE_FILES); ++i)
            if (strcmp (pStart, IGNORE_INCLUDE_FILES[i]) == 0)
            {
              // found an ignore include file name
              bIgnore = true;
              break;
            }

          if (!bIgnore)
          {
            aLocalIncludes.cadd (pStart, p - pStart);
          }
        }
        else
        {
          __ErrorMsg ("%s -> found invalid user include in line %ld", sName, nLineNumber);
        }
      }
      else
      // system includes ...
      if (*p == '<')
      {
        if (g_bScanSystemIncludes)
        {
          p = _strinc (p);
          pStart = p;

          // goto next '>' or EOS
          while (*p && *p != '>')
          {
            p = _strinc (p);
          }

          if (*p)
          {
            // okay -> kill '>' char
            *p = '\0';
            aLocalIncludes.cadd (pStart, p - pStart);
          }
          else
          {
            __ErrorMsg ("%s -> found invalid system include in line %ld", sName, nLineNumber);
          }
        }
      }
      else
      {
        // some files have weird includes (e.g. other macros)
        // -> they are filtered with a static string list
        bool bWeird = false;
        int i;
        for (i = 0; i < CHECKDEP_SIZEOF (FILES_WITH_WEIRD_INCLUDES); ++i)
          if (strcmp (sName, FILES_WITH_WEIRD_INCLUDES[i]) == 0)
          {
            // found a weird include file name
            bWeird = true;
            break;
          }

        if (!bWeird)
        {
          __ErrorMsg ("%s -> found unknown character '%c' after #include in line %ld", sName, *p, nLineNumber);
        }
      }
    } // include
  }

#ifdef _WIN32
  CloseHandle (g_hFoundFile);
#else
  fclose (g_f);
#endif

  // reset variables so that they are handled correctly next time (g_read_end < g_read_pos)
  g_read_pos = g_read_buffer + MAX_CHAR;
  g_read_end = g_read_buffer + MAX_CHAR - 1;

  // add first file only if not specified otherwise
  if (!(pReferencingFile == NULL && g_bIgnoreSelfFile))
    pAllScannedFiles->cadd (g_sActPath_s, g_sActPath_len);

  if (g_bBeVerbose)
  {
    fprintf (stdout, "# '%s': %ld lines (current total: %ld)\n",
                    sName,
                    nLineNumber,
                    *pTotalLineNumbers);
  }

  // don't use sPathBuffer here ... will be overwritten in recursive call

  // now recursively scan all locally includes files
  FileList::const_iterator cit = aLocalIncludes.begin ();
  for (; !(cit == aLocalIncludes.end ()); ++cit)
  {
    // if it is an include with a path -> ensure correct separator
    _OS_EnsureCorrectPathSeparators ((*cit)->sName);

    // do scanning
    _Main_ScanFile ((*cit)->sName,      /* <- local file name */
                    pTotalLineNumbers,  /* <- just an info */
                    sName,              /* <- current file as referencing file */
                    pAllScannedFiles);  /* <- list with all scanned files */
  }

  // remove the temporary path?
  if (bRemoveLastPath)
  {
    // simply kill the last path
    _SP_RemoveLast ();
  }

  if (g_bBeVerbose)
  {
    // dec indent level
    --g_nLevel;
  }

  return true;
}

/*! This function prints the name of the target to stdout.
    If the name contains a whitspace, it is surrounded by
      double quoted.
 */
//--------------------------------------------------------------------
static void _Help_PrintQuotedFilename (const char* sName)
//--------------------------------------------------------------------
{
  // does any of the strings file contain a space???
  const bool bSpace = (strchr (sName, ' ') != NULL);

  // add leading double-quote
  if (bSpace)
    fputc ('"', stdout);

  // print name (already contains prefix and suffix)
  fputs (sName, stdout);

  // add trailing double-quote
  if (bSpace)
    fputc ('"', stdout);
}

/* get basename of filename:
   search the last '.' in the filename
     if a '.' was found -> replace it with a '\0'
     -> only the basename is left and the extension is skipped
 */
//--------------------------------------------------------------------
static void _Help_CutExtension (char* p)
//--------------------------------------------------------------------
{
  char* pDot = strrchr (p, '.');
  if (pDot)
    *pDot = '\0';
}

//--------------------------------------------------------------------
static void _Main_ShowResult (char* sFilename,
                              DWORD  nTotalLineNumbers,
                              FileList*  pAllScannedFiles)
//--------------------------------------------------------------------
{
#define MAX_TARGETS  (2)

  bool bMatchesTargetName;
  char *pTargets[MAX_TARGETS];
  int i;

  /* invalidate all targets */
  memset (pTargets, 0, MAX_TARGETS * sizeof (char*));

  // do output for the MAKEFILE?
  if (g_bWriteMakefile)
  {
    // any scanned file?
    if (!pAllScannedFiles->empty ())
    {
      // append to existing makefile?
      if (g_bWriteMakefileTargets)
      {
        _Help_CutExtension (sFilename);

        if (g_pUserDefMakeFilePrefix && g_pUserDefMakeFileSuffix)
        {
          // if both prefix and suffix are user defined, we don't
          // need any differences between Windows and Linux
          // because they are only different in prefix and suffix
          pTargets[0] = _Str_Concat3 (g_pUserDefMakeFilePrefix,
                                      sFilename,
                                      g_pUserDefMakeFileSuffix);
        }
        else
        {
          // first the Windows target
          pTargets[0] = _Str_Concat3 (g_pUserDefMakeFilePrefix ? g_pUserDefMakeFilePrefix : MAKEFILE_WIN_PREFIX,
                                      sFilename,
                                      g_pUserDefMakeFileSuffix ? g_pUserDefMakeFileSuffix : MAKEFILE_WIN_SUFFIX);

          // then the Linux target
          pTargets[1] = _Str_Concat3 (g_pUserDefMakeFilePrefix ? g_pUserDefMakeFilePrefix : MAKEFILE_LIN_PREFIX,
                                      sFilename,
                                      g_pUserDefMakeFileSuffix ? g_pUserDefMakeFileSuffix : MAKEFILE_LIN_SUFFIX);
        }

        // print all targets
        for (i = 0; i < MAX_TARGETS; ++i)
          if (pTargets[i])
          {
            // separate from previous entry if it is the second (or latter)
            if (i > 0)
              fputc (' ', stdout);

            // print it
            _Help_PrintQuotedFilename (pTargets[i]);
          }

        // each target definition ends with a colon and a "continue-on-next-line-char"
        fputs (": \\\n", stdout);
      }

      // write all files on what the target depends
      FileList::const_iterator cit = pAllScannedFiles->begin ();
      while (!(cit == pAllScannedFiles->end ()))
      {
        // check if the current dependency entry equals any
        // target name -> cyclic dependencies!
        bMatchesTargetName = false;
        for (i = 0; i < MAX_TARGETS; ++i)
          if (pTargets[i] != NULL &&
              strcmp (pTargets[i], (*cit)->sName) == 0)
          {
            bMatchesTargetName = true;
            break;
          }

        if (bMatchesTargetName)
        {
          // goto next
          ++cit;
        }
        else
        {
          // -> non cyclic dependency

          // write indentation
          fputc ('\t', stdout);

          // if necessary, quote the filename
          _Help_PrintQuotedFilename ((*cit)->sName);

          // do it after printing the filename and before scanning for end of list!
          ++cit;

          // if there is another dependency
          // or we're using --no_targets close the line with another ' \'
          if (!(cit == pAllScannedFiles->end ()) || !g_bWriteMakefileTargets)
            fputs (" \\\n", stdout);
          else
            fputs ("\n\n", stdout);
        }
      }
    }
  }
  else
  {
    // not makefile output
    if (g_bCountingOutput)
    {
      fprintf (stdout, "# %ld lines included\n", nTotalLineNumbers);
    }
    else
    {
      // remove the file itself from counting
      fprintf (stdout, "%s: %ld includes and %ld lines\n",
                       sFilename,
                       pAllScannedFiles->size () - 1,
                       nTotalLineNumbers);
    }
  }

  // finally free all possibly created target strings
  for (i = 0; i < MAX_TARGETS; ++i)
    if (pTargets[i])
      free (pTargets[i]);
}

//--------------------------------------------------------------------
static void _Main_ShowHeadline ()
//--------------------------------------------------------------------
{
  static bool bCanShowHeadline = true;

  if (bCanShowHeadline)
  {
    fprintf (stdout, "# checkdep %s %s (c) 1999-2009 by PHaX (%s %s) #\n",
#ifdef WIN32
                     "[win32]",
#else
                     "[*nix]",
#endif
                     CHECKDEP_VERSION,
                     __DATE__,
                     __TIME__);

    bCanShowHeadline = false;
  }
}

//--------------------------------------------------------------------
static bool _Main_ShowSyntax ()
//--------------------------------------------------------------------
{
  // to avoid showing the synatx more than once!
  static bool bCanShowSyntax = true;

  if (bCanShowSyntax)
  {
    _Main_ShowHeadline ();

    fprintf (stdout, "\n"
                     "Syntax: checkdep filename [option ...]\n"
                     "\n"
                     "Valid options:\n"

                     "  -?\n"
                     "  --help             - show this help screen\n"

                     "  -c\n"
                     "  --count            - output for counting\n"

#ifdef _WIN32
                     "  -C\n"
                     "  --no_console       - do not use console window headline\n"
#endif

                     "  -i#\n"
                     "  -I#\n"
                     "  --include#         - insert the path # into the search path list\n"

                     "  --ignore_self      - do not add the source file to the dependency list\n"

                     "  -m\n"
                     "  --makefile         - create MAKEFILE output (def: '" MAKEFILE_WIN_PREFIX"_basename_" MAKEFILE_WIN_SUFFIX"')\n"

                     "  -m#\n"
                     "  --prefix#          - set makefile target prefix '#'\n"

                     "  --suffix#          - set makefile target suffix '#' (the filename extension)\n"

                     "  -M\n"
                     "  --no_targets       - do not write makefile target name\n"

                     "  -n\n"
                     "  --nologo           - do not show the startup logo\n"

                     "  -q\n"
                     "  --use_env          - use ENVIRONMENT variable INCLUDE\n"

                     "  -s\n"
                     "  --system_includes  - also scan system includes (<..>)\n"

                     "  -v\n"
                     "  --verbose          - be verbose\n"

                     "  -x\n"
                     "  --check_pathes     - check if pathes exist (slow)\n"
                     );

    bCanShowSyntax = false;
  }

  return false;
}

/*! Append the string pToAdd to the global g_pUserDefMakeFilePrefix
    variable. Buffer management is done inside.
 */
//--------------------------------------------------------------------
static void _AddPrefix (const char* pToAdd)
//--------------------------------------------------------------------
{
  size_t nLen = strlen (pToAdd);

  if (!g_pUserDefMakeFilePrefix)
  {
    // first --prefix creates a new buffer
    g_pUserDefMakeFilePrefix = (char*) malloc (nLen + 1);
    strcpy (g_pUserDefMakeFilePrefix, pToAdd);
  }
  else
  {
    // all following --prefix append to the existing buffer
    nLen += strlen (g_pUserDefMakeFilePrefix);
    g_pUserDefMakeFilePrefix = (char*) realloc (g_pUserDefMakeFilePrefix, nLen + 1);
    strcat (g_pUserDefMakeFilePrefix, pToAdd);
  }
}

/*! Append the string pToAdd to the global g_pUserDefMakeFileSuffix
    variable. Buffer management is done inside.
 */
//--------------------------------------------------------------------
static void _AddSuffix (const char* pToAdd)
//--------------------------------------------------------------------
{
  size_t nLen = strlen (pToAdd);

  if (!g_pUserDefMakeFileSuffix)
  {
    // first --suffix creates a new buffer
    g_pUserDefMakeFileSuffix = (char*) malloc (nLen + 1);
    strcpy (g_pUserDefMakeFileSuffix, pToAdd);
  }
  else
  {
    // all following --suffix append to the existing buffer
    nLen += strlen (g_pUserDefMakeFileSuffix);
    g_pUserDefMakeFileSuffix = (char*) realloc (g_pUserDefMakeFileSuffix, nLen + 1);
    strcat (g_pUserDefMakeFileSuffix, pToAdd);
  }
}

//--------------------------------------------------------------------
void _ScanSingleFile (char* sFilename)
//--------------------------------------------------------------------
{
  DWORD nTotalLineNumbers = 0;
  FileList aAllScannedFiles;  /* <- list of all scanned files. */

  // scan main file
  _Main_ScanFile (sFilename,            /* < file to scan */
                  &nTotalLineNumbers,   /* < info only */
                  NULL,                 /* < referencing file :) */
                  &aAllScannedFiles);   /* <- list of scanned files */

  // show the result
  _Main_ShowResult (sFilename,           /* scanned file */
                    nTotalLineNumbers,   /* total scanned lines */
                    &aAllScannedFiles);  /* list of scanned files */
}

//--------------------------------------------------------------------
void _ScanMultipleFiles (const char* sMask)
//--------------------------------------------------------------------
{
#ifdef _WIN32
  HANDLE h;
  WIN32_FIND_DATA fd;

  // scan all files in the searchmask
  h = FindFirstFile (sMask, &fd);
  if (h == INVALID_HANDLE_VALUE)
  {
    __ErrorMsg ("Checkdep: Found no matching files for '%s'", sMask);
  }
  else
  {
    // successful - path is valid!
    do
    {
      // ensure correct path sep
      _OS_EnsureCorrectPathSeparators (fd.cFileName);

      // do scan
      _ScanSingleFile (fd.cFileName);
    } while (FindNextFile (h, &fd) != 0);

    FindClose (h);
  }
#else
  int count, i;
  struct dirent **files;

  // set global filemask - evaluated in _LinuxSelectFile!
  g_pFindMask = sMask;

  // scan dit
  count = scandir (".", &files, _LinuxSelectFile, alphasort);

  for (i = 0; i < count; i++)
  {
    // do scan single file
    _ScanSingleFile (files[i]->d_name);
  }

  // free memory again
  free (files);
#endif
}

//--------------------------------------------------------------------
static bool _HandleSwitch (char* p)
//--------------------------------------------------------------------
{
  bool bReturn = true;

  // only if it is a switch
  if (*p == '-')
  {
    ++p;

    if (*p == '-')
    {
      ++p;
           if (strcmp  (p, "check_pathes")    == 0) g_bCheckIfPathExists = true;
      else if (strcmp  (p, "count")           == 0) g_bCountingOutput = true;
      else if (strcmp  (p, "help")            == 0) bReturn = _Main_ShowSyntax ();
      else if (strcmp  (p, "ignore_self")     == 0) g_bIgnoreSelfFile = true;
      else if (strncmp (p, "include", 7)      == 0) _SP_AddPath (p + 7);
      else if (strcmp  (p, "makefile")        == 0) g_bWriteMakefile = true;
      else if (strcmp  (p, "no_console")      == 0) g_bUseCmdHeadline = false;
      else if (strcmp  (p, "no_targets")      == 0) g_bWriteMakefileTargets = false;
      else if (strcmp  (p, "nologo")          == 0) g_bNoLogo = true;
      else if (strncmp (p, "prefix", 6)       == 0) _AddPrefix (p + 6);
      else if (strncmp (p, "suffix", 6)       == 0) _AddSuffix (p + 6);
      else if (strcmp  (p, "system_includes") == 0) g_bScanSystemIncludes = true;
      else if (strcmp  (p, "use_env")         == 0) g_bInsertEnv = true;
      else if (strcmp  (p, "verbose")         == 0) g_bBeVerbose = true;
      else __ErrorMsg ("Checkdep warning: Ignoring parameter '--%s'", p);
    }
    else
    {
           if (*p == 'c') g_bCountingOutput = true;
      else if (*p == 'C') g_bUseCmdHeadline = false;
      else if (*p == 'i' || *p == 'I') _SP_AddPath (p + 1);
      else if (*p == 'm') _AddPrefix (p + 1);
      else if (*p == 'M') g_bWriteMakefileTargets = false;
      else if (*p == 'n') g_bNoLogo = true;
      else if (*p == 'q') g_bInsertEnv = true;
      else if (*p == 's') g_bScanSystemIncludes = true;
      else if (*p == 'v') g_bBeVerbose = true;
      else if (*p == 'x') g_bCheckIfPathExists = true;
      else if (*p == '?') bReturn = _Main_ShowSyntax ();
      else __ErrorMsg ("Checkdep warning: Ignoring parameter '-%s'", p);
    }
  }

  return bReturn;
}

//--------------------------------------------------------------------
static bool _HandleFilename (char* p)
//--------------------------------------------------------------------
{
  // only if no switch!
  if (*p != '-')
  {
/*
    const char* pPrefix = "checkdep " CHECKDEP_VERSION " ";

    // set filename as cmdline title
    char *pBla = (char*) _alloca (strlen (pPrefix) + strlen (p) + 1);
    strcpy (pBla, pPrefix);
    strcat (pBla, p);
    SetConsoleTitle (pBla);
*/

    ++g_nCommandlineFileSpecCounter;

    // if p contains any wildcards -> resolve them :)
    if (strpbrk (p, "?*") != NULL)
    {
      _ScanMultipleFiles (p);
    }
    else
    {
      _ScanSingleFile (p);
    }
  }

  // ALWAYS need to return true
  return true;
}

//--------------------------------------------------------------------
static bool _ReadCfgFile (const char* pFile, bool (*FctCallback) (char*))
//--------------------------------------------------------------------
{
  bool bReturn = true;
  FILE *f = fopen (pFile, "rt");
  char sLine[MAX_CHAR];
  int n;
  char* p;

  if (!f)
    __ErrorMsg ("Error: failed to open config file '%s'\n", pFile);
  else
  {
    while (fgets (sLine, MAX_CHAR, f))
    {
      p = sLine;

      // skip leading spaces
      while (isspace (*p))
        ++p;

      // skip trailing newlines
      n = strlen (p);
      if (p[n - 1] == '\n')
        p[--n] = '\0';
      while (n > 0 && isspace (p[n - 1]))
        p[--n] = '\0';

      // only non-empty lines
      if (n > 0)
        bReturn = bReturn && FctCallback (p);
    }
    fclose (f);
  }
  return bReturn;
}

//--------------------------------------------------------------------
static bool _Main_GetCmdlineSwitches (int argc, char** argv)
//--------------------------------------------------------------------
{
  char* p;
  int i;
  bool bReturn = true;

  for (i = 1; i < argc; i++)
  {
    p = argv[i];

    // is it a flag?
    if (*p == '-')
      bReturn = bReturn && _HandleSwitch (p);
    else
    if (*p == '@')
      bReturn = bReturn && _ReadCfgFile (p + 1, _HandleSwitch);
  }

  // check command dependencies
  if (g_pUserDefMakeFilePrefix ||
      g_pUserDefMakeFileSuffix)
  {
    g_bWriteMakefile = true;
    g_bWriteMakefileTargets = true;
  }

  if (!g_bWriteMakefileTargets)
    g_bWriteMakefile = true;

  return bReturn;
}

//--------------------------------------------------------------------
int __cdecl main (int argc, char **argv)
//--------------------------------------------------------------------
{
  int i, nReturnValue = 0;
  char* p;
  char sOldCmdlineHeader[512];

  // set console title :)
  GetConsoleTitle (sOldCmdlineHeader, 512);
  SetConsoleTitle ("checkdep " CHECKDEP_VERSION);

  // eval commandline
  if (_Main_GetCmdlineSwitches (argc, argv))
  {
    // add environment path?
    if (g_bInsertEnv)
      _SP_AddEnvPathes ();

    // print logo if necessary!
    if (g_bBeVerbose && !g_bNoLogo && !g_bCountingOutput)
    {
      _Main_ShowHeadline ();
    }

    // loop over all non paramters!
    for (i = 1; i < argc; ++i)
    {
      p = argv[i];

      // ignore all switches
      if (*p == '-')
        continue;

      // read config file
      if (*p == '@')
        _ReadCfgFile (p + 1, _HandleFilename);
      else
        _HandleFilename (p);
    }

    // was anything found?
    if (g_nCommandlineFileSpecCounter == 0)
    {
      _Main_ShowSyntax ();
      nReturnValue = 2;
    }
  }

  // free global makefile prefix & suffix
  if (g_pUserDefMakeFilePrefix)
    free (g_pUserDefMakeFilePrefix);

  if (g_pUserDefMakeFileSuffix)
    free (g_pUserDefMakeFileSuffix);

  // restore console headline
  SetConsoleTitle (sOldCmdlineHeader);
  return nReturnValue;
}

