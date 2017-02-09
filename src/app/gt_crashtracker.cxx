/*!
  Original code (c) Microsoft Systems Journal, May 1997 by Matt Pietrek
  Alter by PHaX
 */

#pragma warning (disable:4786)

// already set for VC7:
#if (defined _MSC_VER) && (_MSC_VER == 1200)
#define DECLSPEC_DEPRECATED  // otherwise compiler error :(
#endif

#define _IMAGEHLP64

#include "gt_crashtracker.hxx"
#include <tchar.h>
#include <dbghelp.h>

#include "gt_version.hrc"

// always do a stacktrace
const bool bDoStackTrace = true;

//
// defines
//

#define MSG_CRLF   _T ("\r\n")
#define DLLNAME    _T ("dbghelp.dll")
#define MSG_TITLE  _T ("An unrecoverable error occured")

//
// global vars
//

LPTOP_LEVEL_EXCEPTION_FILTER g_pPreviousExceptionFilter = NULL;
TCHAR                        g_sNextLogFileName[MAX_PATH] = _T ("");
HANDLE                       g_hProcess = 0;

template <class T>
T const& _max (T const& a, T const& b)
{
  return b < a ? a : b;
}

/*! Check we're currently on an NTZ platform.
    \retval true we are on an NT platform
    \retval false we're on any 9x/ME
 */
//--------------------------------------------------------------------
static bool __isNTPlatform ()
//--------------------------------------------------------------------
{
  OSVERSIONINFO aVI;
  aVI.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
  GetVersionEx (&aVI);
  return (aVI.dwPlatformId == VER_PLATFORM_WIN32_NT);
}

//--------------------------------------------------------------------
class Win32File
//--------------------------------------------------------------------
{
protected:
  TCHAR  m_sName[MAX_PATH];
  HANDLE m_h;
  bool   m_bEncrypt;

  //------------------------------------------------------------------
  explicit Win32File ()
  //------------------------------------------------------------------
    : m_h        (INVALID_HANDLE_VALUE),
      m_bEncrypt (true)
  {
    memset (m_sName, 0, sizeof (m_sName));
  }

  //------------------------------------------------------------------
  void __DoEncrypt (LPTSTR pBuf, size_t nLen)
  //------------------------------------------------------------------
  {
    // now encrypt the logfile slightly :)
    if (!_tgetenv (_T ("DUMP_NO_ENCRYPT")) && m_bEncrypt)
    {
      for (size_t i = 0; i < nLen; ++i)
      {
        // XOR with 0x2C - kindof encryption
        pBuf[i] ^= TCHAR (0x2c);
      }
    }
  }

public:
  //------------------------------------------------------------------
  static Win32File& Instance ()
  //------------------------------------------------------------------
  {
    static Win32File aInstance;
    return aInstance;
  }

  //------------------------------------------------------------------
  ~Win32File ()
  //------------------------------------------------------------------
  {
    if (IsOpen ())
    {
      Close ();
    }
  }

  //------------------------------------------------------------------
  void BuildName ()
  //------------------------------------------------------------------
  {
    TCHAR sModName[MAX_PATH];
    TCHAR sTempDir[MAX_PATH];

    memset (sModName, 0, sizeof (sModName));
    memset (sTempDir, 0, sizeof (sTempDir));

    // Figure out what the report file will be named, and store it away
    GetModuleFileName (0, sModName, MAX_PATH);

    // first checks TMP, than TEMP
    // if TMP or TEMP are invalid:
    // on 9x: the current directory
    // on NT: the windows directory :(
    DWORD nLen = GetTempPath (MAX_PATH, sTempDir);

    // if there is an error, use the current directory!
    if (nLen == 0)
      _tcscpy (sTempDir, _T ("."));

    // search for path delimiter in module name
    LPTSTR pBaseName = _tcsrchr (sModName, _T ('\\'));
    if (pBaseName)
    {
      // go to the first char after the last '\'
      pBaseName = _tcsinc (pBaseName);
    }
    else
    {
      // no path in module name -> basename is the full module name!
      pBaseName = sModName;
    }

    // create the logfile string - start with an empty string
    _tcscpy (m_sName, _T (""));

    // did we find a temp variable?
    if (sTempDir && *sTempDir)
    {
      // add temp directory
      _tcscat (m_sName, sTempDir);

      // check whether the last char was a '\' or a '/' - if not, add one
      TCHAR cLast = sTempDir[_tcslen (sTempDir) - 1];
      if (cLast != _T ('\\') && cLast != _T ('/'))
        _tcscat (m_sName, _T ("\\"));
    }

    // add basename
    _tcscat (m_sName, pBaseName);

#ifdef _DEBUG
    // determine whether we have a debug version or not
    _tcscat (m_sName, _T (".debug"));
#endif

    // and a suffix
    _tcscat (m_sName, _T (".log"));
  }

  //------------------------------------------------------------------
  bool OpenForCrash ()
  //------------------------------------------------------------------
  {
    _ASSERTE (!IsOpen ());

    m_h = CreateFile (m_sName,
                      GENERIC_WRITE,
                      FILE_SHARE_READ,
                      0,
                      OPEN_ALWAYS,
                      FILE_FLAG_WRITE_THROUGH, //!!!
                      0);

    // success???
    return IsOpen ();
  }

  //------------------------------------------------------------------
  bool OpenForDump (LPTSTR sOtherName)
  //------------------------------------------------------------------
  {
    _ASSERTE (!IsOpen ());

    m_h = CreateFile (*sOtherName
                        ? sOtherName
                        : m_sName,
                      GENERIC_WRITE,
                      FILE_SHARE_READ,
                      0,
                      OPEN_ALWAYS,
                      FILE_FLAG_WRITE_THROUGH, //!!!
                      0);

    // reset dumpfile name!
    _tcscpy (sOtherName, _T (""));

    // success???
    if (IsOpen ())
    {
#ifdef _UNICODE
      WORD nUnicodeID = 0xFEFF;
      DWORD cbWritten;
      WriteFile (m_h, &nUnicodeID, sizeof (WORD), &cbWritten, 0);
#endif
      return true;
    }

    return false;
  }

  //------------------------------------------------------------------
  void Close ()
  //------------------------------------------------------------------
  {
    _ASSERTE (IsOpen ());

    CloseHandle (m_h);
    m_h = INVALID_HANDLE_VALUE;
  }

  //------------------------------------------------------------------
  bool IsOpen ()
  //------------------------------------------------------------------
  {
    return (m_h != INVALID_HANDLE_VALUE);
  }

  //------------------------------------------------------------------
  void MoveToEnd ()
  //------------------------------------------------------------------
  {
    _ASSERTE (IsOpen ());

    SetFilePointer (m_h, 0, 0, FILE_END);
  }

  //------------------------------------------------------------------
  int GT_CDECL Log (LPCTSTR sFmt, ...)
  //------------------------------------------------------------------
  {
    _ASSERTE (IsOpen ());

    const size_t MAX_LOG = 4096;
    TCHAR szBuff[MAX_LOG + 1];
    int nLen;
    DWORD cbWritten;
    va_list argptr;

    // make a solid string
    va_start (argptr, sFmt);
    nLen = _vsntprintf (szBuff, MAX_LOG, sFmt, argptr);
    va_end (argptr);

    // try to encrypt
    __DoEncrypt (szBuff, nLen);

    // and write the stuff
    WriteFile (m_h, szBuff, nLen * sizeof (TCHAR), &cbWritten, 0);

    return nLen;
  }

  //------------------------------------------------------------------
  void EnableEncryption (const bool bEnable)
  //------------------------------------------------------------------
  {
    m_bEncrypt = bEnable;
  }

  //------------------------------------------------------------------
  LPCTSTR GetName () const
  //------------------------------------------------------------------
  {
    return m_sName;
  }

  //------------------------------------------------------------------
  bool LogWin32Cmd (const BOOL bCmdResult, LPCTSTR sCmdName)
  //------------------------------------------------------------------
  {
    // if command was successful, do not write anything!
    if (bCmdResult)
      return true;

    // print error!!
    Log (_T ("# [%s failed with EC %d]") MSG_CRLF,
         sCmdName,
         GetLastError ());
    return false;
  }

};

#define __CheckWin32(_expr) Win32File::Instance ().LogWin32Cmd (_expr, _T (#_expr))

char*    _strrchr (char* a, char b) { return strrchr (a, b); }
wchar_t* _strrchr (wchar_t* a, wchar_t b) { return wcsrchr (a, b); }

