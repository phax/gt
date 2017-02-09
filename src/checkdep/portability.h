#ifndef _CHECKDEP_PORTABILITY_H_
#define _CHECKDEP_PORTABILITY_H_

#ifdef _WIN32

#include <crtdbg.h>
#include <direct.h>
#include <windows.h>
#include <tchar.h>

#define ASSERT _ASSERTE

#else  // _WIN32

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// for file searching
#include <dirent.h>
#include <fnmatch.h>
#include <stdlib.h>

// windows types
#define CONST const
typedef int BOOL;
typedef void VOID;
typedef VOID* LPVOID;
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef UINT* UINT_PTR;
typedef long LONG;
typedef unsigned long DWORD;

// windows constants
#define MAX_PATH  260

// assertions
#define _ASSERT  assert
#define _ASSERTE assert
#define ASSERT   assert

// calling conventions
#define __cdecl
#define __stdcall
#define __fastcall

// string stuff
#define _alloca           alloca
#define _chdir            chdir
#define _itoa             itoa
#define _ltoa             ltoa
#define _mkdir(x)         mkdir (x, S_IRWXU | S_IRWXG | S_IRWXO)
#define _strdec(p1,p2)    ((p1) >= (p2) ? 0L : (p2) - 1)
#define _strdup           strdup
#define _stricmp          strcasecmp
#define _strinc(p)        ((p) + 1)
#define _strlwr           strlwr
#define _strnicmp         strncasecmp
#define _strninc(p,n)     ((p) + (n))
#define _strupr           strupr

// tchar compatibility
#define _T(x)           x
typedef char            TCHAR;
typedef TCHAR*          LPTSTR;
typedef const TCHAR*    LPCTSTR;


#define _fputtc         fputc
#define _fputts         fputs
#define _ftprintf       fprintf
#define _itot           _itoa
#define _ltot           _ltoa
#define _sntprintf      snprintf
#define _stprintf       sprintf
#define _tcscat         strcat
#define _tcschr         strchr
#define _tcscmp         strcmp
#define _tcscpy         strcpy
#define _tcsdec         _strdec
#define _tcsicmp        _stricmp
#define _tcsinc         _strinc
#define _tcslen         strlen
#define _tcslwr         strlwr
#define _tcsncmp        strncmp
#define _tcsncpy        strncpy
#define _tcsnicmp       _strnicmp
#define _tcsninc        _strninc
#define _tcsnset        memset
#define _tcspbrk        strpbrk
#define _tcsrchr        strrchr
#define _tcsstr         strstr
#define _tcsupr         strupr
#define _tgetenv        getenv
#define _tmain          main
#define _tprintf        printf
#define _ttoi           atoi
#define _ttol           atol
#define _ultot          _ultoa
#define _vsntprintf     vsnprintf

inline void itoa (const int n, char* buf, int radix)
{
  assert (radix == 10);
  _stprintf (buf, "%d", n);
}

inline void ltoa (const long n, char* buf, int radix)
{
  assert (radix == 10);
  _stprintf (buf, "%ld", n);
}

// not contained in RTL
inline char* strlwr (char* p)
{
  char* po = p;
  for (; *p; ++p)
    *p = tolower (*p);
  return po;
}

// not contained in RTL
inline char* strupr (char* p)
{
  char* po = p;
  for (; *p; ++p)
    *p = toupper (*p);
  return po;
}

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif

// Windows SDK memory checks
#define IsBadCodePtr(x)       (0)
#define IsBadReadPtr(x,y)     (0)
#define IsBadStringPtr(x,y)   (0)
#define IsBadWritePtr(x,y)    (0)
#define GetConsoleTitle(x,y)  (*(x) = '\0')
#define SetConsoleTitle(x)    ((void) 0)

#endif  // _WIN32

#endif  // _BOC_WIN_ON_LIN_H_
