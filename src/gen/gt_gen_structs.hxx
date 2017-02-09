#ifndef _GT_GEN_STRUCTS_HXX_
#define _GT_GEN_STRUCTS_HXX_

#ifdef _MSC_VER
#pragma once
#endif

namespace GT {

#if (defined _MSC_VER) || (defined __GNUC__)
#pragma pack (push, 1)
#endif

//--------------------------------------------------------------------
struct APE_Header
//--------------------------------------------------------------------
{
  char     sID[4];              // should equal 'MAC '
  gtuint16 nVersion;            // version number * 1000 (3.81 = 3810)
  gtuint16 nCompressionLevel;   // the compression level
  gtuint16 nFormatFlags;        // any format flags (for future use)
  gtuint16 nChannels;           // the number of channels (1 or 2)
  gtuint32 nSampleRate;         // the sample rate (typically 44100)
  gtuint32 nHeaderBytes;        // the bytes after the MAC header that compose the WAV header
  gtuint32 nTerminatingBytes;   // the bytes after that raw data (for extended info)
  gtuint32 nTotalFrames;        // the number of frames in the file
  gtuint32 nFinalFrameBlocks;   // the number of samples in the final frame
};

//--------------------------------------------------------------------
struct APETAG_Footer
//--------------------------------------------------------------------
{
  char     sID[8];       // should equal 'APETAGEX'
  gtuint32 nVersion;     // currently 1000 (version 1.000)
  gtuint32 nLength;      // the complete size of the tag, including this footer (no, excluding!)
  gtuint32 nTagCount;    // the number of fields in the tag
  gtuint32 nFlags;       // the tag flags (none currently defined)
  gtuint8  _res1[8];     // reserved for later use
};

/* The only difference to the footer is, that the length of the tag
     is including the header size.
   That's just a semantically difference but it's better to have different
     comments
 */
//--------------------------------------------------------------------
struct APETAG_Header
//--------------------------------------------------------------------
{
  char     sID[8];       // should equal 'APETAGEX'
  gtuint32 nVersion;     // currently 1000 (version 1.000)
  gtuint32 nLength;      // the complete size of the tag, including this header
  gtuint32 nTagCount;    // the number of fields in the tag
  gtuint32 nFlags;       // the tag flags (none currently defined)
  gtuint8  _res1 [8];    // reserved for later use
};

//--------------------------------------------------------------------
struct BMF_Header
//--------------------------------------------------------------------
{
  gtuint16 nID;
  char     sVersion[2];
  gtuint16 nWidth;
  gtuint16 nHeight;
};

//! Win1 Bitmaps
//--------------------------------------------------------------------
struct BMP_Win1_Header
//--------------------------------------------------------------------
{
  gtuint16 nID;            // 0x00
  gtuint16 nWidth;
  gtuint16 nHeight;
  gtuint16 nByteWidth;
  gtuint8  nPlanes;        // 1
  gtuint8  nBitsPerPixel;  // 1, 4, 8
};

//! Win2-95/NT Bitmaps
//--------------------------------------------------------------------
struct BMP_MainHeader
//--------------------------------------------------------------------
{
  gtuint16 nID;             // 0x4D42
  gtint32  nSize;           // pBuffer->GetFileSize () if compressed else 0
  gtuint16 _res1;           // 0x00
  gtuint16 _res2;           // 0x00
  gtint32  nStartOfs;
  gtint32  nHeaderSize;
  // Win2:12; Win3=WinNT=40; Win95=108
};

/*
  if BitsPerPixel = 1, 4 or 8
    entries = 1 shl BitsPerPixel
  or
    entries = (nStartOfs -
               SizeOf (HeaderWin2_1) -
               SizeOf (HeaderWin2_2)) / SizeOf (Win2_Palette)
*/
//--------------------------------------------------------------------
struct BMP_Win3_PalEntry
//--------------------------------------------------------------------
{
  gtuint8 nBlue;
  gtuint8 nGreen;
  gtuint8 nRed;
};

//--------------------------------------------------------------------
struct BMP_Win4_PalEntry
//--------------------------------------------------------------------
{
  gtuint8 nBlue;
  gtuint8 nGreen;
  gtuint8 nRed;
  gtuint8 _res1;    // 0
};

//! if BPP is 16 or 32 else Win3_Palette
//--------------------------------------------------------------------
struct BMP_BitfieldMask
//--------------------------------------------------------------------
{
  gtint32 nRedMask;
  gtint32 nGreenMask;
  gtint32 nBlueMask;
};

//! Win2 Bitmaps
//--------------------------------------------------------------------
struct BMP_Win2_Header
//--------------------------------------------------------------------
{
  gtuint16 nWidth;
  gtuint16 nHeight;
  gtuint16 nPlanes;        // 1
  gtuint16 nBitsPerPixel;  // 1, 4, 8, 24
};

//! Win3 and WinNT Bitmaps
//--------------------------------------------------------------------
struct BMP_Win3_Header
//--------------------------------------------------------------------
{
  gtuint32 nWidth;
  gtuint32 nHeight;
  gtuint16 nPlanes;           // 1
  gtuint16 nBitsPerPixel;     // 3.x: 1, 4, 8, 32
                              // NT:  1, 4, 8, 16, 24, 32
  gtint32  nCompression;      // 3.x: 0, 1, 2
                              // NT:  0, 1, 2, 3 (BPP must be 16 or 32)
  gtint32  nSizeOfBMP;        // 0 if compression is 0
  gtint32  nHorizRes;
  gtint32  nVertRes;
  gtint32  nColorsUsed;       // if (nColorsUsed == 0 && nBitsPerPixel < 16) then
                              //   nColorsUsed = 1 << nBitsPerPixel;
  gtint32  nColorsImportant;  // if nColorsImportant == 0 then
                              //   nColorsImportant = nColorsUsed
};

//--------------------------------------------------------------------
struct BMP_LongRGBValue
//--------------------------------------------------------------------
{
  gtint32 X;
  gtint32 Y;
  gtint32 Z;
};

//--------------------------------------------------------------------
struct BMP_LongRGB
//--------------------------------------------------------------------
{
  gtint32 nRed;
  gtint32 nGreen;
  gtint32 nBlue;
};

//--------------------------------------------------------------------
struct BMP_Win95_Header
//--------------------------------------------------------------------
// Win95 bitmaps
//--------------------------------------------------------------------
{
  gtuint32         nWidth;
  gtuint32         nHeight;
  gtuint16         nPlanes;           // 1
  gtuint16         nBitsPerPixel;     // 1, 4, 8, 16, 24, 32
  gtint32          nCompression;      // 0, 1, 2, 3 (BPP must be 16 or 32)
  gtint32          nSizeOfBMP;        // 0 if compressions is 0
  gtint32          nHorizRes;
  gtint32          nVertRes;
  gtint32          nColorsUsed;       // if (nColorsUsed == 0 && nBitsPerPixel < 16) then
                                      //   nColorsUsed = 1 << nBitsPerPixel;
  gtint32          nColorsImportant;  // if nColorsImportant == 0 then
                                      //   nColorsImportant = nColorsUsed
  BMP_LongRGB      aMask;
  gtint32          nMaskAlpha;
  gtint32          nCSType;
  BMP_LongRGBValue aRed;              // only used if nCSType == 0
  BMP_LongRGBValue aGreen;
  BMP_LongRGBValue aBlue;
  BMP_LongRGBValue aGamma;
};

//--------------------------------------------------------------------
struct BMP_OS2_Header
//--------------------------------------------------------------------
// OS2 Bitmaps
//--------------------------------------------------------------------
{
  gtuint32 nWidth;
  gtuint32 nHeight;
  gtuint16 nBitPlanes;
  gtuint16 nBitsPerPixel;
  gtint32  nCompressions;
  gtint32  nImageDataSize;
  gtuint32 nHorizRes;      // X resolution
  gtuint32 nVertRes;       // Y resolution
  gtuint32 nColorsUsed;
  gtuint32 nColorsImportant;
  gtuint16 nUnits;
  gtuint16 res1;
  gtuint16 nRecording;
  gtuint16 nRendering;
  gtint32  nSize1;
  gtint32  nSize2;
  gtint32  nColorEncoding;
  gtint32  nIdentifier;
};

// 56 bytes
//--------------------------------------------------------------------
struct CHM_Header
//--------------------------------------------------------------------
{
  gtuint32 nID;      // "ITSF"
  gtuint32 nVersion; // 3
  gtuint32 nTotalHeaderLength;
  gtuint32 res_1;  // 1

  gtuint32 nDT;
  gtuint32 nLangID;
  gtuint8  aGUID1[16];
  gtuint8  aGUID2[16];
};

// 16 bytes
//--------------------------------------------------------------------
struct CHM_HeaderSection
//--------------------------------------------------------------------
{
  gtuint64 nOffset;
  gtuint64 nLength;
};

// 24 bytes
//--------------------------------------------------------------------
struct CHM_HeaderSection0
//--------------------------------------------------------------------
{
  gtuint32 nID;  // 0x000001fe
  gtuint32 unknown_1;
  gtuint64 nFileSize;

  gtuint32 unknown_2;
  gtuint32 unknown_3;
};

//--------------------------------------------------------------------
struct CHM_DirectoryHeader
//--------------------------------------------------------------------
{
  gtuint32 nID;      // "ITSP"  0x50535449
  gtuint32 nVersion; // 1
  gtuint32 nSizeOfHeader;
  gtuint32 unknown_1;    // 0x0a

  gtuint32 nDirChunkSize;
  gtuint32 nQuickRefDensity;
  gtuint32 nIndexTreeDepth;
  gtuint32 nRootIndexChunkNumber;

  gtuint32 nFirstPMGLChunkNumber;
  gtuint32 nLastPMGLChunkNumber;
  gtuint32 unknown_2;
  gtuint32 nChunkCount;

  gtuint32 nLangID;
  gtuint8  aGUID[16];
  gtuint32 nSizeOfHeaderAgain;
  gtuint32 unknown_3;
  gtuint32 unknown_4;

  gtuint32 unknown_5;
};

//--------------------------------------------------------------------
struct CHM_ListingChunk
//--------------------------------------------------------------------
{
  gtuint32 nID;  // "PMGL" 0x4c474d50
  gtuint32 nFreeSpaceAfterwards;
  gtuint32 zero_0;
  gtuint32 nPrevChunkNumber;  // -1 if first

  gtuint32 nNextChunkNumber;  // -1 if first
  // DirList entries
};

//--------------------------------------------------------------------
struct CHM_IndexChunk
//--------------------------------------------------------------------
{
  gtuint32 nID;  // "PMGI" 0x49474d50
  gtuint32 nFreeSpaceAfterwards;
  gtuint32 nIndexEntries;
};

//--------------------------------------------------------------------
struct COFF_Header
//--------------------------------------------------------------------
{
  gtuint16 nMachineType;
  gtuint16 nNumberOfSections;
  gtuint32 nDateTimeStamp;
  gtuint32 nPtrToSymbolTable;
  gtuint32 nNumberOfSymbols;
  gtuint16 nSizeOfOptionalHeader;
  gtuint16 nCharacteristics;
};

//--------------------------------------------------------------------
struct DBF_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint8  nFlag;
  char     sLastMod[3];       // YYMMDD format
  gtint32  nRecordCount;
  gtuint16 nHeaderSize;
  gtuint16 nRecordSize;
  gtuint16 _res1;
  gtuint8  bBreakOnOperation; // 1 == aborted; 0 == oky
  gtuint8  bEncrypted;        // 1 == yes, 0 == no
  gtuint8  _res2[12];
  gtuint8  bMDXFile;          // 1 == yes, 0 == no
  gtuint8  _res3[3];
  // 1..nRecordCount of DBFLocalHeader
  // GetUint8 (x) == 10
};

