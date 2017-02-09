#include "gt_exe_pe.hxx"

#include "gt_cmdline_params.hxx"
#include "gt_errormsg.hxx"
#include "gt_exception.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_utils.hxx"
#include "gt_locale.hxx"

namespace GT {

#define RES_TYPE_BYTES  0
#define RES_TYPE_STRING 1

/*! Get the name of the PE resource type from resource.
 */
//--------------------------------------------------------------------
static void _getResourceTypeString
                                        (gtuint32 nIndex,
                                         pstring& sBuf)
//--------------------------------------------------------------------
{
  static const resid_t RESOURCE_TYPE_NAMES [] = {
    HRC_GTLIB_PERESTYPE_CURSOR,
    HRC_GTLIB_PERESTYPE_BITMAP,
    HRC_GTLIB_PERESTYPE_ICON,
    HRC_GTLIB_PERESTYPE_MENU,
    HRC_GTLIB_PERESTYPE_DIALOG,
    HRC_GTLIB_PERESTYPE_STRING,
    HRC_GTLIB_PERESTYPE_FONTDIR,
    HRC_GTLIB_PERESTYPE_FONT,
    HRC_GTLIB_PERESTYPE_ACCEL,
    HRC_GTLIB_PERESTYPE_RCDATA,
    HRC_GTLIB_PERESTYPE_MSGTABLE,
    HRC_GTLIB_PERESTYPE_GROUPCURSOR,
    HRC_GTLIB_PERESTYPE_EMPTY1,
    HRC_GTLIB_PERESTYPE_GROUPICON,
    HRC_GTLIB_PERESTYPE_EMPTY2,
    HRC_GTLIB_PERESTYPE_VERSION,
    HRC_GTLIB_PERESTYPE_DIALOGINCLUDE,
    HRC_GTLIB_PERESTYPE_EMPTY3,
    HRC_GTLIB_PERESTYPE_PNP,
    HRC_GTLIB_PERESTYPE_VXD,
    HRC_GTLIB_PERESTYPE_ANIMCURSOR,
    HRC_GTLIB_PERESTYPE_ANIMICON,
    HRC_GTLIB_PERESTYPE_HTML,
    HRC_GTLIB_PERESTYPE_MANIFEST,
  };

  // valid index?
  if (nIndex < GT_ARRAY_SIZE (RESOURCE_TYPE_NAMES))
    str_assign_from_res (sBuf, RESOURCE_TYPE_NAMES [nIndex]);
  // special names for special indeces
  else if (nIndex == 0x80000A30) sBuf = _T ("DIB");
  else if (nIndex == 0x80000A66) sBuf = _T ("RT_RCDATA");
  else if (nIndex == 0x80000A7A) sBuf = _T ("AVI");
  else if (nIndex == 0x80000A82) sBuf = _T ("PNGF");
  else
    sBuf.clear ();
}

/*! This little helper function determines what chars are to rip
      off a resource string.
    The \0 byte is important because the value length contains
      the trailing \0 as well.
 */
//--------------------------------------------------------------------
static bool _isStringSkipByte (const TCHAR c)
//--------------------------------------------------------------------
{
  return c == _T ('\0') || _istspace (c);
}

/*! Determine the language name from the resource string.
    Source: http://msdn.microsoft.com/en-us/library/aa381049(VS.85).aspx
 */
//--------------------------------------------------------------------
static void _getLanguageFromResLang (WORD nLanguage,
                                     pstring& ret)
//--------------------------------------------------------------------
{
#define MACRO_COMPARE_LANG(_id,_resid)   if (nLanguage == _id) str_assign_from_res (ret,_resid); else
  MACRO_COMPARE_LANG (0x0401, HRC_EXELIB_PE_RES_0401_ARABIC)
  MACRO_COMPARE_LANG (0x0402, HRC_EXELIB_PE_RES_0402_BULGARIAN)
  MACRO_COMPARE_LANG (0x0403, HRC_EXELIB_PE_RES_0403_CATALAN)
  MACRO_COMPARE_LANG (0x0404, HRC_EXELIB_PE_RES_0404_TRADITIONAL_CHINESE)
  MACRO_COMPARE_LANG (0x0405, HRC_EXELIB_PE_RES_0405_CZECH)
  MACRO_COMPARE_LANG (0x0406, HRC_EXELIB_PE_RES_0406_DANISH)
  MACRO_COMPARE_LANG (0x0407, HRC_EXELIB_PE_RES_0407_GERMAN)
  MACRO_COMPARE_LANG (0x0408, HRC_EXELIB_PE_RES_0408_GREEK)
  MACRO_COMPARE_LANG (0x0409, HRC_EXELIB_PE_RES_0409_US_ENGLISH)
  MACRO_COMPARE_LANG (0x040A, HRC_EXELIB_PE_RES_040A_CASTILIAN_SPANISH)
  MACRO_COMPARE_LANG (0x040B, HRC_EXELIB_PE_RES_040B_FINNISH)
  MACRO_COMPARE_LANG (0x040C, HRC_EXELIB_PE_RES_040C_FRENCH)
  MACRO_COMPARE_LANG (0x040D, HRC_EXELIB_PE_RES_040D_HEBREW)
  MACRO_COMPARE_LANG (0x040E, HRC_EXELIB_PE_RES_040E_HUNGARIAN)
  MACRO_COMPARE_LANG (0x040F, HRC_EXELIB_PE_RES_040F_ICELANDIC)
  MACRO_COMPARE_LANG (0x0410, HRC_EXELIB_PE_RES_0410_ITALIAN)
  MACRO_COMPARE_LANG (0x0411, HRC_EXELIB_PE_RES_0411_JAPANESE)
  MACRO_COMPARE_LANG (0x0412, HRC_EXELIB_PE_RES_0412_KOREAN)
  MACRO_COMPARE_LANG (0x0413, HRC_EXELIB_PE_RES_0413_DUTCH)
  MACRO_COMPARE_LANG (0x0414, HRC_EXELIB_PE_RES_0414_NORWEGIAN_BOKMAL)
  MACRO_COMPARE_LANG (0x0415, HRC_EXELIB_PE_RES_0415_POLISH)
  MACRO_COMPARE_LANG (0x0416, HRC_EXELIB_PE_RES_0416_PORTUGUESE_BRAZIL)
  MACRO_COMPARE_LANG (0x0417, HRC_EXELIB_PE_RES_0417_RHAETO_ROMANIC)
  MACRO_COMPARE_LANG (0x0418, HRC_EXELIB_PE_RES_0418_ROMANIAN)
  MACRO_COMPARE_LANG (0x0419, HRC_EXELIB_PE_RES_0419_RUSSIAN)
  MACRO_COMPARE_LANG (0x041A, HRC_EXELIB_PE_RES_041A_CROATO_SERBIAN_LATIN)
  MACRO_COMPARE_LANG (0x041B, HRC_EXELIB_PE_RES_041B_SLOVAK)
  MACRO_COMPARE_LANG (0x041C, HRC_EXELIB_PE_RES_041C_ALBANIAN)
  MACRO_COMPARE_LANG (0x041D, HRC_EXELIB_PE_RES_041D_SWEDISH)
  MACRO_COMPARE_LANG (0x041E, HRC_EXELIB_PE_RES_041E_THAI)
  MACRO_COMPARE_LANG (0x041F, HRC_EXELIB_PE_RES_041F_TURKISH)
  MACRO_COMPARE_LANG (0x0420, HRC_EXELIB_PE_RES_0420_URDU)
  MACRO_COMPARE_LANG (0x0421, HRC_EXELIB_PE_RES_0421_BAHASA)
  MACRO_COMPARE_LANG (0x0804, HRC_EXELIB_PE_RES_0804_SIMPLIFIED_CHINESE)
  MACRO_COMPARE_LANG (0x0807, HRC_EXELIB_PE_RES_0807_SWISS_GERMAN)
  MACRO_COMPARE_LANG (0x0809, HRC_EXELIB_PE_RES_0809_UK_ENGLISH)
  MACRO_COMPARE_LANG (0x080A, HRC_EXELIB_PE_RES_080A_MEXICAN_SPANISH)
  MACRO_COMPARE_LANG (0x080C, HRC_EXELIB_PE_RES_080C_BELGIAN_FRENCH)
  MACRO_COMPARE_LANG (0x0810, HRC_EXELIB_PE_RES_0810_SWISS_ITALIAN)
  MACRO_COMPARE_LANG (0x0813, HRC_EXELIB_PE_RES_0813_BELGIAN_DUTCH)
  MACRO_COMPARE_LANG (0x0814, HRC_EXELIB_PE_RES_0814_NORWEGIAN_NYNORSK)
  MACRO_COMPARE_LANG (0x0816, HRC_EXELIB_PE_RES_0816_PORTUGUESE_PORTUGAL)
  MACRO_COMPARE_LANG (0x081A, HRC_EXELIB_PE_RES_081A_SERBO_CROATIAN_CYRILLIC)
  MACRO_COMPARE_LANG (0x0C0C, HRC_EXELIB_PE_RES_0C0C_CANADIAN_FRENCH)
  MACRO_COMPARE_LANG (0x100C, HRC_EXELIB_PE_RES_100C_SWISS_FRENCH)
  {
    // else branch
    TCHAR sBuf[5];
    _sntprintf (sBuf, 5, _T ("%04X"), nLanguage);
    str_assign (ret, sBuf);
  }
}

/*! Determine the charset name from the resource string.
    Source: http://msdn.microsoft.com/en-us/library/aa381049(VS.85).aspx
 */
//--------------------------------------------------------------------
static void _getCharsetFromResLang (WORD nCharset,
                                    pstring& ret)
//--------------------------------------------------------------------
{
#define MACRO_COMPARE_CHARSET(_id,_resid)   if (nCharset == _id) str_assign_from_res (ret,_resid); else
  MACRO_COMPARE_CHARSET (0x0000, HRC_EXELIB_PE_RES_0000_7BIT_ASCII)
  MACRO_COMPARE_CHARSET (0x03A4, HRC_EXELIB_PE_RES_03A4_JAPAN_SHIFT_JIS_X0208)
  MACRO_COMPARE_CHARSET (0x03B5, HRC_EXELIB_PE_RES_03B5_KOREA_SHIFT_KSC_5601)
  MACRO_COMPARE_CHARSET (0x03B6, HRC_EXELIB_PE_RES_03B6_TAIWAN_BIG5)
  MACRO_COMPARE_CHARSET (0x04B0, HRC_EXELIB_PE_RES_04B0_UNICODE)
  MACRO_COMPARE_CHARSET (0x04E2, HRC_EXELIB_PE_RES_04E2_LATIN2_EASTERN_EUROPEAN)
  MACRO_COMPARE_CHARSET (0x04E3, HRC_EXELIB_PE_RES_04E3_CYRILLIC)
  MACRO_COMPARE_CHARSET (0x04E4, HRC_EXELIB_PE_RES_04E4_MULTILINGUAL)
  MACRO_COMPARE_CHARSET (0x04E5, HRC_EXELIB_PE_RES_04E5_GREEK)
  MACRO_COMPARE_CHARSET (0x04E6, HRC_EXELIB_PE_RES_04E6_TURKISH)
  MACRO_COMPARE_CHARSET (0x04E7, HRC_EXELIB_PE_RES_04E7_HEBREW)
  MACRO_COMPARE_CHARSET (0x04E8, HRC_EXELIB_PE_RES_04E8_ARABIC)
  {
    // else branch
    TCHAR sBuf[5];
    _sntprintf (sBuf, 5, _T ("%04X"), nCharset);
    str_assign (ret, sBuf);
  }
}

//--------------------------------------------------------------------
static gtuint16 _extractResString (      gtuint8* pData,
                                   const gtuint16 nSpecifiedValueLength,
                                         pstring& dst)
//--------------------------------------------------------------------
{
  gtuint16 nRealLength;
  // [ph]: the following block is a nice work around, but leads to crashes on standard conforming files!
  if (false)
  {
    // Check if the string is 0-terminated and shorter than the given length.
    // Example: Odbcstf.dll
    // nStrValueLength == 23; Text is "Microsoft Corporation" \x00 \x62
    nRealLength = 0;
    while (*((wchar_t*) (pData + (nRealLength * 2))) != 0 && nRealLength < nSpecifiedValueLength)
      ++nRealLength;

    // Add the trailing '\0' if found!
    if (nRealLength < nSpecifiedValueLength)
      ++nRealLength;
  }
  else
  {
    // Required e.g. for yetisports6.exe
    nRealLength = nSpecifiedValueLength;

    // Ensure that last char is a '\0'
    while (nRealLength > 0 && *(((wchar_t*) pData) + nRealLength - 1) != 0)
      --nRealLength;

    ASSERT (nRealLength > 0 || nSpecifiedValueLength == nRealLength);
  }

  // Small note if calculated length does not match specified length
  if (debug && nRealLength != nSpecifiedValueLength)
    out_error_format (_T ("[dbg] String item length mismatch: %02Xh - %02Xh\n"), nRealLength, nSpecifiedValueLength);

  // assign text
  str_assign (dst, (wchar_t*) pData, nRealLength);

  // remove trailing zeroes and spaces from the string
  size_t nLen = dst.length ();
  while (nLen > 0 && _isStringSkipByte (dst[nLen - 1]))
    --nLen;
  dst.resize (nLen);
  return nRealLength;
}

static gtuint16 _GET_UINT16(gtuint8* pData, size_t n, size_t nDataSize)
{
  ASSERT(n < nDataSize);
  return *((gtuint16*) (pData + n));
}

#define GET_UINT16(n)   _GET_UINT16 (pData,n,nDataSize)
#define ALIGN_RESINDEX  nResIndex = ALIGN_4 (nResIndex);

//--------------------------------------------------------------------
class PEResVersionInfoBlockHeader
//--------------------------------------------------------------------
{
private:
  size_t   m_nStartIndex;
  gtuint16 m_nTotalSize;
  gtuint16 m_nValueLength;
  gtuint16 m_nType;
  pstring  m_sName;

public:
  PEResVersionInfoBlockHeader ()
  {}

  /* struct StringFileInfo {
       WORD        wLength;
       WORD        wValueLength;
       WORD        wType;
       WCHAR       szKey[];
       WORD        Padding[];
       StringTable Children[];
     };
   */
  void read (gtuint8* pData, size_t& nResIndex, const gtuint32 nDataSize)
  {
    // store the start position of StringFileInfo
    m_nStartIndex = nResIndex;

    /* Specifies the length, in bytes, of the entire StringFileInfo
        block, including all structures indicated by the Children
        member.
     */
    m_nTotalSize = GET_UINT16 (nResIndex);
    nResIndex += 2;
    ASSERT (m_nTotalSize > 0);
    ASSERT (m_nTotalSize < nDataSize);

    /* This member is always equal to zero.
     */
    m_nValueLength = GET_UINT16 (nResIndex);
    nResIndex += 2;
    ASSERT (m_nValueLength == 0);

    /* Specifies the type of data in the version resource. This
        member is 1 if the version resource contains text data
        and 0 if the version resource contains binary data.
     */
    m_nType = GET_UINT16 (nResIndex);
    nResIndex += 2;
    ASSERT (m_nType == RES_TYPE_BYTES || m_nType == RES_TYPE_STRING);

    // read name (0-terminated)
    str_assign (m_sName, (wchar_t*) (pData + nResIndex));
    nResIndex += (m_sName.length () + 1) * sizeof (wchar_t);

    /* Contains as many zero words as necessary to align the
        Children member on a 32-bit boundary.
     */
    ALIGN_RESINDEX
  }

  const size_t   getStartIndex () const { return m_nStartIndex; }
  const pstring& getName () const { return m_sName; }
  const gtuint16 getTotalSize () const { return m_nTotalSize; }
};

//--------------------------------------------------------------------
void GT_CALL EXE_PE_ResourceAnalyzer::_ListVersionInfo
                                        (      gtuint8* pData,
                                         const gtuint32 nDataSize)
//--------------------------------------------------------------------
{
  ASSERT (pData);

  out_append (rc (HRC_EXELIB_PE_RES_VERSION_LIST));
  out_incindent ();

  pstring sKey, sValue, sLanguageID, sLanguageName, sCharsetName;

  /*  struct VS_VERSIONINFO {
        WORD  wLength;
        WORD  wValueLength;
        WORD  wType;
        WCHAR szKey[];
        WORD  Padding1[];
        VS_FIXEDFILEINFO Value;
        WORD  Padding2[];
        WORD  Children[];
      };
   */

  /* Specifies the length, in bytes, of the VS_VERSIONINFO structure.
     This length does not include any padding that aligns any
       subsequent version resource data on a 32-bit boundary.
   */
  //  const gtuint16 nResLength = GET_UINT16 (0);

  /* Specifies the length, in bytes, of the Value member.
     This value is zero if there is no Value member associated
       with the current version structure.
   */
  const gtuint16 nResValueLength = GET_UINT16 (2);

  /* Specifies the type of data in the version resource.
     This member is 1 if the version resource contains text data
       and 0 if the version resource contains binary data.
   */
  //  const gtuint16 nResType = GET_UINT16 (4);

  /* Contains the Unicode string "VS_VERSION_INFO".
     [ph]: including the trailing \0
   */
  if (wcscmp ((wchar_t*) (pData + 6), L"VS_VERSION_INFO") != 0)
  {
    out_error_append (rc (HRC_EXELIB_PE_RES_VERSION_NO_VS_VERSION_INFO));
  }
  else
  {
    // 6 static bytes and the length of the "VS_VERSION_INFO\0" Unicode string
    size_t nResIndex = 6 + ((15 + 1) * sizeof (wchar_t));

    /* Contains as many zero words as necessary to align the Value
        member on a 32-bit boundary.
     */
    ALIGN_RESINDEX

    /* Contains a VS_FIXEDFILEINFO structure that specifies arbitrary
       data associated with this VS_VERSIONINFO structure. The
       wValueLength member specifies the length of this member;
       if wValueLength is zero, this member does not exist.
     */
    if (nResValueLength > 0)
    {
      // assume all data is present
      ASSERT (EXE_PE_FIXEDFILEINFO_SIZE == nResValueLength);

      // use as FFI
      EXE_PE_FixedFileInfo const* pFFI = (EXE_PE_FixedFileInfo*) (pData + nResIndex);

      // file version
      out_format (rc (HRC_EXELIB_PE_RES_VERSION_FILEVER),
                  HI_WORD (pFFI->dwFileVersionMS),
                  LO_WORD (pFFI->dwFileVersionMS),
                  HI_WORD (pFFI->dwFileVersionLS),
                  LO_WORD (pFFI->dwFileVersionLS));

      // product version
      out_format (rc (HRC_EXELIB_PE_RES_VERSION_PRODVER),
                  HI_WORD (pFFI->dwProductVersionMS),
                  LO_WORD (pFFI->dwProductVersionMS),
                  HI_WORD (pFFI->dwProductVersionLS),
                  LO_WORD (pFFI->dwProductVersionLS));

      // FileFlags are not evaluated because of the FileFlagsMask

      out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS));
      switch (pFFI->dwFileOS)
      {
        case EXE_PE_VOS_UNKNOWN:       out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_UNKNOWN)); break;
        case EXE_PE_VOS__WINDOWS16:    out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_WIN16)); break;
        case EXE_PE_VOS__PM16:         out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_PM16)); break;
        case EXE_PE_VOS__PM32:         out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_PM32)); break;
        case EXE_PE_VOS__WINDOWS32:    out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_WIN32)); break;
        case EXE_PE_VOS_DOS:           out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_DOS)); break;
        case EXE_PE_VOS_DOS_WINDOWS16: out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_DOS_WIN16)); break;
        case EXE_PE_VOS_DOS_WINDOWS32: out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_DOS_WIN32)); break;
        case EXE_PE_VOS_OS216:         out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_OS216)); break;
        case EXE_PE_VOS_OS216_PM16:    out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_OS216_PM)); break;
        case EXE_PE_VOS_OS232:         out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_OS232)); break;
        case EXE_PE_VOS_OS232_PM32:    out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_PS232_PM)); break;
        case EXE_PE_VOS_NT:            out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_WIN2K)); break;
        case EXE_PE_VOS_NT_WINDOWS32:  out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_WIN32_2K)); break;
        case EXE_PE_VOS_WINCE:         out_append (rc (HRC_EXELIB_PE_RES_VERSION_OS_WINCE)); break;
        default:                       out_format (rc (HRC_EXELIB_PE_RES_VERSION_OS_INVALID), pFFI->dwFileOS); break;
      }
      out_append (_T ("\n"));

      // inc index
      nResIndex += nResValueLength;
    }

    /* Contains as many zero words as necessary to align the Children
         member on a 32-bit boundary. These bytes are not included in
         wValueLength. This member is optional.
     */
    ALIGN_RESINDEX

    // any bytes left?
    /* Specifies an array of zero or one StringFileInfo structures,
         and zero or one VarFileInfo structures that are children
         of the current VS_VERSIONINFO structure.
     */
    if (nDataSize > nResIndex)
    {
      PEResVersionInfoBlockHeader aBlockHeader;
      aBlockHeader.read (pData, nResIndex, nDataSize);

      if (aBlockHeader.getName () == _T ("StringFileInfo"))
      {
        out_incindent ();
//********************************************************************
if (1) {
        /* This listing contains an additional layer for the language.
           In this case, the item size represents the size of the full string table
         */
        size_t nLanguageStartIndex;
        int nStringTableBytes = 0;
        while (aBlockHeader.getTotalSize () > gtuint16 (nResIndex - aBlockHeader.getStartIndex ()))
        {
          const bool bIsNewLanguage = nStringTableBytes == 0;
          if (bIsNewLanguage)
            nLanguageStartIndex = nResIndex;

          // total item size
          const gtuint16 nItemSize = GET_UINT16 (nResIndex);
          nResIndex += 2;
          // at least 3 word fields and 1 \0-byte
          ASSERT (nItemSize > 2 + 2 + 2 + 2);

          // total value length - may be 0
          const gtuint16 nValueLength = GET_UINT16 (nResIndex);
          nResIndex += 2;
          ASSERT (nValueLength < nItemSize);

          // text or binary data?
          const gtuint16 nItemType = GET_UINT16 (nResIndex);
          nResIndex += 2;
          ASSERT (nItemType == RES_TYPE_BYTES || nItemType == RES_TYPE_STRING);

          // read item key
          str_assign (sKey, (wchar_t*) (pData + nResIndex));
          nResIndex += (sKey.length () + 1) * sizeof (wchar_t);

          // align index
          ALIGN_RESINDEX

          if (bIsNewLanguage)
          {
            ASSERT (nValueLength == 0);
            const LCID nLocale = (LCID) str_hex_to_gtint32 (sKey.c_str ());
            _getLanguageFromResLang (WORD (nLocale >> 16), sLanguageName);
            _getCharsetFromResLang (WORD (nLocale & 0xffff), sCharsetName);
            out_format (rc (HRC_EXELIB_PE_RES_VERSION_LANGUAGE), sLanguageName.c_str (), sCharsetName.c_str ());
          }
          else
            if (nItemType == RES_TYPE_STRING)
            {
              // text
              const int nRealLength = _extractResString (pData + nResIndex, nValueLength, sValue);

              // print key and value
              out_format (rc (HRC_EXELIB_PE_RES_VERSION_STRING_TEXT),
                          sKey.c_str (),
                          sValue.c_str ());

              // value is in chars
              nResIndex += nRealLength * 2;

              // C:\Programme\DU Meter\unins000.exe
              // -> has many strings with length mismatch -> nStringTableBytes are left at the end!
              // nStringTableBytes -= (nValueLength - nRealLength);
            }
            else
            {
              // binary stuff
              out_format (rc (HRC_EXELIB_PE_RES_VERSION_STRING_BIN),
                          sKey.c_str (),
                          nValueLength);

              // skip size of value
              nResIndex += nValueLength;
            }

          // align after value
          ALIGN_RESINDEX

          if (bIsNewLanguage)
          {
            // new language....
            nStringTableBytes = nItemSize - int (nResIndex - nLanguageStartIndex);
            out_incindent ();
          }
          else
          {
            // Last item may not be aligned
            ASSERT (nItemSize <= nStringTableBytes);
            if (nStringTableBytes == nItemSize)
              nStringTableBytes = 0;
            else
              nStringTableBytes -= ALIGN_4 (nItemSize);

            if (0) out_error_format (_T ("[dbg] nStringTableBytes %04Xh / %u\n"), nStringTableBytes, nStringTableBytes);

            if (nStringTableBytes == 0)
              out_decindent ();
          }
        }
        // No string table bytes should be left!
        // Happens e.g. in C:\Programme\DU Meter\unins000.exe
        if (0) ASSERT (nStringTableBytes == 0);
        // #f# hack alert
        if (nStringTableBytes > 0)
          out_decindent ();
} else {
//********************************************************************
        /* Contains an array of one or more StringTable structures.
            Each StringTable structure's szKey member indicates the
            appropriate language and code page for displaying the text
            in that StringTable structure.
         */
        // determine how many bytes are left in the SFI structure
        gtuint16 nSFIRestBytes = ALIGN_4 (aBlockHeader.getTotalSize () - gtuint16 (nResIndex - aBlockHeader.getStartIndex ()));
        if (0) out_format (_T ("[dbg] StringFileInfo byte %04Xh / %u\n"), nSFIRestBytes, nSFIRestBytes);
        while (nSFIRestBytes > 0)
        {
          /* struct StringTable {
               WORD   wLength;
               WORD   wValueLength;
               WORD   wType;
               WCHAR  szKey[];
               WORD   Padding[];
               String Children[];
             };
           */

          //out_format (_T ("[dbg %04Xh] ChildBytes = %u\n"), nResIndex, nSFIRestBytes);
          const size_t nStringTableStartIndex = nResIndex;

          /* Specifies the length, in bytes, of this StringTable structure,
              including all structures indicated by the Children member.
           */
          const gtuint16 nStringTableTotalSize = ALIGN_4 (GET_UINT16 (nResIndex));
          nResIndex += 2;
          ASSERT (nStringTableTotalSize > 0);
          ASSERT (nStringTableTotalSize < aBlockHeader.getTotalSize ());
          if (0) out_format (_T ("[dbg %04Xh]   nStringTableTotalSize = %04Xh\n"), nResIndex, nStringTableTotalSize);

          // dec child count
          /* [ph] uncommented because of other fix!
          if (nSFIRestBytes == 2)
          {
            // needed for IEXPLORE.EXE 6.00.2800.1106
            nSFIRestBytes = 0;
            break;
          }
          */
          ASSERT (nStringTableTotalSize <= nSFIRestBytes);
          nSFIRestBytes -= nStringTableTotalSize;
          if (0) out_format (_T ("[dbg %04Xh]   nSFIRestBytes = %04Xh\n"), nResIndex, nSFIRestBytes);

          /* This member is always equal to zero.
           */
          {
            const gtuint16 nStringTableValueLength = GET_UINT16 (nResIndex);
            nResIndex += 2;
            ASSERT (nStringTableValueLength == 0);
          }

          /* Specifies the type of data in the version resource. This
              member is 1 if the version resource contains text data
              and 0 if the version resource contains binary data.
           */
          const gtuint16 nStringTableType = GET_UINT16 (nResIndex);
          nResIndex += 2;
          ASSERT (nStringTableType == RES_TYPE_BYTES || nStringTableType == RES_TYPE_STRING);

          /* Specifies an 8-digit hexadecimal number stored as a
              Unicode string. The four most significant digits
              represent the language identifier. The four least
              significant digits represent the code page for which
              the data is formatted. Each Microsoft Standard Language
              identifier contains two parts: the low-order 10 bits
              specify the major language, and the high-order 6 bits
              specify the sublanguage. For a table of valid
              identifiers see.
             [ph] Contains a trailing '\0'
           */
          str_assign (sLanguageID, (wchar_t*) (pData + nResIndex), 8);
          nResIndex += (8 + 1) * 2;

          // get name from codes
          // #f# may not be fully OK (see above comment)
          _getLanguageFromResLang (str_hex_to_gtuint16 (sLanguageID.c_str ()), sLanguageName);
          _getCharsetFromResLang (str_hex_to_gtuint16 (sLanguageID.c_str () + 4), sCharsetName);
          out_format (rc (HRC_EXELIB_PE_RES_VERSION_LANGUAGE),
                      sLanguageName.c_str (),
                      sCharsetName.c_str ());

          out_incindent ();

          /* Contains as many zero words as necessary to align the
              Children member on a 32-bit boundary.
           */
          ALIGN_RESINDEX

          // [ph] nStringTableTotalSize == 0 in "C:\Programme\Adobe\Acrobat 6.0\Acrobat Elements\Acrobat Elements.exe"
          // [ph] nStringTableTotalSize > gtuint16 (nResIndex - nStringTableStartIndex)    avoid an overflow!
          if (nStringTableTotalSize > 0)
          {
            /* Specifies an array of one or more String structures.
               [ph] Beware of overflow!
             */
            gtuint16 nStringTableBytes = nStringTableTotalSize - gtuint16 (nResIndex - nStringTableStartIndex);
            ASSERT (nStringTableBytes < nStringTableTotalSize);
            while (nStringTableBytes > 0)
            {
              /* struct String {
                   WORD   wLength;
                   WORD   wValueLength;
                   WORD   wType;
                   WCHAR  szKey[];
                   WORD   Padding[];
                   WORD   Value[];
                 };
               */

              //out_format (_T ("[dbg %04Xh]   nStringTableBytes = %u\n"), nResIndex, nStringTableBytes);

              /* Specifies the length, in bytes, of this String structure.
               */
              const gtuint16 nStrLength = GET_UINT16 (nResIndex);
              nResIndex += 2;
              ASSERT (nStrLength > 2);

              //out_format (_T ("[dbg %04Xh]     nStrLength = %u\n"), nResIndex, nStrLength);

              // dec from child bytes
              // [ph] Not part of the spec but works!
              /* [ph 2008 05 04 - may not be needed anymore!
              if (nStrLength > nStringTableBytes)
              {
                // go back 2 bytes!
                // works for "Acrobat Elements.exe"
                nResIndex -= 2;
                nStringTableBytes = 0;
                break;
              }
              */

              ASSERT (nStrLength <= nStringTableBytes);
              if (nStringTableBytes >= ALIGN_4 (nStrLength))
                nStringTableBytes -= ALIGN_4 (nStrLength);
              else
              {
                // The last item may not be aligned!
                nStringTableBytes -= nStrLength;
                ASSERT (nStringTableBytes == 0);
              }

              /* Specifies the size, in words, of the Value member
                 [ph] Including the \0 if present and nStrType == 1
               */
              const gtuint16 nStrValueLength = GET_UINT16 (nResIndex);
              nResIndex += 2;

              /* Specifies the type of data in the version resource. This
                  member is 1 if the version resource contains text data
                  and 0 if the version resource contains binary data.
               */
              const gtuint16 nStrType = GET_UINT16 (nResIndex);
              nResIndex += 2;
              ASSERT (nStrType == RES_TYPE_BYTES || nStrType == RES_TYPE_STRING);

              /* The key specifies an arbitrary Unicode string.
                 May be any string but 0-terminated.
               */
              str_assign (sKey, (wchar_t*) (pData + nResIndex));
              nResIndex += (sKey.length () + 1) * 2;

              /* Contains as many zero words as necessary to align the
                  Value member on a 32-bit boundary.
               */
              ALIGN_RESINDEX

              /* Specifies a zero-terminated string. See the szKey member
                  description for more information.
               */
              if (nStrType == RES_TYPE_STRING)
              {
                const int nRealLength = _extractResString (pData + nResIndex, nStrValueLength, sValue);
                ASSERT (nRealLength <= nStrValueLength);

                // print value
                out_format (rc (HRC_EXELIB_PE_RES_VERSION_STRING_TEXT),
                            sKey.c_str (),
                            sValue.c_str ());

                // value is in chars
                nResIndex += nRealLength * 2;

                // C:\Programme\DU Meter\unins000.exe
                // -> has many strings with length mismatch -> nStringTableBytes are left at the end!
                nStringTableBytes -= (nStrValueLength - nRealLength);
              }
              else
              {
                // binary stuff
                out_format (rc (HRC_EXELIB_PE_RES_VERSION_STRING_BIN),
                            sKey.c_str (),
                            nStrValueLength);

                // value is in bytes!
                nResIndex += nStrValueLength;
              }

              // [ph] required e.g. for "Acrobat Elements.exe"
              ALIGN_RESINDEX
            }

            // should be nothing left
            ASSERT (nStringTableBytes == 0);
          }

          out_decindent ();
        }
        // should be nothing left
        ASSERT (nSFIRestBytes == 0);
//********************************************************************
}

        // read for an eventually following VarFileInfo block
        if (nResIndex < nDataSize)
          aBlockHeader.read (pData, nResIndex, nDataSize);

        out_decindent ();
      }

      // Check if a VarFileInfo block is present
      if (aBlockHeader.getName () == _T ("VarFileInfo"))
      {
        out_incindent ();

        while (aBlockHeader.getTotalSize () > gtuint16 (nResIndex - aBlockHeader.getStartIndex ()))
        {
          // total item size
          const gtuint16 nItemSize = GET_UINT16 (nResIndex);
          nResIndex += 2;

          // total value length
          const gtuint16 nValueLength = GET_UINT16 (nResIndex);
          nResIndex += 2;

          // text or binary data?
          const gtuint16 nItemType = GET_UINT16 (nResIndex);
          nResIndex += 2;
          ASSERT (nItemType == RES_TYPE_BYTES || nItemType == RES_TYPE_STRING);

          // read item key
          str_assign (sKey, (wchar_t*) (pData + nResIndex));
          nResIndex += (sKey.length () + 1) * 2;

          // align index
          ALIGN_RESINDEX

          if (nItemType == RES_TYPE_STRING)
          {
            // text
            int nRealLength = _extractResString (pData + nResIndex, nValueLength, sValue);

            // print value
            out_format (rc (HRC_EXELIB_PE_RES_VERSION_STRING_TEXT),
                        sKey.c_str (),
                        sValue.c_str ());

            // value is in chars
            nResIndex += nRealLength * 2;
          }
          else
          {
            // binary stuff
            out_format (rc (HRC_EXELIB_PE_RES_VERSION_STRING_BIN),
                        sKey.c_str (),
                        nValueLength);

            // skip size of value
            nResIndex += nValueLength;
          }

          // align after value
          ALIGN_RESINDEX
        }

        out_decindent ();
      }
    }
  }

  out_decindent ();
  out_append (_T ("\n"));
}

