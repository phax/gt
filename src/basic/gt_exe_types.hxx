#ifndef _GT_EXE_TYPES_HXX_
#define _GT_EXE_TYPES_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gt_analyzer.hxx"

namespace GT {

//
// PE Section characteristics.
// [ph]: Ripped from winnt.h
//
//             SECTION_FLAG_TYPE_REG                    = 0x00000000;  // Reserved.
//             SECTION_FLAG_TYPE_DSECT                  = 0x00000001;  // Reserved.
//             SECTION_FLAG_TYPE_NOLOAD                 = 0x00000002;  // Reserved.
//             SECTION_FLAG_TYPE_GROUP                  = 0x00000004;  // Reserved.
const gtuint32 SECTION_FLAG_TYPE_NO_PAD                 = 0x00000008;  // Reserved.
//             SECTION_FLAG_TYPE_COPY                   = 0x00000010;  // Reserved.

const gtuint32 SECTION_FLAG_CNT_CODE                    = 0x00000020;  // Section contains code.
const gtuint32 SECTION_FLAG_CNT_INITIALIZED_DATA        = 0x00000040;  // Section contains initialized data.
const gtuint32 SECTION_FLAG_CNT_UNINITIALIZED_DATA      = 0x00000080;  // Section contains uninitialized data.

const gtuint32 SECTION_FLAG_LNK_OTHER                   = 0x00000100;  // Reserved.
const gtuint32 SECTION_FLAG_LNK_INFO                    = 0x00000200;  // Section contains comments or some other type of information.
//             SECTION_FLAG_TYPE_OVER                   = 0x00000400;  // Reserved.
const gtuint32 SECTION_FLAG_LNK_REMOVE                  = 0x00000800;  // Section contents will not become part of image.
const gtuint32 SECTION_FLAG_LNK_COMDAT                  = 0x00001000;  // Section contents comdat.
//                                                      = 0x00002000;  // Reserved.
//             SECTION_FLAG_MEM_PROTECTED - Obsolete    = 0x00004000;
const gtuint32 SECTION_FLAG_NO_DEFER_SPEC_EXC           = 0x00004000;  // Reset speculative exceptions handling bits in the TLB entries for this section.
const gtuint32 SECTION_FLAG_GPREL                       = 0x00008000;  // Section content can be accessed relative to GP
const gtuint32 SECTION_FLAG_MEM_FARDATA                 = 0x00008000;
//             SECTION_FLAG_MEM_SYSHEAP  - Obsolete     = 0x00010000;
const gtuint32 SECTION_FLAG_MEM_PURGEABLE               = 0x00020000;
const gtuint32 SECTION_FLAG_MEM_16BIT                   = 0x00020000;
const gtuint32 SECTION_FLAG_MEM_LOCKED                  = 0x00040000;
const gtuint32 SECTION_FLAG_MEM_PRELOAD                 = 0x00080000;

const gtuint32 SECTION_FLAG_ALIGN_1BYTES                = 0x00100000;  //
const gtuint32 SECTION_FLAG_ALIGN_2BYTES                = 0x00200000;  //
const gtuint32 SECTION_FLAG_ALIGN_4BYTES                = 0x00300000;  //
const gtuint32 SECTION_FLAG_ALIGN_8BYTES                = 0x00400000;  //
const gtuint32 SECTION_FLAG_ALIGN_16BYTES               = 0x00500000;  // Default alignment if no others are specified.
const gtuint32 SECTION_FLAG_ALIGN_32BYTES               = 0x00600000;  //
const gtuint32 SECTION_FLAG_ALIGN_64BYTES               = 0x00700000;  //
const gtuint32 SECTION_FLAG_ALIGN_128BYTES              = 0x00800000;  //
const gtuint32 SECTION_FLAG_ALIGN_256BYTES              = 0x00900000;  //
const gtuint32 SECTION_FLAG_ALIGN_512BYTES              = 0x00A00000;  //
const gtuint32 SECTION_FLAG_ALIGN_1024BYTES             = 0x00B00000;  //
const gtuint32 SECTION_FLAG_ALIGN_2048BYTES             = 0x00C00000;  //
const gtuint32 SECTION_FLAG_ALIGN_4096BYTES             = 0x00D00000;  //
const gtuint32 SECTION_FLAG_ALIGN_8192BYTES             = 0x00E00000;  //
// Unused                                               = 0x00F00000;
const gtuint32 SECTION_FLAG_ALIGN_MASK                  = 0x00F00000;

const gtuint32 SECTION_FLAG_LNK_NRELOC_OVFL             = 0x01000000;  // Section contains extended relocations.
const gtuint32 SECTION_FLAG_MEM_DISCARDABLE             = 0x02000000;  // Section can be discarded.
const gtuint32 SECTION_FLAG_MEM_NOT_CACHED              = 0x04000000;  // Section is not cachable.
const gtuint32 SECTION_FLAG_MEM_NOT_PAGED               = 0x08000000;  // Section is not pageable.
const gtuint32 SECTION_FLAG_MEM_SHARED                  = 0x10000000;  // Section is shareable.
const gtuint32 SECTION_FLAG_MEM_EXECUTE                 = 0x20000000;  // Section is executable.
const gtuint32 SECTION_FLAG_MEM_READ                    = 0x40000000;  // Section is readable.
const gtuint32 SECTION_FLAG_MEM_WRITE                   = 0x80000000;  // Section is writeable.


#ifdef _DEBUG

//------------------------------------------------------------------
class rva_t
//------------------------------------------------------------------
{
public:
  typedef rva_t self_t;
  typedef gtuint32 data_t;

private:
  data_t m_nValue;

public:
  rva_t ()
    : m_nValue (0xffffffff)
  {}

  explicit rva_t (const data_t& nValue)
    : m_nValue (nValue)
  {}

  rva_t (const self_t& rhs)
    : m_nValue (rhs.m_nValue)
  {}

  self_t& operator = (const self_t& rhs)
  {
    m_nValue = rhs.m_nValue;
    return *this;
  }

  void operator += (const data_t n) { m_nValue += n; }
  void operator -= (const data_t n) { m_nValue -= n; }

  self_t operator + (const self_t rhs) const { return self_t (m_nValue + rhs.m_nValue); }
  self_t operator - (const self_t rhs) const { return self_t (m_nValue - rhs.m_nValue); }

  self_t operator + (const data_t rhs) const { return self_t (m_nValue + rhs); }
  self_t operator - (const data_t rhs) const { return self_t (m_nValue - rhs); }

  data_t operator & (const data_t rhs) const { return m_nValue & rhs; }

  bool operator <  (const self_t& rhs) const { return m_nValue <  rhs.m_nValue; }
  bool operator <= (const self_t& rhs) const { return m_nValue <= rhs.m_nValue; }
  bool operator >  (const self_t& rhs) const { return m_nValue >  rhs.m_nValue; }
  bool operator >= (const self_t& rhs) const { return m_nValue >= rhs.m_nValue; }
  bool operator == (const self_t& rhs) const { return m_nValue == rhs.m_nValue; }
  bool operator != (const self_t& rhs) const { return m_nValue != rhs.m_nValue; }

  data_t data () const { return m_nValue; }
};

inline rva_t::data_t RVA_VAL (const rva_t& x) { return x.data (); }

GT_COMPILER_ASSERT (sizeof (rva_t) == 4);

#else

typedef gtuint32 rva_t;
#define RVA_VAL(x) (x)

#endif

// used in PE EXE handler
inline bool  GT_ISSET_BIT_0   (rva_t n) { return n & 0x80000000 ? true : false; }
inline rva_t GT_UPPER_31_BITS (rva_t n) { return rva_t (n & 0x7FFFFFFF); }

#define EXE_PARAGRAPH_TO_BYTE(i)       gtuint32 ((i) << 4)
#define EXE_BYTE_TO_PARAGRAPH(i)       gtuint16 ((i) >> 4)

#define EXE_BUFFER_SIZE                (80)

// PE EXE data direcotry entries
#define EXE_PE_DIRECTORY_EXPORT         0
#define EXE_PE_DIRECTORY_IMPORT         1
#define EXE_PE_DIRECTORY_RESOURCE       2
#define EXE_PE_DIRECTORY_EXCEPTION      3
#define EXE_PE_DIRECTORY_SECURITY       4
#define EXE_PE_DIRECTORY_FIXUPS         5
#define EXE_PE_DIRECTORY_DEBUG          6
#define EXE_PE_DIRECTORY_DESCRIPTION    7
#define EXE_PE_DIRECTORY_GLOBALPTR      8
#define EXE_PE_DIRECTORY_TLSORT         9
#define EXE_PE_DIRECTORY_LOADCONFIG    10

// VERSIONINFO.fileflags
#define EXE_PE_VS_FF_DEBUG        0x00000001L
#define EXE_PE_VS_FF_PRERELEASE   0x00000002L
#define EXE_PE_VS_FF_PATCHED      0x00000004L
#define EXE_PE_VS_FF_PRIVATEBUILD 0x00000008L
#define EXE_PE_VS_FF_INFOINFERRED 0x00000010L
#define EXE_PE_VS_FF_SPECIALBUILD 0x00000020L

// VERSIONINFO.fileos
#define EXE_PE_VOS_UNKNOWN             0x00000000L
#define EXE_PE_VOS__WINDOWS16          0x00000001L
#define EXE_PE_VOS__PM16               0x00000002L
#define EXE_PE_VOS__PM32               0x00000003L
#define EXE_PE_VOS__WINDOWS32          0x00000004L
#define EXE_PE_VOS_DOS                 0x00010000L
#define EXE_PE_VOS_DOS_WINDOWS16       0x00010001L
#define EXE_PE_VOS_DOS_WINDOWS32       0x00010004L
#define EXE_PE_VOS_OS216               0x00020000L
#define EXE_PE_VOS_OS216_PM16          0x00020002L
#define EXE_PE_VOS_OS232               0x00030000L
#define EXE_PE_VOS_OS232_PM32          0x00030003L
#define EXE_PE_VOS_NT                  0x00040000L
#define EXE_PE_VOS_NT_WINDOWS32        0x00040004L
#define EXE_PE_VOS_WINCE               0x00050000L
// C:\Programme\Adobe\Acrobat 8.0\Acrobat\AdobeCollabSync.exe has 0x00050004L!

// NE segment flags
const int NE_SEG_IS_DATA            = 0x0001;
const int NE_SEG_ALREADY_ALLOCATED  = 0x0002;
const int NE_SEG_LOADED             = 0x0004;
/* - 0x0008  - reserved - */
const int NE_SEG_MOVEABLE           = 0x0010;
const int NE_SEG_SHAREABLE          = 0x0020;
const int NE_SEG_PRELOAD            = 0x0040;
const int NE_SEG_EXECUTE_ONLY       = 0x0080;  /* - if CODE - */
const int NE_SEG_READONLY           = 0x0080;  /* - if DATA - */
const int NE_SEG_RELOCATIONS        = 0x0100;
/* - 0x0200  - reserved - */
/* - 0x0400  - reserved - */
/* - 0x0800  - reserved - */
const int NE_SEG_DISCARDABLE        = 0x1000;
/* - 0x2000  - reserved - */
/* - 0x4000  - reserved - */
/* - 0x8000  - reserved - */

#if (defined _MSC_VER) || (defined __GNUC__)
#pragma pack (push, 1)
#endif

//--------------------------------------------------------------------
struct EXE_DOS_Header
//--------------------------------------------------------------------
{
  gtuint16 nID;                  // 0x00
  gtuint16 nLastPage;            // 0x02
  gtuint16 nSumPages;            // 0x04
  gtuint16 nRelocEntries;        // 0x06
  gtuint16 nHeaderSize;          // 0x08
  gtuint16 nMinMem;              // 0x0a
  gtuint16 nMaxMem;              // 0x0c
  gtuint16 nSS;                  // 0x0e
  gtuint16 nSP;                  // 0x10
  gtuint16 nCheckSum;            // 0x12
  gtuint16 nIP;                  // 0x14
  gtuint16 nCS;                  // 0x16
  gtuint16 nRelocOffset;         // 0x18
  gtuint16 res_1;                // 0x1a
  gtuint16 res_2;                // 0x1c
  gtuint16 res_3;                // 0x1e
};

//--------------------------------------------------------------------
struct EXE_LE_Header
//--------------------------------------------------------------------
{
  gtuint16 nID;
  gtuint8  nByteOrder;
  gtuint8  nWordOrder;
  gtuint32 nEXEFormatLevel;
  gtuint16 nCPUType;
  gtuint16 nTargetSystem;
  gtuint32 nModuleVersion;
  gtuint32 nModuleType;
  gtuint32 nMemoryPages;
  gtuint32 nInitCS;
  gtuint32 nInitEIP;
  gtuint32 nInitSS;
  gtuint32 nInitESP;
  gtuint32 nMemoryPageSize;
  gtuint32 nLastPageBytes;
  gtuint32 nFixupSize;
  gtuint32 nFixupChecksum;
  gtuint32 nLoaderSectionSize;
  gtuint32 nLoaderChecksum;
  gtuint32 nObjectTableOfs;
  gtuint32 nObjectTableEntries;
  gtuint32 nObjectPageMapOfs;
  gtuint32 nObjectIterateDataMapOfs;
  gtuint32 nResOfs;
  gtuint32 nResEntries;
  gtuint32 nResidentNamesTableOfs;
  gtuint32 nEntryTableOfs;
  gtuint32 nModuleDirectivesTableOfs;
  gtuint32 nModuleDirectivesEntries;
  gtuint32 nFixupPageTableOfs;
  gtuint32 nFixupRecordTableOfs;
  gtuint32 nImportedModulesNameTableOfs;
  gtuint32 nImportedModulesCount;
  gtuint32 nImportedProceduresNameTableOfs;
  gtuint32 nPerPageChecksumTableOfs;
  gtuint32 nDataPagesOfsFromTopOfFile;
  gtuint32 nPreloadPageCount;
  gtuint32 nNonResidentNamesTableOfsFromTopOfFile;
  gtuint32 nNonResidentNamesTableLength;
  gtuint32 nNonResidentNamesTableChecksum;
  gtuint32 nAutomaticDataObject;
  gtuint32 nDebugInfoOfs;
  gtuint32 nDebugInfoLength;
  gtuint32 nPreloadInstancePagesNumber;
  gtuint32 nDemandInstancePagesNumber;
  gtuint32 nExtraHeapAllocation;
  gtuint32 Unknown;
};

//--------------------------------------------------------------------
struct EXE_LE_Object
//--------------------------------------------------------------------
{
  gtuint32 nVirtualSegmentSize;
  gtuint32 nRelocationBaseAddr;
  gtuint32 nFlags;
  gtuint32 nPageTableIndex;
  gtuint32 nPageTableEntries;
  gtuint32 res;
};

//--------------------------------------------------------------------
enum EXE_NE_OS
//--------------------------------------------------------------------
{
  GT_EXE_NE_OS_UNKNOWN     = 0x00,
  GT_EXE_NE_OS_OS2         = 0x01,
  GT_EXE_NE_OS_WINDOWS     = 0x02,
  GT_EXE_NE_OS_DOS4        = 0x03,
  GT_EXE_NE_OS_WIN386      = 0x04,
  GT_EXE_NE_OS_BOSS        = 0x05,
  GT_EXE_NE_OS_PHARLAP_OS2 = 0x81,
  GT_EXE_NE_OS_PHARLAP_WIN = 0x82
};

//--------------------------------------------------------------------
struct EXE_NE_Header
//--------------------------------------------------------------------
{
  char     sID[2];  // no TCHAR!
  gtuint8  nLinkerMajorVersion;
  gtuint8  nLinkerMinorVersion;
  gtuint16 nEntryTableOfs;
  gtuint16 nEntryTableLen;

  gtuint32 nFileLoadCRC;  // TPW: 0x00000000
  gtuint8  nProgramFlags;
  gtuint8  nApplicationFlags;
  gtuint16 nAutoDataSegmentIndex;

  gtuint16 nInitialLocalHeapSize;
  gtuint16 nInitialStackSize;
  gtuint16 nIP;
  gtuint16 nCS;

  gtuint16 nSP;
  gtuint16 nSS;
  gtuint16 nSegmentCount;
  gtuint16 nModuleReferenceCount;

  gtuint16 nSizeOfNonresidentNamesTable;
  gtuint16 nSegmentTableOfs;
  gtuint16 nResourceTableOfs;
  gtuint16 nResidentNamesTableOfs;

  gtuint16 nModuleReferenceTableOfs;
  gtuint16 nImportedNamesTableOfs;
  gtuint32 nAbsoluteOfsToNonresidentNamesTable;

  gtuint16 nCountOfMoveableEntryPointsListedInEntryTable;
  gtuint16 nFileAlignmentSizeShiftCount;
  gtuint16 nResourceSegmentCount;
  gtuint8  nTargetOS;
  gtuint8  nOtherEXEFlags;

  gtuint16 nOffsetToReturnThunksOrStartOfGangload;
  gtuint16 nOffsetToSegmentReferenceThunksOrLengthOfGangloadArea;
  gtuint16 nMinimumCodeSwapAreaSize;
  gtuint8  nExpectedWinVersionMinor;
  gtuint8  nExpectedWinVersionMajor;
};

//--------------------------------------------------------------------
struct EXE_NE_Segment
//--------------------------------------------------------------------
{
  gtuint16 nRelativeOffset;
  gtuint16 nLength;
  gtuint16 nFlags;
  gtuint16 nAllocSize;
};

//--------------------------------------------------------------------
struct EXE_PE_ImageFileHeader
//--------------------------------------------------------------------
{
  gtuint32 nID;
  gtuint16 nCPUType;
  gtuint16 nNumberOfSections;

  gtuint32 nDateTimeStamp;
  gtuint32 nSymbolTablePtr;

  gtuint32 nNumberOfSymbols;
  gtuint16 nOptionalHeaderSize;
  gtuint16 nFlags;
};

//--------------------------------------------------------------------
struct EXE_PE_OptionalFileHeader
//--------------------------------------------------------------------
{
// default fields
  gtuint16 nMagic;
  gtuint8  nLinkerMajor;
  gtuint8  nLinkerMinor;
  gtuint32 nSizeOfCode;

  gtuint32 nSizeOfInitializedData;
  gtuint32 nSizeOfUninitializedData;

  rva_t    nAddressOfEntryPoint;
  gtuint32 nBaseOfCode;

  gtuint32 nBaseOfData;
// NT additional fields
  gtuint32 nImageBase;

  gtuint32 nSectionAlignment;
  gtuint32 nFileAlignment;

  gtuint16 nOSMajor;
  gtuint16 nOSMinor;
  gtuint16 nUserMajor;
  gtuint16 nUserMinor;

  gtuint16 nSubSystemMajor;
  gtuint16 nSubSystemMinor;
  gtuint32 res_1;

  gtuint32 nSizeOfImage;
  gtuint32 nSizeOfHeader;

  gtuint32 nCheckSum;
  gtuint16 nSubSystem;
  gtuint16 nDLLCharacteristics;

  gtuint32 nSizeOfStackReserve;
  gtuint32 nSizeOfStackCommit;

  gtuint32 nSizeOfHeapReserve;
  gtuint32 nSizeOfHeapCommit;

  gtuint32 nLoaderFlags;             // obsolete
  gtuint32 nNumberOfRVAsAndSizes;
};

//--------------------------------------------------------------------
struct EXE_PE_ImageDataDirectory
//--------------------------------------------------------------------
{
  rva_t    nRVA;
  gtuint32 nSize;
};

//--------------------------------------------------------------------
struct EXE_PE_Section
//--------------------------------------------------------------------
{
  char     sName[8];        // must not be an ASCIIZ string!!

  gtuint32 nVirtualSize;
  rva_t    nRVA;

  gtuint32 nPhysicalSize;
  gtuint32 nPhysicalOffset;

  gtuint32 nRelocationOffset;
  gtuint32 nLineNumberOffset;

  gtuint16 nRelocationCount;
  gtuint16 nLineNumberCount;
  gtuint32 nFlags;
};

//--------------------------------------------------------------------
struct EXE_PE_ExportHeader
//--------------------------------------------------------------------
{
  gtuint32 nCharacteristics;
  gtuint32 nTimeDateStamp;

  gtuint32 nVersion;
  rva_t    nDLLNameRVA;

  gtuint32 nOrdinalBase;
  gtuint32 nNumberOfFunctions;

  gtuint32 nNumberOfNames;
  rva_t    nFunctionsRVA;

  rva_t    nNamesRVA;
  rva_t    nNamesOrdinalRVA;
};

//--------------------------------------------------------------------
struct EXE_PE_ImportTableEntry
//--------------------------------------------------------------------
{
  rva_t    nImportLookupTableRVA;
  gtuint32 nTimeDateStamp;

  gtuint32 nForwarderChain;
  rva_t    nNameRVA;

  rva_t    nImportAddressTableRVA;
};

//--------------------------------------------------------------------
struct EXE_PE_ResourceDirectoryTable
//--------------------------------------------------------------------
{
  gtuint32 nCharacteristics;  // for future use
  gtuint32 nTimeDateStamp;

  gtuint16 nMajorVersion;
  gtuint16 nMinorVersion;
  gtuint16 nNameEntryCount;
  gtuint16 nIDEntryCount;
};

//--------------------------------------------------------------------
struct EXE_PE_ResourceDirectoryEntry
//--------------------------------------------------------------------
{
  gtuint32 nIDOrRVA;  /* union not possible */
  rva_t    nDataRVA;
};

//--------------------------------------------------------------------
struct EXE_PE_ResourceDataEntry
//--------------------------------------------------------------------
{
  rva_t    nDataRVA;
  gtuint32 nDataSize;
  gtuint32 nCodepage;
  gtuint32 nReserved;
};

/*! This is a the same as the VS_FIXEDFILEINFO structure
 */
//--------------------------------------------------------------------
struct EXE_PE_FixedFileInfo
//--------------------------------------------------------------------
{
  gtuint32 dwSignature;
  gtuint32 dwStrucVersion;
  gtuint32 dwFileVersionMS;
  gtuint32 dwFileVersionLS;
  gtuint32 dwProductVersionMS;
  gtuint32 dwProductVersionLS;
  gtuint32 dwFileFlagsMask;
  gtuint32 dwFileFlags;
  gtuint32 dwFileOS;
  gtuint32 dwFileType;
  gtuint32 dwFileSubtype;
  gtuint32 dwFileDateMS;
  gtuint32 dwFileDateLS;
};

//--------------------------------------------------------------------
struct EXE_PE_FixupBlock
//--------------------------------------------------------------------
{
  rva_t    nPageRVA;
  gtuint32 nBlockSize;
};

//--------------------------------------------------------------------
struct DOTNET_MetaDataHeader
//--------------------------------------------------------------------
{
  gtuint32 nID;
  gtuint16 nMajorVer;
  gtuint16 nMinorVer;
  gtuint32 _res1;
  // gtuint32  nVersionStringLen;
  // version string for nVersionStringLen byte
  // padding to next 32bit boundary!
  // gtuint16 nFlags; // always 0
  // gtuint16 nStreamCount;
  // GT_DOTNET_StreamHdr [];
};

#if (defined _MSC_VER) || (defined __GNUC__)
#pragma pack (pop)
#endif

DECLARE_SIZEOF (EXE_DOS_HEADER_SIZE,           EXE_DOS_Header,                32);
DECLARE_SIZEOF (EXE_LE_HEADER_SIZE,            EXE_LE_Header,                 176);
DECLARE_SIZEOF (EXE_LE_OBJECT_SIZE,            EXE_LE_Object,                 24);
DECLARE_SIZEOF (EXE_NE_SEGEMENT_SIZE,          EXE_NE_Segment,                8);
DECLARE_SIZEOF (EXE_NE_HEADER_SIZE,            EXE_NE_Header,                 64);
DECLARE_SIZEOF (EXE_PE_IMAGEHEADER_SIZE,       EXE_PE_ImageFileHeader,        24);
DECLARE_SIZEOF (EXE_PE_OPTIONALHEADER_SIZE,    EXE_PE_OptionalFileHeader,     96);
DECLARE_SIZEOF (EXE_PE_DATADIRECTORY_SIZE,     EXE_PE_ImageDataDirectory,     8);
DECLARE_SIZEOF (EXE_PE_SECTION_SIZE,           EXE_PE_Section,                40);
DECLARE_SIZEOF (EXE_PE_EXPORTHEADER_SIZE,      EXE_PE_ExportHeader,           40);
DECLARE_SIZEOF (EXE_PE_IMPORTTABLEENTRY_SIZE,  EXE_PE_ImportTableEntry,       20);
DECLARE_SIZEOF (EXE_PE_RESOURCEDIRTABLE_SIZE,  EXE_PE_ResourceDirectoryTable, 16);
DECLARE_SIZEOF (EXE_PE_RESOURCEDIRENTRY_SIZE,  EXE_PE_ResourceDirectoryEntry, 8);
DECLARE_SIZEOF (EXE_PE_RESOURCEDATAENTRY_SIZE, EXE_PE_ResourceDataEntry,      16);
DECLARE_SIZEOF (EXE_PE_FIXEDFILEINFO_SIZE,     EXE_PE_FixedFileInfo,          52);
DECLARE_SIZEOF (EXE_PE_FIXUPBLOCK_SIZE,        EXE_PE_FixupBlock,             8);
DECLARE_SIZEOF (EXE_NET_METADATA_HDR,          DOTNET_MetaDataHeader,         12);

/*! Note: BasicAnalyzer is needed because of m_pBuffer
 */
//--------------------------------------------------------------------
class GT_EXPORT_BASIC EXEHeader : public BasicAnalyzer
//--------------------------------------------------------------------
{
protected:
  bool           m_bIsValid;
  file_t         m_nStartOffset;
  EXE_DOS_Header m_aDOSHeader;

public:
  explicit EXEHeader (FileBuffer *pBuffer);

  bool     IsValidCOM      () const;
  bool     IsValidHeaderID () const { return m_aDOSHeader.nID == 0x5A4D || IsEuropeanEXE (); }
  bool     IsValidEXE      () const { return m_bIsValid && IsValidHeaderID (); }
  bool     IsEuropeanEXE   () const { return m_aDOSHeader.nID == 0x4D5A; }
  gtuint32 GetHeaderSize   () const { return EXE_PARAGRAPH_TO_BYTE (m_aDOSHeader.nHeaderSize); }
  gtuint32 GetCS           () const { return EXE_PARAGRAPH_TO_BYTE (m_aDOSHeader.nCS); }
  gtuint16 GetIP           () const { return m_aDOSHeader.nIP; }
  gtuint32 GetSizeInHeader () const { return m_aDOSHeader.nLastPage == 0 ? (gtuint32) 512 * m_aDOSHeader.nSumPages : (gtuint32) 512 * (m_aDOSHeader.nSumPages - 1) + m_aDOSHeader.nLastPage; }
  gtuint32 GetAbsoluteIP   () const;
  bool     HasOverlay      () const { return GetOverlaySize () > 0; }
  file_t   GetOverlaySize  () const;

  void CalculateHeaderSize ()
  {
    m_aDOSHeader.nHeaderSize = EXE_BYTE_TO_PARAGRAPH (/*16 + */m_aDOSHeader.nRelocOffset + (m_aDOSHeader.nRelocEntries << 2));
    if (m_aDOSHeader.nHeaderSize < 2)
      m_aDOSHeader.nHeaderSize = 2;
  }

  //! implementation of BasicAnalyzer
  virtual bool GT_CALL AnalyzerMatch () { return IsValidCOM () || IsValidEXE (); }
};

}  // namespace

#endif