//--------------------------------------------------------------------
struct DBF_LocalHeader
//--------------------------------------------------------------------
{
  char     sName[11];
  gtuint8  nType;
  gtint32  _res1;
  gtuint8  nLength;
  gtuint8  nDecimals;
  gtuint16 _res2;
  gtuint8  nWorkspaceID;
  gtuint8  _res3[11];
};

//--------------------------------------------------------------------
struct ELF_Header
//--------------------------------------------------------------------
{
  gtuint8  aID[4];
  gtuint8  nClass;
  gtuint8  nDataEncoding;
  gtuint8  nFileVersion;
  gtuint8  aPadding[9];
  gtuint16 nType;
  gtuint16 nMachine;
  gtuint32 nVersion;
  gtuint32 nEntryRVA;
  gtuint32 nProgramHeaderTableOffset;
  gtuint32 nSectionHeaderTableOffset;
  gtuint32 nFlags;
  gtuint16 nHeaderSize;
  gtuint16 nProgramHeaderTableEntrySize;  /* size of a single entry */
  gtuint16 nProgramHeaderTableEntryCount;
  gtuint16 nSectionHeaderTableEntrySize;  /* size of a single entry */
  gtuint16 nSectionHeaderTableEntryCount;
  gtuint16 nSectionHeaderTableSectionNameStringTableIndex;
};