// funciton types
typedef BOOL    (__stdcall *FctStackWalk) (DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
typedef BOOL    (__stdcall *FctSymCleanup) (HANDLE hProcess);
typedef BOOL    (__stdcall *FctSymEnumerateModules) (HANDLE hProcess, PSYM_ENUMMODULES_CALLBACK64 EnumModulesCallback, PVOID UserContext);
typedef BOOL    (__stdcall *FctSymEnumSymbols) (HANDLE hProcess, ULONG64 BaseOfDll, PCSTR Mask, PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback, PVOID UserContext);
typedef PVOID   (__stdcall *FctSymFunctionTableAccess) (HANDLE hProcess, DWORD64 AddrBase);
typedef BOOL    (__stdcall *FctSymGetLineFromAddr) (HANDLE hProcess, DWORD64 dwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line);
typedef DWORD64 (__stdcall *FctSymGetModuleBase) (HANDLE hProcess, DWORD64 dwAddr);
typedef BOOL    (__stdcall *FctSymGetModuleInfo) (HANDLE hProcess, DWORD64 dwAddr, PIMAGEHLP_MODULE64 ModuleInfo);
typedef BOOL    (__stdcall *FctSymGetSearchPath) (HANDLE hProcess, PSTR SearchPath, DWORD SearchPathLength);
typedef BOOL    (__stdcall *FctSymGetSymFromAddr) (HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PIMAGEHLP_SYMBOL64 Symbol);
typedef BOOL    (__stdcall *FctSymGetTypeInfo) (HANDLE hProcess, DWORD64 ModBase, ULONG TypeId, IMAGEHLP_SYMBOL_TYPE_INFO GetType, PVOID pInfo);
typedef BOOL    (__stdcall *FctSymInitialize) (HANDLE hProcess, PCSTR UserSearchPath, BOOL fInvadeProcess);
typedef BOOL    (__stdcall *FctSymSetContext) (HANDLE hProcess, PIMAGEHLP_STACK_FRAME StackFrame, PIMAGEHLP_CONTEXT Context);
typedef DWORD   (__stdcall *FctSymSetOptions) (DWORD SymOptions);

// global vars
static FctStackWalk pStackWalk = NULL;
static FctSymCleanup pSymCleanup = NULL;
static FctSymEnumerateModules pSymEnumerateModules = NULL;
static FctSymEnumSymbols pSymEnumSymbols = NULL;
static FctSymFunctionTableAccess pSymFunctionTableAccess = NULL;
static FctSymGetLineFromAddr pSymGetLineFromAddr = NULL;
static FctSymGetModuleBase pSymGetModuleBase = NULL;
static FctSymGetModuleInfo pSymGetModuleInfo = NULL;
static FctSymGetSearchPath pSymGetSearchPath = NULL;
static FctSymGetSymFromAddr pSymGetSymFromAddr = NULL;
static FctSymGetTypeInfo pSymGetTypeInfo = NULL;
static FctSymInitialize pSymInitialize = NULL;
static FctSymSetContext pSymSetContext = NULL;
static FctSymSetOptions pSymSetOptions = NULL;

//--------------------------------------------------------------------
static bool __initFunctionPointer ()
//--------------------------------------------------------------------
{
  // already inited?
  if (pStackWalk &&
      pSymCleanup &&
      pSymEnumerateModules &&
      pSymEnumSymbols &&
      pSymFunctionTableAccess &&
      pSymGetLineFromAddr &&
      pSymGetModuleBase &&
      pSymGetModuleInfo &&
      pSymGetSearchPath &&
      pSymGetSymFromAddr &&
      pSymGetTypeInfo &&
      pSymInitialize &&
      pSymSetContext &&
      pSymSetOptions)
    return true;

  // load DLL
  HMODULE hDLL = ::LoadLibrary (DLLNAME);
  if (!hDLL)
    return false;

#define resolve(x) \
                   p##x = (Fct##x) ::GetProcAddress (hDLL, #x); \
                   if (!p##x) \
                     return false;

#define resolve64(x) \
                   p##x = (Fct##x) ::GetProcAddress (hDLL, #x##"64"); \
                   if (!p##x) \
                     return false;

  resolve64 (StackWalk)
  resolve (SymCleanup)
  resolve64 (SymEnumerateModules)
  resolve (SymEnumSymbols)  // XP or higher
  resolve64 (SymFunctionTableAccess)
  resolve64 (SymGetLineFromAddr)
  resolve64 (SymGetModuleBase)
  resolve64 (SymGetModuleInfo)
  resolve (SymGetSearchPath)
  resolve64 (SymGetSymFromAddr)
  resolve (SymGetTypeInfo)  // XP or higher
  resolve (SymInitialize)
  resolve (SymSetContext)  // XP or higher
  resolve (SymSetOptions)

  return true;
}

//--------------------------------------------------------------------
template <class T>
static bool __SkipLastPath (T *pBuf)
//--------------------------------------------------------------------
{
  T* pBackslash = _strrchr (pBuf, T ('\\'));
  T* pSlash     = _strrchr (pBuf, T ('/'));

  // nothing at all? -> error
  if (!pBackslash && !pSlash)
    return false;

  if (pBackslash)
  {
    // backslash was found!
    if (pSlash)
    {
      // both backslash and slash were found - take the last one!
      *(_max (pSlash, pBackslash)) = T ('\0');
    }
    else
    {
      // only backslash, no slash
      *pBackslash = T ('\0');
    }
  }
  else
  {
    // no backslash -> slash was found
    *pSlash = T ('\0');
  }

  return true;
}

//--------------------------------------------------------------------
static bool __GetModuleDirectory (DWORD nBuf, LPTSTR sBuf)
//--------------------------------------------------------------------
{
  // get name of executable module
  GetModuleFileName (0, sBuf, nBuf);

  // remove filename
  if (__SkipLastPath (sBuf))
    return true;

  // error - no path found :(
  _tcscpy (sBuf, _T ("[Error no slash and no backslash]"));
  return false;
}

//--------------------------------------------------------------------
class DebugSymbolHandler
//--------------------------------------------------------------------
{
private:
  bool m_bInited;
  bool m_bTried;

  //------------------------------------------------------------------
  explicit DebugSymbolHandler ()
  //------------------------------------------------------------------
    : m_bInited (false),
      m_bTried  (false)
  {
    g_hProcess = GetCurrentProcess ();
  }

public:
  //------------------------------------------------------------------
  static DebugSymbolHandler& Instance ()
  //------------------------------------------------------------------
  {
    static DebugSymbolHandler aInstance;
    return aInstance;
  }

  //------------------------------------------------------------------
  bool Init ()
  //------------------------------------------------------------------
  {
    // init only once!
    if (!m_bTried)
    {
      m_bTried = true;

      if (!__initFunctionPointer ())
        return false;

      // note: SearchPathes are recursivly scanned!!!!
      char sSearchPath[2048];
      char sAddOnDir[1024];

      // get name of executable module
      GetModuleFileNameA (0, sSearchPath, 2048);
      __SkipLastPath (sSearchPath);

      // also add windows system directory (e.g. c:\winnt\system32)!
      GetSystemDirectoryA (sAddOnDir, 1024);
      strcat (sSearchPath, ";");
      strcat (sSearchPath, sAddOnDir);

      // also add windows base directory (e.g. c:\winnt)!
      GetWindowsDirectoryA (sAddOnDir, 1024);
      strcat (sSearchPath, ";");
      strcat (sSearchPath, sAddOnDir);


      // init dbghelp - must be the VERY first action!
      // true: enumerates the loaded modules for the process and effectively calls the
      //       SymLoadModule function for each module - therefore it is very important
      //       to pass the searchpath in SymInitialize!
      if (__CheckWin32 (pSymInitialize (g_hProcess, sSearchPath, true)))
      {
        // dbghelp options:
        // -) load source line informations
        // -) undecorate names (?0AnyBla@foo@AJZ)
        __CheckWin32 (pSymSetOptions (SYMOPT_LOAD_LINES | SYMOPT_UNDNAME));

        // and remember that initialization worked!
        m_bInited = true;
      }
    }
    return m_bInited;
  }

  //------------------------------------------------------------------
  bool UnInit ()
  //------------------------------------------------------------------
  {
    if (!m_bInited)
      return false;

    __CheckWin32 (pSymCleanup (g_hProcess));
    m_bInited = false;
    return true;
  }
};

#define _LOG Win32File::Instance ().Log

// Calculates the difference between two SYSTEM times!
// this is done by converting them to _int64 and substracting them from another :-)
//--------------------------------------------------------------------
static bool __TimeDifference (LPSYSTEMTIME a, LPSYSTEMTIME b, LPSYSTEMTIME res)
//--------------------------------------------------------------------
{
  #define _I64(x)  *((_int64*) &x)

  // if any of the ptr is invalid -> error!
  if (!a || !b || !res)
    return false;

  // first convert them to FILETIME which are _int64's!
  FILETIME af, bf, resf;
  SystemTimeToFileTime (a, &af);
  SystemTimeToFileTime (b, &bf);

  // FILETIME consists of 2 DWORDS so handle them as _in64 variables!
  // the macro just casts them to _int64 -> better readable!
  _I64 (resf) = _I64 (af) - _I64 (bf);

  // and convert the result back!
  FileTimeToSystemTime (&resf, res);
  return true;
}

//--------------------------------------------------------------------
// Given an exception code, returns a pointer to a static string with a
// description of the exception
//--------------------------------------------------------------------
static LPTSTR __GetExceptionString (DWORD dwCode)
//--------------------------------------------------------------------
{
  #define EXCEPTION(x) case EXCEPTION_##x: return _T (#x);

  switch (dwCode)
  {
    EXCEPTION (ACCESS_VIOLATION)
    EXCEPTION (DATATYPE_MISALIGNMENT)
    EXCEPTION (BREAKPOINT)
    EXCEPTION (SINGLE_STEP)
    EXCEPTION (ARRAY_BOUNDS_EXCEEDED)
    EXCEPTION (FLT_DENORMAL_OPERAND)
    EXCEPTION (FLT_DIVIDE_BY_ZERO)
    EXCEPTION (FLT_INEXACT_RESULT)
    EXCEPTION (FLT_INVALID_OPERATION)
    EXCEPTION (FLT_OVERFLOW)
    EXCEPTION (FLT_STACK_CHECK)
    EXCEPTION (FLT_UNDERFLOW)
    EXCEPTION (INT_DIVIDE_BY_ZERO)
    EXCEPTION (INT_OVERFLOW)
    EXCEPTION (PRIV_INSTRUCTION)
    EXCEPTION (IN_PAGE_ERROR)
    EXCEPTION (ILLEGAL_INSTRUCTION)
    EXCEPTION (NONCONTINUABLE_EXCEPTION)
    EXCEPTION (STACK_OVERFLOW)
    EXCEPTION (INVALID_DISPOSITION)
    EXCEPTION (GUARD_PAGE)
    EXCEPTION (INVALID_HANDLE)
  }

  // If not one of the "known" exceptions, try to get the string
  // from NTDLL.DLL's message table.

  static TCHAR szBuffer[512] = { 0 };

  FormatMessage (FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                 GetModuleHandle (_T ("NTDLL.DLL")),
                 dwCode,
                 0,
                 szBuffer,
                 GT_ARRAY_SIZE (szBuffer),
                 0);

  return szBuffer;
}

//--------------------------------------------------------------------
// Given a linear address, locates the module, section, and offset containing
// that address.
//
// Note: the szModule paramater buffer is an output buffer of length specified
// by the len parameter (in characters!)
//--------------------------------------------------------------------
static bool __GetLogicalAddress
                                        (PVOID addr,
                                         PTSTR szModule,
                                         DWORD len,
                                         DWORD& section,
                                         DWORD& offset)
//--------------------------------------------------------------------
{
  MEMORY_BASIC_INFORMATION mbi;

  // VirtualQuery:
  // The VirtualQuery function provides information about a range of pages in the
  // virtual address space of the calling process.
  if (!__CheckWin32 (VirtualQuery (addr, &mbi, sizeof (mbi)) != 0))
  {
    return false;
  }

  // is mbi.AllocationBase is NULL we cannot resolve it to a valid module!!
  if (mbi.AllocationBase == NULL)
  {
    _LOG (MSG_CRLF _T ("# [mbi.AllocationBase == NULL]") MSG_CRLF);
    return false;
  }

  // mbi.AllocationBase:
  // Pointer to the base address of a range of pages allocated by the VirtualAlloc function.
  // The page pointed to by the BaseAddress member is contained within this allocation range.
  // [PH][mbi.AllocationBase points to the executables image in memory!!! "MZ..."]
  DWORD hMod = (DWORD) mbi.AllocationBase;

  // GetModuleFileName:
  // The GetModuleFileName function retrieves the full path and file name for the file
  // containing the specified module.
  // Windows 95/98/Me: The GetModuleFilename function retrieves long file names when
  // an application's version number is greater than or equal to 4.00 and the long file
  // name is available. Otherwise, it returns only 8.3 format file names.
  if (!__CheckWin32 (GetModuleFileName ((HMODULE) hMod, szModule, len) != 0))
  {
    return false;
  }

  // Point to the DOS header in memory
  PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER) hMod;

  // From the DOS header, find the NT (PE) header
  PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

  // get a pointer to the first section in the executable (== export section)
  PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION (pNtHdr);

  // RVA is offset from module load address
  DWORD rva = (DWORD) addr - hMod;

  // Iterate through the section table, looking for the one that encompasses
  // the linear address.
  for (size_t i = 0;
       i < pNtHdr->FileHeader.NumberOfSections;
       i++, pSection++)
  {
    DWORD sectionStart = pSection->VirtualAddress;
    DWORD sectionEnd = sectionStart + _max (pSection->SizeOfRawData, pSection->Misc.VirtualSize);

       // Is the address in this section???
    if ((rva >= sectionStart) && (rva <= sectionEnd))
    {
      // Yes, address is in the section.  Calculate section and offset,
      // and store in the "section" & "offset" params, which were
      // passed by reference.
      section = i + 1;
      offset = rva - sectionStart;
      return true;
    }
  }

  // small note :)
  _LOG (MSG_CRLF _T ("# '%s': Could not find RVA %08Xh in %d PE sections") MSG_CRLF,
                              szModule,
                              rva,
                              pNtHdr->FileHeader.NumberOfSections);

  return false;   // Should never get here!
}

//--------------------------------------------------------------------
static void __IntelStackWalk (PCONTEXT pContext)
//--------------------------------------------------------------------
{
  _LOG (_T ("  Call stack:") MSG_CRLF);
  _LOG (_T ("  Address   Frame     Logical addr  Module") MSG_CRLF);

  // get current EIP
  DWORD pc = pContext->Eip;
  PDWORD pFrame, pPrevFrame;

  // get base pointer of current exception context
  pFrame = (PDWORD) pContext->Ebp;

  do
  {
    TCHAR szModule[MAX_PATH + 1];
    DWORD section = 0;
    DWORD offset = 0;
    memset (szModule, 0, sizeof (szModule));

    if (!__GetLogicalAddress ((PVOID) pc,
                              szModule,
                              GT_ARRAY_SIZE (szModule),
                              section,
                              offset))
    {
      // Ooops
      _LOG (_T ("# [__GetLogicalAddress failed]") MSG_CRLF);
      break;
    }

    _LOG (_T ("  %08X  %08X  %04X:%08X %s") MSG_CRLF,
                                pc, pFrame, section, offset, szModule);

    pc = pFrame[1];

    pPrevFrame = pFrame;

    pFrame = (PDWORD)pFrame[0]; // proceed to next higher frame on stack

    if ((DWORD)pFrame & 3)    // Frame pointer must be aligned on a DWORD boundary.  Bail if not so.
    {
      _LOG (_T ("# [Frame pointer %08Xh aligned incorrectly]") MSG_CRLF, pFrame);
      break;
    }

    // current frame pointer must be higher than previous frame ptr!
    if (pFrame <= pPrevFrame)
    {
      _LOG (_T ("# [Frame pointer %08Xh > previous frame %08Xh]") MSG_CRLF, pFrame, pPrevFrame);
      break;
    }

    // Can two DWORDs be read from the supposed frame address?
    if (IsBadWritePtr (pFrame, sizeof (PVOID) * 2))
    {
      _LOG (_T ("# [Cannot read 2 frame pointer at %08Xh]") MSG_CRLF, pFrame);
      break;
    }

  } while (1);
}

// {
//--------------------------------------------------------------------
enum BasicType
//--------------------------------------------------------------------
{
   btNoType = 0,
   btVoid = 1,
   btChar = 2,
   btWChar = 3,
   btInt = 6,
   btUInt = 7,
   btFloat = 8,
   btBCD = 9,
   btBool = 10,
   btLong = 13,
   btULong = 14,
   btCurrency = 25,
   btDate = 26,
   btVariant = 27,
   btComplex = 28,
   btBit = 29,
   btBSTR = 30,
   btHresult = 31
};

// from CVCONST.H from the DIA SDK
//--------------------------------------------------------------------
enum SymTagEnum
//--------------------------------------------------------------------
{
   SymTagNull,
   SymTagExe,
   SymTagCompiland,
   SymTagCompilandDetails,
   SymTagCompilandEnv,
   SymTagFunction,
   SymTagBlock,
   SymTagData,
   SymTagAnnotation,
   SymTagLabel,
   SymTagPublicSymbol,
   SymTagUDT,
   SymTagEnum,
   SymTagFunctionType,
   SymTagPointerType,
   SymTagArrayType,
   SymTagBaseType,
   SymTagTypedef,
   SymTagBaseClass,
   SymTagFriend,
   SymTagFunctionArgType,
   SymTagFuncDebugStart,
   SymTagFuncDebugEnd,
   SymTagUsingNamespace,
   SymTagVTableShape,
   SymTagVTable,
   SymTagCustom,
   SymTagThunk,
   SymTagCustomType,
   SymTagManagedType,
   SymTagDimension
};

//--------------------------------------------------------------------
BasicType __GetBasicType
                                        (DWORD typeIndex,
                                         DWORD64 modBase)
//--------------------------------------------------------------------
{
  BasicType basicType;
  if (pSymGetTypeInfo (g_hProcess, modBase, typeIndex,
                       TI_GET_BASETYPE, &basicType))
  {
    return basicType;
  }

  // Get the real "TypeId" of the child.  We need this for the
  // SymGetTypeInfo (TI_GET_TYPEID) call below.
  DWORD typeId;
  if (pSymGetTypeInfo (g_hProcess,
                       modBase,
                       typeIndex,
                       TI_GET_TYPEID,
                       &typeId))
  {
    if (pSymGetTypeInfo (g_hProcess,
                         modBase,
                         typeId,
                         TI_GET_BASETYPE,
                         &basicType))
    {
      return basicType;
    }
  }

  return btNoType;
}

//--------------------------------------------------------------------
LPTSTR __GetNameOfBasicType (BasicType basicType)
//--------------------------------------------------------------------
{
  static TCHAR sUnk[128];
  switch (basicType)
  {
    case btNoType:
      return _T ("no_type");
    case btVoid:
      return _T ("void");
    case btInt:
      return _T ("int");
    case btUInt:
      return _T ("unsigned int");
    case btLong:
      return _T ("long");
    case btULong:
      return _T ("unsigned long");
    default:
      _stprintf (sUnk, _T ("unknown type %d"), basicType);
      return sUnk;
  }
}

//--------------------------------------------------------------------
void __FormatOutputValue
                                        (BasicType basicType,
                                         DWORD64   length,
                                         PVOID     pAddress)
//--------------------------------------------------------------------
{
//  _LOG (_T (" [%u|%u]"), basicType, length);

  // Format appropriately (assuming it's a 1, 2, or 4 bytes (!!!)
  if (length == 1)
    _LOG (_T (" = 0x%02X [unsigned char]"), *(PBYTE)pAddress);
  else if (length == 2)
    _LOG (_T (" = 0x%04X [unsigned short]"), *(PWORD)pAddress);
  else if (length == 4)
  {
    if (basicType == btFloat)
    {
      _LOG (_T (" = %f [float]"), *(PFLOAT)pAddress);
    }
    else if (basicType == btChar)
    {
      if (!IsBadStringPtrA (*(PSTR*)pAddress, 32))
        _LOG (_T (" = \"%.31hs\" [char*]"), *(PDWORD)pAddress);
      else
        _LOG (_T (" = 0x%08X [ptr]"), *(PDWORD)pAddress);
    }
    else
    {
      _LOG (_T (" = 0x%08X [%s]"), *(PDWORD)pAddress, __GetNameOfBasicType (basicType));
    }
  }
  else if (length == 8)
  {
    if (basicType == btFloat)
      _LOG (_T (" = %lf [double]"), *(double *)pAddress);
    else
      _LOG (_T (" = 0x%I64X [int64]"), *(DWORD64*)pAddress);
  }
  else
    _LOG (_T (" = [complex value]"));
}

//////////////////////////////////////////////////////////////////////////////
// If it's a user defined type (UDT), recurse through its members until we're
// at fundamental types.  When he hit fundamental types, return
// bHandled = false, so that __FormatSymbolValue() will format them.
//////////////////////////////////////////////////////////////////////////////
void __DumpTypeIndex
                                        (DWORD64 modBase,
                                         DWORD dwTypeIndex,
                                         unsigned nestingLevel,
                                         DWORD_PTR offset,
                                         bool & bHandled,
                                         bool bDumpAType)
{
//  _LOG (_T (" DTI"));

  bHandled = false;

  // [ph] check whether it is a method -we don't want them!
  DWORD dwMemberOffset;
  if (!bDumpAType)
    if (!pSymGetTypeInfo (g_hProcess, modBase, dwTypeIndex,
                          TI_GET_OFFSET, &dwMemberOffset))
    {
      bHandled = true;
      return;
    }

  // Get the name of the symbol.  This will either be a Type name (if a UDT),
  // or the structure member name.
  LPWSTR pwszTypeName;
  if (pSymGetTypeInfo (g_hProcess,
                       modBase,
                       dwTypeIndex,
                       TI_GET_SYMNAME,
                       &pwszTypeName))
  {
    if (nestingLevel > 0)
    {
      // Add appropriate indentation level (since this routine is recursive)
      _LOG (_T ("      "));
      for (unsigned j = 0; j < nestingLevel; j++)
        _LOG (_T ("  "));
    }

    if (bDumpAType)
      _LOG (_T (" of type '%ls'"), pwszTypeName);
    else
      _LOG (_T ("'%ls'"), pwszTypeName);
    LocalFree (pwszTypeName);
  }
  else
  {
    // [ph] no name
    return;
  }

  // Determine how many children this type has.
  DWORD dwChildrenCount = 0;
  pSymGetTypeInfo (g_hProcess,
                   modBase,
                   dwTypeIndex,
                   TI_GET_CHILDRENCOUNT,
                   &dwChildrenCount);

  if (!dwChildrenCount)     // If no children, we're done
  {
//    _LOG (_T (" [no childs]"));
    return;
  }

  // Prepare to get an array of "TypeIds", representing each of the children.
  // SymGetTypeInfo(TI_FINDCHILDREN) expects more memory than just a
  // TI_FINDCHILDREN_PARAMS struct has.  Use derivation to accomplish this.
  struct FINDCHILDREN : TI_FINDCHILDREN_PARAMS
  {
    ULONG MoreChildIds[1024];
    FINDCHILDREN ()
    {
      Count = sizeof (MoreChildIds) / sizeof(MoreChildIds[0]);
    }
  } children;

  children.Count = dwChildrenCount;
  children.Start = 0;

  // Get the array of TypeIds, one for each child type
  if (!pSymGetTypeInfo (g_hProcess, modBase, dwTypeIndex, TI_FINDCHILDREN,
                        &children))
  {
    _LOG (_T (" [TI_FINDCHILDREN failed]"));
    return;
  }

  // Append a line feed
  _LOG (_T (" [%u] {") MSG_CRLF, dwChildrenCount);

  // Iterate through each of the children
  for (unsigned i = 0; i < dwChildrenCount; i++)
  {
    // Recurse for each of the child types
    bool bHandled2;
    __DumpTypeIndex (modBase,
                     children.ChildId[i],
                     nestingLevel + 1,
                     offset,
                     bHandled2,
                     false);

    // If the child wasn't a UDT, format it appropriately
    if (!bHandled2)
    {
//      _LOG (_T (" [unh]"));

      // Get the offset of the child member, relative to its parent
      if (!pSymGetTypeInfo (g_hProcess, modBase, children.ChildId[i],
                            TI_GET_OFFSET, &dwMemberOffset))
      {
        // happens on methods etc.
        _LOG (_T (" [TI_GET_TYPEID failed]") MSG_CRLF);
        continue;
      }

      // Get the real "TypeId" of the child.  We need this for the
      // SymGetTypeInfo (TI_GET_TYPEID) call below.
      DWORD typeId;
      if (!pSymGetTypeInfo (g_hProcess, modBase, children.ChildId[i],
                            TI_GET_TYPEID, &typeId))
      {
        _LOG (_T (" [TI_GET_TYPEID failed]") MSG_CRLF);
        continue;
      }

      // Get the size of the child member
      ULONG64 length;
      if (!pSymGetTypeInfo (g_hProcess, modBase, typeId, TI_GET_LENGTH, &length))
      {
        _LOG (_T (" [TI_GET_LENGTH failed]") MSG_CRLF);
        continue;
      }

      // Calculate the address of the member

      DWORD_PTR dwFinalOffset = offset + dwMemberOffset;

      BasicType basicType = __GetBasicType(children.ChildId[i], modBase);

      __FormatOutputValue (basicType, length, (PVOID)dwFinalOffset);

      _LOG (MSG_CRLF);
    }
  }

  _LOG (_T ("      "));
  for (unsigned j = 0; j < nestingLevel; j++)
    _LOG (_T ("  "));
  _LOG (_T ("}") MSG_CRLF);

  bHandled = true;
}

//////////////////////////////////////////////////////////////////////////////
// Given a SYMBOL_INFO representing a particular variable, displays its
// contents.  If it's a user defined type, display the members and their
// values.
//////////////////////////////////////////////////////////////////////////////
bool __FormatSymbolValue
                                        (PSYMBOL_INFO pSym,
                                         STACKFRAME64* sf)
{
  // Indicate if the variable is a local or parameter
  if (pSym->Flags & IMAGEHLP_SYMBOL_INFO_PARAMETER)
    _LOG (_T ("Parameter "));
  else
  if (pSym->Flags & IMAGEHLP_SYMBOL_INFO_LOCAL)
    _LOG (_T ("Local "));
  else
    _LOG (_T ("Whatever [%u]"), pSym->Flags);

  // If it's a function, don't do anything.
  if (pSym->Tag == SymTagFunction)
    return false;

  // Emit the variable name
  _LOG (_T ("\'%hs\'"), pSym->Name);

  DWORD_PTR pVariable = 0;    // Will point to the variable's data in memory

  if (pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGRELATIVE)
  {
    // if (pSym->Register == 8)   // EBP is the value 8 (in DBGHELP 5.1)
    {                               //  This may change!!!
      pVariable = sf->AddrFrame.Offset;
      pVariable += (DWORD_PTR)pSym->Address;
    }
    // else
    //  return false;
  }
  else if (pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGISTER)
  {
    // Don't try to report register variable
    return false;
  }
  else
  {
    // It must be a global variable
    pVariable = (DWORD_PTR)pSym->Address;
  }

  // Determine if the variable is a user defined type (UDT).  If so, bHandled
  // will return true.
  bool bHandled;
  __DumpTypeIndex (pSym->ModBase,
                   pSym->TypeIndex,
                   0,
                   pVariable,
                   bHandled,
                   true);

  if (!bHandled)
  {
    // happens for global automatic vars (long, int, float etc.)

    // The symbol wasn't a UDT, so do basic, stupid formatting of the
    // variable.  Based on the size, we're assuming it's a char, WORD, or
    // DWORD.
    BasicType basicType = __GetBasicType (pSym->TypeIndex, pSym->ModBase);

    __FormatOutputValue (basicType,
                         pSym->Size,
                         (PVOID)pVariable);

    _LOG (MSG_CRLF);
  }
  return true;
}

BOOL CALLBACK EnumerateSymbolsCallback
                                        (PSYMBOL_INFO pSymInfo,
                                         ULONG        SymbolSize,
                                         PVOID        UserContext)
{
  __try
  {
    _LOG (_T ("      "));
    __FormatSymbolValue (pSymInfo, (STACKFRAME64*)UserContext);
  }
  __except (1)
  {
    _LOG (_T ("Exception on symbol '%hs'") MSG_CRLF, pSymInfo->Name);
  }
  return TRUE;
}

// }



//--------------------------------------------------------------------
static void _DoDebugHelpStackWalk (PCONTEXT pContext, bool bWithVars)
//--------------------------------------------------------------------
{
  // Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag

  STACKFRAME64 sf;
  ZeroMemory (&sf, sizeof (sf));

  // Initialize the STACKFRAME structure for the first call.  This is only
  // necessary for Intel CPUs, and isn't mentioned in the documentation.
  sf.AddrPC.Offset    = pContext->Eip;
  sf.AddrPC.Mode      = AddrModeFlat;
  sf.AddrStack.Offset = pContext->Esp;
  sf.AddrStack.Mode   = AddrModeFlat;
  sf.AddrFrame.Offset = pContext->Ebp;
  sf.AddrFrame.Mode   = AddrModeFlat;

  _LOG (_T ("  Call stack:") MSG_CRLF);
  _LOG (_T ("    Address   Frame") MSG_CRLF);

  while (1)
  {
    if (0 == sf.AddrFrame.Offset)
    {
      // check it also before calling StackWalk to have a comparison!!
      _LOG (_T ("# before StackWalk: AddrFrame.Offset == 0") MSG_CRLF);
    }

    if (!pStackWalk (IMAGE_FILE_MACHINE_I386,       // machine type
                     g_hProcess,                    // process handle
                     GetCurrentThread (),           // thread handle
                     &sf,                           // StackFrame
                     pContext,                      // ContextRecord
                     NULL,                          // ReadMemoryRoutine
                     pSymFunctionTableAccess,       // FunctionTableAccessRoutine  (imagehlp.SymFunctionTableAccess)
                     pSymGetModuleBase,             // GetModuleBaseRoutine        (imagehlp.SymGetModuleBase)
                     NULL))                         // TranslateAddressRoutine
    {
      const DWORD nLastError = GetLastError ();
      switch (nLastError)
      {
        case 0:
          // no err
          break;
        case 487:
        {
          _LOG (_T ("# StackWalk (487): Attempt to access invalid address") MSG_CRLF);
          break;
        }
        case 998:
        {
          _LOG (_T ("# StackWalk (998): Invalid access to memory location") MSG_CRLF);
          break;
        }
        default:
        {
          _LOG (_T ("# StackWalk failed with EC %d") MSG_CRLF, nLastError);
          break;
        }
      }
      break;
    }

    // Basic sanity check to make sure the frame is OK.  Bail if not.
    if (0 == sf.AddrFrame.Offset)
    {
      _LOG (_T ("# AddrFrame.Offset == 0") MSG_CRLF);
      break;
    }

    // print EIP and ESP
    _LOG (_T ("    %08X  %08X  "), sf.AddrPC.Offset, sf.AddrFrame.Offset);

    // IMAGEHLP is wacky, and requires you to pass in a pointer to an
    // IMAGEHLP_SYMBOL structure.  The problem is that this structure is
    // variable length.  That is, you determine how big the structure is
    // at runtime.  This means that you can't use sizeof(struct).
    // So...make a buffer that's big enough, and make a pointer
    // to the buffer.  We also need to initialize not one, but TWO
    // members of the structure before it can be used.

    BYTE symbolBuffer [sizeof (IMAGEHLP_SYMBOL64) + 1024];

    PIMAGEHLP_SYMBOL64 pSymbol = (PIMAGEHLP_SYMBOL64) symbolBuffer;
    pSymbol->SizeOfStruct = sizeof (symbolBuffer);
    pSymbol->MaxNameLength = 1024;

    TCHAR szModule[MAX_PATH + 1] = _T ("");
    DWORD section = 0, offset = 0;

    // also try to find module name etc...
    const BOOL bLogicalAddressOK = __GetLogicalAddress ((PVOID) sf.AddrPC.Offset,  // EIP
                                                        szModule,
                                                        GT_ARRAY_SIZE (szModule),
                                                        section,
                                                        offset);

    // now get line number
    IMAGEHLP_LINE64 aLine;
    ZeroMemory (&aLine, sizeof (aLine));
    aLine.SizeOfStruct = sizeof (aLine);

    // Displacement of the input address, relative to the start of the symbol
    DWORD lineDisplacement = 0;
    if (pSymGetLineFromAddr (g_hProcess,
                             sf.AddrPC.Offset,
                             &lineDisplacement,
                             &aLine))
    {
      if (lineDisplacement == 0)
      {
        _LOG (_T ("[%-12hs line %d] "),
                                    aLine.FileName,
                                    aLine.LineNumber);
      }
      else
      {
        _LOG (_T ("[%-12hs line %d + %Xh] "),
                                    aLine.FileName,
                                    aLine.LineNumber,
                                    lineDisplacement);
      }
    }
    else
    {
      _LOG (_T ("[no lines] "));
    }

    // The SymGetSymFromAddr function locates the symbol for the specified address
    DWORD64 symDisplacement = 0;
    if (pSymGetSymFromAddr (g_hProcess,
                            sf.AddrPC.Offset,    // EIP
                            &symDisplacement,
                            pSymbol))
    {
      // print the name of the symbol and the offset of the address from the beginning of the function
      // %hs means single-byte string!!
      // %ls would mean wide-char string!!
      _LOG (_T ("%hs + %Xh"), pSymbol->Name, symDisplacement);

      // also try to find module name etc...
      if (bLogicalAddressOK)
      {
        _LOG (_T (" [%04X:%08X %s]"), section, offset, szModule);
      }
      else
      {
        _LOG (_T (" [__GetLogicalAddress failed]"));
      }
    }
    else
    {
      // No symbol found.
      _LOG (_T ("[no symbol] "));

      // Print out the logical address only.
      if (bLogicalAddressOK)
      {
        _LOG (_T ("%04X:%08X %s"), section, offset, szModule);
      }
      else
      {
        _LOG (_T (" [__GetLogicalAddress failed]"));
      }
    }

    _LOG (MSG_CRLF);

    // write local vars
    if (bWithVars)
    {
      // Use SymSetContext to get just the locals/params for this frame
      IMAGEHLP_STACK_FRAME imagehlpStackFrame;
      imagehlpStackFrame.InstructionOffset = sf.AddrPC.Offset;
      pSymSetContext (g_hProcess, &imagehlpStackFrame, 0);

      // Enumerate the locals/parameters
      if (pSymEnumSymbols)
        pSymEnumSymbols (g_hProcess, 0, 0, EnumerateSymbolsCallback, &sf);

      _LOG (MSG_CRLF);
    }
  }
}

//--------------------------------------------------------------------
static void _ReportHeader (LPCTSTR sAction)
//--------------------------------------------------------------------
{
  //! Start with a banner
  LPCTSTR pVersion = GT_VERSION_STR;
  _LOG (MSG_CRLF _T ("#[%s]-[") GT_VERSION_STR _T ("]") MSG_CRLF,
                              sAction);

#ifdef _DEBUG
  // special not if it is a debug version
  _LOG (_T ("    Debug version") MSG_CRLF);
#endif

  //! print commandline
  LPTSTR pCmdline = ::GetCommandLine ();
  _LOG (_T ("    Commandline: '%s'\n"),
                              pCmdline ? pCmdline : _T ("[null]"));

  // empty line
  _LOG (MSG_CRLF);
}

// print current directory and working directory
//--------------------------------------------------------------------
static void _ReportDirectories ()
//--------------------------------------------------------------------
{
  // print current working directory
  TCHAR sDir[1024];
  __CheckWin32 (GetCurrentDirectory (1024, sDir));
  _LOG (_T ("  Current directory: %s") MSG_CRLF, sDir);

  // print module directory
  __GetModuleDirectory (1024, sDir);
  _LOG (_T ("  Module directory: %s") MSG_CRLF, sDir);

  // newline
  _LOG (MSG_CRLF);
}

// print Windows version
//--------------------------------------------------------------------
static void _ReportWinVersion ()
//--------------------------------------------------------------------
{
  OSVERSIONINFO aVI;
  aVI.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
  GetVersionEx (&aVI);

  _LOG (_T ("  Windows version: "));

  if (aVI.dwMajorVersion == 4 && aVI.dwMinorVersion ==  0)
  {
    if (aVI.dwPlatformId == VER_PLATFORM_WIN32_NT)
      _LOG (_T ("Windows NT 4"));
    else
      _LOG (_T ("Windows 95"));
  }
  else
  if (aVI.dwMajorVersion == 4 && aVI.dwMinorVersion == 10)
  {
    _LOG (_T ("Windows 98"));
  }
  else
  if (aVI.dwMajorVersion == 4 && aVI.dwMinorVersion == 90)
  {
    _LOG (_T ("Windows ME"));
  }
  else
  if (aVI.dwMajorVersion == 3 && aVI.dwMinorVersion == 51)
  {
    _LOG (_T ("Windows NT 3.51"));
  }
  else
  if (aVI.dwMajorVersion == 5 && aVI.dwMinorVersion == 0)
  {
    _LOG (_T ("Windows 2000"));
  }
  else
  if (aVI.dwMajorVersion == 5 && aVI.dwMinorVersion == 1)
  {
    _LOG (_T ("Windows XP"));
  }
  else
  {
    _LOG (_T ("unknown (%d.%d)"),
                                aVI.dwMajorVersion,
                                aVI.dwMinorVersion);
  }

  // print into every detail =|:-)
  _LOG (_T (" %s (%d.%d.%d [%d])") MSG_CRLF,
                              aVI.szCSDVersion
                                ? aVI.szCSDVersion
                                : _T ("[null]"),
                              aVI.dwMajorVersion,
                              aVI.dwMinorVersion,
                              aVI.dwBuildNumber,
                              aVI.dwPlatformId);

  _LOG (MSG_CRLF);
}

/*! Print current system and local time
 */
//--------------------------------------------------------------------
static void _ReportSystemTime ()
//--------------------------------------------------------------------
{
  SYSTEMTIME aSysT;
  GetSystemTime (&aSysT);

  SYSTEMTIME aLocalT;
  GetLocalTime (&aLocalT);

  _LOG (_T ("  System crash time: %02d.%02d.%04d, %02d:%02d.%02d") MSG_CRLF,
                              aSysT.wDay, aSysT.wMonth, aSysT.wYear, aSysT.wHour, aSysT.wMinute, aSysT.wSecond);

  _LOG (_T ("  Local crash time.: %02d.%02d.%04d, %02d:%02d.%02d") MSG_CRLF,
                              aLocalT.wDay, aLocalT.wMonth, aLocalT.wYear, aLocalT.wHour, aLocalT.wMinute, aLocalT.wSecond);

  _LOG (MSG_CRLF);
}

/*! write module information
 */
//--------------------------------------------------------------------
static void _ReportModuleInfo ()
//--------------------------------------------------------------------
{
  TCHAR sModName[MAX_PATH];
  DWORD nLastError;
  DWORD nFileSize = 0, nFileSizeHigh = 0;
  FILETIME aFTCreation, aFTLastAccess, aFTLastWrite;
  SYSTEMTIME aSTCreation, aSTLastAccess, aSTLastWrite;

  memset (sModName,       0, sizeof (sModName));

  memset (&aFTCreation,   0, sizeof (FILETIME));
  memset (&aFTLastAccess, 0, sizeof (FILETIME));
  memset (&aFTLastWrite,  0, sizeof (FILETIME));

  memset (&aSTCreation,   0, sizeof (SYSTEMTIME));
  memset (&aSTLastAccess, 0, sizeof (SYSTEMTIME));
  memset (&aSTLastWrite,  0, sizeof (SYSTEMTIME));

  // get module name
  GetModuleFileName (0, sModName, MAX_PATH);
  _LOG (_T ("  Module name......: '%s'") MSG_CRLF, sModName);

  // try to open for reading!
  HANDLE hThis = CreateFile (sModName,
                             0,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);
  if (hThis == INVALID_HANDLE_VALUE)
  {
    nLastError = GetLastError ();

    // failed to open file
    _LOG (_T ("# File size and time could not be determined (EC %d)") MSG_CRLF, nLastError);
  }
  else
  {
    nFileSize = GetFileSize (hThis, &nFileSizeHigh);
    GetFileTime (hThis, &aFTCreation, &aFTLastAccess, &aFTLastWrite);
    CloseHandle (hThis);

    FileTimeToSystemTime (&aFTCreation,   &aSTCreation);
    FileTimeToSystemTime (&aFTLastAccess, &aSTLastAccess);
    FileTimeToSystemTime (&aFTLastWrite,  &aSTLastWrite);

    // only possible, if we could open the crashing EXE for reading!
    _LOG (_T ("  Module file size.: %08X%08Xh %ld (high: %ld)") MSG_CRLF,
                                nFileSizeHigh,
                                nFileSize,
                                nFileSize, nFileSizeHigh);

    _LOG (_T ("  Executable times:") MSG_CRLF);
    _LOG (_T ("    Creation time....: %02d.%02d.%04d, %02d:%02d.%02d") MSG_CRLF,
                                aSTCreation.wDay,
                                aSTCreation.wMonth,
                                aSTCreation.wYear,
                                aSTCreation.wHour,
                                aSTCreation.wMinute,
                                aSTCreation.wSecond);

    _LOG (_T ("    Last access time.: %02d.%02d.%04d, %02d:%02d.%02d") MSG_CRLF,
                                aSTLastAccess.wDay,
                                aSTLastAccess.wMonth,
                                aSTLastAccess.wYear,
                                aSTLastAccess.wHour,
                                aSTLastAccess.wMinute,
                                aSTLastAccess.wSecond);

    _LOG (_T ("    Last write time..: %02d.%02d.%04d, %02d:%02d.%02d") MSG_CRLF,
                                aSTLastWrite.wDay,
                                aSTLastWrite.wMonth,
                                aSTLastWrite.wYear,
                                aSTLastWrite.wHour,
                                aSTLastWrite.wMinute,
                                aSTLastWrite.wSecond);
  }

  // newline
  _LOG (MSG_CRLF);
}

/* temporary not of interest!
struct CallBackInfo
{
  ULONG nModuleBase;
  ULONG nModuleSize;
  LPSTR sModuleName;

  ~CallBackInfo ()
  {
    free (sModuleName);
  }
};

typedef map <const ULONG, CallBackInfo*> CallBackInfoMap;

//--------------------------------------------------------------------
BOOL CALLBACK _ReportLoadedModulesCallBack
                                        (LPSTR sModuleName,
                                         ULONG nModuleBase,
                                         ULONG nModuleSize,
                                         PVOID pUserContext)
//--------------------------------------------------------------------
{
  CallBackInfo *pInfo = new CallBackInfo;
  pInfo->nModuleBase = nModuleBase;
  pInfo->nModuleSize = nModuleSize;
  pInfo->sModuleName = strdup (sModuleName);

  CallBackInfoMap *pMap = (CallBackInfoMap*) pUserContext;
  pMap->insert (CallBackInfoMap::value_type (nModuleBase, pInfo));

  return true;
}

//--------------------------------------------------------------------
static void _ReportLoadedModules ()
//--------------------------------------------------------------------
{
  _LOG (MSG_CRLF _T ("  Loaded modules:") MSG_CRLF _T ("    Base (hex) Size (hex) Name") MSG_CRLF);

  CallBackInfoMap aMap;

  if (!EnumerateLoadedModules (g_hProcess, _ReportLoadedModulesCallBack, (LPVOID) &aMap))
  {
    _LOG (_T ("# EnumerateLoadedModules failed (EC %d)") MSG_CRLF, GetLastError ());
  }
  else
  {
    ULONG nTotalSize = 0;

    _LOG (_T ("  ================================") MSG_CRLF);

    // now print all modules, ordered by base address!
    CallBackInfoMap::const_iterator cit = aMap.begin ();
    for (; !(cit == aMap.end ()); ++cit)
    {
      CallBackInfo *pInfo = (*cit).second;

      // print it!
      _LOG (_T ("    %08Xh  %08Xh  %s") MSG_CRLF,
                 pInfo->nModuleBase,
                 pInfo->nModuleSize,
                 pInfo->sModuleName);

      nTotalSize += pInfo->nModuleSize;

      // free memory
      delete pInfo;
    }

    _LOG (_T ("  ================================") MSG_CRLF
               "  Total: %d modules with %d byte / %d KB / %d MB") MSG_CRLF,
               aMap.size (),
               nTotalSize,
               nTotalSize / 1024,
               nTotalSize / (1024 * 1024));

    aMap.clear ();
  }

  _LOG (MSG_CRLF);
}
*/

//--------------------------------------------------------------------
static void _ReportEnvVars ()
//--------------------------------------------------------------------
{
  _LOG (_T ("  Environment variables:") MSG_CRLF);

#ifdef _UNICODE
  TCHAR** p = _wenviron;
#else
  TCHAR** p = _environ;
#endif

  for (; *p; ++p)
  {
    if (p)
    {
      // write current env var
      _LOG (_T ("    %s") MSG_CRLF, *p);
    }
    else
    {
      // NULL pointer - should never occur
      _LOG (_T ("    NULL pointer") MSG_CRLF);
    }
  }

  _LOG (MSG_CRLF);
}

//--------------------------------------------------------------------
static void _ReportApplicationUpTime ()
//--------------------------------------------------------------------
{
  typedef bool (WINAPI *_GetProcessTimesFct) (HANDLE, LPFILETIME, LPFILETIME, LPFILETIME, LPFILETIME);

  // print application up time (only on NT/2000 systems!!)
  // This is done by dynamically loading the kernel32.dll and getting the
  // function pointer to the "GetProcessTimes" function...
  if (__isNTPlatform ())
  {
    HMODULE hKernel32Mod = LoadLibrary (_T ("kernel32.dll"));
    if (hKernel32Mod)
    {
      // GetProcessTimes has no A/W version!!!!
      _GetProcessTimesFct pGetProcessTimes = (_GetProcessTimesFct) GetProcAddress (hKernel32Mod, "GetProcessTimes");

      // was a valid pointer returned???
      if (pGetProcessTimes)
      {
        // init FILETIME variables
        FILETIME aFTCreation, aFTExit, aFTKernelTime, aFTUserTime;
        ZeroMemory (&aFTCreation,   sizeof (FILETIME));
        ZeroMemory (&aFTKernelTime, sizeof (FILETIME));
        ZeroMemory (&aFTUserTime,   sizeof (FILETIME));

        // call the function itself
        if (!pGetProcessTimes (g_hProcess,
                               &aFTCreation,
                               &aFTExit,   // not of interest - process has no yet exited!
                               &aFTKernelTime,
                               &aFTUserTime))
        {
          _LOG (_T ("# GetProcessTimes failed with EC %d") MSG_CRLF, GetLastError ());
        }
        else
        {
          // and convert them to to SYSTEMTIME vars!
          SYSTEMTIME aPTCreation, aPTKernelTime, aPTUserTime;
          ZeroMemory (&aPTCreation,   sizeof (SYSTEMTIME));
          ZeroMemory (&aPTKernelTime, sizeof (SYSTEMTIME));
          ZeroMemory (&aPTUserTime,   sizeof (SYSTEMTIME));

          FileTimeToSystemTime (&aFTCreation,   &aPTCreation);
          FileTimeToSystemTime (&aFTKernelTime, &aPTKernelTime);
          FileTimeToSystemTime (&aFTUserTime,   &aPTUserTime);

          _LOG (_T ("  Process times:") MSG_CRLF);
          _LOG (_T ("    Creation time....: %02d.%02d.%04d, %02d:%02d.%02d") MSG_CRLF,
                                      aPTCreation.wDay, aPTCreation.wMonth, aPTCreation.wYear, aPTCreation.wHour, aPTCreation.wMinute, aPTCreation.wSecond);

          _LOG (_T ("    Kernel time......: %02d:%02d.%02d.%03d") MSG_CRLF,
                                      aPTKernelTime.wHour, aPTKernelTime.wMinute, aPTKernelTime.wSecond, aPTKernelTime.wMilliseconds);

          _LOG (_T ("    User time........: %02d:%02d.%02d.%03d") MSG_CRLF,
                                      aPTUserTime.wHour, aPTUserTime.wMinute, aPTUserTime.wSecond, aPTUserTime.wMilliseconds);

          SYSTEMTIME aSysT;
          GetSystemTime (&aSysT);

          SYSTEMTIME aDiffTime;
          if (__TimeDifference (&aSysT, &aPTCreation, &aDiffTime))
          {
            _LOG (_T ("    Up time..........: %dd %02dh %02dm %02ds %03dms") MSG_CRLF,
                                        (aDiffTime.wMonth - 1) * 30 + (aDiffTime.wDay - 1),
                                        aDiffTime.wHour,
                                        aDiffTime.wMinute,
                                        aDiffTime.wSecond,
                                        aDiffTime.wMilliseconds);
          }
        }
      }
      else
        _LOG (_T ("# Failed to find GetProcessTimes in kernel32.dll") MSG_CRLF);

      FreeLibrary (hKernel32Mod);
    }
    else
    {
      _LOG (_T ("# Failed to load kernel32.dll") MSG_CRLF);
    }
  }
  else
  {
    _LOG (_T ("# GetProcessTimes is not supported") MSG_CRLF);
  }

  _LOG (MSG_CRLF);
}

// print system up time
//--------------------------------------------------------------------
static void _ReportSystemUpTime ()
//--------------------------------------------------------------------
{
  const DWORD MSECS_S = 1000;
  const DWORD MSECS_M = MSECS_S * 60;
  const DWORD MSECS_H = MSECS_M * 60;
  const DWORD MSECS_D = MSECS_H * 24;

  // number of milli seconds OS is up - overflow after 49.7 days!!!
  DWORD nSystemMilliSecs = GetTickCount ();

  // determine the days
  DWORD nSystemDays = nSystemMilliSecs / MSECS_D;
  nSystemMilliSecs -= (nSystemDays * MSECS_D);

  // determine the hours
  DWORD nSystemHours = nSystemMilliSecs / MSECS_H;
  nSystemMilliSecs -= (nSystemHours * MSECS_H);

  // determine the minutes
  DWORD nSystemMins = nSystemMilliSecs / MSECS_M;
  nSystemMilliSecs -= (nSystemMins * MSECS_M);

  // determine the seconds
  DWORD nSystemSecs = nSystemMilliSecs / MSECS_S;
  nSystemMilliSecs -= (nSystemSecs * MSECS_S);

  _LOG (_T ("  System up time.....: %dd %02dh %02dm %02ds %03dms") MSG_CRLF,
                              nSystemDays,
                              nSystemHours,
                              nSystemMins,
                              nSystemSecs,
                              nSystemMilliSecs);

  _LOG (MSG_CRLF);
}

//--------------------------------------------------------------------
static void _ReportRegisters (PCONTEXT pCtx)
//--------------------------------------------------------------------
{
  // Show the registers
  if (pCtx)
  {
    _LOG (_T ("  Registers:") MSG_CRLF
                                _T ("    EAX: %08Xh") MSG_CRLF
                                _T ("    EBX: %08Xh") MSG_CRLF
                                _T ("    ECX: %08Xh") MSG_CRLF
                                _T ("    EDX: %08Xh") MSG_CRLF
                                _T ("    ESI: %08Xh") MSG_CRLF
                                _T ("    EDI: %08Xh") MSG_CRLF,
                                pCtx->Eax,
                                pCtx->Ebx,
                                pCtx->Ecx,
                                pCtx->Edx,
                                pCtx->Esi,
                                pCtx->Edi);

    _LOG (_T ("    CS:EIP %04Xh:%08Xh") MSG_CRLF, pCtx->SegCs, pCtx->Eip);
    _LOG (_T ("    SS:ESP %04Xh:%08Xh  EBP:%08Xh") MSG_CRLF, pCtx->SegSs, pCtx->Esp, pCtx->Ebp);
    _LOG (_T ("    DS:%04Xh  ES:%04Xh  FS:%04Xh  GS:%04Xh") MSG_CRLF, pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs);
    _LOG (_T ("    Flags: %08Xh") MSG_CRLF, pCtx->EFlags);
  }
  else
  {
    _LOG (_T ("  Could not list registers - NULL context") MSG_CRLF);
  }

  _LOG (MSG_CRLF);
}

//--------------------------------------------------------------------
static void _ReportPossibleCrashReason (PCONTEXT pCtx)
//--------------------------------------------------------------------
{
  // try to find out some possible reasons... ;-)
  if ((pCtx->Ecx & 0xffff0000) == 0xcccc0000)
  {
    // new automatic object space is filled with 0xcccccccc if /Gz is specified
    _LOG (_T ("  Possible reason: accessing uninitialized automatic object (ECX)") MSG_CRLF);
  }

  if ((pCtx->Ecx & 0xffff0000) == 0xcdcd0000)
  {
    // new dynamic object space is filled with 0xcdcdcdcd
    _LOG (_T ("  Possible reason: accessing uninitialized dynamic object (ECX)") MSG_CRLF);
  }

  if ((pCtx->Ecx & 0xffff0000) == 0xdddd0000)
  {
    // deleted dynamic space is filled with 0xdddddd
    _LOG (_T ("  Possible reason: accessing deleted dynamic object (ECX)") MSG_CRLF);
  }

  if ((pCtx->Ecx & 0xffff0000) == 0xfdfd0000)
  {
    // No man's land is filled with 0xfdfdfdfd
    _LOG (_T ("  Possible reason: accessing no-man's land (ECX)") MSG_CRLF);
  }

  if ((pCtx->Ecx & 0xfffff000) == 0x00000000)
  {
    // invalid objects are around 0x00000000
    _LOG (_T ("  Possible reason: accessing NULL pointer object (ECX)") MSG_CRLF);
  }

  if ((pCtx->Eax & 0xffff0000) == 0xcccc0000)
  {
    // new automatic object space is filled with 0xcccccccc if /Gz is specified
    _LOG (_T ("  Possible reason: accessing uninitialized automatic object (EAX)") MSG_CRLF);
  }

  if ((pCtx->Eax & 0xfffff000) == 0x00000000)
  {
    _LOG (_T ("  Possible reason: invalid function pointer???? (EAX)") MSG_CRLF);
  }

  _LOG (MSG_CRLF);
}

//--------------------------------------------------------------------
static void _ReportDebugHelpSearchPath ()
//--------------------------------------------------------------------
{
  // print symbol search path
  const DWORD SYMSEARCHPATH_BUFLEN = 2048;

  char sCurrentSymSearchPath[SYMSEARCHPATH_BUFLEN + 1];
  memset (sCurrentSymSearchPath, 0, sizeof (sCurrentSymSearchPath));

  if (__CheckWin32 (pSymGetSearchPath (g_hProcess, sCurrentSymSearchPath, SYMSEARCHPATH_BUFLEN)))
  {
    // always char!
    _LOG (_T ("  Symbols are searched in: '%hs'") MSG_CRLF, sCurrentSymSearchPath);
  }
  _LOG (MSG_CRLF);
}

//--------------------------------------------------------------------
BOOL CALLBACK __SymbolModulsEnumCallback
#if (defined _MSC_VER) && (_MSC_VER >= 1500)
                                        (PCSTR pModuleName,
#else
                                        (PSTR pModuleName,
#endif
                                         DWORD64 nBaseOfDll,
                                         PVOID pUserContext)
//--------------------------------------------------------------------
{
  {
    IMAGEHLP_MODULE64 aInfo;
    ZeroMemory (&aInfo, sizeof (aInfo));
    aInfo.SizeOfStruct = sizeof (aInfo);

    if (__CheckWin32 (pSymGetModuleInfo (g_hProcess, nBaseOfDll, &aInfo)))
    {
      // write addr and debug type
      _LOG (_T ("    %08I64Xh %-11s"),
            aInfo.BaseOfImage,
            aInfo.SymType == SymCoff     ? _T ("COFF") :
            aInfo.SymType == SymCv       ? _T ("CodeView") :
            aInfo.SymType == SymDeferred ? _T ("deferred") :
            aInfo.SymType == SymDia      ? _T ("DIA") :
            aInfo.SymType == SymExport   ? _T ("ExportTable") :
            aInfo.SymType == SymNone     ? _T ("none") :
            aInfo.SymType == SymPdb      ? _T ("PDB") :
            aInfo.SymType == SymSym      ? _T ("SYM") : _T ("unknown"));

      // write filename
      _LOG (_T (" [%c] %12hs"),
            *aInfo.LoadedImageName
              ? _T ('L')
              : *aInfo.ImageName
                ? _T ('I')
                : _T ('M'),
            *aInfo.LoadedImageName
              ? aInfo.LoadedImageName
              : *aInfo.ImageName
                ? aInfo.ImageName
                : aInfo.ModuleName);
    }
    else
    {
      // no symbol info present
      _LOG (_T ("    %08I64Xh %hs"),
            nBaseOfDll,
            pModuleName);
    }

    _LOG (MSG_CRLF);
  }

  // continue enumeration!
  return TRUE;
}

//--------------------------------------------------------------------
static void _ReportLoadedSymbolsModules ()
//--------------------------------------------------------------------
{
  _LOG (_T ("  Symbols are loaded for the following modules:") MSG_CRLF);
  _LOG (_T ("    DLL Base  Type        Name") MSG_CRLF);

  pSymEnumerateModules (g_hProcess, __SymbolModulsEnumCallback, NULL);
  _LOG (MSG_CRLF);
}

//--------------------------------------------------------------------
static void _ReportCallStack (PCONTEXT pCtx, LPCTSTR szFaultingModule)
//--------------------------------------------------------------------
{
  if (!DebugSymbolHandler::Instance ().Init ())
  {
    // Walk the stack using x86 specific code
    _LOG (_T ("  [Start generic stack tracing]") MSG_CRLF);

    __IntelStackWalk (pCtx);
  }
  else
  {
    _LOG (_T ("  [Start ") DLLNAME _T (" stack tracing]") MSG_CRLF);

    _ReportDebugHelpSearchPath ();
//    _ReportLoadedSymbolsModules ();
    _DoDebugHelpStackWalk (pCtx, true);

//    _LOG (_T("Local Variables and parameters") MSG_CRLF);
//    _DoDebugHelpStackWalk (pCtx, true);

    if (szFaultingModule)
    {
      _LOG (_T("Global Variables") MSG_CRLF);
      pSymEnumSymbols (GetCurrentProcess (),
                       (DWORD64)GetModuleHandle (szFaultingModule),
                       0, EnumerateSymbolsCallback, 0);

      _LOG (MSG_CRLF);
    }
  }

  _LOG (_T ("  End of stack trace") MSG_CRLF);
  _LOG (MSG_CRLF);
}

//--------------------------------------------------------------------
static void _ReportExceptionInfo
                                        (PEXCEPTION_RECORD pExceptionRecord,
                                         LPTSTR szFaultingModule)
//--------------------------------------------------------------------
{
  DWORD section, offset;

  // First print information about the type of fault
  _LOG (_T ("  Exception code: %08Xh %s") MSG_CRLF,
        pExceptionRecord->ExceptionCode,
        __GetExceptionString (pExceptionRecord->ExceptionCode));

  // Now print information about where the fault occured
  __GetLogicalAddress (pExceptionRecord->ExceptionAddress,
                       szFaultingModule,
                       GT_ARRAY_SIZE (szFaultingModule),
                       section,
                       offset);

  _LOG (_T ("  Fault address:  %08Xh %04Xh:%08Xh %s") MSG_CRLF,
        pExceptionRecord->ExceptionAddress,
        section,
        offset,
        szFaultingModule);

  _LOG (MSG_CRLF);
}

//--------------------------------------------------------------------
static void _ReportCurrentAddress (PCONTEXT pCtx)
//--------------------------------------------------------------------
{
  // Now print information about where the fault occured
  TCHAR szCurrentModule[MAX_PATH + 1];
  DWORD section, offset;

  // init mem
  memset (szCurrentModule, 0, sizeof (szCurrentModule));

  __GetLogicalAddress ((LPVOID) pCtx->Eip,
                       szCurrentModule,
                       GT_ARRAY_SIZE (szCurrentModule),
                       section,
                       offset);

  _LOG (_T ("  Current address:  %08Xh %04Xh:%08Xh %s") MSG_CRLF,
                              pCtx->Eip,
                              section,
                              offset,
                              szCurrentModule);

  _LOG (MSG_CRLF);
}

//--------------------------------------------------------------------
void GenerateExceptionReport (PEXCEPTION_POINTERS pExceptionInfo)
//--------------------------------------------------------------------
{
  PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

  TCHAR szFaultingModule[MAX_PATH];
  ZeroMemory (szFaultingModule, sizeof (szFaultingModule));

  _ReportHeader (_T ("GT2 Crash dump"));
  _ReportWinVersion ();
  _ReportDirectories ();
  _ReportSystemTime ();
  _ReportModuleInfo ();
//  _ReportLoadedModules ();
  _ReportEnvVars ();
  _ReportApplicationUpTime ();
  _ReportSystemUpTime ();
  _ReportExceptionInfo (pExceptionRecord, szFaultingModule);
  _ReportRegisters (pExceptionInfo->ContextRecord);
  _ReportPossibleCrashReason (pExceptionInfo->ContextRecord);

  if (bDoStackTrace)
  {
    _ReportCallStack (pExceptionInfo->ContextRecord, szFaultingModule);
  }
  else
  {
    _LOG (_T ("# stack tracing is disabled!") MSG_CRLF);
  }

  // write small "done" note
  _LOG (_T ("[DONE]") MSG_CRLF);
  _LOG (MSG_CRLF);
}

//--------------------------------------------------------------------
// Entry point where control comes on an unhandled exception
//--------------------------------------------------------------------
LONG WINAPI _GeneralExceptionHandler (PEXCEPTION_POINTERS pExceptionInfo)
//--------------------------------------------------------------------
{
  TCHAR sMsg[512];
  bool bOpenedFile = false;
  static bool s_bDone = false;

  // do not protocol breakpoints
  if (pExceptionInfo->ExceptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT &&
      !s_bDone)
  {
    fprintf (stderr, "Writing a crash dump\n");

    // set waiting cusor!
    LoadCursor (NULL, IDC_WAIT);

    // open the logfile - if it does not exist, create it (OPEN_ALWAYS)!!
    if (!Win32File::Instance ().IsOpen ())
    {
      if (Win32File::Instance ().OpenForCrash ())
      {
        // go to the end of the file - append!
        Win32File::Instance ().MoveToEnd ();

        bOpenedFile = true;
      }
      else
      {
        // failed to open file -> show msg!
        wsprintf (sMsg, _T ("Unable to create dump file\n")
                        _T ("  '%s'\n")
                        _T ("Errorcode %d!\n\n")
                        _T ("Maybe a dump file was created!! - check the file anyway!"),
                        Win32File::Instance ().GetName (),
                        GetLastError ());

        // reset the mouse cursor to the arrow
        LoadCursor (NULL, IDC_ARROW);

        // Show msg box
        MessageBox (NULL,
                    sMsg,
                    MSG_TITLE,
                    MB_OK | MB_ICONERROR | MB_APPLMODAL);
      }
    }

    if (Win32File::Instance ().IsOpen ())
    {
      // do it!
      GenerateExceptionReport (pExceptionInfo);

      // cleanup symbols
      if (DebugSymbolHandler::Instance ().UnInit ())
      {
        _LOG (_T ("  [Cleaned up symbols]") MSG_CRLF);
      }

      if (bOpenedFile)
      {
        // and close it!
        Win32File::Instance ().Close ();
      }

      if (bDoStackTrace)
      {
        _ftprintf (stderr, _T ("A dump file was written to '%s'\n"), Win32File::Instance ().GetName ());

        // show the messagebox only if a dump file was possibly created!
        wsprintf (sMsg, _T ("A dump file was written to\n")
                        _T ("  '%s'\n"),
                        Win32File::Instance ().GetName ());

        // reset the mouse cursor to the arrow
        LoadCursor (NULL, IDC_ARROW);

        MessageBox (NULL,
                    sMsg,
                    MSG_TITLE,
                    MB_OK | MB_ICONERROR | MB_APPLMODAL);
      }
    }

    // next one should be encrypted again!
    Win32File::Instance ().EnableEncryption (true);

    // reset cusor!
    LoadCursor (NULL, IDC_ARROW);

    fprintf (stderr, "done\n");
    s_bDone = true;
  }
  else
  {
    // it's a user breakpoint (int 3)
    // -> don't do anything
//    return EXCEPTION_CONTINUE_EXECUTION;
  }

  // call previous handler (if any)
  if (g_pPreviousExceptionFilter)
    return g_pPreviousExceptionFilter (pExceptionInfo);
  else
    return EXCEPTION_CONTINUE_SEARCH;
}

//--------------------------------------------------------------------
static void __DoGenerateMiniDump (PCONTEXT pContext)
//--------------------------------------------------------------------
{
  _ReportHeader (_T ("GT2 Mini Dump"));
  _ReportWinVersion ();
  _ReportDirectories ();
  _ReportSystemTime ();
  _ReportModuleInfo ();
//  _ReportLoadedModules ();
  _ReportEnvVars ();
  _ReportApplicationUpTime ();
  _ReportSystemUpTime ();
  _ReportCurrentAddress (pContext);
  _ReportRegisters (pContext);
  _ReportCallStack (pContext, NULL);

  // write small "done" note
  _LOG (_T ("[DONE]") MSG_CRLF);
  _LOG (MSG_CRLF);
}

//--------------------------------------------------------------------
struct ThreadContextInfo
//--------------------------------------------------------------------
{
  HANDLE hDesiredThread;
  HANDLE hEvent;
};

//--------------------------------------------------------------------
static DWORD CALLBACK __ThreadFct_DumpOtherThread (LPVOID lpParam)
//--------------------------------------------------------------------
{
  // init CONTEXT struct
  CONTEXT aContext;
  memset (&aContext, 0, sizeof (aContext));
  aContext.ContextFlags = CONTEXT_FULL |
                          CONTEXT_DEBUG_REGISTERS |
                          CONTEXT_EXTENDED_REGISTERS;

  // get the HANDLE to the calling thread which is passed as parameter
  ThreadContextInfo *pInfo = (ThreadContextInfo*) lpParam;

  // suspend the calling thread, retrieve it's context and resume it!
  if (SuspendThread (pInfo->hDesiredThread) != DWORD (-1))
  {
    if (GetThreadContext (pInfo->hDesiredThread, &aContext))
    {
      // the mini dump must be created while the thread is suspended!!!
      __DoGenerateMiniDump (&aContext);

      // now we can restart the thread!
      if (ResumeThread (pInfo->hDesiredThread) == DWORD (-1))
      {
        MessageBox (NULL, _T ("ResumeThread failed"), _T ("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
      }
    }
    else
    {
      MessageBox (NULL, _T ("GetThreadContext failed"), _T ("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
    }
  }
  else
  {
    MessageBox (NULL, _T ("SuspendThread failed"), _T ("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
  }

  // we did it -> signal it!
  SetEvent (pInfo->hEvent);

  // we don't need thís thread anymore
  ExitThread (0);
  return 0;
}

/*! Retrieve the context of the thread specified by the given thread ID!
    Important note: We cannot use the HANDLE returned by GetCurrentThread
      because this is a pseudo handle which always uses the current thread
      no matter which thread is used! Therefore we have to call
      DuplicateHandle to get the real thread ID!
 */
//--------------------------------------------------------------------
static bool __dumpThisThread ()
//--------------------------------------------------------------------
{
  HANDLE hGetContextThread;
  DWORD nGetContextThreadID;
  HANDLE hEvent;
  HANDLE hThisThread;
  ThreadContextInfo aTCI;
  bool bResult = true;

  // create event which signals when the thread has finished!
  hEvent = CreateEvent (NULL,                         // no security attributes
                        false,                        // manual reset?
                        false,                        // initial state: not signaled
                        _T ("__dumpThisThread"));  // name of the event
  if (hEvent)
  {
    // get handle of this thread (don't forget to close it)!
    if (DuplicateHandle (g_hProcess,              // source process
                         GetCurrentThread (),     // source handle
                         g_hProcess,              // target process
                         &hThisThread,            // target handle
                         0,                       // access mode (ignored because of DUPLICATE_SAME_ACCESS)
                         true,                    // inheritable
                         DUPLICATE_SAME_ACCESS))  // options
    {
      // init structure
      aTCI.hDesiredThread = hThisThread;
      aTCI.hEvent         = hEvent;

      // create the new thread
      hGetContextThread = CreateThread (NULL,                        // no security attributes
                                        0,                           // use default stack size
                                        __ThreadFct_DumpOtherThread, // thread function
                                        &aTCI,                       // argument to thread function
                                        0,                           // use default creation flags
                                        &nGetContextThreadID);       // returns the thread identifier
      if (hGetContextThread)
      {
        // wait until the signal from the thread is called
        DWORD nWaitState;
        do
        {
          // wait for 30 sec (=30 000 milli seconds)
          nWaitState = WaitForSingleObject (hEvent, 30000);

#ifdef _DEBUG
          if (nWaitState == WAIT_TIMEOUT)
          {
            if (MessageBox (NULL, _T ("DumpThread: WaitForSingleObject returned WAIT_TIMEOUT!\nContinue waiting?"), _T ("[Debug message by PH]"), MB_YESNO) == IDNO)
              nWaitState = WAIT_OBJECT_0;
          }
          else if (nWaitState == WAIT_FAILED)
          {
            MessageBox (NULL, _T ("DumpThread: WaitForSingleObject returned WAIT_FAILED!?"), _T ("[Debug message by PH]"), MB_OK);
            nWaitState = WAIT_OBJECT_0;
          }
          else if (nWaitState == WAIT_ABANDONED)
          {
            MessageBox (NULL, _T ("DumpThread: WaitForSingleObject returned WAIT_ABANDONED!"), _T ("[Debug message by PH]"), MB_OK);
            nWaitState = WAIT_OBJECT_0;
          }
#endif
        } while (nWaitState != WAIT_OBJECT_0);

        // close thread handle
        CloseHandle (hGetContextThread);
      }
      else
      {
        MessageBox (NULL, _T ("CreateThread failed"), _T ("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
        bResult = false;
      }

      // we have to close the duplicated handle!!
      CloseHandle (hThisThread);
    }
    else
    {
      MessageBox (NULL, _T ("DuplicateHandle failed"), _T ("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
      bResult = false;
    }

    // close event - not used anymore
    CloseHandle (hEvent);
  }
  else
  {
    MessageBox (NULL, _T ("CreateEvent failed"), _T ("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL);
    bResult = false;
  }

  // and leave function
  return bResult;
}

/*! Creates a small dump file :)
 */
//--------------------------------------------------------------------
EDumpEC GT_CALL Fct_CreateMiniDump
                                        (LPCTSTR sFile,
                                         DWORD   nLine)
//--------------------------------------------------------------------
{
  bool bOpenedFile = false;

  // open the logfile - if it does not exist, create it (OPEN_ALWAYS)!!
  if (!Win32File::Instance ().IsOpen ())
  {
    if (!Win32File::Instance ().OpenForDump (g_sNextLogFileName))
    {
      // failed to open file :(
      return DUMP_FILEOPEN_ERROR;
    }

    // go to the end of the file - append!
    Win32File::Instance ().MoveToEnd ();

    // remember that we opened the file!
    bOpenedFile = true;
  }

  _LOG (_T ("# === [Mini Dump for %s line %d]") MSG_CRLF,
                              sFile, nLine);

  if (!__dumpThisThread ())
  {
    _LOG (_T ("# [failed to create the minidump - EC %d]") MSG_CRLF,
                                GetLastError ());
  }

  // and close it!
  if (bOpenedFile)
  {
    Win32File::Instance ().Close ();
  }

  // next one should be encrypted again!
  Win32File::Instance ().EnableEncryption (true);

  // all done!
  return DUMP_SUCCESS;
}

//--------------------------------------------------------------------
void GT_CALL Fct_SetMiniDumpFileName
                                        (LPCTSTR sDumpFileName)
//--------------------------------------------------------------------
{
  // check :)
  _ASSERTE (sDumpFileName && *sDumpFileName);

  // ignore empty values!
  if (sDumpFileName && *sDumpFileName)
  {
    // check maximum length!
    _ASSERTE (_tcslen (sDumpFileName) < MAX_PATH);

    // copy string
    _tcsncpy (g_sNextLogFileName, sDumpFileName, MAX_PATH);
    g_sNextLogFileName[MAX_PATH] = _T ('\0');

    // next dump is not encrypted!
    Win32File::Instance ().EnableEncryption (false);
  }
  else
  {
    // another dumpfile name was set, but no dumpfile was ever created!
    _tcscpy (g_sNextLogFileName, _T (""));
  }
}

//--------------------------------------------------------------------
void GT_CALL Fct_EnableDumpFileCreation ()
//--------------------------------------------------------------------
{
  /* empty */
}

//--------------------------------------------------------------------
static void __init ()
//--------------------------------------------------------------------
{
  // Install the unhandled exception filter function
  g_pPreviousExceptionFilter = SetUnhandledExceptionFilter (_GeneralExceptionHandler);

  // and determine, how the logfile should be named!
  Win32File::Instance ().BuildName ();
}

//--------------------------------------------------------------------
static void __done ()
//--------------------------------------------------------------------
{
  // uninit DebugHelp
  DebugSymbolHandler::Instance ().UnInit ();

  // restore previous handler!
  SetUnhandledExceptionFilter (g_pPreviousExceptionFilter);
}

void GT_CALL Fct_InitDumpfileForEXE ()
{
  __init ();
}

void GT_CALL Fct_CleanupDumpfileForEXE ()
{
  __done ();
}

// only if we're building a DLL
#ifdef _DLL

// Must be 'extern "C"' for GCC!
//--------------------------------------------------------------------
extern "C" BOOL WINAPI DllMain
                                        (HINSTANCE hinstDLL,      // handle to the DLL module
                                         DWORD     fdwReason,     // reason for calling function
                                         LPVOID    lpvReserved)   // reserved
//--------------------------------------------------------------------
{
  switch (fdwReason)
  {
    case DLL_PROCESS_ATTACH:
    {
      __init ();
      break;
    }
    case DLL_PROCESS_DETACH:
    {
      __done ();
      break;
    }
  }

  return true;
}
#endif
