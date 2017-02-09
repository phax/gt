#ifndef _GT_ARC_STRUCTS_HXX_
#define _GT_ARC_STRUCTS_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#if (defined _MSC_VER) || (defined __GNUC__)
#pragma pack (push, 1)
#endif

//--------------------------------------------------------------------
struct _777_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint8 cID[3];
  gtuint8 _res1[3];
  gtuint8 nMethod;
  gtuint8 _res2;
};

//--------------------------------------------------------------------
struct _777_SolidGlobalHeader
//--------------------------------------------------------------------
{
  gtuint8 _res1[13];
  gtint32 nDirectoryOffset;
  gtuint8 _res2[2];
};

//--------------------------------------------------------------------
struct _777_LocalHeader
//--------------------------------------------------------------------
{
  gtuint32 nLastAccess;
  gtuint8  nPassword;
  gtuint8  _res1[3];
  gtuint8  nMethod;  // 1 2 3 4 5 6 9
  gtuint8  _res2[4];
  gtint32  nAttr;
  gtuint64 nCreationDT;
  gtuint64 nLastAccessDT;
  gtuint64 nModificationDT;
  gtuint8  _res3[5];
  gtint32  nPackedSize;
  gtint32  nOriginalSize;
  gtuint16 nFilenameLen;
  /* .. filename .. */
};

//--------------------------------------------------------------------
struct _777_SolidLocalHeader
//--------------------------------------------------------------------
{
  gtuint8  _res1[28];
  gtuint32 nCRC32;
  gtuint8  nMethod;
  gtuint32 nOriginalSize;
  gtuint16 nFilenameLen;
  /* .. filename .. */
};

//--------------------------------------------------------------------
struct ACE_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint16 _res1;
  gtuint16 nDataOffset;
  gtuint8  _res2[3];
  gtuint8  aID[7];
  gtuint8  nCompressVersion;
  gtuint8  nDecompressVersion;
};

//--------------------------------------------------------------------
struct ACE_LocalHeaderFile
//--------------------------------------------------------------------
{
  gtuint16 _res1;
  gtuint16 nHeaderSize;
  gtuint8  nType;  /* nHeaderSize starts here!! */
  gtuint16 nFlags;
  gtint32  nPackedSize;
  gtint32  nOriginalSize;
  gtint32  nDateTime;
  gtuint8  _res3[14];
  gtuint16 nFilenameLen;
};

//--------------------------------------------------------------------
struct ACE_LocalHeaderEnvelope
//--------------------------------------------------------------------
{
  gtuint16 _res1;
  gtuint16 nHeaderSize;
  gtuint8  nType;  /* nHeaderSize starts here!! */
  gtuint8  _res2;
  gtuint8  nFlags;
  gtint32  nPackedSize;
};

//--------------------------------------------------------------------
struct AIN_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint16 nID;
  gtuint8  _res1[6];
  gtuint16 nFilesInArchive;
  gtuint8  _res2[4];
  gtint32  nDataSize;
};

//--------------------------------------------------------------------
struct AKT7_GlobalHeader
//--------------------------------------------------------------------
{
  char     sID[3];
  gtuint8  nHeaderVersion;
  gtuint16 nFilesInArchive;
  gtuint16 nCommentLength;
  gtuint8  nCompressionMethod;
  gtint32  _res1;
  // comment
};

//--------------------------------------------------------------------
struct AKT7_LocalHeader
//--------------------------------------------------------------------
{
  // gtuint8 nPathLen
  // path
  char     sFilename[12];
  gtint32  nDateTime;
  gtuint16 nAttr;
  gtint32  nStartPos;
  gtint32  nEndPos;
};

//--------------------------------------------------------------------
struct AKT9_GlobalHeader
//--------------------------------------------------------------------
{
  char     sID[3];
  gtuint8  nHeaderVersion;
  gtuint8  nHeaderSize;
  gtuint16 nCommentSize;
  gtuint8  zeroes[2];
};

//--------------------------------------------------------------------
struct AKT9_LocalHeader
//--------------------------------------------------------------------
{
  // gtuint8 nPathLen
  // path[]
  char    sFilename[12];
  gtint32 nPackedSize;
  gtint32 nOriginalSize;
  gtint32 nCRC32;
  gtint32 nDateTime;
  gtuint8 nAttribute;
  gtuint8 nCompressionMethod;
  gtuint8 _res1;
};

//--------------------------------------------------------------------
struct AMG_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint16 nID;
  gtuint8  nVersion;
  gtuint8  _res1[7];
  gtuint16 nCommentLen;
};

//--------------------------------------------------------------------
struct AMG_LocalHeader
//--------------------------------------------------------------------
{
  gtuint16 nID;
  gtint32  nPackedSize;
  gtint32  nOriginalSize;
  gtint32  nDateTime;
  gtuint8  nAttr;
  gtuint16 nFlags;
  gtint32  nCRC32;
  char     sFilename[8];
  char     sExt[3];
  gtuint8  nPathLen;
  gtuint16 nFileCommentLen;
};

//--------------------------------------------------------------------
struct ARC_Header
//--------------------------------------------------------------------
{
  gtuint8  nID;
  gtuint8  nCompressionMethod;
  char     sFilename[13];
  gtint32  nPackedSize;
  gtint32  nDateTime;
  gtuint16 nCRC16;
  gtint32  nOriginalSize;
};

//--------------------------------------------------------------------
struct ARG_Header
//--------------------------------------------------------------------
{
  gtuint8 nID;  // 0!
  char    sFilename[13];
  gtint32 nDateTime;
  gtint32 nOriginalSize;
  gtint32 nPackedSize;
};

