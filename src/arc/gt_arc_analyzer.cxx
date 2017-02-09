#include "gt_arc_analyzer.hxx"
#include "gt_arc_lister.hxx"
#include "gt_arc_structs.hxx"

#include "gt_cmdline_params.hxx"
#include "gt_direntry.hxx"
#include "gt_exception.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_utils.hxx"

namespace GT {

// init static variables
static gtuint32 g_nTotalArchives = 0;
static gtuint32 g_nDamagedArchives = 0;

//--------------------------------------------------------------------
enum EArchiveOverlayPos
//--------------------------------------------------------------------
{
  ARC_OVERLAY_NONE = -7,
  ARC_OVERLAY_0,
  ARC_OVERLAY_ANY,
};

//--------------------------------------------------------------------
struct GTArchive_InfoEntry
//--------------------------------------------------------------------
{
  LPCTSTR            sName;
  EArchiveOverlayPos eOvrPos;
};

//--------------------------------------------------------------------
const GTArchive_InfoEntry GTArchive_Info[] =
//--------------------------------------------------------------------
{
  { _T ("_none_"),           ARC_OVERLAY_ANY },
  { _T ("777"),              ARC_OVERLAY_ANY },
  { _T ("ACE"),              ARC_OVERLAY_ANY },
  { _T ("AIN"),              ARC_OVERLAY_ANY },
  { _T ("AKT7"),             ARC_OVERLAY_ANY },
  { _T ("AKT8"),             ARC_OVERLAY_ANY },
  { _T ("AKT9"),             ARC_OVERLAY_ANY },
  { _T ("AMG"),              ARC_OVERLAY_NONE },  // too many false detections
  { _T ("ARC"),              ARC_OVERLAY_ANY },
  { _T ("ARG"),              ARC_OVERLAY_ANY },
  { _T ("ARI"),              ARC_OVERLAY_ANY },
  { _T ("ARJ"),              ARC_OVERLAY_ANY },
  { _T ("ARK"),              ARC_OVERLAY_ANY },
  { _T ("ARQ"),              ARC_OVERLAY_0 },
  { _T ("ARS"),              ARC_OVERLAY_ANY },
  { _T ("ARX"),              ARC_OVERLAY_ANY },
  { _T ("AR7"),              ARC_OVERLAY_ANY },
  { _T ("ASD"),              ARC_OVERLAY_ANY },
  { _T ("BIX"),              ARC_OVERLAY_ANY },
  { _T ("BLI"),              ARC_OVERLAY_ANY },
  { _T ("BOA"),              ARC_OVERLAY_ANY },
  { _T ("BS2"),              ARC_OVERLAY_ANY },
  { _T ("BSA"),              ARC_OVERLAY_ANY },
  { _T ("BSN"),              ARC_OVERLAY_ANY },
  { _T ("BZ"),               ARC_OVERLAY_0 },
  { _T ("CAB"),              ARC_OVERLAY_ANY },
  { _T ("CHZ"),              ARC_OVERLAY_ANY },
  { _T ("COD"),              ARC_OVERLAY_ANY },
  { _T ("COMP16"),           ARC_OVERLAY_ANY },
  { _T ("CPAC"),             ARC_OVERLAY_ANY },
  { _T ("DPA"),              ARC_OVERLAY_ANY },
  { _T ("DWC"),              ARC_OVERLAY_ANY },
  { _T ("ELI"),              ARC_OVERLAY_ANY },
  { _T ("ESP"),              ARC_OVERLAY_ANY },
  { _T ("FINISH"),           ARC_OVERLAY_ANY },
  { _T ("GZip"),             ARC_OVERLAY_ANY },
  { _T ("HA"),               ARC_OVERLAY_ANY },
  { _T ("HAP"),              ARC_OVERLAY_ANY },
  { _T ("HPA"),              ARC_OVERLAY_ANY },
  { _T ("HQX"),              ARC_OVERLAY_ANY },
  { _T ("Hyper 2.5"),        ARC_OVERLAY_ANY },
  { _T ("Hyper 2.6"),        ARC_OVERLAY_ANY },
  { _T ("Inno"),             ARC_OVERLAY_0     },
  { _T ("IMP"),              ARC_OVERLAY_ANY },
  { _T ("INS"),              ARC_OVERLAY_ANY },
  { _T ("ISC"),              ARC_OVERLAY_ANY },
  { _T ("ISO"),              ARC_OVERLAY_ANY },
  { _T ("JAR"),              ARC_OVERLAY_ANY },
  { _T ("JRC"),              ARC_OVERLAY_ANY },
  { _T ("LG"),               ARC_OVERLAY_0 },
  { _T ("LIM"),              ARC_OVERLAY_ANY },
  { _T ("Links LS"),         ARC_OVERLAY_ANY },
  { _T ("LZA"),              ARC_OVERLAY_ANY },
  { _T ("LZH"),              ARC_OVERLAY_ANY },
  { _T ("LZO"),              ARC_OVERLAY_ANY },
  { _T ("MAR"),              ARC_OVERLAY_ANY },
  { _T ("MSC"),              ARC_OVERLAY_ANY },
  { _T ("Nero 5"),           ARC_OVERLAY_ANY },
  { _T ("Nero 4"),           ARC_OVERLAY_ANY },
  { _T ("NSIS 1.0f-1.1d"),   ARC_OVERLAY_0 },
  { _T ("NSIS 1.1e-1.2g"),   ARC_OVERLAY_0 },
  { _T ("NSIS 1.30-1.59"),   ARC_OVERLAY_0 },
  { _T ("NSIS 1.60-2.0"),    ARC_OVERLAY_0 },
  { _T ("IBM Pack"),         ARC_OVERLAY_ANY },
  { _T ("oPAQue"),           ARC_OVERLAY_ANY },
  { _T ("paq8"),             ARC_OVERLAY_ANY },
  { _T ("PAR"),              ARC_OVERLAY_ANY },
  { _T ("PHP"),              ARC_OVERLAY_ANY },
  { _T ("PPMZ"),             ARC_OVERLAY_ANY },
  { _T ("Q"),                ARC_OVERLAY_ANY },
  { _T ("QFC"),              ARC_OVERLAY_ANY },
  { _T ("old RAR"),          ARC_OVERLAY_ANY },
  { _T ("RAR"),              ARC_OVERLAY_ANY },
  { _T ("RAX"),              ARC_OVERLAY_ANY },
  { _T ("RKive (1)"),        ARC_OVERLAY_0 },
  { _T ("RKive (2)"),        ARC_OVERLAY_0 },
  { _T ("RNC"),              ARC_OVERLAY_ANY },
  { _T ("SAR"),              ARC_OVERLAY_ANY },
  { _T ("SBC"),              ARC_OVERLAY_ANY },
  { _T ("SBX"),              ARC_OVERLAY_ANY },
  { _T ("SKY"),              ARC_OVERLAY_ANY },
  { _T ("SOF"),              ARC_OVERLAY_ANY },
  { _T ("Sony IMG"),         ARC_OVERLAY_ANY },
  { _T ("SQWEZ"),            ARC_OVERLAY_ANY },
  { _T ("SQZ"),              ARC_OVERLAY_ANY },
  { _T ("Stirling"),         ARC_OVERLAY_ANY },
  { _T ("SWAG"),             ARC_OVERLAY_ANY },
  { _T ("Synetic"),          ARC_OVERLAY_ANY },
  { _T ("SZIP"),             ARC_OVERLAY_ANY },
  { _T ("TAR"),              ARC_OVERLAY_ANY },
  { _T ("TIP"),              ARC_OVERLAY_ANY },
  { _T ("UC2"),              ARC_OVERLAY_ANY },
  { _T ("UFA"),              ARC_OVERLAY_ANY },
  { _T ("UHA"),              ARC_OVERLAY_ANY },
  { _T ("XIE"),              ARC_OVERLAY_ANY },
  { _T ("XL"),               ARC_OVERLAY_ANY },
  { _T ("XPA"),              ARC_OVERLAY_ANY },
  { _T ("YC"),               ARC_OVERLAY_ANY },
  { _T ("YZ"),               ARC_OVERLAY_ANY },
  { _T ("ZET"),              ARC_OVERLAY_ANY },
  { _T ("ZIP"),              ARC_OVERLAY_ANY },
  { _T ("ZOO"),              ARC_OVERLAY_ANY },
  { _T ("ZPK"),              ARC_OVERLAY_ANY },
  { _T ("_last_"),           ARC_OVERLAY_ANY }
};

// check that number in string array is equivalent to enum in .hxx
GT_COMPILER_ASSERT (GT_ARRAY_SIZE (GTArchive_Info) == (GT_PACKERENUM_LAST + 1));

//--------------------------------------------------------------------
ArchiveAnalyzer::ArchiveAnalyzer
                                        (FileBuffer* pBuffer)
//--------------------------------------------------------------------
  : BasicAnalyzer  (pBuffer),
    m_ePacker      (GT_PACKERENUM_NONE),
    m_nLocalOffset (0)
{
  ASSERT (pBuffer);

  // if the file is analyzed at offset 0 -> the archives must be at
  // the beginning of the file!!
  if (m_pBuffer->GetScanOffset () == 0)
  {
    // only search at the beginning
    _GetPacker (0);
  }
  else
  {
    // check the first 256 bytes...
    for (int i = 0; i < 256; i++)
    {
      _GetPacker (i);
      if (m_ePacker != GT_PACKERENUM_NONE)
      {
        m_nLocalOffset = i;
        break;
      }
    }
  }

  // inc global archive counter
  if (m_ePacker != GT_PACKERENUM_NONE)
  {
    ++g_nTotalArchives;
  }
}

//--------------------------------------------------------------------
bool ArchiveAnalyzer::_IsHAArchive (const int nIndex) const
//--------------------------------------------------------------------
{
  //------------------------------------------------------------------
  struct Helper
  //------------------------------------------------------------------
  {
    //----------------------------------------------------------------
    static bool IsFilenameA
                                            (const char*  s,
                                             const size_t nLen)
    //----------------------------------------------------------------
    {
      size_t nDotCount = 0;
      size_t i = 0;

      for (; i < nLen; i++, s++)
      {
        if (*s != '\0' &&
            *s != ' ' &&
            *s != '-' &&
            *s != '.' &&
            *s != '_' &&
            !isalnum (gtuint8 (*s)))
        {
          return false;
        }

        // only one dot allowed
        if (*s == '.' && ++nDotCount > 1)
          return false;
      }

      return true;
    }
  };

  ASSERT (nIndex >= 0);

  try
  {
    // 0x41=A  0x48=G
    if (m_pBuffer->GetUint16 (nIndex + 0) != 0x4148 || m_pBuffer->IsText ())
      return false;

    char sFilename[_MAX_PATH + 1];
    size_t nFilenameLen;
    gtuint8 nByte;
    HA_LocalHeader aLH;

    m_pBuffer->SetActFilePos (nIndex + GT_HA_GH_SIZE);

    while (!m_pBuffer->IsEOF ())
    {
      // read current header
      if (!m_pBuffer->GetBuffer (&aLH, GT_HA_LH_SIZE))
        return false;

      // skip directory
      if (!m_pBuffer->SkipUntil (0))
        return false;

      // read filename
      if (!m_pBuffer->GetUntil (0, sFilename, _MAX_PATH, nFilenameLen))
        return false;

      // invalid file position?
      if (m_pBuffer->GetActFilePos () + 1 >= m_pBuffer->GetFileSize ())
        return false;

      // comment (???)
      nByte = m_pBuffer->GetUint8 ();
      m_pBuffer->IncActFilePos (nByte);

      // valid information?
      if (!Helper::IsFilenameA (sFilename, nFilenameLen) ||
          aLH.nPackedSize < 0 ||
          file_t (aLH.nPackedSize) > m_pBuffer->GetFileSize () ||
          aLH.nOriginalSize < aLH.nPackedSize)
        return false;

      // skip packed data
      m_pBuffer->IncActFilePos (aLH.nPackedSize);
    }
  }
  catch (const Excpt &)
  {
    return false;
  }

  return true;
}

//--------------------------------------------------------------------
void ArchiveAnalyzer::_GetPacker (const int nIndex)
//--------------------------------------------------------------------
{
  ASSERT (nIndex >= 0);

  // Only if the file has at least 22 bytes
  // ZIP: minimum for empty file is 22 bytes
  if (m_pBuffer->GetFileSize () - nIndex < 22)
  {
    m_ePacker = GT_PACKERENUM_NONE;
    return;
  }

  DirEntry aDE (m_pBuffer->GetFileName ());
  const gtuint8  b0  = m_pBuffer->GetUint8 (nIndex + 0);
  const gtuint8  b2  = m_pBuffer->GetUint8 (nIndex + 2);
  const gtuint8  b4  = m_pBuffer->GetUint8 (nIndex + 4);
  const gtuint8  b5  = m_pBuffer->GetUint8 (nIndex + 5);
  const gtuint8  b6  = m_pBuffer->GetUint8 (nIndex + 6);
  const gtuint8  b7  = m_pBuffer->GetUint8 (nIndex + 7);
  const gtuint16 w0  = m_pBuffer->GetUint16 (nIndex + 0);
  const gtuint16 w1  = m_pBuffer->GetUint16 (nIndex + 1);
  const gtuint16 w2  = m_pBuffer->GetUint16 (nIndex + 2);
  const gtuint16 w4  = m_pBuffer->GetUint16 (nIndex + 4);
  const gtuint32 l0  = m_pBuffer->GetInt32 (nIndex + 0);
  const gtuint32 l2  = m_pBuffer->GetInt32 (nIndex + 2);
  const gtuint32 l4  = m_pBuffer->GetInt32 (nIndex + 4);
//  const gtuint32 l8  = m_pBuffer->GetInt32 (nIndex + 8);
  const gtuint32 l12 = m_pBuffer->GetInt32 (nIndex + 12);
//  const gtuint32 l16 = m_pBuffer->GetInt32 (nIndex + 16);
  const gtuint32 l20 = m_pBuffer->GetInt32 (nIndex + 20);
  const gtuint32 t0  = m_pBuffer->GetInt32 (nIndex + 0) & 0x00FFFFFF;
  const gtuint32 _l4  = *((gtuint32*) m_pBuffer->GetLastBuffer (4));
  const gtuint32 _l8  = *((gtuint32*) m_pBuffer->GetLastBuffer (8));
  const gtuint32 _l12 = *((gtuint32*) m_pBuffer->GetLastBuffer (12));

  // TAR must be first because it is plain text format
       if (m_pBuffer->GetFileSize () > (nIndex + 257 + 5) &&
           (m_pBuffer->CompareA (nIndex + 257, 5, "ustar") ||
            aDE.HasFileNameExtension (_T ("tar"))))
    m_ePacker = TAR;
  else if (m_pBuffer->CompareA (nIndex + 0, 3, "777") &&
           (b6 == 0 || b6 == 1) &&
           !m_pBuffer->IsText ())
    m_ePacker = _777;
  // ACE SFX
  else if (m_pBuffer->CompareA (nIndex, 5, "!sfx!"))
    m_ePacker = ACE;
  // ACE archive
  else if (m_pBuffer->CompareA (nIndex + 7, 7, "**ACE**"))
    m_ePacker = ACE;
  else if (w0 == 0x1221)
    m_ePacker = AIN;
  else if (l0 == 0x07544B41)
    m_ePacker = AKT7;
  else if (l0 == 0x08544B41)
    m_ePacker = AKT8;
  else if (l0 == 0x09544B41)
    m_ePacker = AKT9;
  else if (w0 == 0x36AD &&
           gtint16 (w2) >= 0)
    m_ePacker = AMG;  // w2 is packed size of first record
  // ARC
  // ARG
  // ARI
  else if (w0 == 0xEA60 &&
           b5 > 0)
    m_ePacker = ARJ;
  else if (l0 == 0x00100437)
    m_ePacker = ARK;
  else if (w0 == 0x5767)
    m_ePacker = ARQ;
  else if (m_pBuffer->CompareA (nIndex + 0, 14, "\x0D(C) STEPANYUK"))
    m_ePacker = ARS;
  else if (m_pBuffer->CompareA (nIndex + 2, 5, "-lh1-") &&
           b7 == 0)
    m_ePacker = ARX;
  else if (l0 == 0x3752412C)
    m_ePacker = AR7;
  else if (l0 == 0x30445341)
    m_ePacker = ASD;
  else if (l0 == 0x30584942)
    m_ePacker = BIX;
  else if (m_pBuffer->CompareA (nIndex + 0, 15, "Blink by D.T.S."))
    m_ePacker = BLI;
  else if (l0 == 0x00414F42)
    m_ePacker = BOA;
  else if (l0 == 0x425303D4)
    m_ePacker = BS2;
  else if (b0 > 0 &&
           w2 == 0x07AE &&
           m_pBuffer->GetUint16 (18) == 0)  // additional check
    m_ePacker = BSA;
  else if (l0 == 0x475342FF &&
           w4 == 0x0000)
    m_ePacker = BSN;
  else if (w0 == 0x5A42)
    m_ePacker = BZ;
  else if (l0 == 0x4643534D)
    m_ePacker = CAB;
  else if (l0 == 0x46684353)
    m_ePacker = CHZ;
  // COD
  else if (w0 == 0x9D1F)
    m_ePacker = COMP16;
  else if (m_pBuffer->CompareA (nIndex + 0, 7, "DSIGDCC"))
    m_ePacker = CPAC;
  else if (m_pBuffer->CompareA (nIndex + 0, 10, "Dirk Paehl"))
    m_ePacker = DPA;
  // DWC: LastThree "DWC"
  else if (l0 == 0x2061724F)
    m_ePacker = ELI;
  else if (l0 == 0x3E505345)
    m_ePacker = ESP;
  else if (l0 == 0x14005000)
    m_ePacker = FINISH;
  else if (w0 == 0x8B1F && _l4 > 0)
    m_ePacker = GZ;
  else if (_IsHAArchive (nIndex))
    m_ePacker = HA;
  else if (l0 == 0x46483391)
    m_ePacker = HAP;
  else if (l0 == 0x00415048)
    m_ePacker = HPA;
  else if (m_pBuffer->GetFileSize () > 64 &&
           m_pBuffer->CompareA (nIndex + 0, 25, "(This file must be conver"))
    m_ePacker = HQX;
  else if (b0 == 0x16 &&
           (w1 == 0x5048 || w1 == 0x5453))
    m_ePacker = HYP25;
  else if (l0 == 0x2650481A)
    m_ePacker = HYP26;
  else if (m_pBuffer->CompareA (nIndex + 0, 7, "idska32\0x1A"))
    m_ePacker = INNO;
  else if (l0 == 0x0A504D49)
    m_ePacker = IMP;
  else if (l0 == gtuint32 (0xD879AB2A))
    m_ePacker = INS;
  else if (l0 == 0x28635349)
    m_ePacker = ISC;
  else if (m_pBuffer->GetFileSize () >= 65536 &&
           m_pBuffer->CompareA (nIndex + 0x8000, 7, "\x01" "CD001" "\x01"))
    m_ePacker = ISO;
  else if (m_pBuffer->CompareA (nIndex + 14, 5, "\x1AJar\x1B"))
    m_ePacker = JAR;
  else if (m_pBuffer->CompareA (nIndex + 0, 7, "JRchive"))
    m_ePacker = JRC;
  else if (w0 == 0x474C &&
           !m_pBuffer->IsText ())
    m_ePacker = LG;
  // LS
  // LIM
  else if (m_pBuffer->CompareA (nIndex + 3, 7, "ChfLZ_2"))
    m_ePacker = LZA;
  else if (w2 == 0x6C2D &&
           (b4 == 'h' || b4 == 'z') &&
           b6 == '-')
    m_ePacker = LZH;
  else if (l0 == 0x4F5A4C89)
    m_ePacker = LZO;
  else if (m_pBuffer->CompareA (nIndex + 2, 5, "-ah0-"))
    m_ePacker = MAR;
  else if (l0 == 0x44445A53)
    m_ePacker = MSC;
  else if (_l8 == 0x4F52454E)
    m_ePacker = NERO;
  else if (_l12 == 0x3552454E)
    m_ePacker = NERO5;
  // different NSIS versions - second is deadbeed!!!
  else if (l0 == 0xdeadbeef &&
           m_pBuffer->CompareA (nIndex + 4, 11, "nsisinstall"))
    m_ePacker = NSIS10;
  else if (l0 == 0xdeadbeed &&
           m_pBuffer->CompareA (nIndex + 4, 12, "NullSoftInst"))
    m_ePacker = NSIS11E;
  else if (l4 == 0xdeadbeef &&
           m_pBuffer->CompareA (nIndex + 8, 12, "NullSoftInst"))
    m_ePacker = NSIS13;
  else if (l4 == 0xdeadbeef &&
           m_pBuffer->CompareA (nIndex + 8, 12, "NullsoftInst"))
    m_ePacker = NSIS16;
  else if (w0 == 0x96A5)
    m_ePacker = PACK;
  else if (l0 == 0x5A2B1527)
    m_ePacker = PAQ;
  else if (l0 == 0x38716170)
    m_ePacker = PAQ8;
  else if (m_pBuffer->CompareA (nIndex + 0, 7, "PHILIPP"))
    m_ePacker = PAR;
  else if (w0 == 0x4850 &&
           b2 < 2)
    m_ePacker = PHP;
  else if (l0 == 0x5A4D5050)
    m_ePacker = PPZ;
  // Q
  else if (l0 == 0x1A46511A)
    m_ePacker = QFC;
  else if (l0 == 0x5E7E4552)
    m_ePacker = RAR0;
  else if (l0 == 0x21726152)
    m_ePacker = RAR;
  else if (l0 == 0x42454C55)
    m_ePacker = RAX;
  else if (w1 == 0x0F80)
    m_ePacker = RKV1; // 0x0F80 or 0x800F ??
  else if (w1 == 0x07C0)
    m_ePacker = RKV2; // 0x07C0 or 0xC007 ??
  else if (l0 == 0x41434E52)
    m_ePacker = RNC;
  else if (l2 == 0x35484C20)
    m_ePacker = SAR;  // 0x35484C20 or 0x4C203548 ??
  else if (t0 == 0x00434253)
    m_ePacker = SBC;
  else if (l0 == 0x00314253)
    m_ePacker = SBX;
  else if (l0 == 0x001040BC)
    m_ePacker = SKY;
  else if (l0 == 0x06034B50)
    m_ePacker = SOF;
  else if (l0 == 0x4e4a4354 &&
           m_pBuffer->GetFileSize () > 0x200)
    m_ePacker = SONY;
  else if (l0 == 0x45575153)
    m_ePacker = SQWEZ;
  else if (m_pBuffer->CompareA (nIndex + 0, 5, "HLSQZ"))
    m_ePacker = SQZ;
  else if (l0 == gtuint32 (0x8C655D13))
    m_ePacker = STI;
  else if (m_pBuffer->CompareA (nIndex + 2, 5, "-sw1-"))
    m_ePacker = SWG;
  else if (l0 == 0x53594E46)
    m_ePacker = SYN;
  else if (l0 == 0x00010601 ||
           l0 == 0x040A5A53)
    m_ePacker = SZIP;
  else if (l0 == 0x04034B54)
    m_ePacker = TIP;   // TuneUp Icon Package
  else if (t0 == 0x324355)
    m_ePacker = UC2;   // 0x00324355 or 0x55433200 ??
  else if (l0 == gtuint32 (0xC6414655))
    m_ePacker = UFA;
  else if (l0 == 0x02414855)
    m_ePacker = UHA;
  // MSXIE
  else if (l0 == gtuint32 (0x82024C58))
    m_ePacker = XL;
  else if (l0 == 0x00617078)
    m_ePacker = XPA;
  else if (l12 == 0x43590100)
    m_ePacker = YC;
  else if (w0 == 0x7A79)
    m_ePacker = YZ;
  else if (w0 == 0x5A4F &&
           b2 == 0xDD)
    m_ePacker = ZET;
  else if (l0 == 0x04034B50 ||
           l0 == 0x04034B4F ||   // the second for WWPack SFX
           l0 == 0x06054B50 ||   // ZIP CDSE (corrupted, or emptied ZIPs)
           l0 == 0x08074B50 ||   // multi volume ZIP
           l0 == 0x30304B50)     // multi volume ZIP
    m_ePacker = ZIP;
  else if (l20 == gtuint32 (0xFDC4A7DC))
    m_ePacker = ZOO;
  else if (l0 == 0x4B505A01)
    m_ePacker = ZPK;
  else
    m_ePacker = GT_PACKERENUM_NONE;

  if (m_ePacker != GT_PACKERENUM_NONE && m_pBuffer->GetScanOffset () > 0)
  {
    // if we're in an overlay, check whether this archive can be an overlay ||
    // if we're in an offset but not at offset 0, check whether this is allowed
    // NOTE m_nLocalIndex is not yet set!
    if ((GTArchive_Info[m_ePacker].eOvrPos == ARC_OVERLAY_NONE) ||
        (GTArchive_Info[m_ePacker].eOvrPos == ARC_OVERLAY_0 && nIndex != 0))
    {
      m_ePacker = GT_PACKERENUM_NONE;
    }
    // -> now it should be okay to list the archive!
  }
}

//--------------------------------------------------------------------
void ArchiveAnalyzer::_ListArchiveName () const
//--------------------------------------------------------------------
{
  if (m_ePacker != GT_PACKERENUM_NONE)
  {
    // found XYZ archive
    out_format (rc (HRC_ARCLIB_FOUND_ARCHIVE),
                GTArchive_Info [m_ePacker].sName);

    if (m_nLocalOffset > 0)
    {
      // additional offset (e.g. in SFX overlay)
      out_format (rc (HRC_ARCLIB_AT_OFFSET),
                  m_nLocalOffset, m_nLocalOffset);
    }

    // is the archive listing allowed
    if (CmdlineParam_GetBool (GT_FLAG_NOARCS))
      out_append (rc (HRC_ARCLIB_LISTING_IS_DISABLED));

    out_append (_T ("\n"));
  }
}

//--------------------------------------------------------------------
bool ArchiveAnalyzer::_ListArchiveContent ()
//--------------------------------------------------------------------
{
  ASSERT (m_ePacker != GT_PACKERENUM_NONE);

  bool bListed = true;
  BasicArchiveLister *pLister = NULL;

#define CREATE_LISTER(fmt) \
               pLister = new fmt##ArchiveLister (m_pBuffer, m_nLocalOffset); \
               break

  switch (m_ePacker)
  {
    case AR7:
    case ARS:
    case BZ:
    case ESP:
    case HPA:
    case HQX:
    case JAR:
    case PAR:
    case RKV1:
    case RKV2:
    case UC2:
    case UHA:
    case YZ:
    {
      bListed = false;
      CREATE_LISTER (Basic);
    }
    case _777:    CREATE_LISTER (_777);
    case ACE:     CREATE_LISTER (ACE);
    case AIN:     CREATE_LISTER (AIN);
    case AMG:     CREATE_LISTER (AMG);
    case AKT7:    CREATE_LISTER (AKT7);
    case AKT9:    CREATE_LISTER (AKT9);
    case ARC:     CREATE_LISTER (ARC);
    case ARG:     CREATE_LISTER (ARG);
    case ARJ:     CREATE_LISTER (ARJ);
    case ARQ:     CREATE_LISTER (ARQ);
    case ARX:     CREATE_LISTER (ARX);
    case ASD:     CREATE_LISTER (ASD);
    case BIX:     CREATE_LISTER (BIX);
    case BLI:     CREATE_LISTER (BLI);
    case BOA:     CREATE_LISTER (BOA);
    case BSA:     CREATE_LISTER (BSA);
    case BSN:     CREATE_LISTER (BSN);
    case CAB:     CREATE_LISTER (CAB);
    case DPA:     CREATE_LISTER (DPA);
    case GZ:      CREATE_LISTER (GZ);
    case HA:      CREATE_LISTER (HA);
    case INNO:    CREATE_LISTER (INNO);
    case IMP:     CREATE_LISTER (IMP);
    case INS:     CREATE_LISTER (INS);
    case ISC:     CREATE_LISTER (ISC);
    case ISO:     CREATE_LISTER (ISO);
    case LG:      CREATE_LISTER (LG);
    case LZH:     CREATE_LISTER (LZH);
    case LZO:     CREATE_LISTER (LZO);
    case NERO:    CREATE_LISTER (NERO);
    case NERO5:   CREATE_LISTER (NERO5);
    case NSIS13:  CREATE_LISTER (NSIS13);
    case NSIS16:  CREATE_LISTER (NSIS16);
    case PAQ:     CREATE_LISTER (PAQ);
    case PAQ8:    CREATE_LISTER (PAQ8);
    case PPZ:     CREATE_LISTER (PPZ);
    case RAR:     CREATE_LISTER (RAR);
    case SBC:     CREATE_LISTER (SBC);
    case SONY:    CREATE_LISTER (SONY);
    case STI:     CREATE_LISTER (STI);
    case SYN:     CREATE_LISTER (SYN);
    case TAR:     CREATE_LISTER (TAR);
    case XPA:     CREATE_LISTER (XPA);
    case YC:      CREATE_LISTER (YC);
    case ZET:     CREATE_LISTER (ZET);
    case TIP:     /* fall through */
    case ZIP:     CREATE_LISTER (ZIP);
    case ZOO:     CREATE_LISTER (ZOO);
    default:
    {
      // no lister present...
      out_error_append (rc (HRC_ARCLIB_NO_LISTER));
      return false;
    }
  }

  ASSERT (pLister);

  // change status text (console headline)
  {
    pstring sText;
    str_assign_from_res (sText, HRC_ARCLIB_READING_ARCHIVE);
    sText += m_pBuffer->GetFileName ();
    out_status (sText.c_str ());
  }

  try
  {
    // fill structures
    pLister->ListFiles ();
  }
  catch (const Excpt&)
  {
    // exception!
    pLister->SetErrorCode (ARCERR_EXCEPTION);
  }

  // error in lister?
  if (pLister->GetErrorCode () != ARCERR_SUCCESS)
    ++g_nDamagedArchives;

  // display the content of the archive
  pLister->Display ();

  // check whether there is some data behind the archive ....
  if (pLister->GetErrorCode () == ARCERR_SUCCESS &&
      bListed &&
      m_pBuffer->GetActFilePos () < m_pBuffer->GetFileSize ())
  {
    m_nOverlayOffset = m_pBuffer->GetActFilePos ();
  }

  // don't forget to free memory
  delete pLister;

  return true;
}

//--------------------------------------------------------------------
void GT_CALL ArchiveAnalyzer::_ShowResults ()
//--------------------------------------------------------------------
{
  // write arc header
  _ListArchiveName ();

  // list arc
  if (!CmdlineParam_GetBool (GT_FLAG_NOARCS))
  {
    out_incindent ();
    _ListArchiveContent ();
    out_decindent ();
  }
}

//--------------------------------------------------------------------
gtuint32 GT_CALL ArchiveAnalyzer::GetTotalArchiveCount ()
//--------------------------------------------------------------------
{
  return g_nTotalArchives;
}

//--------------------------------------------------------------------
gtuint32 GT_CALL ArchiveAnalyzer::GetDamagedArchiveCount ()
//--------------------------------------------------------------------
{
  return g_nDamagedArchives;
}

}  // namespace