//--------------------------------------------------------------------
struct FLI_Header
//--------------------------------------------------------------------
{
  gtuint32 nFileSize;          /* total size of the file */
  gtuint16 nFileID;            /* file format indicator */
  gtuint16 nNumberOfFrames;    /* total number of frames */
  gtuint16 nWidth;             /* screen width in pixel */
  gtuint16 nHeight;            /* screen height in pixel */
  gtuint16 nPixelDepth;        /* number of bits per pixel */
  gtuint16 nFlags;             /* set to 03h */
  gtuint32 nFrameDelay;        /* time delay between frames */
  gtuint16 _res1;              /* not used (set to 00h) */
  /* the following fields are zero in FLI files; only set in FLC files */
  gtuint32 nDateCreated;       /* Time/Date the file was created */
  gtuint32 nCreatorSN;         /* serial number of creator program */
  gtuint32 nLastUpdated;       /* Time/Date the file last changed */
  gtuint32 nUpdaterSN;         /* serial number of updater program */
  gtuint16 nXAspect;           /* X-axis of display aspect ration */
  gtuint16 nYAspect;           /* Y-axis of display aspect ration */
  gtuint8  _res2[38];          /* not used (set to 00h) */
  gtuint32 nFrame1Offset;      /* Offset of first frame */
  gtuint32 nFrame2Offset;      /* Offset of second frame */
  gtuint8  _res3[40];          /* not used (set to 00h) */
};

