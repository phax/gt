#include "gt_exe_ne.hxx"

#include "gtc_data.hxx"
#include "gt_cmdline_params.hxx"
#include "gt_compare.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_table.hxx"
#include "gt_utils.hxx"

namespace GT {

// these defines must be < 0!!
const gtuint32 GT_EXE_NE_SGLHDL_ERR_NOENTRYPOINT = gtuint32 (-1);
const gtuint32 GT_EXE_NE_SGLHDL_ERR_TOOLARGE     = gtuint32 (-2);
const gtuint32 GT_EXE_NE_SGLHDL_ERR_EMPTY        = gtuint32 (-3);

//--------------------------------------------------------------------
EXE_NE_HeaderHdl::EXE_NE_HeaderHdl
                                        (const gtuint32    nNEOffset,
                                               FileBuffer* pBuffer)
//--------------------------------------------------------------------
  : BasicBuffer (pBuffer),
    m_nNEOffset (nNEOffset)
{
  // read header
  if (!m_pBuffer->GetBuffer (m_nNEOffset, &m_aHeader, EXE_NE_HEADER_SIZE))
    out_error_append (_T ("Error: failed to init NE EXE handler\n"));
}

//--------------------------------------------------------------------
EXE_NE_SegmentHandler::EXE_NE_SegmentHandler
                                        (FileBuffer*       pBuffer,
                                         EXE_NE_HeaderHdl* pHeader)
//--------------------------------------------------------------------
  : BasicBuffer (pBuffer),
    m_pHeader   (pHeader),
    m_pSegments (NULL)
{
  ASSERT (pHeader);

  m_pEXEHeader = (EXE_NE_Header*) m_pHeader->GetpHeader ();
  ASSERT (m_pEXEHeader);

  // init basic member vars
  m_nSectorSize = m_pHeader->GetSectorSize ();
  m_nTableLen   = m_pEXEHeader->nSegmentCount;
  const size_t nTableSize  = m_nTableLen * EXE_NE_SEGEMENT_SIZE;

  // and read the segments...
  if (m_nTableLen > 0)
  {
    m_pSegments = new EXE_NE_Segment [m_nTableLen];
    if (!m_pBuffer->GetBuffer (m_pHeader->GetAbsoluteSegmentTableOffset (),
                               m_pSegments,
                               nTableSize))
    {
      out_error_append (rc (HRC_EXELIB_NE_ERROR_READ_SEGMENT_TABLE));
    }
  }
  else
  {
    // no segment table present
    out_error_append (rc (HRC_EXELIB_NE_NO_VALID_SEGMENT_TABLE));
  }
}

//--------------------------------------------------------------------
EXE_NE_SegmentHandler::~EXE_NE_SegmentHandler ()
//--------------------------------------------------------------------
{
  delete [] m_pSegments;
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_NE_SegmentHandler::GetEndPos
                                        (const size_t n) const
//--------------------------------------------------------------------
{
  ASSERT (n >= 0 && n < m_nTableLen);

  if (m_pSegments[n].nRelativeOffset == 0)
    return 0;
  else
    return GetStartPos (n) + GetLength (n);
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_NE_SegmentHandler::GetLength
                                        (const size_t n) const
//--------------------------------------------------------------------
{
  ASSERT (n >= 0 && n < m_nTableLen);

  if (m_pSegments[n].nLength == 0 && m_pSegments[n].nRelativeOffset != 0)
    return 0x10000;
  else
    return m_pSegments[n].nLength;
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_NE_SegmentHandler::GetStartPos
                                        (const size_t n) const
//--------------------------------------------------------------------
{
  ASSERT (n >= 0 && n < m_nTableLen);
  return m_pSegments[n].nRelativeOffset * m_nSectorSize;
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_NE_SegmentHandler::GetAllocSize
                                        (const size_t n) const
//--------------------------------------------------------------------
{
  ASSERT (n >= 0 && n < m_nTableLen);
  return m_pSegments[n].nAllocSize;
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_NE_SegmentHandler::GetEntryPoint () const
//--------------------------------------------------------------------
{
  if (m_pEXEHeader->nCS == 0)
    return GT_EXE_NE_SGLHDL_ERR_NOENTRYPOINT;

  if (m_pEXEHeader->nCS > m_nTableLen)
    return GT_EXE_NE_SGLHDL_ERR_TOOLARGE;

  if (m_pHeader->HasSelfLoader ())
    return GetStartPos (1 - 1);   // CS:IP = 0001:0000  counter starts at 0 and not at 1!!

  const gtuint32 nStartPos = GetStartPos (m_pEXEHeader->nCS - 1);

  return (nStartPos == 0)
           ? GT_EXE_NE_SGLHDL_ERR_EMPTY
           : nStartPos + m_pEXEHeader->nIP;
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_NE_SegmentHandler::GetCalculatedFileSize () const
//--------------------------------------------------------------------
{
  if (m_nTableLen == 0)
    return 0;

  size_t nIndex = m_nTableLen;
  gtuint32 nEndPos;

  while (nIndex > 0)
  {
    if ((nEndPos = GetEndPos (nIndex - 1)) > 0)
      return nEndPos;

    nIndex--;
  }

  return 0;
}

//--------------------------------------------------------------------
LPCTSTR GT_CALL EXE_NE_SegmentHandler::GetType
                                        (const size_t n) const
//--------------------------------------------------------------------
{
  ASSERT (n >= 0 && n < m_nTableLen);
  return (m_pSegments [n].nFlags & NE_SEG_IS_DATA)
            ? _T ("DATA")
            : _T ("CODE");
}

//--------------------------------------------------------------------
bool GT_CALL EXE_NE_Lister::Check_NE_0J
                                        (const gtuint32 nStartPos)
//--------------------------------------------------------------------
{
  gtuint8 aBuffer[EXE_BUFFER_SIZE];
  m_pBuffer->GetBuffer (nStartPos, aBuffer, EXE_BUFFER_SIZE);

  SignatureCmp aCompare (aBuffer,
                         GT_COMPARE_MAGIC_NE,
                         CmdlineParam_GetLong (GT_FLAG_FTN),
                         m_pBuffer,
                         NULL);

  for (int i = 0; i < GT_NE_0J_Count (); i++)
    if (aCompare.Matches (GT_NE_0J_Index (i)) == GTCOMP_MATCH)
      return true;

  return false;
}

//--------------------------------------------------------------------
void GT_CALL EXE_NE_Lister::Execute ()
//--------------------------------------------------------------------
{
  out_append (_T ("\n"));
  out_info_format (rc (HRC_EXELIB_NE_AT_OFFSET),
                   m_nNEOffset, m_nNEOffset);
  out_incindent ();

  EXE_NE_SegmentHandler aSegmentHandler (m_pBuffer, this);

  // list non resident names
  {
    const gtuint8 nLen = m_pBuffer->GetUint8 (m_aHeader.nAbsoluteOfsToNonresidentNamesTable);

    pstring sNonResidentName;
    m_pBuffer->GetFixedSizeAString (sNonResidentName, nLen);

    out_format (_T ("'%s'\n"), sNonResidentName.c_str ());
  }

  // handle the entrypoint
  const gtuint32 nEntryPoint = aSegmentHandler.GetEntryPoint ();
  // calculate the file size according to the segment handler
  const gtuint32 nCalculatedFileSize = aSegmentHandler.GetCalculatedFileSize ();
  const gtuint32 nAbsSegTabOfs    = GetAbsoluteSegmentTableOffset ();
  const gtuint32 nAbsResTabOfs    = GetAbsoluteResourceTableOffset ();
  const gtuint32 nAbsResNamTabOfs = GetAbsoluteResidentNamesTableOffset ();
  const gtuint32 nAbsNodRefTabOfs = GetAbsoluteModuleReferenceTableOffset ();
  const gtuint32 nAbsImportTabOfs = GetAbsoluteImportedNamesTableOffset ();
  const gtuint32 nAbsEPointTabOfs = GetAbsoluteEntryPointTableOffset ();

  switch (nEntryPoint)
  {
    case GT_EXE_NE_SGLHDL_ERR_NOENTRYPOINT:
    {
      out_info_append (rc (HRC_EXELIB_NE_NO_EP));
      break;
    }
    case GT_EXE_NE_SGLHDL_ERR_TOOLARGE:
    {
      out_error_append (rc (HRC_EXELIB_NE_CS_TOO_LARGE));
      break;
    }
    case GT_EXE_NE_SGLHDL_ERR_EMPTY:
    {
      out_error_append (rc (HRC_EXELIB_NE_CS_EMPTY));
      break;
    }
    default:
    {
      if (nEntryPoint < 0 || file_t (nEntryPoint) > m_pBuffer->GetFileSize ())
      {
        out_error_append (rc (HRC_EXELIB_NE_EP_IS_INVALID));
      }
      else
      {
        out_format (rc (HRC_EXELIB_NE_EP),
                    nEntryPoint, nEntryPoint);

        // check for code at entrypoint
        Check_NE_0J (nEntryPoint);
      }
      break;
    }
  }

  // check for self loader
  if (HasSelfLoader ())
    out_info_append (rc (HRC_EXELIB_NE_SELFLOADER));

  // and print some header data...
  {
    Table aTable;
    aTable.EnableHeadline (false);
    aTable.AddColumn (_T (""), STR,       40, ALIGN_LEFT);
    aTable.AddColumn (_T (""), NUM_SPACE, 12, 10);
    aTable.AddColumn (_T (""), HEX,        9, 8);

    aTable.AddStr (0, HRC_EXELIB_NE_CALCED_EXE_SIZE);
    aTable.AddInt (1, nCalculatedFileSize);
    aTable.AddInt (2, nCalculatedFileSize);

    aTable.AddStr (0, HRC_EXELIB_NE_SEG_TABLE_OFS);
    aTable.AddInt (1, nAbsSegTabOfs);
    aTable.AddInt (2, nAbsSegTabOfs);

    aTable.AddStr (0, HRC_EXELIB_NE_RES_TABLE_OFS);
    aTable.AddInt (1, nAbsResTabOfs);
    aTable.AddInt (2, nAbsResTabOfs);

    aTable.AddStr (0, HRC_EXELIB_NE_NAMES_TABLE_OFS);
    aTable.AddInt (1, nAbsResNamTabOfs);
    aTable.AddInt (2, nAbsResNamTabOfs);

    aTable.AddStr (0, HRC_EXELIB_NE_MODULE_REF_TABLE_OFS);
    aTable.AddInt (1, nAbsNodRefTabOfs);
    aTable.AddInt (2, nAbsNodRefTabOfs);

    aTable.AddStr (0, HRC_EXELIB_NE_IMP_NAMES_TABLE_OFS);
    aTable.AddInt (1, nAbsImportTabOfs);
    aTable.AddInt (2, nAbsImportTabOfs);

    aTable.AddStr (0, HRC_EXELIB_NE_EP_TABLE_OFS);
    aTable.AddInt (1, nAbsEPointTabOfs);
    aTable.AddInt (2, nAbsEPointTabOfs);

    out_table (&aTable);
  }

  // write target OS
  out_append (rc (HRC_EXELIB_NE_TARGET_OS));
  switch (m_aHeader.nTargetOS)
  {
    case GT_EXE_NE_OS_UNKNOWN:
      out_append (rc (HRC_EXELIB_NE_OS_UNKNOWN));
      break;
    case GT_EXE_NE_OS_OS2:
    {
      out_append (rc (HRC_EXELIB_NE_OS_OS2));

      out_incindent ();
      if (m_aHeader.nOtherEXEFlags & BITS[0])
        out_append (rc (HRC_EXELIB_NE_OS_OS2_LFN_SUPPORT));
      if (m_aHeader.nOtherEXEFlags & BITS[1])
        out_append (rc (HRC_EXELIB_NE_OS_OS2_PROTECTED_MODE));
      if (m_aHeader.nOtherEXEFlags & BITS[2])
        out_append (rc (HRC_EXELIB_NE_OS_OS2_PROPORTIONAL_FONT));
      if (m_aHeader.nOtherEXEFlags & BITS[3])
        out_append (rc (HRC_EXELIB_NE_OS_OS2_FASTLOAD));
      out_decindent ();

      break;
    }
    case GT_EXE_NE_OS_WINDOWS:
      out_format (rc (HRC_EXELIB_NE_OS_WINDOWS),
                  m_aHeader.nExpectedWinVersionMajor,
                  m_aHeader.nExpectedWinVersionMinor);
      break;
    case GT_EXE_NE_OS_DOS4:
      out_info_append (rc (HRC_EXELIB_NE_OS_EUROPEAN_MSDOS));
      break;
    case GT_EXE_NE_OS_WIN386:
      out_append (rc (HRC_EXELIB_NE_OS_WINDOWS386));
      break;
    case GT_EXE_NE_OS_BOSS:
      out_info_append (rc (HRC_EXELIB_NE_OS_BOSS));
      break;
    case GT_EXE_NE_OS_PHARLAP_OS2:
      out_info_append (rc (HRC_EXELIB_NE_OS_PHAR_LAP_OS2));
      break;
    case GT_EXE_NE_OS_PHARLAP_WIN:
      out_info_append (rc (HRC_EXELIB_NE_OS_PHAR_LAP_WIN));
      break;
    default:
      out_info_format (rc (HRC_EXELIB_NE_OS_UNKNOWN_NUM),
                       m_aHeader.nTargetOS);
      break;
  }

  // write linker version
  out_format (rc (HRC_EXELIB_NE_LINKER_VERSION),
              m_aHeader.nLinkerMajorVersion,
              m_aHeader.nLinkerMinorVersion);

  // print flags table
  {
    out_append (rc (HRC_EXELIB_NE_PROGRAM_FLAGS));

    // get global yes/no strings
    pstring sYes, sNo;
    str_assign_from_res (sYes, HRC_YES);
    str_assign_from_res (sNo,  HRC_NO);

    // start table
    Table aTable;
    aTable.EnableHeadline (false),
    aTable.AddColumn (_T (""), STR, 37, ALIGN_LEFT);
    aTable.AddColumn (_T (""), STR, 3,  ALIGN_LEFT);

    aTable.AddStr (0, HRC_EXELIB_NE_FLAGS_DLL);
    aTable.AddStr (1, m_aHeader.nProgramFlags & BITS[0] ? sYes : sNo);

    aTable.AddStr (0, HRC_EXELIB_NE_FLAGS_APP);
    aTable.AddStr (1, m_aHeader.nProgramFlags & BITS[1] ? sYes : sNo);

    aTable.AddStr (0, HRC_EXELIB_NE_FLAGS_PROTMODE);
    aTable.AddStr (1, m_aHeader.nProgramFlags & BITS[3] ? sYes : sNo);

    aTable.AddStr (0, HRC_EXELIB_NE_FLAGS_8086);
    aTable.AddStr (1, m_aHeader.nProgramFlags & BITS[4] ? sYes : sNo);

    aTable.AddStr (0, HRC_EXELIB_NE_FLAGS_80286);
    aTable.AddStr (1, m_aHeader.nProgramFlags & BITS[5] ? sYes : sNo);

    aTable.AddStr (0, HRC_EXELIB_NE_FLAGS_80386);
    aTable.AddStr (1, m_aHeader.nProgramFlags & BITS[6] ? sYes : sNo);

    aTable.AddStr (0, HRC_EXELIB_NE_FLAGS_80x87);
    aTable.AddStr (1, m_aHeader.nProgramFlags & BITS[7] ? sYes : sNo);

    out_incindent ();
    out_table (&aTable);
    out_decindent ();
  }

  // print the segment table
  if (m_aHeader.nSegmentCount > 0)
  {
    out_append (rc (HRC_EXELIB_NE_SEG_TABLE_LISTING));

    Table aTable;
    aTable.AddColumn (rc (HRC_EXELIB_NE_SEG_TABLE_NR),       NUM_SPACE, 4,  2);
    aTable.AddColumn (rc (HRC_EXELIB_NE_SEG_TABLE_TYPE),     STR,       6,  ALIGN_LEFT);
    aTable.AddColumn (rc (HRC_EXELIB_NE_SEG_TABLE_STARTPOS), HEX,       11, 8);
    aTable.AddColumn (rc (HRC_EXELIB_NE_SEG_TABLE_ENDPOS),   HEX,       11, 8);
    aTable.AddColumn (rc (HRC_EXELIB_NE_SEG_TABLE_SIZE),     HEX,       7,  4);
    aTable.AddColumn (rc (HRC_EXELIB_NE_SEG_TABLE_ALLOC),    HEX,       5,  4);

    for (gtuint32 i = 0; i < m_aHeader.nSegmentCount; i++)
    {
      aTable.AddInt (0, i);
      aTable.AddStr (1, aSegmentHandler.GetType (i));
      aTable.AddInt (2, aSegmentHandler.GetStartPos (i));
      aTable.AddInt (3, aSegmentHandler.GetEndPos (i));
      aTable.AddInt (4, aSegmentHandler.GetLength (i));
      aTable.AddInt (5, aSegmentHandler.GetAllocSize (i));
    }

    out_incindent ();
    out_table (&aTable);
    out_decindent ();
  }

  // check for overlays!!
  m_nOverlayOffset = nCalculatedFileSize;

  out_decindent ();
}

}  // namespace