//--------------------------------------------------------------------
struct ARJ_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint16 nID;
  gtuint16 nBasicHeaderSize;
  gtuint8  nFirstHeaderSize;
  gtuint8  nPackVersion;
  gtuint8  nExtractVersion;
  gtuint8  nHostOS;
  gtuint8  nFlags;
  gtuint8  nSecurityVersion;
  gtuint8  nFileType;
  gtuint8  _res1;
  gtint32  nDTOriginal;
  gtint32  nDTModified;
  gtint32  nArchiveSize;
  gtint32  nSecurityEnvelope;
  gtuint16 nFilespecPosInFilename;
  gtuint16 nSecurityEnvelopeLength;
  gtuint16 _res2;
};

//--------------------------------------------------------------------
struct ARJ_LocalHeader
//--------------------------------------------------------------------
{
  gtuint16 nID;
  gtuint16 nHeaderSize;
  gtuint8  nFirstHeaderSize;
  gtuint8  nPackVersion;
  gtuint8  nExtractVersion;
  gtuint8  nHostOS;
  gtuint8  nFlags;
  gtuint8  nMethod;
  gtuint8  nFileType;
  gtuint8  _res1;
  gtint32  nDT;
  gtint32  nPackedSize;
  gtint32  nOriginalSize;
  gtint32  nOriginalCRC32;
  gtuint16 nFilespecPosInFilename;
  gtuint16 nFileAccessMode;
  gtuint16 nHostData;
};

//--------------------------------------------------------------------
struct ARQ_Header
//--------------------------------------------------------------------
{
  gtuint16 nID;
  gtuint8  _res1;
  gtuint8  nType;
  gtuint16 nVersion;
  gtuint16 nFilenameLen;
};

//--------------------------------------------------------------------
struct ARQ_Body
//--------------------------------------------------------------------
{
  gtint32  nDateTime;  // special format!
  gtuint8  _res1[6];
  gtint32  nPackedSize;
  gtint32  nOriginalSize;
  gtint32  nCRC32;
  gtuint16 _res2;
  gtuint8  nFlags;
  gtint32  _res3;
  gtint32  _res4;
};

//--------------------------------------------------------------------
struct ARX_Header
//--------------------------------------------------------------------
{
  gtuint8  nID1;
  gtuint8  nHeaderSize;
  char     sID[6];
  gtint32  nPackedSize;
  gtint32  nOriginalSize;
  gtint32  nDateTime;
  gtuint16 nAttribute;
  // pascal type string
};

//--------------------------------------------------------------------
struct ASD_GlobalHeader
//--------------------------------------------------------------------
{
  char     sID[5];
  gtuint8  nSubVersion;
  gtuint16 nFilesInArchive;
};

//--------------------------------------------------------------------
struct ASD_LocalHeader
//--------------------------------------------------------------------
{
  // filename
  gtuint32 nOriginalSize;
  gtuint32 nCRC32;
  gtuint32 nDateTime;
  gtuint16 nAttribute;
};

//--------------------------------------------------------------------
struct BIX_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint32 nID;
  char     _res[20];
};

//--------------------------------------------------------------------
struct BIX_LocalHeader
//--------------------------------------------------------------------
{
  gtuint32 nHeaderCRC32;
  char     _res1[3];
  gtuint16 nHeaderSize;
  gtuint32 nPackedSize;
  gtuint32 nOriginalSize;
  gtuint32 _maybe_crc32;
  gtuint32 nDT;
  gtuint16 _constant_01;
  gtuint8  nMethod;
  gtuint16 nFilenameLen;
  gtuint32 _maybe_attrs;
  /* filename for nFilenameLen bytes */
};

//--------------------------------------------------------------------
struct BLI_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint8  sID[16];
  gtuint16 nVersion;
};

//--------------------------------------------------------------------
struct BLI_SkipHeader
//--------------------------------------------------------------------
{
  gtuint8  nMethod;
  gtuint16 nBytesToSkip;
};

//--------------------------------------------------------------------
struct BLI_LocalHeader250
//--------------------------------------------------------------------
{
  gtuint16 nDate;
  gtuint16 nTime;
  gtint32  nOriginalSize;
  char     sFilename[13];
  gtuint8  nAttribute;
};

//--------------------------------------------------------------------
struct BLI_LocalHeader251
//--------------------------------------------------------------------
{
  char     sFilename[13];
  gtuint8  nAttribute;
  gtint32  nOriginalSize;
  gtuint16 nDate;
  gtuint16 nTime;
};

//--------------------------------------------------------------------
struct BOA_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint32 nID;
  gtuint8  nMinorVersion;
  gtuint8  nMajorVersion;
  gtuint8  nMBUsed;
};

//--------------------------------------------------------------------
struct BOA_LocalHeader
//--------------------------------------------------------------------
{
  gtuint32 nDT;               /* Unix style format */
  gtuint8  nCompressionType;  /* ?? */
  gtuint32 nOriginalSize;
  gtuint32 nPackedSize;
  gtuint32 nCRC32;
  gtuint16 res1;
  gtuint8  nAttr;
};

//--------------------------------------------------------------------
struct BSA_Header
//--------------------------------------------------------------------
{
  gtuint16 nHeaderSize;
  gtuint16 nID;
  gtuint32 nPackedSize;

  gtuint32 nOriginalSize;
  gtuint32 nDT;

  gtuint16 nCRC16;
  gtuint16 _res1_always_0000;
  gtuint16 nAttr;
  gtuint8  nFilenameLen;
};

//--------------------------------------------------------------------
struct BSN_Header1
//--------------------------------------------------------------------
{
  gtuint32 nID;
  gtuint16 nAttr;
  gtuint32 _res1;
  gtuint32 nDT;
};