/*! Recursive method to list resource directory table entries
 */
//--------------------------------------------------------------------
bool GT_CALL EXE_PE_ResourceAnalyzer::_ListTable
                                        (const rva_t nTableRVA)
//--------------------------------------------------------------------
{
  const gtuint32 nCurrentOffset = m_pSectionTable->r2o (nTableRVA);
  rva_t nCurrentRVA = nTableRVA;
  bool bReturn = true;

  // read ResourceDirectoryTable
  EXE_PE_ResourceDirectoryTable aResDirTable;
  if (!m_pBuffer->GetBuffer (nCurrentOffset,
                             &aResDirTable,
                             EXE_PE_RESOURCEDIRTABLE_SIZE))
  {
    out_error_append (rc (HRC_EXELIB_PE_RES_FAILED_TO_READ_TABLE));
    return false;
  }

  // inc recursion counter
  m_nRecursionDepth++;

  if (m_nListLevel & LIST_ENTRY)
    out_incindent ();

  // inc counter
  m_nTOCSize  += EXE_PE_RESOURCEDIRTABLE_SIZE;
  nCurrentRVA += EXE_PE_RESOURCEDIRTABLE_SIZE;

  // loop over all name entries
  int i;
  for (i = 0; i < aResDirTable.nNameEntryCount; i++)
  {
    if (!_ListEntry (nCurrentRVA))
    {
      bReturn = false;
      break;
    }
    nCurrentRVA += EXE_PE_RESOURCEDIRENTRY_SIZE;
  }

  // loop over all ID entries
  for (i = 0; i < aResDirTable.nIDEntryCount; i++)
  {
    if (!_ListEntry (nCurrentRVA))
    {
      bReturn = false;
      break;
    }
    nCurrentRVA += EXE_PE_RESOURCEDIRENTRY_SIZE;
  }

  if (m_nListLevel & LIST_ENTRY)
    out_decindent ();

  // and dec recursion counter at the end!
  m_nRecursionDepth--;
  return bReturn;
}