//--------------------------------------------------------------------
struct GIF_Header
//--------------------------------------------------------------------
{
  char     sID[6];
  gtuint16 nWidth;
  gtuint16 nHeight;
  gtuint8  nFlags;
  gtuint8  nBackgroundColorIndex;
  gtuint8  nZeroed;
};

//--------------------------------------------------------------------
struct GIF_ImageDescriptor
//--------------------------------------------------------------------
{
  gtuint8  nID;
  gtuint16 nImageLeft;
  gtuint16 nImageTop;
  gtuint16 nImageWidth;
  gtuint16 nImageHeight;
  gtuint8  nFlags;
};

//--------------------------------------------------------------------
struct ICO_Header
//--------------------------------------------------------------------
{
  gtuint16 nID;
  gtuint16 nType;
  gtuint16 nIconCount;
};

//--------------------------------------------------------------------
struct ID3_GlobalHeader
//--------------------------------------------------------------------
{
  char     sID[3];
  gtuint8  nVersion;
  gtuint8  nRevision;
  gtuint8  nFlags;
  union
  {
    gtuint32 nSize;
    gtuint8  nFields[4];  // take only 7 bit!
  };
};

//--------------------------------------------------------------------
struct ID3_FrameHeader
//--------------------------------------------------------------------
{
  char     sID[4];
  union
  {
    gtuint32 nSize;
    gtuint8  nFields[4];  // take only 7 bit!
  };
  gtuint16 nFlags;
};

//--------------------------------------------------------------------
struct IFF_Header
//--------------------------------------------------------------------
{
  gtint32 nID;
  gtint32 nSize;
};

//--------------------------------------------------------------------
struct IFF_BMHD
//--------------------------------------------------------------------
{
  gtuint16 nWidth;
  gtuint16 nHeight;
  gtuint16 nLeft;
  gtuint16 nTop;
  gtuint8  nBitPlanes;
  gtuint8  nMasking;
  gtuint8  nCompress;
  gtuint8  nPadding;
  gtuint16 nTransparency;
  gtuint8  nXAspectRation;
  gtuint8  nYAspectRation;
  gtuint16 nPageWidth;
  gtuint16 nPageHeight;
};

//--------------------------------------------------------------------
struct JPG_Block
//--------------------------------------------------------------------
{
  gtuint8  nID;
  gtuint8  nType;
  gtuint16 nSize;  // reverse bit order! without nID and nType
};