//--------------------------------------------------------------------
struct BSN_Header2
//--------------------------------------------------------------------
{
  gtuint32 nOriginalSize;
  gtuint32 nPackedSize;
  gtuint32 _res2;
  gtuint32 _res3;
};

//--------------------------------------------------------------------
struct CAB_GlobalHeader
//--------------------------------------------------------------------
{
  gtint32  nID;
  gtint32  nNull1;
  gtint32  nFileSize;
  gtint32  nNull2;
  gtint32  nDataStart;
  gtint32  _res1;
  gtint32  _res2;
  gtuint16 nFileCount;
  gtuint16 _res3;
};

//--------------------------------------------------------------------
struct CAB_LocalHeader
//--------------------------------------------------------------------
{
  gtint32 nOriginalSize;
  gtint32 _res1;
  gtint32 nDateTime;
  gtint32 _res2;
};

//--------------------------------------------------------------------
struct DPA_Header
//--------------------------------------------------------------------
{
  char     sID[20];
  gtuint16 _res1;
  gtint32  nOriginalSize;
  gtint32  nNextOffset;
  char     sFilename[14];
};

//--------------------------------------------------------------------
struct HA_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint16 nID;
  gtuint16 nFilesInArchive;
};

//--------------------------------------------------------------------
struct HA_LocalHeader
//--------------------------------------------------------------------
{
  gtuint8 nVersion;
  gtint32 nPackedSize;
  gtint32 nOriginalSize;
  gtint32 nCRC32;
  gtint32 nDateTime;
  // ...#0
  // ...#0
  // len(1)
  // 1..len
};

//--------------------------------------------------------------------
struct IMP_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint32 nID;                    /*  0 */
  gtuint32 nDataOffset;            /*  4 */
  gtuint32 nEntriesInDataOffset;   /*  8 */
  gtuint32 _ecoffset;              /* 12 */
  gtuint32 nCommentOffset;         /* 16 */
  gtuint32 _length;                /* 20 */
  gtuint32 _csize;                 /* 24 */
  gtuint16 _mvsig;                 /* 28 */
  gtuint16 _disk;                  /* 30 */
  gtuint16 _dirdisk;               /* 32 */
  gtuint16 _cmdisk;                /* 34 */
  gtuint16 _ecdisk;                /* 36 */
  gtuint16 nFlags;                 /* 38 */
  gtuint16 nCRC16;                 /* 40 */
};

//--------------------------------------------------------------------
struct IMP_LocalHeader
//--------------------------------------------------------------------
{
  gtuint16 nVersion;     /*  0 */
  gtuint16 nDiskNr;      /*  2 */
  gtuint32 nFilePos;     /*  4 */
  gtuint16 nAttribs;     /*  8 */
  gtuint8  nCommentLen;  /* 10 */
  gtuint8  nFlags;       /* 11 */
  gtuint32 nExtractedOfs;/* 12 */
  gtuint32 nOrigSize;    /* 16 */
  gtuint32 nCRC32;       /* 20 */
  gtuint16 extra;        /* 24 */
  gtuint16 nNameLen;     /* 26 */
  gtuint16 ctime;        /* 28 */
  gtuint16 cdate;        /* 30 */
  gtuint32 nFileDT;      /* 32 */
  gtuint16 entcrc;       /* 36 */
};

//--------------------------------------------------------------------
struct INNO_Header
//--------------------------------------------------------------------
{
  char     sID[8];
  gtuint32 nFileSize;
  char    _res1[52];
};

//--------------------------------------------------------------------
struct INNO_EndHeader
//--------------------------------------------------------------------
{
  gtint32 _res1[8];
  gtint32 _ofs1;
  gtint32 nSignatureOffset;
  gtint32 _res2;
};

//--------------------------------------------------------------------
struct INS_LocalHeader
//--------------------------------------------------------------------
{
  gtint32 _res1;
  gtint32 nPackedSize;
  gtint32 nOriginalSize;
  gtint32 _res2;
  gtint32 _res3;
};

//--------------------------------------------------------------------
struct ISC_GlobalHeader
//--------------------------------------------------------------------
{
  gtint32 nID;
  gtint32 _res1;
  gtint32 _res2;
  gtint32 nAnalyzeOffset;
  gtint32 _res3;
  gtint32 nDataOffset;
};

//--------------------------------------------------------------------
struct ISC_LocalHeader
//--------------------------------------------------------------------
{
  gtint32  nNameOffset;
  gtint32  _res1;
  gtuint16 nFlags;
  gtint32  nOriginalSize;
  gtint32  nPackedSize;
};

// ISO9660: 7.1.1
//--------------------------------------------------------------------
struct ISO_711
//--------------------------------------------------------------------
{
  gtuint8 nByte;
};

// ISO9660: 7.2.1
//--------------------------------------------------------------------
struct ISO_721
//--------------------------------------------------------------------
{
  gtuint16 nBigEndian;
};

// ISO9660: 7.2.2
//--------------------------------------------------------------------
struct ISO_722
//--------------------------------------------------------------------
{
  gtuint16 nLittleEndian;
};

// ISO9660: 7.2.3
//--------------------------------------------------------------------
struct ISO_723
//--------------------------------------------------------------------
{
  gtuint16 nBigEndian;
  gtuint16 nLittleEndian;
};

// ISO9660: 7.3.1
//--------------------------------------------------------------------
struct ISO_731
//--------------------------------------------------------------------
{
  gtuint32 nBigEndian;
};

// ISO9660: 7.3.2
//--------------------------------------------------------------------
struct ISO_732
//--------------------------------------------------------------------
{
  gtuint32 nLittleEndian;
};

