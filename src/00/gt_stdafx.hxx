#ifndef _GT_STDAFX_HXX_
#define _GT_STDAFX_HXX_

/* check whether any valid compiler was used
   gcc check: gcc -dM -E -x c++ /dev/null
 */
#if !(defined _MSC_VER) && !(defined __GNUC__)
# error Invalid compiler used
#endif

#if (defined _MSC_VER) && (_MSC_VER < 1300)
#error Requires Microsoft Visual C++ 7.1/2003/13.00
#endif

#if (defined _MSC_VER) && (_MSC_VER >= 1400)
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif

/*
MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
MSVC++ 7.0  _MSC_VER == 1300
MSVC++ 6.0  _MSC_VER == 1200
MSVC++ 5.0  _MSC_VER == 1100
*/

#ifdef _MSC_VER
#pragma once
#pragma message ("[Compiling PCH file")
#define _ALLOW_RTCc_IN_STL
#endif

// RTL + STL
#include <cstdio>
#include <cstdlib>
#include <string>

#if (defined _MSC_VER)
#include <crtdbg.h>
#endif
#include <windows.h>
#include <tchar.h>
#include <malloc.h>

#if (defined _MSC_VER)
#pragma warning (disable:4201)  // nonstandard extension used: nameless struct/union
#pragma warning (disable:4251)  // '...' : class '...' needs to have dll-interface to be used by clients of class '...'
#pragma warning (disable:4512)  // assignment operator could not be generated
#pragma warning (disable:4514)  // unreferenced inline function has been removed
#pragma warning (disable:4710)  // function ... not inlined
#pragma warning (disable:4711)  // .. selected for automatic expansion
#pragma warning (disable:4800)  // 'type' : forcing value to bool 'true' or 'false' (performance warning)
#endif

#define DLL_API __cdecl
#define GT_CALL __cdecl
#define GT_CDECL __cdecl

#ifdef __cplusplus
#define START_EXTERN_C   extern "C" {
#define END_EXTERN_C     } //  extern "C"
#else
#define START_EXTERN_C   /**/
#define END_EXTERN_C     /**/
#endif

#ifdef _DEBUG
const int debug = 1;
#else
const int debug = 0;
#endif

#ifdef _UNICODE
const int unicode = 1;
#else
const int unicode = 0;
#endif

typedef char             gtint8;
typedef unsigned char    gtuint8;
typedef short            gtint16;
typedef unsigned short   gtuint16;
typedef long             gtint32;
typedef unsigned long    gtuint32;
typedef __int64          gtint64;
typedef unsigned __int64 gtuint64;

// the compile time assertion to use
#define GT_COMPILER_ASSERT(x)       typedef char _compile_time_assert_t [(x) ? 1 : -1]

GT_COMPILER_ASSERT (sizeof (gtint8)   == 1);
GT_COMPILER_ASSERT (sizeof (gtuint8)  == 1);
GT_COMPILER_ASSERT (sizeof (gtint16)  == 2);
GT_COMPILER_ASSERT (sizeof (gtuint16) == 2);
GT_COMPILER_ASSERT (sizeof (gtint32)  == 4);
GT_COMPILER_ASSERT (sizeof (gtuint32) == 4);
GT_COMPILER_ASSERT (sizeof (gtint64)  == 8);
GT_COMPILER_ASSERT (sizeof (gtuint64) == 8);

typedef gtint64  file_t;

typedef ULONG_PTR resid_t;

// some sanity checks
GT_COMPILER_ASSERT (sizeof (file_t) >= 8);
GT_COMPILER_ASSERT (sizeof (size_t) >= 4);

// either string or wstring
typedef std::basic_string<TCHAR> pstring;

// use std namespace
using std::string;
using std::wstring;

// global array size macro
#define GT_ARRAY_SIZE(x)                   (sizeof (x) / sizeof (x[0]))

// assertion replacement :)
#if (defined _MSC_VER)
#define ASSERT _ASSERTE
#endif

// gcc has no _ASSERTE -> use standard assert
#if (defined __GNUC__)
#include <assert.h>
#define ASSERT   assert
#define _ASSERTE assert
#endif

#ifdef _DEBUG
#define VERIFY(x) ASSERT (x)
#else
#define VERIFY(x) (x)
#endif

// declare compiler assertion if size of (s) != x
#define DECLARE_SIZEOF(n,s,x)  const size_t n = sizeof (s); GT_COMPILER_ASSERT ((n) == (x));

//--------------------------------------------------------------------
enum GT_ListResult
//--------------------------------------------------------------------
{
  GT_LIST_IDENTIFIED_CONTINUE = 42,  //!< identified, but continue...
  GT_LIST_IDENTIFIED_BREAK,          //!< identified, but do NOT continue...
  GT_LIST_NOT_IDENTIFIED,            //!< not identified
  GT_LIST_OPEN_ERROR,                //!< failed to open the file
  GT_LIST_NOT_ALLOWED                //!< listing was not allowed because of cmdline flags (e.g. /noovr)
};

// bits as uint32
//--------------------------------------------------------------------
const gtuint32 BITS [32] =
//--------------------------------------------------------------------
{
  0x00000001,
  0x00000002,
  0x00000004,
  0x00000008,
  0x00000010,
  0x00000020,
  0x00000040,
  0x00000080,
  0x00000100,
  0x00000200,
  0x00000400,
  0x00000800,
  0x00001000,
  0x00002000,
  0x00004000,
  0x00008000,
  0x00010000,
  0x00020000,
  0x00040000,
  0x00080000,
  0x00100000,
  0x00200000,
  0x00400000,
  0x00800000,
  0x01000000,
  0x02000000,
  0x04000000,
  0x08000000,
  0x10000000,
  0x20000000,
  0x40000000,
  0x80000000
};

#if (defined _MSC_VER)
#pragma message ("done]")
#endif

#endif