//--------------------------------------------------------------------
struct JPG_SOF
//--------------------------------------------------------------------
{
  gtuint8  nID;
  gtuint8  nType;
  gtuint16 nSize;
  gtuint8  nPrecision;
  gtuint16 nHeight;
  gtuint16 nWidth;
  gtuint8  nComponents;
};

//--------------------------------------------------------------------
struct LIB_Header
//--------------------------------------------------------------------
{
  char     m_sName[16];
  char     m_sDate[12];        // seconds since 1.1.1970 UTC in decimal
  char     m_sUserID[6];
  char     m_sGroupID[6];
  char     m_sMode[8];
  char     m_sMemberSize[10];  // without header
  gtuint16 m_nEnd;             // 0x0a60
};

//--------------------------------------------------------------------
struct MAC_Header
//--------------------------------------------------------------------
{
  gtuint8  nVersion;            //   0
  gtuint8  nFileNameLength;     //   1
  gtuint8  sFilename[63];       //   2
  gtuint32 nFileType;           //  65
  gtuint32 nFileCreator;        //  69
  gtuint8  nFileFlags;          //  73
  gtuint8  _res1;               //  74
  gtuint16 nFileVertPos;        //  75
  gtuint16 nFileHorizPos;       //  77
  gtuint16 nWindowID;           //  79
  gtuint8  nProtected;          //  81
  gtuint8  _res2;               //  82
  gtuint32 nSizeOfDataFork;     //  83
  gtuint32 nSizeOfResourceFork; //  87
  gtuint32 nCreationDT;         //  91
  gtuint32 nModificationDT;     //  95
  gtuint16 nGetInfoLength;      //  99
  /* the following fields were added for MacBinary II */
  gtuint16 nFinderFlags;        // 101
  gtuint8  _res3[14];           // 103
  gtuint32 nUnpackedLength;     // 117
  gtuint16 nSecondHeadLength;   // 121
  gtuint8  nUploadVersion;      // 123
  gtuint8  nReadVersion;        // 124
  gtuint16 nCRC16;              // 125
  gtuint8  _res4[2];            // 127
};

//--------------------------------------------------------------------
struct MIDI_Header
//--------------------------------------------------------------------
{
  gtint32  nID;
  gtint32  nHeaderLen;
  gtuint16 nFormat;
  gtuint16 nTrackBlocks;
  gtint16  nDeltaTime;
};

//--------------------------------------------------------------------
struct MP3TAG
//--------------------------------------------------------------------
{
  char sID[3];
  char sTitle[30];
  char sArtist[30];
  char sAlbum[30];
  char sYear[4];
  char sComment[30];
  char res1;
};

//--------------------------------------------------------------------
struct PCX_Header
//--------------------------------------------------------------------
{
  gtuint8  nID;  /* 0x0A */
  gtuint8  nVersion;
  gtuint8  nEncoding;
  gtuint8  nBitsPerPixel;
  gtuint16 nXMin;
  gtuint16 nYMin;
  gtuint16 nXMax;
  gtuint16 nYMax;
  gtuint16 nHRes;
  gtuint16 nVRes;
  gtuint8  aEGAPalette[48];
  gtuint8  _res1;  /* 0x00 */
  gtuint8  nNumberOfBitPlanes;
  gtuint16 nBytesPerLine;
  gtuint16 nPaletteType;
  gtuint8  _res2[58];  /* all 0x00 */
};

//--------------------------------------------------------------------
struct PKG_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint16 nID;  // 0xA34A
  gtuint32 nOffsetToArchiveList;
  gtuint32 nVersionNumber;  // 0x00000002
};

//--------------------------------------------------------------------
struct PKG_ArchiveHeader1
//--------------------------------------------------------------------
{
  gtuint16 _res1;
  gtuint16 nDiskStart;
  gtuint16 nDiskCount;
};

//--------------------------------------------------------------------
struct PKG_ArchiveHeader2
//--------------------------------------------------------------------
{
  gtuint16 _res1;
  gtuint32 nOffsetToFiles;
};

//--------------------------------------------------------------------
struct PNG_Header
//--------------------------------------------------------------------
{
  char     sID[8];
  gtuint8  _res1[8];
  gtint32  nWidth;
  gtint32  nHeight;
  gtuint8  nColorDepth;
  gtuint8  nColorType;
  gtuint16 _res2;
  gtuint8  nFlags;
};