// ISO9660: 7.3.3
//--------------------------------------------------------------------
struct ISO_733
//--------------------------------------------------------------------
{
  gtuint32 nBigEndian;
  gtuint32 nLittleEndian;
};

// ISO9660: 9.1.5
//--------------------------------------------------------------------
struct ISO_DateTime
//--------------------------------------------------------------------
{
  gtuint8 nYearsSince1900;
  gtuint8 nMonth;   // 1-12
  gtuint8 nDay;     // 1-31
  gtuint8 nHour;    // 0-23
  gtuint8 nMinute;  // 0-59
  gtuint8 nSecond;  // 0-59
  gtuint8 nGMTOffset;  // in 15 minute intervals from -48(west) to +52(East)
};

// ISO9660: 8.4.26.1
//--------------------------------------------------------------------
struct ISO_DateTimeExt
//--------------------------------------------------------------------
{
  gtuint32 nYear;    // 0-9999
  gtuint16 nMonth;   // 1-12
  gtuint16 nDay;     // 1-31
  gtuint16 nHour;    // 0-23
  gtuint16 nMinute;  // 0-59
  gtuint16 nSecond;  // 0-59
  gtuint16 nHSecond;
  gtuint8  nGMTOffset;  // in 15 minute intervals from -48(west) to +52(East)
};

//--------------------------------------------------------------------
struct ISO_DirectoryRecord
//--------------------------------------------------------------------
{
  gtuint8      nDirectoryRecordLength;     // in Bytes
  gtuint8      nExtAttributesRecordLength; // in Bytes
  ISO_733      nFirstLogicalBlock;
  ISO_733      nDataLength;
  ISO_DateTime aRecordingDT;
  gtuint8      nFileFlags;
  gtuint8      nFileUnitSize;
  gtuint8      nInterleaveGapSize;
  gtuint32     nVolumeSequenceNumber;
  gtuint8      nFilenameLength;
  // filename
  // padding byte for even alignment (2-byte boundary)
  // gtuint8 nSystemPaddingByte;
};

//--------------------------------------------------------------------
struct ISO_PathTableRecord
//--------------------------------------------------------------------
{
  gtuint8  nDirectoryIdentifierLength;      // in Bytes
  gtuint8  nExtAttributesRecordLength;      // in Bytes
  gtuint32 nLogicalBlockNumber;
  gtuint16 nRecordNumberInParentDirectory;
  // identifier
  // padding byte for even alignment (2-byte boundary)
};

//--------------------------------------------------------------------
struct ISO_PrimaryVolumeDesc
//--------------------------------------------------------------------
{
  ISO_711             nVolumeDescriptorType;     // == 0x01
  gtuint8             sStandardIdentifier[5];       // == CD001
  ISO_711             nVolumeDescriptorVersion;  // == 0x01
  gtuint8             nZero1;
  gtuint8             sA_CharctersAllowed[32];
  gtuint8             sD_CharctersAllowed[32];
  gtuint8             nZero2[8];
  ISO_733             nLogicalBlockCount;
  gtuint8             nZero3[32];
  ISO_723             nVolumeSetSize;
  ISO_723             nVolumeSequenceNumber;
  ISO_723             nLogicalBlockSize;
  ISO_733             nPathTableLength;          // in Bytes
  ISO_731             nLocationOfType_L_PathTable;
  ISO_731             nLocationOfOptionalType_L_PathTable;
  ISO_732             nLocationOfType_M_PathTable;
  ISO_732             nLocationOfOptionalType_M_PathTable;
  ISO_DirectoryRecord aRootDirectoryRecord;
  gtuint8             nRootDirectoryRecordPadding;
  gtuint8             sVolumeSetIdentifier[128];
  gtuint8             sPublisherIdentifier[128];
  gtuint8             sDataPreparerIdentifier[128];
  gtuint8             sApplicationIdentifier[128];
  gtuint8             sCopyrightFileIdentifier[37];
  gtuint8             sAbstractFileIdentifier[37];
  gtuint8             sBibliographicFileIdentifier[37];
  ISO_DateTimeExt     aVolumeCreationDT;
  ISO_DateTimeExt     aVolumeModificationDT;
  ISO_DateTimeExt     aVolumeExpirationDT;
  ISO_DateTimeExt     aVolumeEffectiveDT;
  ISO_711             nFileStructureVersion;
  gtuint8             nZero4;
  gtuint8             aApplicationUse[512];
  gtuint8             aReserved[653];
};

//--------------------------------------------------------------------
struct LG_Header
//--------------------------------------------------------------------
{
  gtuint8 nFlags;
  gtint32 nPackedSize;
  gtint32 nOriginalSize;
  gtint32 nCRC32;
  gtint32 nDateTime;
  // directoryname .. \0
  // filename .. \0
  // 2 more bytes
};

//--------------------------------------------------------------------
struct LZH_Header
//--------------------------------------------------------------------
{                            //ofs
  gtuint8 nHeaderSize;       //  0   0
  gtuint8 nCheckSum;         //  1   1
  char    sID[5];            //  2   2
  gtint32 nPackedSize;       //  7   7
  gtint32 nOriginalSize;     // 11   B
  union {
    struct {
      gtint32 nDateTime;        // 15   F
      gtuint8 nAttr;            // 19  13
      gtuint8 nHeaderVersion;   // 20  14
    } aNorm;
    struct {
      gtuint8  n00;             // 15   F
      gtuint8  n00_or_80;       // 16  10
      gtuint16 nTargetSegment;  // 17  11
      gtuint8  n20;             // 19  13
      gtuint8  n01;             // 20  14
    } aAward;
  };
};

//--------------------------------------------------------------------
struct LZO_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint8  aID[9];
  gtuint16 w1;
  gtuint16 w2;
  gtuint16 w3;
  gtuint8  b1;
  gtuint8  b2;
  gtint32  l1;
  gtint32  l2;
  gtint32  nDateTime;
  gtint32  l4;
};

