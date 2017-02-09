#ifndef _GTC_DATATYPES_HXX_
#define _GTC_DATATYPES_HXX_

#ifdef _MSC_VER
#pragma once
#endif

// forward declare the structs
struct _C2EEntry;
struct _EXEEntry;

// declare special function pointer
typedef LPCTSTR              (*FARPROC_CHARPTR) ();
typedef int                  (*FARPROC_INT) ();
typedef struct _C2EEntry*    (*FARPROC_C2E) (const int);
typedef struct _EXEEntry*    (*FARPROC_EXE) (const int);
typedef struct _ExtEXEEntry* (*FARPROC_EXTEXE) (const int);

//--------------------------------------------------------------------
enum EFileTypes
//--------------------------------------------------------------------
{
  COM = 0,
  EXE,
  COM_EXE,
  PE_EXE,
  LE_EXE,
  NE_EXE,
  SYS
};

/*! Note: must start with 0 because it is used as array index!
 */
//--------------------------------------------------------------------
enum ENameTypes
//--------------------------------------------------------------------
{
  NORMAL = 0,
  PACKER,          // save space
  ENCRYPTER,       // encrypt
  COMPILER,        // converted from source to binary
  LINKER,          // linked by (unknown compiler)
  CONVERTER,       // converted from COM <-> EXE
  PROTECTOR,       // encrypted and binary checks
  STICKER,         // attached code to EXE
  EXTENDER,        // DOS Extended
  PASSWORD,        // Password protected by
  MODIFIER,        // generic: modified by
  _LAST_TYPE
};

//--------------------------------------------------------------------
typedef struct _C2EEntry
//--------------------------------------------------------------------
{
  LPTSTR  sName;
  gtuint8 aData[24];
} C2EEntry;

//--------------------------------------------------------------------
typedef struct _EXEEntry
//--------------------------------------------------------------------
{
        gtuint8    nDataLen;
  const gtuint8*   pData;
        EFileTypes nFileType;
        ENameTypes nNameType;
        LPCTSTR    sName;
} EXEEntry;

typedef EXEEntry SYSEntry;

/* it is not allowed to have const arrays of derived classes
   therefore the datamembers are copied and extended by the macro field :(
 */
//--------------------------------------------------------------------
typedef struct _ExtEXEEntry
//--------------------------------------------------------------------
{
        gtuint8    nDataLen;    //!< length of the data
  const gtuint8*   pData;       //!< the data itself
        EFileTypes nFileType;   //!< appropriate for which type of executable
        ENameTypes nNameType;   //!< what kind of program is it?
        LPCTSTR    sName;       //!< name of the program
        LPCTSTR    sMacro;      //!< special macro to use
} ExtEXEEntry;

const gtuint32 GT_FIXED_EXT_FLAG_CAN_DETECT    = 0x0001;  //!< content can be detected
const gtuint32 GT_FIXED_EXT_FLAG_SHOULD_DETECT = 0x0002;  //!< content should be detected
const gtuint32 GT_FIXED_EXT_FLAG_FILEXT_COM    = 0x8000;  //!< ripped from filext.com

//--------------------------------------------------------------------
typedef struct _FixedExtensionEntry
//--------------------------------------------------------------------
{
  LPCTSTR  m_sExtension;    /* mixedcase */
  LPCTSTR  m_sDescription;  /* resource ID - without '\n' */
  gtuint32 m_nFlags;        /* flags - see above GT_FIXED_EXT_FLAG* bit fields */
} FixedExtensionEntry;

//--------------------------------------------------------------------
typedef struct _FixedNameEntry
//--------------------------------------------------------------------
{
  LPCTSTR m_sDirectory;    /* Can be NULL!, mixedcase (stricmp on Win) */
  LPCTSTR m_sFilename;     /* mixedcase (stricmp on Win) */
  resid_t m_sDescription;  /* resource ID - without '\n' */
  LPCTSTR m_sURL;          /* URL of the entry */
} FixedNameEntry;

#endif
