MAKE_ANALYZER (Basic);

namespace Basic {

resid_t g_nResID = resid_t (-1);
file_t  g_nOverlayOffset = -1;

//--------------------------------------------------------------------
static bool _IsUUencodedChar (TCHAR c)
//--------------------------------------------------------------------
{
  return c >= '0' && c <= '7';
}

}  // namespace Basic

//--------------------------------------------------------------------
MAKE_IS_A (Basic)
//--------------------------------------------------------------------
{
  // 4 easier code
  FileBuffer* pBuffer = aGI.m_pBuffer;
  resid_t& nResID = Basic::g_nResID;

  // real start
  nResID = 0;
  Basic::g_nOverlayOffset = 0;

  DirEntry aDE (aGI.m_pBuffer->GetFileName ());
  const gtuint8  b0  = pBuffer->GetUint8 (0);
//  const gtuint8  b1  = pBuffer->GetUint8 (1);
  const gtuint8  b2  = pBuffer->GetUint8 (2);
  const gtuint8  b3  = pBuffer->GetUint8 (3);
  const gtuint8  b4  = pBuffer->GetUint8 (4);
//  const gtuint8  b5  = pBuffer->GetUint8 (5);
  const gtuint8  b6  = pBuffer->GetUint8 (6);
  const gtuint8  b7  = pBuffer->GetUint8 (7);
  const gtuint8  b8  = pBuffer->GetUint8 (8);
  const gtuint16 w0  = pBuffer->GetUint16 (0);
  const gtuint16 w1  = pBuffer->GetUint16 (1);
//  const gtuint16 w2  = pBuffer->GetUint16 (2);
  const gtuint16 w4  = pBuffer->GetUint16 (4);
  const gtuint32 l0  = pBuffer->GetInt32 (0);
  const gtuint32 l4  = pBuffer->GetInt32 (4);
//  const gtuint32 l8  = pBuffer->GetInt32 (8);
//  const gtuint32 l12 = pBuffer->GetInt32 (12);
  const gtuint32 t0  = l0 & 0x00FFFFFF;
  const file_t   fs  = pBuffer->GetFileSize ();

  // no matter whether text or binary
       if (l0 == 0x46445025)            nResID = HRC_GENLIB_BASIC_PDF;
  else if (l0 == 0x46444625)            nResID = HRC_GENLIB_BASIC_FDF;
  else if (l0 == 0x646E732E)            nResID = HRC_GENLIB_BASIC_SUN_SOUND;
  else if (b0 == 0x80 && ((w1 == b3 + 3) || (w1 == b3 + 2)))
                                        nResID = HRC_GENLIB_BASIC_BORLAND_OBJ;
  else
  // text only
  if (pBuffer->IsText ())
  {
         if (pBuffer->CompareA (0, 4, "<Asx"))
                                        nResID = HRC_GENLIB_BASIC_ASX; // >
    else if (pBuffer->CompareA (3, 5, "Start") && aDE.HasFileNameExtension (_T ("map")))
    {
                                        nResID = HRC_GENLIB_BASIC_BORLAND_MAP;
    }
    else if (pBuffer->CompareA (0, 6, "{\\rtf1"))
                                        nResID = HRC_GENLIB_BASIC_RTF; // }
    else if (pBuffer->CompareA (0, 7, "^TITLE^"))
                                        nResID = HRC_GENLIB_BASIC_APATCH;
    else if (pBuffer->CompareA (0, 8, "REGEDIT4"))
                                        nResID = HRC_GENLIB_BASIC_WIN_REG4_TEXT;
    else if (pBuffer->CompareA (0, 8, "<MIFFile"))
                                        nResID = HRC_GENLIB_BASIC_FRAMEMAKER; // >
    else if (pBuffer->CompareA (0, 6, "begin ") &&
             Basic::_IsUUencodedChar (b6) &&
             Basic::_IsUUencodedChar (b7) &&
             Basic::_IsUUencodedChar (b8))
                                        nResID = HRC_GENLIB_BASIC_UUE;
    else if (pBuffer->CompareA (0, 10, "%!PS-Adobe"))
                                        nResID = HRC_GENLIB_BASIC_PS;
    else if (pBuffer->CompareA (0, 11, "begin:vcard", false))
                                        nResID = HRC_GENLIB_BASIC_VCF;
    else if (pBuffer->CompareA (0, 16, "ibo Ablauf-Profi"))
                                        nResID = HRC_GENLIB_BASIC_IBO;
    else if (pBuffer->CompareA (0, 36, "Windows Registry Editor Version 5.00"))
                                        nResID = HRC_GENLIB_BASIC_WIN_REG5_TEXT;
    else if (pBuffer->CompareA (0, 37, "Microsoft Visual Studio Solution File"))
                                        nResID = HRC_GENLIB_BASIC_WIN_VS_SLN;
    else if (pBuffer->iFind ("ECHO OFF", 0, 200) != GT_NOT_FOUND)
                                        nResID = HRC_GENLIB_BASIC_BAT;
  }
  else
  // binary only
  {
         if (pBuffer->CompareA (0, 8, "DVDVIDEO"))
                                                    nResID = HRC_GENLIB_BASIC_IFO;
    else if (pBuffer->CompareA (0, 12, ">>>McA1ph<<<") && pBuffer->GetScanOffset () > 0)
                                                    nResID = HRC_GENLIB_BASIC_MCAFEE;
    else if (pBuffer->CompareA (0, 12, "SSCE Lexicon"))
                                                    nResID = HRC_GENLIB_BASIC_SSCE_LEXICON;
    else if (pBuffer->CompareA (0, 15, "Client UrlCache"))
                                                    nResID = HRC_GENLIB_BASIC_IE_CACHE;
    else if (pBuffer->CompareA (0, 15, "JASC BROWS FILE"))
                                                    nResID = HRC_GENLIB_BASIC_PSP_BROWSE;
    else if (pBuffer->CompareA (0, 20, "Thunderbyte checksum"))
                                                    nResID = HRC_GENLIB_BASIC_THUNDERBYTE;
    else if (pBuffer->CompareA (0, 22, "TURBO PASCAL HELP FILE"))
                                                    nResID = HRC_GENLIB_BASIC_TP_HLP;
    else if (pBuffer->CompareA (0, 25, "Microsoft Linker Database"))
                                                    nResID = HRC_GENLIB_BASIC_MS_LINKER;
    else if (pBuffer->CompareA (0, 25, "Paint Shop Pro Image File"))
                                                    nResID = HRC_GENLIB_BASIC_PSP_IMAGE;
    else if (pBuffer->CompareA (0, 31, "Turbo Pascal Configuration File"))
                                                    nResID = HRC_GENLIB_BASIC_TP_CFG;
    else if (pBuffer->CompareA (0, 32, "Microsoft C/C++ program database"))
                                                    nResID = HRC_GENLIB_BASIC_MS_PDB;
    else if (pBuffer->CompareA (0, 5, "MsDos") && pBuffer->GetScanOffset () > 0)
                                                    nResID = HRC_GENLIB_BASIC_JERUSALEM;
    else if (pBuffer->CompareA (0, 6, "IDAIDS"))    nResID = HRC_GENLIB_BASIC_IDS;
    else if (pBuffer->CompareA (0, 6, "IImGxP"))    nResID = HRC_GENLIB_BASIC_PAX2;
    else if (pBuffer->CompareA (0, 6, "VCPCH0"))    nResID = HRC_GENLIB_BASIC_MS_PCH;
    else if (pBuffer->CompareA (0, 7, "ARJ_SFX"))
    {
      nResID = HRC_GENLIB_BASIC_ARJ;
      Basic::g_nOverlayOffset = 16;
    }
    else if (pBuffer->CompareA (0, 7, "MS-PROF"))   nResID = HRC_GENLIB_BASIC_MS_PBI;
    else if (pBuffer->CompareA (0, 7, "WordPro"))   nResID = HRC_GENLIB_BASIC_LOTUS_WORDPRO;
    else if (pBuffer->CompareA (0, 8, "[ver]\xD\xA\x9"))
                                                    nResID = HRC_GENLIB_BASIC_LOTUS_AMIPRO;
    else if (pBuffer->CompareA (0, 8, "ARMV0006"))  nResID = HRC_GENLIB_BASIC_ARMADILLO_PRJ;
    else if (pBuffer->CompareA (0, 8, "PNCIUNDO"))  nResID = HRC_GENLIB_BASIC_NU_DD_UNDO;
    else if (pBuffer->CompareA (0, 8, "PS-X EXE"))  nResID = HRC_GENLIB_BASIC_PS_EXE;
    else if (pBuffer->CompareA (0, 10, "!<symlink>"))
                                                    nResID = HRC_GENLIB_BASIC_CYGWIN_SYMLINK;
    else if (pBuffer->CompareA (4, 15, "Standard Jet DB"))
                                                    nResID = HRC_GENLIB_BASIC_MS_MDB;
    else if (l0 == 0x00000001 && l4 == 0x00000068)  nResID = HRC_GENLIB_BASIC_EMF;
    else if (l0 == 0x0000004C)                      nResID = HRC_GENLIB_BASIC_LNK;
    else if (l0 == 0x00000107)                      nResID = HRC_GENLIB_BASIC_FPK_OBJ;
    else if (l0 == 0x00000DF0)                      nResID = HRC_GENLIB_BASIC_CPP_LIB;
    else if (l0 == 0x00000F01)                      nResID = HRC_GENLIB_BASIC_SQL_DB;
    else if (l0 == 0x00004944)                      nResID = HRC_GENLIB_BASIC_MS_DBG_INFO;
    else if (l0 == 0x00007E30)                      nResID = HRC_GENLIB_BASIC_COBOL_DATA;
    else if (l0 == 0x0000A871)                      nResID = HRC_GENLIB_BASIC_IS_UNINSTALL;
    else if (l0 == 0x0000BE31)                      nResID = HRC_GENLIB_BASIC_MS_WORD;
    else if (l0 == 0x0000FE30)                      nResID = HRC_GENLIB_BASIC_COBOL_INDEX;
    else if (l0 == 0x00020000)                      nResID = HRC_GENLIB_BASIC_CUR;
    else if (l0 == 0x00060409)                      nResID = HRC_GENLIB_BASIC_MS_EXCEL4;
    else if (l0 == 0x000CC9B8)                      nResID = HRC_GENLIB_BASIC_IS_SCRIPT;
    else if (l0 == 0x00013350)
    {
      nResID = HRC_GENLIB_BASIC_PHARLAP;
      Basic::g_nOverlayOffset = pBuffer->GetInt32 (6);
    }
    else if (l0 == 0x00112BAD)                      nResID = HRC_GENLIB_BASIC_CPP_SYM;
    else if (l0 == 0x001A0000)                      nResID = HRC_GENLIB_BASIC_LOTUS123;
    else if (l0 == 0x002DA5DB)                      nResID = HRC_GENLIB_BASIC_MS_WORD2;
    else if (l0 == 0x08084B50)                      nResID = HRC_GENLIB_BASIC_BGI_FONT;
    else if (l0 == 0x08086B70 || l0 == 0x44474246)  nResID = HRC_GENLIB_BASIC_BGI_DRIVER;
    else if (l0 == 0x01045F3F)                      nResID = HRC_GENLIB_BASIC_MS_VC_HLP;
    else if (l0 == 0x01505349)                      nResID = HRC_GENLIB_BASIC_IBM_HLP;
    else if (l0 == 0x0300001A)                      nResID = HRC_GENLIB_BASIC_NOTES45;
    else if (l0 == 0x07000200)                      nResID = HRC_GENLIB_BASIC_MS_SBR;
    else if (l0 == 0x0A0DF23B)                      nResID = HRC_GENLIB_BASIC_PYTHON;
    else if (l0 == 0x12001B02)                      nResID = HRC_GENLIB_BASIC_LATEX_FONT;
    else if (l0 == 0x1A626C7A)                      nResID = HRC_GENLIB_BASIC_INNO_ZLIB;
    else if (l0 == 0x3047534A)                      nResID = HRC_GENLIB_BASIC_MS_MDP;
    else if (l0 == 0x33535054)                      nResID = HRC_GENLIB_BASIC_BORLAND_PSM;
    else if (l0 == 0x33495056)                      nResID = HRC_GENLIB_BASIC_VP_UNIT3;
    else if (l0 == 0x414D4D45)                      nResID = HRC_GENLIB_BASIC_EMMA;
    else if (l0 == 0x44518641 && file_t (l4) == fs)
                                                    nResID = HRC_GENLIB_BASIC_BORLAND_DUNIT3;
    else if (l0 == 0x46484246)                      nResID = HRC_GENLIB_BASIC_BORLAND_TV_HELP;
//    else if (l0 == 0x46535449)                      nResID = HRC_GENLIB_BASIC_MS_CHM;
    else if (l0 == 0x47455243)                      nResID = HRC_GENLIB_BASIC_WIN_REG_BIN;
    else if (l0 == 0x4768A6D8 && file_t (l4) == fs)
                                                    nResID = HRC_GENLIB_BASIC_BORLAND_DUNIT4;
    else if (l0 == 0x4957504E)                      nResID = HRC_GENLIB_BASIC_NUMEGA_BC_RES;
    else if (l0 == 0x4A41574B)                      nResID = HRC_GENLIB_BASIC_MS_COMPRESS;
    else if (l0 == 0x4C534854)                      nResID = HRC_GENLIB_BASIC_LINKS_LS;
    else if (l0 == 0x4D435380)                      nResID = HRC_GENLIB_BASIC_LOTUS_SCREENCAM;
    else if (l0 == 0x4D4F4F5A)                      nResID = HRC_GENLIB_BASIC_FRACT_MOVIE;
    else if (l0 == 0x4D535150)                      nResID = HRC_GENLIB_BASIC_MS_QP_UNIT;
    else if (l0 == 0x54000AFF)                      nResID = HRC_GENLIB_BASIC_BORLAND_DELPHI_FORM;
    else if (l0 == 0x5446534D)                      nResID = HRC_GENLIB_BASIC_MS_TLB;
    else if (l0 == 0x544C4150)                      nResID = HRC_GENLIB_BASIC_FRACT_PALETTE;
    else if (l0 == 0x564F4246)                      nResID = HRC_GENLIB_BASIC_BORLAND_OVR;
    else if (l0 == 0x75B22630 && l4 == 0x11CF668E)  nResID = HRC_GENLIB_BASIC_WMV;
    else if (l0 == 0x830102F7)                      nResID = HRC_GENLIB_BASIC_DVI;
    else if (l0 == 0xA09093A4)                      nResID = HRC_GENLIB_BASIC_DURILCA;
    else if (l0 == 0xBA010000)                      nResID = HRC_GENLIB_BASIC_VOB;
    else if (l0 == 0xBEEFCACE)                      nResID = HRC_GENLIB_BASIC_RESOURCE;
    else if (l0 == 0xE1FFD8FF)                      nResID = HRC_GENLIB_BASIC_CANON_THUMB;
    else if (l0 == 0xE3D06882)                      nResID = HRC_GENLIB_BASIC_FOOBAR200_PLAYLIST;
    else if (l0 == 0xF24AF34D)                      nResID = HRC_GENLIB_BASIC_DEMOSHIELD;
    else if (l0 == 0xFFFFFFDD && l4 == 0xA1981FF1)  nResID = HRC_GENLIB_BASIC_SYMANTEC_ITDF;
    else if (l4 == 0x00000154)                      nResID = HRC_GENLIB_BASIC_EUDORA_AD;
    else if (l4 == 0x766F6F6D)                      nResID = HRC_GENLIB_BASIC_MOV;
    else if (l4 == 0x862A0906)                      nResID = HRC_GENLIB_BASIC_PKCS7;
    else if (t0 == 0x00535746)                      nResID = HRC_GENLIB_BASIC_FLASH;
    else if (w0 == 0x424E)                          nResID = HRC_GENLIB_BASIC_MS_DBG;
    else if (w0 == 0x8230 && w4 == 0x8230)          nResID = HRC_GENLIB_BASIC_DER;
    else if (b0 == 0xE0 && b2 == 0x05 && b3 == 0x87 && b4 == 0x44)
                                                    nResID = HRC_GENLIB_BASIC_EMULE_PART_MET;
    else if (fs > 2056 && pBuffer->CompareA (2049, 7, "PCD_IPI"))
                                                    nResID = HRC_GENLIB_BASIC_PHOTOCD;
    else if (l0 == 0x45505954 && pBuffer->GetInt32 (8 + l4) == 0x20524556)
                                                    nResID = HRC_GENLIB_BASIC_WARCRAFT2_PUD;
  }

  return nResID != 0;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (Basic)
//--------------------------------------------------------------------
{
  out_append (rc (Basic::g_nResID));
  out_append (_T ("\n"));

  // determined in IsA
  nOverlayOffset = Basic::g_nOverlayOffset;
}