//--------------------------------------------------------------------
struct PNG_BlockHeader
//--------------------------------------------------------------------
{
  gtint32 nFollowingBytes;
  char    sChunkType[4];
  gtint32 nWidth;
  gtint32 nHeight;
  gtuint8 nBitDepth;
  gtuint8 nColorType;
  gtuint8 nCompressionType;
  gtuint8 nFilterType;
  gtuint8 nInterlaceType;
};

//--------------------------------------------------------------------
struct POIFS_Header
//--------------------------------------------------------------------
{
  gtuint64 nMagic;                // 0x0000 - 0xe11ab1a1e011cfd0
  gtuint32 _res1;                 // 0x0008 - 0
  gtuint32 _res2;                 // 0x000c - 0
  gtuint32 _res3;                 // 0x0010 - 0
  gtuint32 _res4;                 // 0x0014 - 0
  gtuint16 _res5;                 // 0x0018 - 0x003b
  gtuint16 _res6;                 // 0x001a - 0x0003
  gtuint16 _res7;                 // 0x001c - -2
  gtuint16 nLog2BigBlockSize;     // 0x001e - 9
  gtuint32 nLog2SmallBlockSize;   // 0x0020 - 6
  gtuint32 _res8;                 // 0x0024 - 0
  gtuint32 _res9;                 // 0x0028 - 0
  gtuint32 nBATCount;             // 0x002c - #req
  gtuint32 nPropStart;            // 0x0030 - #req
  gtuint32 _res10;                // 0x0034 - 0
  gtuint32 _res11;                // 0x0038 - 0x00001000
  gtuint32 nSBATStart;            // 0x003c - -2
  gtuint32 nSBATCount;            // 0x0040 - 1
  gtuint32 nXBATStart;            // 0x0044 - -2
  gtuint32 nXBATCount;            // 0x0048 - 0
  gtuint32 nBATArray[109];        // 0x004c
};

//--------------------------------------------------------------------
struct POIFS_Property
//--------------------------------------------------------------------
{
  wchar_t  sName[0x20];           // 0x00
  gtuint16 nNameSize;             // 0x40
  gtuint8  nPropertyType;         // 0x42
  gtuint8  nNodeColor;            // 0x43
  gtuint32 nPrevPropIndex;        // 0x44
  gtuint32 nNextPropIndex;        // 0x48
  gtuint32 nFirstChildPropIndex;  // 0x4c
  gtuint32 _res1;                 // 0x50
  gtuint32 _res2;                 // 0x54
  gtuint32 _res3;                 // 0x58
  gtuint32 _res4;                 // 0x5c
  gtuint32 _res5;                 // 0x60
  gtuint64 nDTCreated;            // 0x64
  gtuint64 nDTModified;           // 0x6c
  gtuint32 nStartBATIndex;        // 0x74
  gtuint32 nSize;                 // 0x78
  gtuint32 _res6;                 // 0x7c
};

//--------------------------------------------------------------------
struct PSD_Header
//--------------------------------------------------------------------
{
  gtint32  nID;
  gtint32  _res1;
  gtint32  _res2;
  gtuint16 _res3;
  gtuint32 nHeight;
  gtuint32 nWidth;
};

//--------------------------------------------------------------------
struct RIFF_Header
//--------------------------------------------------------------------
{
  union
  {
    gtuint32 n;
    char     s[4];
  } aID;
  gtuint32 nSize;
};

//--------------------------------------------------------------------
struct SYS_Header
//--------------------------------------------------------------------
{
  gtint32  nAdrOfNextDriver;
  gtuint16 nAttr;
  gtuint16 nStrategyOfs;
  gtuint16 nInterruptOfs;
  char     sName[8];
};

//--------------------------------------------------------------------
struct TIFF_Tag
//--------------------------------------------------------------------
{
  gtuint16 nIdentifyingCode;
  gtuint16 nDataType;
  gtuint32 nValueCount;
  union
  {
    gtuint32 nTagData;
    gtuint32 nTagDataOffset;
  };
};

//--------------------------------------------------------------------
struct TTF_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint32 nID;
  gtuint16 nTables;
  gtuint16 nSearchRange;
  gtuint16 nEntrySelector;
  gtuint16 nRangeShift;
};