//--------------------------------------------------------------------
struct NERO_Block
//--------------------------------------------------------------------
{
  gtuint32 nID;
  gtuint32 nSize;
};

//--------------------------------------------------------------------
struct NSIS13_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint32 flags;    // &1=CRC, &2=uninstall, &4=silent
  gtuint32 siginfo;  // FH_SIG
  gtuint32 nsinst[3]; // FH_INT1,FH_INT2,FH_INT3 (That's "NullSoftInst")

  // these point to the header+sections+entries+stringtable in the datablock
  gtuint32 length_of_header;
  gtuint32 header_ptr;

  // this specifies the length of all the data (including the firstheader and CRC)
  gtuint32 length_of_all_following_data;
};

//--------------------------------------------------------------------
struct NSIS16_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint32 flags; // FH_FLAGS_*
  gtuint32 siginfo;  // FH_SIG

  gtuint32 nsinst[3]; // FH_INT1,FH_INT2,FH_INT3

  // these point to the header+sections+entries+stringtable in the datablock
  gtuint32 length_of_header;

  // this specifies the length of all the data (including the firstheader and CRC)
  gtuint32 length_of_all_following_data;
};

//--------------------------------------------------------------------
struct PAQ_GlobalHeader
//--------------------------------------------------------------------
{
  gtint32 nID1;
  gtint32 nID2;
  gtuint8 _res1[13];
};

//--------------------------------------------------------------------
struct PAQ_LocalHeader
//--------------------------------------------------------------------
{
  gtuint8 _res1;
  gtuint8 nFilenameLength;
  char    sFilename[12];
  gtint32 nOriginalSize;
  gtint32 nPackedSize;
  gtuint8 _res2[7];
};

//--------------------------------------------------------------------
struct PPZ_GlobalHeader
//--------------------------------------------------------------------
{
  gtint32  nID;
  gtuint8  nMajorVersion;
  gtuint8  nMinorVersion;
  gtuint16 _res1;
  gtint32  nOriginalSize;
};

//--------------------------------------------------------------------
struct RAR_MarkHead
//--------------------------------------------------------------------
{
  gtuint16 nHeadCRC;
  gtuint8  nHeadType;
  gtuint16 nHeadFlags;
  gtuint16 _nHeadSize;  /* may have ADD_SIZE (4 bytes)!! */
};

//--------------------------------------------------------------------
struct RAR_MainHead
//--------------------------------------------------------------------
{
  // additional fiels
  gtuint16 _res1;
  gtint32  _res2;
};

//--------------------------------------------------------------------
struct RAR_FileHead
//--------------------------------------------------------------------
{
  gtuint32 nPackedSize;
  gtuint32 nOriginalSize;
  gtuint8  nHostOS;
  gtuint32 nCRC32;
  gtuint32 nDateTime;
  gtuint8  nExtractVersion;
  gtuint8  nMethod;
  gtuint16 nFilenameLen;
  union
  {
    gtuint32 nFileAttr;
    gtuint32 nSubFlags;
  };
  // high_pack_size if headflags & BITS[8]
  // high_unpack_size if headflags & BITS[8]
  // filename (nFilenameLen) bytes
  // comment (if headflags & 0x8)
};

//--------------------------------------------------------------------
struct RAR_FileCommentHead
//--------------------------------------------------------------------
{
  // RAR_MarkHead aMarkhead
  gtuint16 nOriginalSize;
  gtuint8  nExtractVersion;
  gtuint8  nMethod;
  gtuint16 nCRC16;
  // comment
};

//--------------------------------------------------------------------
struct RAR_SubHeader
//--------------------------------------------------------------------
{
  gtint32  nDataSize;
  gtuint16 nSubType;
  gtuint8  _res1;    /* 0x00 */
  // other fields
};

//--------------------------------------------------------------------
struct RAR_OS2SubHeader
//--------------------------------------------------------------------
{
  gtint32  nDataSize;
  gtuint16 nSubType;  // 0x0100
  gtuint8  _res1;     // 0x00
  gtint32  nOriginalSize;
  gtuint8  nExtractVersion;
  gtuint8  nMethod;
  gtint32  nExtAttrCRC;
};

//--------------------------------------------------------------------
struct SBC_Header
//--------------------------------------------------------------------
{
  char     sID[3];
  gtuint16 nVersionAndCipherID;  /* 13b 3b */
  gtuint8  IV[16];
  gtuint8  aKeyHash[16];
};

//--------------------------------------------------------------------
struct SONY_Header
//--------------------------------------------------------------------
{
  gtuint32 nID;
  gtuint32 _res1;
  gtuint32 _res2;
  gtuint32 _null1;

  gtuint32 _null2;
  gtuint32 nPackedSize;
  gtuint32 _res3;
  gtuint32 _res4;

  gtuint8  _res5[0x1e0];
};

//--------------------------------------------------------------------
struct STI_GlobalHeader
//--------------------------------------------------------------------
{
  gtint32  nID;                   /*  0 */
  gtuint8  _res1[8];              /*  4 */
  gtuint16 nFilesInArchive;       /* 12 */
  gtint32  _res2;                 /* 14 */
  gtint32  nArchiveSize;          /* 18 */
  gtint32  nOriginalSize;         /* 22 */
  gtuint8  _res3a[4];             /* 26 */
  gtuint8  nTotalDisks;           /* 30 */
  gtuint8  nCurrentDisk;          /* 31 */
  gtuint8  _res3b[9];             /* 32 */
  gtint32  nDataOffsetWithDirs;   /* 41 */
  gtuint8  _res4[6];              /* 45 */
  gtint32  nDataOffset;           /* 51 */
  gtuint8  _res5;                 /* 55 */
};

