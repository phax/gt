/*!
  \file
  \note Copyright\n
        BOC AdoXX\n
        (C) COPYRIGHT BOC - Business Objectives Consulting 1995 - 2003\n
        All Rights Reserved\n
        Use, duplication or disclosure restricted by BOC ITC\n
        Vienna, 1995 - 2003\n
        Microsoft Systems Journal, May 1997
  \brief Dumpfile genrator method
  \author Matt Pietrek
  \author Philip Helger

  Contains code to generate a dumpfile on Win32 platforms.
  A dumpfile contains a callstack and many more things :)
*/

#ifndef _GT_CRASHTRACKER_HXX_
#define _GT_CRASHTRACKER_HXX_

#ifdef _MSC_VER
#pragma message ("  " __FILE__)
#pragma once
#endif

#include <windows.h>

/* This file is not in a namespace because it is dynamically loaded
   upon application startup and never explicitly linked!
 */

#ifdef __cplusplus
extern "C" {
#endif

/*! Dump generation error code.
 `*/
//--------------------------------------------------------------------
enum EDumpEC
//--------------------------------------------------------------------
{
  DUMP_SUCCESS = 88,
  DUMP_FILEOPEN_ERROR,
  DUMP_WIN32_ERROR,
};

//! create a dumpfile on demand from the current thread
EDumpEC GT_CALL Fct_CreateMiniDump (LPCTSTR sFile, DWORD nLine);

//! call this to set the name of the minidump file ONLY for the next dump!! - The data will be appended!
void GT_CALL Fct_SetMiniDumpFileName (LPCTSTR sDumpFileName);

//! just a dummy - needed to statically link against this DLL
void GT_CALL Fct_EnableDumpFileCreation ();

void GT_CALL Fct_InitDumpfileForEXE ();
void GT_CALL Fct_CleanupDumpfileForEXE ();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