//--------------------------------------------------------------------
struct TTF_LocalHeader
//--------------------------------------------------------------------
{
  gtuint32 nID;
  gtuint32 nCheckSum;
  gtuint32 nOffset;
  gtuint32 nLength;
};

//--------------------------------------------------------------------
struct TTF_Name
//--------------------------------------------------------------------
{
  gtuint16 nFormat;
  gtuint16 nCount;
  gtuint16 nOffset;
};

//--------------------------------------------------------------------
struct TTF_Entry
//--------------------------------------------------------------------
{
  gtuint16 nPlatformID;
  gtuint16 nEncodingID;
  gtuint16 nLanguageID;
  gtuint16 nNameID;
  gtuint16 nStringLength;
  gtuint16 nRelOffset;
};

//--------------------------------------------------------------------
struct VOC_Header
//--------------------------------------------------------------------
{
  char     sID[19];
  gtuint8  _res1;
  gtuint16 nDataOfs;
  gtuint8  nVersionMinor;
  gtuint8  nVersionMajor;
  gtuint16 nComp2;
};

//--------------------------------------------------------------------
struct WAV_fmt
//--------------------------------------------------------------------
{
  gtint16  nTyp;
  gtuint16 nChannels;
  gtuint32 nSamplesPerSec;
  gtuint32 nAverageBytesPerSec;
  gtuint16 nBlockAlign;
  gtuint16 nBitsPerSample;
};

//--------------------------------------------------------------------
struct WMF_Header
//--------------------------------------------------------------------
{
  gtint32  nID;
  gtuint16 nHandle;
  gtint16  nLeft;
  gtint16  nTop;
  gtint16  nRight;
  gtint16  nBottom;
  gtuint16 nInch;
  gtint32  nReserved;
  gtuint16 nChecksum;
};

//--------------------------------------------------------------------
struct WMF_DataHeader
//--------------------------------------------------------------------
{
  gtuint16 nFileType;
  gtuint16 nHeaderSize;  // in WORDS!!
  gtuint16 nVersion;
  gtint32  nFileSize;
  gtuint16 nObjectCount;
  gtint32  nMaxRecordSize;
  gtuint16 nParameterCount;
};

//--------------------------------------------------------------------
struct WPC_Header
//--------------------------------------------------------------------
{
  gtint32  nID;                     /* 0x435057FF */
  gtint32  nDataOffset;             /* 0x10 */
  gtuint8  nProcductType;           /* 0x01 */
  gtuint8  nFileType;               /* e.g. 0x16 WPG */
  gtuint8  nMajorVersion;           /* 0x01 */
  gtuint8  nMinorVersion;           /* 0x00 */
  gtuint16 nEncryptionKey;          /* != 0 if encrypted */
  gtuint16 nReserved;               /* reserved (0x00) */
};

//--------------------------------------------------------------------
struct WPG_Header
//--------------------------------------------------------------------
{
  gtuint16 nWidth;
  gtuint16 nHeight;
  gtuint16 nBitsPerPixel;
};

//--------------------------------------------------------------------
struct XPT_Header
//--------------------------------------------------------------------
{
  char     sID[16];
  gtuint8  nMajorVersion;
  gtuint8  nMinorVersion;
  gtuint16 nInterfaces;
  gtuint32 nFileLength;
  gtuint32 nIDEOffset;  // IDE == InterfaceDirectoryEntries
};

#if (defined _MSC_VER) || (defined __GNUC__)
#pragma pack (pop)
#endif