//--------------------------------------------------------------------
bool GT_CALL EXE_PE_ResourceAnalyzer::_ListEntry
                                        (const rva_t nEntryRVA)
//--------------------------------------------------------------------
{
  // determine the current file offset
  const gtuint32 nCurrentOffset = m_pSectionTable->r2o (nEntryRVA);

  // and read the information into the buffer
  EXE_PE_ResourceDirectoryEntry aResDirEntry;
  if (!m_pBuffer->GetBuffer (nCurrentOffset,
                             &aResDirEntry,
                             EXE_PE_RESOURCEDIRENTRY_SIZE))
  {
    ShowErrorMessage (_T ("EXE_PE_ResourceAnalyzer::_ListEntry::GetBuffer failed"));
  }

  m_nTOCSize += EXE_PE_RESOURCEDIRENTRY_SIZE;

  // does this entry point to a subdirectory?
  const bool bIsSubDirectoryRVA = GT_ISSET_BIT_0 (aResDirEntry.nDataRVA);

  if (bIsSubDirectoryRVA)
  {
    // print level 1: the different resource types
    if (m_nRecursionDepth == 1)
    {
      m_nCurResTypeID = aResDirEntry.nIDOrRVA;

      // if commandline switch /er was used
      if (m_nListLevel & LIST_ENTRY)
      {
        pstring sResourceType;
        _getResourceTypeString (m_nCurResTypeID - 1, sResourceType);

        // if a name exists for the current type -> write the name else the number
        if (!sResourceType.empty ())
        {
          out_format (rc (HRC_EXELIB_PE_RES_LIST_TYPE_NAME),
                      sResourceType.c_str ());
        }
        else
        {
          out_format (rc (HRC_EXELIB_PE_RES_LIST_TYPE_ID),
                      m_nCurResTypeID,
                      m_nCurResTypeID);
        }
      }
    }
    else
    if (m_nRecursionDepth == 2)
    {
      m_nCurResNameID = aResDirEntry.nIDOrRVA;

      if (m_nListLevel & LIST_ENTRY)
        out_format (rc (HRC_EXELIB_PE_RES_LIST_ID),
                    m_nCurResNameID, m_nCurResNameID);
    }
    else
    if (m_nRecursionDepth == 3)
    {
      m_nCurResLanguageID = aResDirEntry.nIDOrRVA;

      if (m_nListLevel & LIST_ENTRY)
        out_format (rc (HRC_EXELIB_PE_RES_LIST_LANG),
                    m_nCurResLanguageID, m_nCurResLanguageID);
    }
    else
    if (m_nRecursionDepth > 3)
    {
      // too deep!
      out_error_format (rc (HRC_EXELIB_PE_RES_RECURSION_WARN),
                        m_nRecursionDepth);
      return false;
    }

    // ListTable returns false if a _ListEntry call returned false
    // because of an invalid recursion depth!
    if (!_ListTable (m_pDataDir->nRVA + GT_UPPER_31_BITS (aResDirEntry.nDataRVA)))
      return false;
  }
  else
  {
    // it's not a sub directory RVA
    const rva_t    nResDatRVA = m_pDataDir->nRVA + GT_UPPER_31_BITS (aResDirEntry.nDataRVA);
    const gtuint32 nResDatOffset = m_pSectionTable->r2o (nResDatRVA);

    // validate offset
    if (nResDatOffset >= m_pBuffer->GetFileSize ())
    {
      out_error_append (rc (HRC_EXELIB_PE_RES_ENTRY_INVALID));
      return false;
    }

    // read ResourceDataEntry
    EXE_PE_ResourceDataEntry aResourceDataEntry;
    if (!m_pBuffer->GetBuffer (nResDatOffset,
                               &aResourceDataEntry,
                               EXE_PE_RESOURCEDATAENTRY_SIZE))
    {
      out_error_append (rc (HRC_EXELIB_PE_RES_FAILED_TO_READ));
      return false;
    }

    // inc counter
    m_nTOCSize += EXE_PE_RESOURCEDATAENTRY_SIZE;

    // Previously a "-1" was used here and I don't know why
    // [ph] -1 was here because I mixed up "string" with "stringtable"
    const gtuint32 nPhysicalOffset = m_pSectionTable->r2o (aResourceDataEntry.nDataRVA);

    if (0)
      out_format (_T ("[dbg reslist] 0x%08Xh 0x%08Xh 0x%08Xh\n"),
                  RVA_VAL (nResDatRVA),
                  nResDatOffset,
                  nPhysicalOffset);

    // check if string index is invalid (eg. EXE packed)
    if (nPhysicalOffset <  m_nOffset ||
        nPhysicalOffset >= m_nOffset + m_pDataDir->nSize)
    {
      // error: offset is invalid!
      out_error_format (rc (HRC_EXELIB_PE_RES_RVA_INVALID),
                        RVA_VAL (nResDatRVA));

      // previously: no code in this block
      // -> because of 'C:\Program Files\iview380full\Languages\Estonia.dll'
//      return false;
    }
    else
    {
      // if commandline switch was used
      if (m_nListLevel & LIST_ENTRY)
      {
        // print single entry
        out_format (rc (HRC_EXELIB_PE_RES_LIST_ENTRY),
                    RVA_VAL (aResourceDataEntry.nDataRVA),
                    nPhysicalOffset,
                    aResourceDataEntry.nDataSize);
      }

      // determine type
      // Note: RT_* constants are from WinUser.h
      switch (m_nCurResTypeID)
      {
        case RT_STRING:
          // list strings if cmdline switch was used and the current
          // resource type is a string
          if (m_nListLevel & LIST_STRING)
          {
            gtuint16 nStringLen;
            size_t nStringTableIndex = 0;
            pstring sData;

            while (1)
            {
              // get string len
              nStringLen = m_pBuffer->GetUint16 (nPhysicalOffset + nStringTableIndex);
              nStringTableIndex += 2;

              // end of stringtable?
              if (nStringLen == 0)
                break;

              if (nStringLen > aResourceDataEntry.nDataSize)
              {
                // string len invalid -> compressed EXE?
                out_error_append (rc (HRC_EXELIB_PE_RES_STRING_LEN_INVALID));
                break;
              }

              // read the unicode string data from the file
              if (!m_pBuffer->GetFixedSizeWString (nPhysicalOffset + nStringTableIndex, sData, nStringLen))
              {
                // happens if length is out of bounds
                out_error_format (rc (HRC_EXELIB_PE_RES_STRING_READ_ERR),
                                  nPhysicalOffset + nStringTableIndex,
                                  aResourceDataEntry.nDataSize);
                break;
              }

              // inc index
              nStringTableIndex += nStringLen * 2;

              // replace the char '\n' with the string "\n"
              str_replace_all (sData, _T ('\n'), _T ("\\n"));

              // print string
              out_format (rc (HRC_EXELIB_PE_RES_STRING_LIST),
                          sData.c_str ());
            }
          }
          break;
        case RT_VERSION:
          // list strings if cmdline switch was used and the current
          // resource type is a version info
          if (m_nListLevel & LIST_VERSION)
          {
            // needs to be 4 byte aligned!
            ASSERT (IS_ALIGNED (nPhysicalOffset, 4));

            if (0 && debug)
              out_format (_T ("[dbg] VersionInfo at offset %08Xh (RVA %08Xh)\n"),
                          nPhysicalOffset,
                          RVA_VAL (aResourceDataEntry.nDataRVA));

            // alloc on stack - it is assumed to be ~1KB
            gtuint8 *pData = (gtuint8*) _alloca (aResourceDataEntry.nDataSize + 1);
            m_pBuffer->GetBuffer (nPhysicalOffset,
                                  pData,
                                  aResourceDataEntry.nDataSize);

            // interprete!
            _ListVersionInfo (pData, aResourceDataEntry.nDataSize);
          }
          break;
        case RT_CURSOR:
        case RT_BITMAP:
        case RT_ICON:
        case RT_MENU:
        case RT_DIALOG:
        case RT_FONTDIR:
        case RT_FONT:
        case RT_ACCELERATOR:
        case RT_RCDATA:
        case RT_MESSAGETABLE:
        case RT_GROUP_CURSOR:
        case RT_GROUP_ICON:
        case RT_DLGINCLUDE:
        case RT_PLUGPLAY:
        case RT_VXD:
        case RT_ANICURSOR:
        case RT_ANIICON:
        case RT_HTML:
        case RT_MANIFEST:
          // Known, but not listed
          break;
        default:
          // Custom resource types start at 0x80000000
          if ((m_nCurResTypeID & 0x80000000) == 0)
            out_error_format (_T ("Unknown resource type %04Xh\n"), m_nCurResTypeID);
          break;
      }  // switch
    }

    // no matter whether data is okay or not
    m_nDataSize += aResourceDataEntry.nDataSize;
  }

  return true;
}

