#include "gt_exe_analyzer.hxx"
#include "gt_exe_le.hxx"
#include "gt_exe_macro.hxx"
#include "gt_exe_ne.hxx"
#include "gt_exe_pe.hxx"
#include "gt_exe_defines.hxx"

#include "gtc_data.hxx"
#include "gt_cmdline_params.hxx"
#include "gt_compare.hxx"
#include "gt_exception.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_table.hxx"
#include "gt_utils.hxx"  // _max

namespace GT {

#define DETERMINE_MAX_BUFFERSIZE(__nIP)   size_t (_min (m_pBuffer->GetFileSize () - __nIP, file_t (EXE_BUFFER_SIZE)))

//--------------------------------------------------------------------
static void _WWPack300PR (const EXEEntry* pEntry,
                          gtuint8*        pStartupCode,
                          FileBuffer* /* = NULL */)
//--------------------------------------------------------------------
{
  ASSERT (pEntry);
  ASSERT (pStartupCode);

  // build display string
  pstring sTmp (pEntry->sName);
  sTmp += pStartupCode [1] == 9
            ? _T ("3.00/3.01 PR")
            : _T ("\?\?\? (3.00)");
  SignatureCmp::DisplayOutput (PACKER, GTCOMP_MATCH, sTmp.c_str ());
}

//--------------------------------------------------------------------
static void _WWPack30xPR (const EXEEntry*      pEntry,
                          gtuint8*       pStartupCode,
                          FileBuffer* /* = NULL */)
//--------------------------------------------------------------------
{
  ASSERT (pEntry);
  ASSERT (pStartupCode);

  pstring sTmp (pEntry->sName);
  switch (pStartupCode [1])
  {
    case  0: sTmp += _T ("3.02 PR"); break;
    case  3: sTmp += _T ("3.03 PR"); break;
    case  7: sTmp += _T ("3.04 PR"); break;
    case 11: sTmp += _T ("3.05á PR"); break;
    default: sTmp += _T ("??? (3.0x PR)"); break;
  }

  SignatureCmp::DisplayOutput (PACKER, GTCOMP_MATCH, sTmp.c_str ());
}

//--------------------------------------------------------------------
static void _WWPack30xP (const EXEEntry*      pEntry,
                         gtuint8*       pStartupCode,
                         FileBuffer* /* = NULL */)
//--------------------------------------------------------------------
{
  ASSERT (pEntry);
  ASSERT (pStartupCode);

  pstring sTmp (pEntry->sName);
  switch (pStartupCode [1])
  {
    case 10: sTmp += _T ("3.00 P"); break;
    case 11: sTmp += _T ("3.00 PU"); break;

    case 12: sTmp += _T ("3.01 P"); break;
    case 13: sTmp += _T ("3.01 PU"); break;

    case 15: sTmp += _T ("3.02 P"); break;
    case 16: sTmp += _T ("3.02 PU"); break;

    case 18: sTmp += _T ("3.03 P"); break;
    case 19: sTmp += _T ("3.03 PU"); break;
    case 20: sTmp += _T ("3.03 PP"); break;

    case 22: sTmp += _T ("3.04 P"); break;
    case 23: sTmp += _T ("3.04 PU"); break;
    case 24: sTmp += _T ("3.04 PP"); break;

    case 26: sTmp += _T ("3.05á P"); break;
    case 27: sTmp += _T ("3.05á PU"); break;
    case 28: sTmp += _T ("3.05á PP"); break;

    default: sTmp += _T ("??? (3.0x P)"); break;
  }

  SignatureCmp::DisplayOutput (PACKER, GTCOMP_MATCH, sTmp.c_str ());
}

//--------------------------------------------------------------------
static void _PKLite (const EXEEntry* pEntry,
                     gtuint8*,
                     FileBuffer*     pFileBuffer /* = NULL */)
//--------------------------------------------------------------------
{
  ASSERT (pEntry);
  ASSERT (pFileBuffer);

  const gtuint8 nLo = pFileBuffer->GetUint8 (28);
  const gtuint8 nHi = pFileBuffer->GetUint8 (29);

  SignatureCmp::DisplayOutput (PACKER, GTCOMP_MATCH, pEntry->sName);

  out_info_format (rc (HRC_EXELIB_DOS_PKLITE_VERSION),
                   nHi & 0x0F, nLo);

  // extra compression?
  if (nHi & 0x10)
    out_info_append (rc (HRC_EXELIB_DOS_PKLITE_EXTRA_COMPRESSION));
}

//--------------------------------------------------------------------
static bool __CheckForPKLite (FileBuffer* pBuffer)
//--------------------------------------------------------------------
{
  const gtuint32 l20 = pBuffer->GetInt32 (20);
  const gtuint8  b28 = pBuffer->GetUint8 (28);
  const gtuint8  b29 = pBuffer->GetUint8 (29);
  const gtuint16 w30 = pBuffer->GetUint16 (30);

  // PKLite generic check
  gtuint8 nHiVersion = gtuint8 (b29 & 0x0F);
  const int bCode = (nHiVersion >= 1 &&
                     nHiVersion <= 2 &&
                     l20 == 0xFFF00100);
  const int bID = (w30 == 0x4B50);

  if (bID && !bCode)
  {
    // found ID but no code of PKLite
    out_info_append (rc (HRC_EXELIB_DOS_PKLITE_ID));
  }
  else
  if (!bID && bCode)
  {
    // found code but no ID of PKLite
    out_info_append (rc (HRC_EXELIB_DOS_PKLITE_CODE));
  }

  if (bCode) // if the code matches, lets say its PKLite ;-)
  {
    // found packer PKLite ...
    const resid_t nTxtID = b29 & 0x10 ? HRC_EXELIB_DOS_FOUND_PKLITE_EXTRA : HRC_EXELIB_DOS_FOUND_PKLITE;
    out_format (rc (nTxtID),
                nHiVersion, b28);
    return true;
  }

  return false;
}

//--------------------------------------------------------------------
static bool __CheckForEXEPACK (FileBuffer* pBuffer)
//--------------------------------------------------------------------
{
  const gtuint8   b8  = pBuffer->GetUint8 (8);
  /*
  const gtuint8   b12 = pBuffer->GetUint8 (12);
  const gtuint8   b13 = pBuffer->GetUint8 (13);
  */
  const gtuint8   b16 = pBuffer->GetUint8 (16);
  const gtuint8   b18 = pBuffer->GetUint8 (18);
  const gtuint8   b20 = pBuffer->GetUint8 (20);

  {
    // EXEPack generic detection
    if (pBuffer->EqualBytes (34, 477, (gtuint8) 0) &&
        b8  == 0x20 &&
        /*
        b12 == 0xFF &&
        b13 == 0xFF &&
        */
        b16 == 0x80)
    {
      SignatureCmp::DisplayOutput (PACKER, GTCOMP_MATCH,
                                   b18 == 0x00 && b20 == 0x10 ? _T ("EXEPack 3.65") :
                                   b18 == 0x99 && b20 == 0x10 ? _T ("EXEPack 4.00") :
                                   b18 == 0x15 && b20 == 0x10 ? _T ("EXEPack 4.03") :
                                   b18 == 0x00 && b20 == 0x12 ? _T ("EXEPack ID_2") : _T ("EXEPack ???"));
      return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------
static bool __CheckForSEAAXE (FileBuffer* pBuffer)
//--------------------------------------------------------------------
{
  char sSEAAXEVersion[12];
  pstring sStr;

  try
  {
    // SEA-AXE generic detection
    if (pBuffer->CompareA (33, 8, "SEA-AXE "))
    {
      // get version
      pBuffer->GetBuffer (33, sSEAAXEVersion, 11);
      sSEAAXEVersion[11] = '\0';

      // convert to TCHAR
      str_assign (sStr, sSEAAXEVersion);
      SignatureCmp::DisplayOutput (PACKER, GTCOMP_MATCH, sStr.c_str ());

      // and original filename
      pBuffer->GetASCIIZ_AString (206, sStr, 12);
      out_info_format (rc (HRC_EXELIB_DOS_SEAAXE_ORIGINAL_FILENAME),
                       sStr.c_str ());

      return true;
    }
  }
  catch (const Excpt&)
  {}

  return false;
}

//--------------------------------------------------------------------
static bool __CheckForTrap12x
                                        (FileBuffer* pBuffer,
                                         const file_t nRealIP)
//--------------------------------------------------------------------
{
  EXE_MacroManager aMacroManager (pBuffer, NULL, NULL);
  const bool       bIsCOM = (nRealIP == 0);

  file_t nStartIP = nRealIP;
  gtuint8 tempbuf [0x330];
  bool bFlag = false;
  INT i = 0;
  INT j = 0;

  if (!bIsCOM || aMacroManager.GetJumpPos (nStartIP, 1))
  {
    const file_t nSpaceBetweenIPAndEOF = pBuffer->GetFileSize () - nStartIP;

    // the decrypter should have a size between 3800 and 4700 bytes
    if (nSpaceBetweenIPAndEOF > 3800 && nSpaceBetweenIPAndEOF < 4700)
    {
      // fill the analysis buffer
      pBuffer->GetBuffer (nStartIP, tempbuf, 0x330);

      //out_append ("pass1\n");

      while (i <= 0x300 && !bFlag)
      {
        if (tempbuf[i] != 0x81 &&
            tempbuf[i + 1] == 0xEB &&
            tempbuf[i + 2] < 0x80)
        {
          //out_format ("Jump at %d\n", i);
          i += tempbuf[i + 2];
          if (++j > 6)
            bFlag = true;
        }
        else
          i++;
      }

      if (bFlag)
      {
        //out_append ("pass2\n");

        i = 0x2BC;
        bFlag = false;
        do
        {
          switch (tempbuf[i])
          {
            case 0x9D:
            {
              if ((tempbuf[i + 1] & 0xF0) == 0x70)
                bFlag = true;
              break;
            }
            case 0xE0:
            case 0xE2:
            {
              if (tempbuf[i + 1] > 0xA0)
                bFlag = true;
              break;
            }
          }
          i++;
        } while (!bFlag && i <= 0x32F);

        if (bFlag && memcmp (&tempbuf[17], "\xB4\x4A\xCD\x21", 4) == 0)
          bFlag = false;

        if (bFlag)
        {
          // out_format ("Currently at %d ", i);
          if (nSpaceBetweenIPAndEOF > 4100)
          {
            // htyp = 1; s = "5";
          }
          else
          {
            // htyp = 0; s = "4";
          }

          // display that it is Trap
          SignatureCmp::DisplayOutput (ENCRYPTER, GTCOMP_MATCH, _T ("Trap 1.2x"));

          // and show decryptor size
          out_info_format (rc (HRC_EXELIB_DOS_TRAP_DECRYPTOR_SIZE),
                           nSpaceBetweenIPAndEOF);
          return true;
        }
      }
    } // space between IP and EOF
  } // COM || macropos

  return false;
}

//--------------------------------------------------------------------
void GT_CALL EXEAnalyzer::_CheckForNewEXE ()
//--------------------------------------------------------------------
{
  // check whether it is an extended EXE
  m_nNewExeHeaderOffset = m_pBuffer->GetInt32 (0x3C);

  // check whether the offset is valid
  // problems with 256 byte files!!
  if (m_nNewExeHeaderOffset >= 0 &&
      file_t (m_nNewExeHeaderOffset) < m_pBuffer->GetFileSize ())
  {
    // read 2 bytes at the specified NewEXEHeaderOffset
    const gtuint16 nNewEXEHeader = m_pBuffer->GetUint16 (m_nNewExeHeaderOffset);

    switch (nNewEXEHeader)
    {
      case 0x4550:  // PE
      {
        m_eEXEType = EXE_PE;
        break;
      }
      case 0x454E:  // NE
      {
        m_eEXEType = EXE_NE;
        break;
      }
      case 0x584C:  // LX
      case 0x454C:  // LE
      {
        m_eEXEType = EXE_LE;
        break;
      }
    }
  }
}

//--------------------------------------------------------------------
bool GT_CALL EXEAnalyzer::_RecalculateHeader
                                        (const gtuint32 nHeaderSize,
                                         const bool     bTooSmall,
                                               Table*   pTable)
//--------------------------------------------------------------------
{
  // size of header is too small/large...
  const resid_t nTxtID = bTooSmall ? HRC_EXELIB_DOS_HEADER_TOO_SMALL : HRC_EXELIB_DOS_HEADER_TOO_LARGE;

  TableCell* pCell = pTable->AddStr (0, str_get_formatted (rc (nTxtID), nHeaderSize));
  pCell->SetColSpan (2);
  pCell->SetColor (eColorERROR);

  this->CalculateHeaderSize ();

  const file_t nNewHeaderSize = this->GetHeaderSize ();

  // calculated header size ...
  pTable->AddStr (0, str_get_formatted (rc (HRC_EXELIB_DOS_CALCED_HEADER_SIZE), nNewHeaderSize, nNewHeaderSize))->SetColSpan (2);

  if (nNewHeaderSize > m_pBuffer->GetFileSize ())
  {
    // header is now too large...
    pCell = pTable->AddStr (0, HRC_EXELIB_DOS_HEADER_NOW_TOO_LARGE);
    pCell->SetColSpan (2);
    pCell->SetColor (eColorERROR);
    return false;
  }

  return true;
}

//--------------------------------------------------------------------
bool GT_CALL EXEAnalyzer::_Check_C2E ()
//--------------------------------------------------------------------
{
  // compare the DOS header!!
  SignatureCmp aCompare ((gtuint8*) &m_aDOSHeader.nHeaderSize,
                         GT_COMPARE_MAGIC_C2E,
                         0,  // no difference allowed
                         m_pBuffer,
                         NULL);

  for (int i = 0; i < GT_C2E_Count (); i++)
  {
    const C2EEntry *pEntry = GT_C2E_Index (i);
    ASSERT (pEntry);

    // compare for 24 bytes
    if (aCompare.ExplicitMatches (pEntry->aData, 24) == GTCOMP_MATCH)
    {
      out_format (rc (HRC_EXELIB_DOS_FOUND_COM2EXE), pEntry->sName);
      return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------
bool GT_CALL EXEAnalyzer::_Check_EXE_0J
                                        (const file_t nStartPos)
//--------------------------------------------------------------------
{
  // create buffer, fill it with 0 and then read from the file
  gtuint8 aBuffer[EXE_BUFFER_SIZE];
  memset (aBuffer, 0, EXE_BUFFER_SIZE);
  m_pBuffer->GetBuffer (nStartPos, aBuffer, DETERMINE_MAX_BUFFERSIZE (nStartPos));

  SignatureCmpProc pFunc;
  const size_t nTolerance = CmdlineParam_GetLong (GT_FLAG_FTE);

  // loop over all EXE codes with no jumps
  for (int i = 0; i < GT_EXE_0J_Count (); i++)
  {
    switch (i)
    {
      // PKLite specific
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:  pFunc = _PKLite; break;
      default: pFunc = NULL; break;
    }

    SignatureCmp aCompare (aBuffer,
                           GT_COMPARE_MAGIC_EXE,
                           nTolerance,
                           m_pBuffer,
                           pFunc);

    if (aCompare.Matches (GT_EXE_0J_Index (i)) == GTCOMP_MATCH)
      return true;
  }

  return false;
}

//--------------------------------------------------------------------
bool GT_CALL EXEAnalyzer::_Check_EXE_1J
                                        (const file_t nStartPos)
//--------------------------------------------------------------------
{
  // create buffer, fill it with 0 and then read from the file
  gtuint8 aBuffer[EXE_BUFFER_SIZE];
  memset (aBuffer, 0, EXE_BUFFER_SIZE);
  m_pBuffer->GetBuffer (nStartPos, aBuffer, DETERMINE_MAX_BUFFERSIZE (nStartPos));

  SignatureCmp aCompare (aBuffer,
                         GT_COMPARE_MAGIC_EXE,
                         CmdlineParam_GetLong (GT_FLAG_FTE),
                         m_pBuffer,
                         NULL);

  for (int i = 0; i < GT_EXE_1J_Count (); i++)
    if (aCompare.Matches (GT_EXE_1J_Index (i)) == GTCOMP_MATCH)
      return true;

  return false;
}

//--------------------------------------------------------------------
bool GT_CALL EXEAnalyzer::_Check_EXE_2J
                                        (const file_t nStartPos)
//--------------------------------------------------------------------
{
  // create buffer, fill it with 0 and then read from the file
  gtuint8 aBuffer[EXE_BUFFER_SIZE];
  memset (aBuffer, 0, EXE_BUFFER_SIZE);
  m_pBuffer->GetBuffer (nStartPos, aBuffer, DETERMINE_MAX_BUFFERSIZE (nStartPos));

  SignatureCmp aCompare (aBuffer,
                         GT_COMPARE_MAGIC_EXE,
                         CmdlineParam_GetLong (GT_FLAG_FTE),
                         m_pBuffer,
                         NULL);

  for (int i = 0; i < GT_EXE_2J_Count (); i++)
    if (aCompare.Matches (GT_EXE_2J_Index (i)) == GTCOMP_MATCH)
      return true;

  return false;
}

//--------------------------------------------------------------------
bool GT_CALL EXEAnalyzer::_Check_EXE_1LJ
                                        (const file_t nStartPos)
//--------------------------------------------------------------------
{
  // create buffer, fill it with 0 and then read from the file
  gtuint8 aBuffer[EXE_BUFFER_SIZE];
  memset (aBuffer, 0, EXE_BUFFER_SIZE);
  m_pBuffer->GetBuffer (nStartPos, aBuffer, DETERMINE_MAX_BUFFERSIZE (nStartPos));

  SignatureCmp aCompare (aBuffer,
                         GT_COMPARE_MAGIC_EXE,
                         CmdlineParam_GetLong (GT_FLAG_FTE),
                         m_pBuffer,
                         NULL);

  for (int i = 0; i < GT_EXE_1LJ_Count (); i++)
    if (aCompare.Matches (GT_EXE_1LJ_Index (i)) == GTCOMP_MATCH)
      return true;

  return false;
}

//--------------------------------------------------------------------
bool GT_CALL EXEAnalyzer::_Check_EXT_EXE
                                        (const file_t nStartPos,
                                         const bool   bIsCOM)
//--------------------------------------------------------------------
{
  ASSERT (nStartPos >= 0);

  gtuint8 aBuffer[EXE_BUFFER_SIZE];
  EXE_MacroManager aMacroMan (m_pBuffer, NULL, NULL);
  const size_t nTolerance = CmdlineParam_GetLong (GT_FLAG_FTE);

  // loop over all EXE codes with macros
  for (int i = 0; i < GT_EXE_Ext_Count (); i++)
  {
    const ExtEXEEntry *pEntry = GT_EXE_Ext_Index (i);
    ASSERT (pEntry);

    // check if macro can be resolved
    file_t nTarget = nStartPos;
    if (!aMacroMan.GetMacroPos (nTarget, pEntry->sMacro, bIsCOM))
      continue;

    // happens on a special COM files...
    if (nTarget < 0)
      continue;

    // fill it with crap!
    memset (aBuffer, 0, EXE_BUFFER_SIZE);

    // yes it could be resolved - ntarget contains the target position
    m_pBuffer->GetBuffer (nTarget, aBuffer, DETERMINE_MAX_BUFFERSIZE (nTarget));

    // initialize the compare buffer
    SignatureCmpProc pFunc = NULL;
    switch (i)
    {
      // WWPack specific
      case 0: pFunc = _WWPack300PR; break;
      case 1: pFunc = _WWPack30xPR; break;
      case 2: pFunc = _WWPack30xP; break;
    }

    SignatureCmp aCompare (aBuffer,
                           GT_COMPARE_MAGIC_EXE,
                           nTolerance,
                           m_pBuffer,
                           pFunc);

    if (aCompare.Matches ((EXEEntry*) pEntry) == GTCOMP_MATCH)
      return true;
  }

  return false;
}

//--------------------------------------------------------------------
void GT_CALL EXEAnalyzer::_Check_EXE_Always
                                        (const file_t /* nRealIP */,
                                         const bool   bIsCOM)
//--------------------------------------------------------------------
{
  if (!bIsCOM)
  {
    // generic TLink linker detection
    if (m_pBuffer->GetUint8 (30) == 0xFB)
    {
      const gtuint8 nTLinkVersion = m_pBuffer->GetUint8 (31);

      // found linker 'Tlink'
      out_info_format (rc (HRC_EXELIB_DOS_FOUND_TLINK),
                       nTLinkVersion >> 4, nTLinkVersion & 0x0F);
    }
  }
}

//--------------------------------------------------------------------
bool GT_CALL EXEAnalyzer::_Check_EXE_Generic
                                        (const file_t nRealIP)
//--------------------------------------------------------------------
{
  if (__CheckForPKLite (m_pBuffer))
    return true;

  if (__CheckForEXEPACK (m_pBuffer))
    return true;

  if (__CheckForSEAAXE (m_pBuffer))
    return true;

  if (__CheckForTrap12x (m_pBuffer, nRealIP))
    return true;

  return false;
}

//--------------------------------------------------------------------
bool GT_CALL EXEAnalyzer::_CheckStartupCode
                                        (const bool bIsCOM)
//--------------------------------------------------------------------
{
  EXE_MacroManager aMacroMan (m_pBuffer, NULL, NULL);
  file_t nIP, nTarget;

  // check for COM 2 EXE converter
  if (bIsCOM)
  {
    nIP = 0;
  }
  else
  {
    // is it a converted COM file?
    if (_Check_C2E ())
    {
      _ListCOM ();
      return true;
    }

    nIP = GetAbsoluteIP ();
  }

  // do check for e.g. TLink
  _Check_EXE_Always (nIP, bIsCOM);

  // check for 0 jump code
  if (_Check_EXE_0J (nIP))
    return true;

  // check for 1 long call code - e.g. Turbo Pascal
  nTarget = nIP;  //must be initialized!!
  if (aMacroMan.GetLongCallPos (nTarget, GetHeaderSize ()))
  {
    if (_Check_EXE_1LJ (nTarget))
      return true;
  }

  // chek for 1 jump code

  nTarget = nIP;  // must be re-initialized!!
  if (aMacroMan.GetJumpPos (nTarget, 1))
  {
    if (_Check_EXE_1J (nTarget))
      return true;

    // check for 2 jump code - nTarget is at the target of jump 1...

    if (aMacroMan.GetJumpPos (nTarget, 1))
    {
      if (_Check_EXE_2J (nTarget))
        return true;
    }
  }

  if (_Check_EXT_EXE (nIP, bIsCOM))
    return true;

  if (!bIsCOM)
  {
    // check only for EXEs
    if (_Check_EXE_Generic (nIP))
      return true;
  }

  // the rest

  return false;
}

//--------------------------------------------------------------------
void GT_CALL EXEAnalyzer::_ListNEEXEs ()
//--------------------------------------------------------------------
{
  // set DOS EXE overlay (may be overridden by LE/NE/PE lister)
  m_nOverlayOffset = GetSizeInHeader ();

  // check whether the offset is valid
  // problems with 256 byte files!!
  if (m_nNewExeHeaderOffset >= 0 &&
      file_t (m_nNewExeHeaderOffset) < m_pBuffer->GetFileSize ())
  {
    switch (m_eEXEType)
    {
      case EXE_PE:
      {
        EXE_PE_Lister aLister (m_pBuffer, m_nNewExeHeaderOffset);
        aLister.Execute ();

        // set overlay
        m_nOverlayOffset = aLister.GetOverlayOffset ();
        break;
      }
      case EXE_NE:
      {
        EXE_NE_Lister aLister (m_pBuffer, m_nNewExeHeaderOffset);
        aLister.Execute ();

        // set overlay
        m_nOverlayOffset = aLister.GetOverlayOffset ();
        break;
      }
      case EXE_LE:
      {
        EXE_LE_Lister aLister (m_pBuffer, m_nNewExeHeaderOffset);
        aLister.Execute ();

        // set overlay
        m_nOverlayOffset = aLister.GetOverlayOffset ();
        break;
      }
    }
  }
}

//--------------------------------------------------------------------
void GT_CALL EXEAnalyzer::_ListCOM ()
//--------------------------------------------------------------------
{
  if (CmdlineParam_GetBool (GT_FLAG_EXEMOD))
  {
    // show only modifier
    _CheckStartupCode (true);
  }
  else
  {
    // DOS COM executable
    out_info_append (rc (HRC_EXELIB_DOS_FOUND_COM));

    // if we're in listmode only the first line is of interest ;-)
    if (CmdlineParam_GetBool (GT_FLAG_LISTMODE))
      return;

    _CheckStartupCode (true);
  }
}

//--------------------------------------------------------------------
void GT_CALL EXEAnalyzer::_ListEXE ()
//--------------------------------------------------------------------
{
  ASSERT (IsValidEXE ());

  // show only modifier?
  if (CmdlineParam_GetBool (GT_FLAG_EXEMOD))
  {
    // false: EXE, not COM
    _CheckStartupCode (false);
    _ListNEEXEs ();
  }
  else
  {
    // default output
    const gtuint32 nHeaderSize = GetHeaderSize ();
    const file_t   nSizeInHeader = GetSizeInHeader ();
    const gtuint32 nAbsIP = GetAbsoluteIP ();
    const file_t   nOverlaySize = GetOverlaySize ();
    const gtuint16 nRelocEntries = m_aDOSHeader.nRelocEntries;
    const gtuint16 nRelocOffset = m_aDOSHeader.nRelocOffset;

    if (m_nStartOffset > 0)
    {
      // is a DOS executable at offset blablabla
      resid_t nStr = m_eEXEType == EXE_PE ? HRC_EXELIB_DOS_FOUND_EXE_PE_OFFSET :
                     m_eEXEType == EXE_NE ? HRC_EXELIB_DOS_FOUND_EXE_NE_OFFSET :
                     m_eEXEType == EXE_LE ? HRC_EXELIB_DOS_FOUND_EXE_LE_OFFSET : HRC_EXELIB_DOS_FOUND_EXE_DOS_OFFSET;
      out_info_format (rc (nStr), m_nStartOffset, m_nStartOffset);
    }
    else
    {
      // is a DOS executable
      resid_t nStr = m_eEXEType == EXE_PE ? HRC_EXELIB_DOS_FOUND_EXE_PE :
                     m_eEXEType == EXE_NE ? HRC_EXELIB_DOS_FOUND_EXE_NE :
                     m_eEXEType == EXE_LE ? HRC_EXELIB_DOS_FOUND_EXE_LE : HRC_EXELIB_DOS_FOUND_EXE_DOS;
      out_info_append (rc (nStr));
    }

    // if we're in listmode only the first line is of interest ;-)
    if (CmdlineParam_GetBool (GT_FLAG_LISTMODE))
      return;

    out_incindent ();
    {
      Table aTable;
      aTable.EnableHeadline (false);
      aTable.AddColumn (_T ("_key_"), STR, TABLE_AUTO_WIDTH, ALIGN_LEFT);
      aTable.AddColumn (_T ("_val_"), STR, TABLE_AUTO_WIDTH, ALIGN_LEFT);

      const size_t KEY_INDEX = 0;
      const size_t VAL_INDEX = 1;

      if (IsEuropeanEXE ())
      {
        // found reverse 'ZM' header
        aTable.AddStr (KEY_INDEX, HRC_EXELIB_DOS_REVERSE_HEADER)->SetColSpan (2);
      }

      // size of header
      aTable.AddStr (KEY_INDEX, HRC_EXELIB_DOS_LIST_HEADERSIZE);
      aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_32, nHeaderSize, nHeaderSize));

      bool bCheckForStartupCode = false;

      // if the header is smaller than possible, recalc it (true: too small)
      if (nHeaderSize < 32 &&
          !_RecalculateHeader (nHeaderSize, true, &aTable))
      {
        // strange DOS EXE file - look only for NE EXEs
      }
      else
      // if the header is larger than possible, recalc it (false: too large)
      if (file_t (nHeaderSize) > m_pBuffer->GetFileSize () &&
          !_RecalculateHeader (nHeaderSize, false, &aTable))
      {
        // strange DOS EXE file - look only for NE EXEs
      }
      else
      {
        // headersize seems to be valid
        bCheckForStartupCode = true;

        // filesize in header (int64)
        aTable.AddStr (KEY_INDEX, HRC_EXELIB_DOS_LIST_FILESIZE);
        aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_64, nSizeInHeader, nSizeInHeader));

        if (nSizeInHeader < 32)
        {
          // file size in header is invalid
          //#f#out_incindent ();
          TableCell* pCell = aTable.AddStr (KEY_INDEX, HRC_EXELIB_DOS_LIST_FILESIZE_INVALID);
          pCell->SetColSpan (2);
          pCell->SetColor (eColorERROR);
          //#f#out_decindent ();
        }

        // entrypoint (uint32)
        aTable.AddStr (KEY_INDEX, HRC_EXELIB_DOS_LIST_ENTRYPOINT);
        aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_32, nAbsIP, nAbsIP));

        // if the EP is inside the header it is "dangerous" ;-)
        if (nAbsIP < _max (nHeaderSize, gtuint32 (32)))
        {
          //#f#out_incindent ();
          TableCell* pCell = aTable.AddStr (KEY_INDEX, HRC_EXELIB_DOS_LIST_ENTRYPOINT_INVALID);
          pCell->SetColSpan (2);
          pCell->SetColor (eColorERROR);
          //#f#out_decindent ();
        }

        if (nOverlaySize > 0)
        {
          // overlay size (int64)
          aTable.AddStr (KEY_INDEX, HRC_EXELIB_DOS_LIST_OVERLAY);
          aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_64, nOverlaySize, nOverlaySize));
        }

        if (nRelocEntries > 0)
        {
          // relocation entries:
          aTable.AddStr (KEY_INDEX, HRC_EXELIB_DOS_LIST_RELOC_COUNT);
          aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_32, nRelocEntries, nRelocEntries));

          // relocation offset:
          aTable.AddStr (KEY_INDEX, HRC_EXELIB_DOS_LIST_RELOC_OFFSET);
          aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_32, nRelocOffset, nRelocOffset));
        }
        else
        {
          // no relocation entries:
          TableCell* pCell = aTable.AddStr (KEY_INDEX, HRC_EXELIB_DOS_LIST_NO_RELOCS);
          pCell->SetColSpan (2);
          pCell->SetColor (eColorINTERESTING);
        }
      }

      out_table (&aTable);

      if (bCheckForStartupCode)
        _CheckStartupCode (false);

      _ListNEEXEs ();
    }
    out_decindent ();
  }
}

//--------------------------------------------------------------------
void GT_CALL EXEAnalyzer::_ShowResults ()
//--------------------------------------------------------------------
{
  if (IsValidEXE ())  // check for EXE not for COM
  {
    _CheckForNewEXE ();
    _ListEXE ();
  }
  else
    _ListCOM ();
}

}  // namespace
