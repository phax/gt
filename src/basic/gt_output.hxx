#ifndef _GT_OUTPUT_HXX_
#define _GT_OUTPUT_HXX_

#ifdef _MSC_VER
#pragma once
#endif

namespace GT {

//--------------------------------------------------------------------
enum EColor
//--------------------------------------------------------------------
{
  // it's ordered like the colors in the Win console properties
  GT_COLOR_BLACK = 0,
  GT_COLOR_BLUE,
  GT_COLOR_GREEN,
  GT_COLOR_CYAN,
  GT_COLOR_RED,
  GT_COLOR_MAGENTA,
  GT_COLOR_OLIVE,
  GT_COLOR_LIGHTGRAY,

  // like 0-7 but with intensity
  GT_COLOR_DARKGRAY,
  GT_COLOR_LIGHTBLUE,
  GT_COLOR_LIGHTGREEN,
  GT_COLOR_LIGHTCYAN,
  GT_COLOR_LIGHTRED,
  GT_COLOR_LIGHTMAGENTA,
  GT_COLOR_YELLOW,
  GT_COLOR_WHITE,

  GT_COLOR_INVALID
};

// defines for Output_FilenameStruct::m_nFlags
#define GT_OUTPUT_FILENAME_DOSTEXT   BITS[0]
#define GT_OUTPUT_FILENAME_UNIXTEXT  BITS[1]
#define GT_OUTPUT_FILENAME_MACTEXT   BITS[2]
#define GT_OUTPUT_FILENAME_BINARY    BITS[3]

// defines for Output_Settings::m_nFlags
#define GT_OUTPUT_SETTINGS_USE_LISTMODE   BITS[0]
#define GT_OUTPUT_SETTINGS_USE_NOCOLOR    BITS[1]
#define GT_OUTPUT_SETTINGS_USE_FLUSH      BITS[2]
#define GT_OUTPUT_SETTINGS_USE_PAUSE      BITS[3]
#define GT_OUTPUT_SETTINGS_USE_LINENUMS   BITS[4]
#define GT_OUTPUT_SETTINGS_USE_ALL        GT_OUTPUT_SETTINGS_USE_LISTMODE | \
                                          GT_OUTPUT_SETTINGS_USE_NOCOLOR | \
                                          GT_OUTPUT_SETTINGS_USE_FLUSH | \
                                          GT_OUTPUT_SETTINGS_USE_PAUSE | \
                                          GT_OUTPUT_SETTINGS_USE_LINENUMS

//--------------------------------------------------------------------
struct Output_FilenameStruct
//--------------------------------------------------------------------
{
  gtuint32 m_nSizeOfStruct;  /* for versioning - use sizeof (Output_FilenameStruct) */
  LPTSTR   m_sFilename;      /* name of the file */
  file_t   m_nTotalFileSize; /* total size of the file */
  file_t   m_nScanOffset;    /* where are we starting to scan? 0 == new file */
  gtuint32 m_nFlags;         /* see GT_OUTPUT_FILENAME_* */
};

//--------------------------------------------------------------------
struct Output_Settings
//--------------------------------------------------------------------
{
  gtuint32  m_nSizeOfStruct;     /* for versioning - use sizeof (Output_FilenameStruct) */
  gtuint32  m_nFlagUsage;        /* determine which members should be used - seee GT_OUTPUT_SETTINGS_USE_* */
  gtuint32  m_nFlagValues;
  gtuint32  m_nPauseAfterLines;  /* is the pause parameter used? */
  HINSTANCE m_hInstance;         /* for data transfer to a window */
  gtint32   m_nCmdShow;
};

// forward decl
class Table;

// function header
typedef void (GT_CALL  *T_out_init)      (Output_Settings*);
typedef void (GT_CALL  *T_out_done)      (void);

typedef void (GT_CALL  *T_out_setcolor)  (const EColor);
typedef void (GT_CALL  *T_out_append)    (LPCTSTR);
typedef void (GT_CDECL *T_out_format)    (LPCTSTR, ...);
typedef void (GT_CALL  *T_out_table)     (Table*);

typedef void (GT_CALL  *T_out_incindent) (void);
typedef void (GT_CALL  *T_out_decindent) (void);

//! after a new [file|level] was started
typedef void (GT_CALL  *T_out_filestart) (Output_FilenameStruct*);
//! after this [file|level] was done but before the sublevel
typedef void (GT_CALL  *T_out_filedone)  (void);
//! after this [file|level] was done and after the sublevel
typedef void (GT_CALL  *T_out_fileend)   (void);

typedef void (GT_CALL  *T_out_flush)     (void);
typedef void (GT_CALL  *T_out_status)    (LPCTSTR);


//--------------------------------------------------------------------
struct OutputFuncPtrs
//--------------------------------------------------------------------
{
  T_out_init      _init;       //!< initialize
  T_out_done      _done;       //!< shutdown
  T_out_setcolor  _setcolor;   //!< change color
  T_out_append    _append;     //!< append string
  T_out_format    _format;     //!< append formatted string
  T_out_table     _table;      //!< write a table
  T_out_incindent _incindent;  //!< increase the indentation
  T_out_decindent _decindent;  //!< decrease the indentation
  T_out_filestart _filestart;  //!< start a new file (or a new level)
  T_out_filedone  _filedone;   //!< end of one file nesting level
  T_out_fileend   _fileend;    //!< absolute end of the file
  T_out_flush     _flush;      //!< flush the output cache
  T_out_status    _status;     //!< update the status bar
};

GT_EXPORT_BASIC void                  GT_CALL SetOutputFuncPtrs (const OutputFuncPtrs& ofp);
GT_EXPORT_BASIC const OutputFuncPtrs& GT_CALL GetOutputFuncPtrs ();

// extern access method macros
#define out_init      GetOutputFuncPtrs ()._init
#define out_done      GetOutputFuncPtrs ()._done

#define out_setcolor  GetOutputFuncPtrs ()._setcolor
#define out_append    GetOutputFuncPtrs ()._append
#define out_format    GetOutputFuncPtrs ()._format
#define out_table     GetOutputFuncPtrs ()._table

#define out_incindent GetOutputFuncPtrs ()._incindent
#define out_decindent GetOutputFuncPtrs ()._decindent

#define out_filestart GetOutputFuncPtrs ()._filestart
#define out_filedone  GetOutputFuncPtrs ()._filedone
#define out_fileend   GetOutputFuncPtrs ()._fileend

#define out_flush     GetOutputFuncPtrs ()._flush
#define out_status    GetOutputFuncPtrs ()._status

// helper macros
#define out_error_append out_setcolor (eColorERROR), out_append
#define out_error_format out_setcolor (eColorERROR), out_format

#define out_info_append  out_setcolor (eColorINTERESTING), out_append
#define out_info_format  out_setcolor (eColorINTERESTING), out_format

// default color constants
#define eColorHEADER       GT_COLOR_CYAN
#define eColorERROR        GT_COLOR_LIGHTRED
#define eColorEXE          GT_COLOR_GREEN
#define eColorINTERESTING  GT_COLOR_CYAN

}  // namespace

#endif