//--------------------------------------------------------------------
struct STI_DirHeader
//--------------------------------------------------------------------
{
  gtuint8  _res1;           /*  0 */
  gtuint8  nID;             /*  1 */
  gtuint16 nHeaderSize;     /*  2 */
  gtuint16 nFilenameLen;    /*  4 */
  // 1 + 4 bytes
};

//--------------------------------------------------------------------
struct STI_FileHeader
//--------------------------------------------------------------------
{
  gtuint8  nDiskID;           /*  0 */
  gtuint8  nDirID;            /*  1 */
  gtuint8  nID;   // 00       /*  2 */
  gtint32  nOriginalSize;     /*  3 */
  gtint32  nPackedSize;       /*  7 */
  gtuint8  _res2[12];         /* 11 */
  gtuint16 nHeaderSize;       /* 23 */
  gtint32  _res3;             /* 25 */
  gtuint8  nFilenameLen;      /* 29 */
  // 1 + 12 bytes
};

//--------------------------------------------------------------------
struct SYN_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint32 nID;
  gtuint32 nNumberOfFiles;
  gtuint8  _res1[8];
};

//--------------------------------------------------------------------
struct SYN_LocalHeader
//--------------------------------------------------------------------
{
  char     sFilename[24];
  gtuint32 nLocalOffset;
  gtuint32 nSize;
};

//--------------------------------------------------------------------
struct TAR_Header
//--------------------------------------------------------------------
{
  char sName[100];
  char sMode[8];
  char sUID[8];
  char sGID[8];
  char sSize[12];
  char sMTime[12];
  char sChkSum[8];
  char sLinkFlag;
  char sLinkName[100];
  char sMagic[8];
  char sUName[32];
  char sGName[32];
  char sDevMajor[8];
  char sDevMinor[8]; // now at byte 345
  char res_1[167];
};

//--------------------------------------------------------------------
struct XPA_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint32 nID;
  gtuint8  nVersion;
};

//--------------------------------------------------------------------
struct XPA_LocalHeader
//--------------------------------------------------------------------
{
  // nFilenameLen:byte;
  // .. filename ..
  gtuint32 nOldSize;
  gtuint32 nNewSize;
};

//--------------------------------------------------------------------
struct YC_GlobalHeader
//--------------------------------------------------------------------
{
  gtint32  _res1;
  gtint32  nSectionSize;
  gtint32  _res2;
  gtint32  nID;
  gtint32  _res3;
  gtuint16 nFilesInSection;
  gtuint16 _res4;
  gtuint16 nFilesInArchive;
  gtuint8  _res5[29];
};

//--------------------------------------------------------------------
struct YC_LocalHeader
//--------------------------------------------------------------------
{
  gtuint16 nFilenameLen;
  gtint32  nDateTime;
  gtuint16 nAttr;
  gtint32  nOriginalSize;
  gtuint16 nCRC16;
  gtint32  _res1;
  // filename
};

//--------------------------------------------------------------------
struct ZET_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint16 nID;
  gtuint8  nSubType;
  gtuint8  nHeaderSize;   // incl. ID
};

//--------------------------------------------------------------------
struct ZET_LocalHeader
//--------------------------------------------------------------------
{
  gtuint16 _res1;
  gtint32  nPackedSize;
  gtint32  _res2;
  gtint32  _res3;
  gtint32  nDateTime;
  gtuint8  nAttr;
  gtuint8  _res5;
  gtint32  nOriginalSize;
  // name #0
};

//--------------------------------------------------------------------
struct ZIP_CDS
//--------------------------------------------------------------------
{
  gtint32  nID;
  gtuint16 nPackerVersion;
  gtuint16 nExtractVersion;
  gtuint16 nFlags;
  gtuint16 nCompressionMethod;
  gtint32  nLastModifiedDT;
  gtint32  nCRC32;
  gtint32  nPackedSize;
  gtint32  nOriginalSize;
  gtuint16 nFilenameLen;
  gtuint16 nExtraLen;
  gtuint16 nCommentLen;
  gtuint16 nDiskNumberStart;
  gtuint16 nInternalAttr;
  gtint32  nExternalAttr;
  gtint32  nLocalOffset;
};

//--------------------------------------------------------------------
struct ZIP_CDSE
//--------------------------------------------------------------------
{
  gtuint32 nID;
  gtuint16 nDiskNumber;
  gtuint16 nCDStartDisk;
  gtuint16 nCDEntriesOnDisk;
  gtuint16 nCDEntries;
  gtuint32 nCDSize;
  gtuint32 nOffsetToFirstDisk;
  gtuint16 nCommentLen;
};

//--------------------------------------------------------------------
struct ZIP_LFH
//--------------------------------------------------------------------
{
  gtuint32 nID;                  /* 0x00  0 */
  gtuint16 nExtractVersion;      /* 0x04  4 */
  gtuint16 nFlags;               /* 0x06  6 */
  gtuint16 nCompressionMethod;   /* 0x08  8 */
  gtuint32 nDT;                  /* 0x0A 10 */
  gtuint32 nCRC32;               /* 0x0E 14 */
  gtuint32 nPackedSize;          /* 0x12 18 */
  gtuint32 nOriginalSize;        /* 0x16 22 */
  gtuint16 nFilenameLen;         /* 0x1A 26 */
  gtuint16 nExtraLen;            /* 0x1C 28 */
};

// only if LFH.nFlags & 8 > 0 (for STDIN data)
//--------------------------------------------------------------------
struct ZIP_DD
//--------------------------------------------------------------------
{
  gtint32 nCRC32;
  gtint32 nPackedSize;
  gtint32 nOriginalSize;
  gtint32 nAttr;
};