DECLARE_SIZEOF (APE_H_SIZE,      APE_Header,          32);
DECLARE_SIZEOF (APETAG_F_SIZE,   APETAG_Footer,       32);
DECLARE_SIZEOF (APETAG_H_SIZE,   APETAG_Header,       32);
DECLARE_SIZEOF (BMF_H_SIZE,      BMF_Header,          8);
DECLARE_SIZEOF (BMP_HM_SIZE,     BMP_MainHeader,      18);
DECLARE_SIZEOF (BMP_OS2_SIZE,    BMP_OS2_Header,      60);
DECLARE_SIZEOF (CHM_H_SIZE,      CHM_Header,          56);
DECLARE_SIZEOF (CHM_HS_SIZE,     CHM_HeaderSection,   16);
DECLARE_SIZEOF (CHM_HS0_SIZE,    CHM_HeaderSection0,  24);
DECLARE_SIZEOF (CHM_DH_SIZE,     CHM_DirectoryHeader, 84);
DECLARE_SIZEOF (CHM_LC_SIZE,     CHM_ListingChunk,    20);
DECLARE_SIZEOF (CHM_IC_SIZE,     CHM_IndexChunk,      12);
DECLARE_SIZEOF (COFF_H_SIZE,     COFF_Header,         20);
DECLARE_SIZEOF (DBF_GH_SIZE,     DBF_GlobalHeader,    32);
DECLARE_SIZEOF (DBF_LH_SIZE,     DBF_LocalHeader,     32);
DECLARE_SIZEOF (ELF_H_SIZE,      ELF_Header,          52);
DECLARE_SIZEOF (FLI_H_SIZE,      FLI_Header,          128);
DECLARE_SIZEOF (GIF_H_SIZE,      GIF_Header,          13);
DECLARE_SIZEOF (GIF_ID_SIZE,     GIF_ImageDescriptor, 10);
DECLARE_SIZEOF (ICO_H_SIZE,      ICO_Header,          6);
DECLARE_SIZEOF (ID3_GH_SIZE,     ID3_GlobalHeader,    10);
DECLARE_SIZEOF (ID3_FH_SIZE,     ID3_FrameHeader,     10);
DECLARE_SIZEOF (IFF_H_SIZE,      IFF_Header,          8);
DECLARE_SIZEOF (IFF_BM_SIZE,     IFF_BMHD,            20);
DECLARE_SIZEOF (JPG_B_SIZE,      JPG_Block,           4);
DECLARE_SIZEOF (JPG_S_SIZE,      JPG_SOF,             10);
DECLARE_SIZEOF (LIB_H_SIZE,      LIB_Header,          60);
DECLARE_SIZEOF (MAC_H_SIZE,      MAC_Header,          129);
DECLARE_SIZEOF (MIDI_H_SIZE,     MIDI_Header,         14);
DECLARE_SIZEOF (MP3TAG_SIZE,     MP3TAG,              128);
DECLARE_SIZEOF (PCX_H_SIZE,      PCX_Header,          128);
DECLARE_SIZEOF (PKG_GH_SIZE,     PKG_GlobalHeader,    10);
DECLARE_SIZEOF (PKG_AH1_SIZE,    PKG_ArchiveHeader1,  6);
DECLARE_SIZEOF (PKG_AH2_SIZE,    PKG_ArchiveHeader2,  6);
DECLARE_SIZEOF (PNG_H_SIZE,      PNG_Header,          29);
DECLARE_SIZEOF (PNG_BH_SIZE,     PNG_BlockHeader,     21);
DECLARE_SIZEOF (POIFS_H_SIZE,    POIFS_Header,        512);
DECLARE_SIZEOF (POIFS_P_SIZE,    POIFS_Property,      128);
DECLARE_SIZEOF (PSD_H_SIZE,      PSD_Header,          22);
DECLARE_SIZEOF (RIFF_H_SIZE,     RIFF_Header,         8);
DECLARE_SIZEOF (SYS_H_SIZE,      SYS_Header,          18);
DECLARE_SIZEOF (TIFF_TAG_SIZE,   TIFF_Tag,            12);
DECLARE_SIZEOF (TTF_GH_SIZE,     TTF_GlobalHeader,    12);
DECLARE_SIZEOF (TTF_LH_SIZE,     TTF_LocalHeader,     16);
DECLARE_SIZEOF (TTF_NAME_SIZE,   TTF_Name,            6);
DECLARE_SIZEOF (TTF_ENTRY_SIZE,  TTF_Entry,           12);
DECLARE_SIZEOF (VOC_H_SIZE,      VOC_Header,          26);
DECLARE_SIZEOF (WAV_FMT_SIZE,    WAV_fmt,             16);
DECLARE_SIZEOF (WMF_H_SIZE,      WMF_Header,          22);
DECLARE_SIZEOF (WMF_DH_SIZE,     WMF_DataHeader,      18);
DECLARE_SIZEOF (WPC_H_SIZE,      WPC_Header,          16);
DECLARE_SIZEOF (WPG_H_SIZE,      WPG_Header,          6);
DECLARE_SIZEOF (XPT_H_SIZE,      XPT_Header,          28);

}  // namespace

#endif