//--------------------------------------------------------------------
EXE_PE_ResourceAnalyzer::EXE_PE_ResourceAnalyzer
                                        (FileBuffer*                  pBuffer,
                                         EXE_PE_SectionTableAnalyzer* pSectionTable,
                                         EXE_PE_ImageDataDirectory*   pDataDir)
//--------------------------------------------------------------------
  : EXE_PE_BasicLister  (pBuffer, pSectionTable, pDataDir),
    m_nRecursionDepth   (0),
    m_nCurResTypeID     (gtuint32 (-1)),
    m_nCurResNameID     (gtuint32 (-1)),
    m_nCurResLanguageID (gtuint32 (-1)),
    m_nDataSize         (0),
    m_nTOCSize          (0),
    m_nListLevel        (0)
{
  if (CmdlineParam_GetBool (GT_FLAG_PERES))
    m_nListLevel |= LIST_ENTRY;
  if (CmdlineParam_GetBool (GT_FLAG_PERES2))
    m_nListLevel |= LIST_STRING;
  if (CmdlineParam_GetBool (GT_FLAG_PEVER))
    m_nListLevel |= LIST_VERSION;
}

//--------------------------------------------------------------------
void GT_CALL EXE_PE_ResourceAnalyzer::List ()
//--------------------------------------------------------------------
{
  // be careful - only if something is to be printed
  if (m_nListLevel != 0)
    out_append (_T ("\n"));

  // check offset for validity
  if (m_nOffset < 0 || m_nOffset >= m_pBuffer->GetFileSize ())
  {
    out_error_format (rc (HRC_EXELIB_PE_RES_INVALID),
                      m_nOffset);
    return;
  }

  // if commandline switch /peres was used
  if (m_nListLevel & LIST_ENTRY)
  {
    // res at [offset] ([offset]) for [size] bytes
    out_format (rc (HRC_EXELIB_PE_RES_AT_OFFSET),
                m_nOffset,
                RVA_VAL (m_pDataDir->nRVA),
                m_pDataDir->nSize);

    out_incindent ();
  }

  try
  {
    // goto resource table
    m_pBuffer->SetActFilePos (m_nOffset);

    // do the main listing at the resource table offset
    _ListTable (m_pDataDir->nRVA);

    if (m_nListLevel & LIST_ENTRY)
    {
      // show some usage info about res data and res TOC
      out_format (rc (HRC_EXELIB_PE_RES_TOTAL),
                  m_nDataSize + m_nTOCSize,
                  m_nDataSize,
                  m_nTOCSize);
    }
  }
  catch (const Excpt &e)
  {
    e.Display ();
  }

  if (m_nListLevel & LIST_ENTRY)
  {
    out_decindent ();
  }
}

}  // namespace