//--------------------------------------------------------------------
struct ZOO_GlobalHeader
//--------------------------------------------------------------------
{
  gtuint8  sHeader[20];
  gtuint32 nID;
  gtuint32 nOffsetOfFirstFile;
  gtuint32 _res1;
  gtuint8  nPackVersion;
  gtuint8  nExtractVersion;
};

//--------------------------------------------------------------------
struct ZOO_LocalHeader
//--------------------------------------------------------------------
{
  gtint32  nID;
  gtuint8  nType;
  gtuint8  nMethod;
  gtuint32 nOffsetNextDir;
  gtuint32 nOffsetNextHeader;
  gtuint32 nDateTime;
  gtuint16 nCRC16;
  gtuint32 nOriginalSize;
  gtuint32 nPackedSize;
  gtuint8  nPackversion;
  gtuint8  nExtractVersion;
  gtuint8  nDeleteFlag;
  gtuint32 nOffsetComment;  // 0 == null
  gtuint16 nCommentLen;
  gtuint8  _res1;
  // filename..#0
};

#if (defined _MSC_VER) || (defined __GNUC__)
#pragma pack (pop)
#endif

DECLARE_SIZEOF (GT_777_GH_SIZE,           _777_GlobalHeader,       8)
DECLARE_SIZEOF (GT_777_SGH_SIZE,          _777_SolidGlobalHeader,  19)
DECLARE_SIZEOF (GT_777_LH_SIZE,           _777_LocalHeader,        56)
DECLARE_SIZEOF (GT_777_SLH_SIZE,          _777_SolidLocalHeader,   39)
DECLARE_SIZEOF (GT_ACE_GH_SIZE,           ACE_GlobalHeader,        16)
DECLARE_SIZEOF (GT_ACE_FH_SIZE,           ACE_LocalHeaderFile,     35)
DECLARE_SIZEOF (GT_ACE_EH_SIZE,           ACE_LocalHeaderEnvelope, 11)
DECLARE_SIZEOF (GT_AIN_GH_SIZE,           AIN_GlobalHeader,        18)
DECLARE_SIZEOF (GT_AKT7_GH_SIZE,          AKT7_GlobalHeader,       13)
DECLARE_SIZEOF (GT_AKT7_LH_SIZE,          AKT7_LocalHeader,        26)
DECLARE_SIZEOF (GT_AKT9_GH_SIZE,          AKT9_GlobalHeader,       9)
DECLARE_SIZEOF (GT_AKT9_LH_SIZE,          AKT9_LocalHeader,        31)
DECLARE_SIZEOF (GT_AMG_GH_SIZE,           AMG_GlobalHeader,        12)
DECLARE_SIZEOF (GT_AMG_LH_SIZE,           AMG_LocalHeader,         35)
DECLARE_SIZEOF (GT_ARC_H_SIZE,            ARC_Header,              29)
DECLARE_SIZEOF (GT_ARG_H_SIZE,            ARG_Header,              26)
DECLARE_SIZEOF (GT_ARJ_GH_SIZE,           ARJ_GlobalHeader,        34)
DECLARE_SIZEOF (GT_ARJ_LH_SIZE,           ARJ_LocalHeader,         34)
DECLARE_SIZEOF (GT_ARQ_H_SIZE,            ARQ_Header,              8)
DECLARE_SIZEOF (GT_ARQ_B_SIZE,            ARQ_Body,                33)
DECLARE_SIZEOF (GT_ARX_H_SIZE,            ARX_Header,              22)
DECLARE_SIZEOF (GT_ASD_GH_SIZE,           ASD_GlobalHeader,        8)
DECLARE_SIZEOF (GT_ASD_LH_SIZE,           ASD_LocalHeader,         14)
DECLARE_SIZEOF (GT_BIX_GH_SIZE,           BIX_GlobalHeader,        24)
DECLARE_SIZEOF (GT_BIX_LH_SIZE,           BIX_LocalHeader,         34)
DECLARE_SIZEOF (GT_BLI_GH_SIZE,           BLI_GlobalHeader,        18)
DECLARE_SIZEOF (GT_BLI_SH_SIZE,           BLI_SkipHeader,          3)
DECLARE_SIZEOF (GT_BLI_LH_SIZE,           BLI_LocalHeader250,      22) //  same size as 251!)
DECLARE_SIZEOF (GT_BOA_GH_SIZE,           BOA_GlobalHeader,        7)
DECLARE_SIZEOF (GT_BOA_LH_SIZE,           BOA_LocalHeader,         20)
DECLARE_SIZEOF (GT_BSA_H_SIZE,            BSA_Header,              23)
DECLARE_SIZEOF (GT_BSN_H1_SIZE,           BSN_Header1,             14)
DECLARE_SIZEOF (GT_BSN_H2_SIZE,           BSN_Header2,             16)
DECLARE_SIZEOF (GT_CAB_GH_SIZE,           CAB_GlobalHeader,        32)
DECLARE_SIZEOF (GT_CAB_LH_SIZE,           CAB_LocalHeader,         16)
DECLARE_SIZEOF (GT_DPA_H_SIZE,            DPA_Header,              44)
DECLARE_SIZEOF (GT_HA_GH_SIZE,            HA_GlobalHeader,         4)
DECLARE_SIZEOF (GT_HA_LH_SIZE,            HA_LocalHeader,          17)
DECLARE_SIZEOF (GT_INNO_H_SIZE,           INNO_Header,             64)
DECLARE_SIZEOF (GT_INNO_EH_SIZE,          INNO_EndHeader,          44)
DECLARE_SIZEOF (GT_IMP_GH_SIZE,           IMP_GlobalHeader,        42)
DECLARE_SIZEOF (GT_IMP_LH_SIZE,           IMP_LocalHeader,         38)
DECLARE_SIZEOF (GT_INS_LH_SIZE,           INS_LocalHeader,         20)
DECLARE_SIZEOF (GT_ISC_GH_SIZE,           ISC_GlobalHeader,        24)
DECLARE_SIZEOF (GT_ISC_LH_SIZE,           ISC_LocalHeader,         18)
DECLARE_SIZEOF (GT_ISO_711_SIZE,          ISO_711,                 1)
DECLARE_SIZEOF (GT_ISO_721_SIZE,          ISO_721,                 2)
DECLARE_SIZEOF (GT_ISO_722_SIZE,          ISO_722,                 2)
DECLARE_SIZEOF (GT_ISO_723_SIZE,          ISO_723,                 4)
DECLARE_SIZEOF (GT_ISO_731_SIZE,          ISO_731,                 4)
DECLARE_SIZEOF (GT_ISO_732_SIZE,          ISO_732,                 4)
DECLARE_SIZEOF (GT_ISO_733_SIZE,          ISO_733,                 8)
DECLARE_SIZEOF (GT_ISO_DT_SIZE,           ISO_DateTime,            7)
DECLARE_SIZEOF (GT_ISO_DTX_SIZE,          ISO_DateTimeExt,         17)
DECLARE_SIZEOF (GT_ISO_DR_SIZE,           ISO_DirectoryRecord,     33)
DECLARE_SIZEOF (GT_ISO_PTR_SIZE,          ISO_PathTableRecord,     8)
DECLARE_SIZEOF (GT_ISO_PVD_SIZE,          ISO_PrimaryVolumeDesc,   2048)
DECLARE_SIZEOF (GT_LG_H_SIZE,             LG_Header,               17)
DECLARE_SIZEOF (GT_LZH_H_SIZE,            LZH_Header,              21)
DECLARE_SIZEOF (GT_LZO_GH_SIZE,           LZO_GlobalHeader,        33)
DECLARE_SIZEOF (GT_NERO_BLOCK_SIZE,       NERO_Block,              8)
DECLARE_SIZEOF (GT_NSIS13_GH_SIZE,        NSIS13_GlobalHeader,     32)
DECLARE_SIZEOF (GT_NSIS16_GH_SIZE,        NSIS16_GlobalHeader,     28)
DECLARE_SIZEOF (GT_PAQ_GH_SIZE,           PAQ_GlobalHeader,        21)
DECLARE_SIZEOF (GT_PAQ_LH_SIZE,           PAQ_LocalHeader,         29)
DECLARE_SIZEOF (GT_PPZ_GH_SIZE,           PPZ_GlobalHeader,        12)
DECLARE_SIZEOF (GT_RAR_MARKHEAD_SIZE,     RAR_MarkHead,            7)
DECLARE_SIZEOF (GT_RAR_ADD_MAINHEAD_SIZE, RAR_MainHead,            6)
DECLARE_SIZEOF (GT_RAR_ADD_FILEHEAD_SIZE, RAR_FileHead,            25)
DECLARE_SIZEOF (GT_RAR_ADD_COMMENTH_SIZE, RAR_FileCommentHead,     6)
DECLARE_SIZEOF (GT_RAR_SH_SIZE,           RAR_SubHeader,           7)
DECLARE_SIZEOF (GT_RAR_OH_SIZE,           RAR_OS2SubHeader,        17)
DECLARE_SIZEOF (GT_SBC_H_SIZE,            SBC_Header,              37)
DECLARE_SIZEOF (GT_SONY_H_SIZE,           SONY_Header,             0x200)
DECLARE_SIZEOF (GT_STI_GH_SIZE,           STI_GlobalHeader,        56)
DECLARE_SIZEOF (GT_STI_DH_SIZE,           STI_DirHeader,           6)
DECLARE_SIZEOF (GT_STI_FH_SIZE,           STI_FileHeader,          30)
DECLARE_SIZEOF (GT_SYN_GH_SIZE,           SYN_GlobalHeader,        16)
DECLARE_SIZEOF (GT_SYN_LH_SIZE,           SYN_LocalHeader,         32)
DECLARE_SIZEOF (GT_TAR_H_SIZE,            TAR_Header,              0x200)
DECLARE_SIZEOF (GT_XPA_GH_SIZE,           XPA_GlobalHeader,        5)
DECLARE_SIZEOF (GT_XPA_LH_SIZE,           XPA_LocalHeader,         8)
DECLARE_SIZEOF (GT_YC_GH_SIZE,            YC_GlobalHeader,         55)
DECLARE_SIZEOF (GT_YC_LH_SIZE,            YC_LocalHeader,          18)
DECLARE_SIZEOF (GT_ZET_GH_SIZE,           ZET_GlobalHeader,        4)
DECLARE_SIZEOF (GT_ZET_LH_SIZE,           ZET_LocalHeader,         24)
DECLARE_SIZEOF (GT_ZIP_LFH_SIZE,          ZIP_LFH,                 30)
DECLARE_SIZEOF (GT_ZIP_DD_SIZE,           ZIP_DD,                  16)
DECLARE_SIZEOF (GT_ZIP_CDS_SIZE,          ZIP_CDS,                 46)
DECLARE_SIZEOF (GT_ZIP_CDSE_SIZE,         ZIP_CDSE,                22)
DECLARE_SIZEOF (GT_ZOO_GH_SIZE,           ZOO_GlobalHeader,        34)
DECLARE_SIZEOF (GT_ZOO_LH_SIZE,           ZOO_LocalHeader,         38)

#endif
