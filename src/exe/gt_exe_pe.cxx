#include "gt_exe_pe.hxx"
#include "gt_exe_macro.hxx"
#include "gt_exe_defines.hxx"

#include "gtc_data.hxx"
#include "gt_cmdline_params.hxx"
#include "gt_compare.hxx"
#include "gt_errormsg.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_table.hxx"
#include "gt_utils.hxx"

namespace GT {

//--------------------------------------------------------------------
EXE_PE_BasicLister::EXE_PE_BasicLister
                                        (FileBuffer*                  pBuffer,
                                         EXE_PE_SectionTableAnalyzer* pSectionTable,
                                         EXE_PE_ImageDataDirectory*   pDataDir)
//--------------------------------------------------------------------
  : BasicBuffer     (pBuffer),
    m_pSectionTable (pSectionTable),
    m_pDataDir      (pDataDir)
{
  ASSERT (pSectionTable);

  // convert RVA to physical offset
  m_nOffset = m_pSectionTable->r2o (pDataDir->nRVA);
  ASSERT (m_nOffset >= 0);
}

////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------
EXE_PE_Lister::EXE_PE_Lister
                                        (      FileBuffer* pBuffer,
                                         const gtuint32    nNEOffset)
//--------------------------------------------------------------------
  : BasicBuffer      (pBuffer),
    m_nNEOffset      (nNEOffset),
    m_pST            (NULL),
    m_nOverlayOffset (0)
{
  ASSERT (nNEOffset >= 0);
}

//--------------------------------------------------------------------
EXE_PE_Lister::~EXE_PE_Lister ()
//--------------------------------------------------------------------
{
  delete m_pST;
}

//--------------------------------------------------------------------
void GT_CALL EXE_PE_Lister::_PrintSections ()
//--------------------------------------------------------------------
{
  // if commandline switch /eps was used
  if (CmdlineParam_GetBool (GT_FLAG_PESECT))
  {
    out_append (_T ("\n"));

    out_format (rc (HRC_EXELIB_PE_SEG_TABLE_LISTING),
                m_aOFH.nSectionAlignment);

    Table aTable;
    aTable.AddColumn (rc (HRC_EXELIB_PE_SEG_TABLE_NAME),      STR, 10, ALIGN_LEFT);
    aTable.AddColumn (rc (HRC_EXELIB_PE_SEG_TABLE_RVA),       HEX, 10, 8);
    aTable.AddColumn (rc (HRC_EXELIB_PE_SEG_TABLE_VIRT_SIZE), HEX, 10, 8);
    aTable.AddColumn (rc (HRC_EXELIB_PE_SEG_TABLE_PHYS_OFS),  HEX, 10, 8);
    aTable.AddColumn (rc (HRC_EXELIB_PE_SEG_TABLE_PHYS_SIZE), HEX, 10, 8);
    aTable.AddColumn (rc (HRC_EXELIB_PE_SEG_TABLE_PHYS_END),  HEX, 10, 8);
    aTable.AddColumn (rc (HRC_EXELIB_PE_SEG_TABLE_FLAGS),     HEX, 10, 8);

    for (size_t i = 0; i < m_aIFH.nNumberOfSections; i++)
    {
      EXE_PE_Section *pSection = m_pST->GetSectionOfPos (i);

      aTable.AddStr (0, m_pST->GetSectionName (i, true));
      aTable.AddInt (1, RVA_VAL (pSection->nRVA));
      aTable.AddInt (2, pSection->nVirtualSize);
      aTable.AddInt (3, pSection->nPhysicalOffset);
      aTable.AddInt (4, pSection->nPhysicalSize);
      aTable.AddInt (5, pSection->nPhysicalOffset + pSection->nPhysicalSize);
      aTable.AddInt (6, pSection->nFlags);
    }

    out_incindent ();
    out_table (&aTable);
    out_decindent ();

    // #f# it would be nice to print out the flags content according
    // to the constants defined in gtlib_exe_types.hxx
  }
}

//--------------------------------------------------------------------
void GT_CALL EXE_PE_Lister::_CheckDataDirectory ()
//--------------------------------------------------------------------
{
  ASSERT (m_pST);

  // Empty data directory?
  if (m_aOFH.nNumberOfRVAsAndSizes == 0)
    return;

  // #f# possibly dangerous - are the offsets always correct???
  EXE_PE_DataDirectoryHandler aDDH (m_pBuffer,
                                    m_pST,
                                    m_aOFH.nNumberOfRVAsAndSizes,
                                    m_nNEOffset + EXE_PE_IMAGEHEADER_SIZE + EXE_PE_OPTIONALHEADER_SIZE);

  // if commandline switch /pedd was used
  if (CmdlineParam_GetBool (GT_FLAG_PEDD))
  {
    out_append (_T ("\n"));

    const size_t nDDHE = aDDH.GetNumberOfUsedEntries ();
    if (nDDHE > 0)
    {
      out_format (rc (HRC_EXELIB_PE_DD_LISTING),
                  nDDHE,
                  m_aOFH.nNumberOfRVAsAndSizes);

      // cols/rows
      // the longest Name is 23 chars -> width = 25
      Table aTable;
      aTable.AddColumn (rc (HRC_EXELIB_PE_DD_NAME),      STR, 25, ALIGN_RIGHT);
      aTable.AddColumn (rc (HRC_EXELIB_PE_DD_PHYS_OFS),  HEX, 11, 8);
      aTable.AddColumn (rc (HRC_EXELIB_PE_DD_RVA),       HEX, 11, 8);
      aTable.AddColumn (rc (HRC_EXELIB_PE_DD_PHYS_SIZE), HEX, 11, 8);
      aTable.AddColumn (rc (HRC_EXELIB_PE_DD_SECTION),   STR, 10, ALIGN_LEFT);

      // List only up to the real directory count, in case the nNumberOfRVAsAndSizes is faked!
      for (gtuint32 i = 0; i < aDDH.GetRealDirectoryCount (); i++)
        aDDH.AddEntryToTable (i, &aTable);

      out_incindent ();
      out_table (&aTable);
      out_decindent ();
    }
    else
    {
      // no single data directory present
      out_info_append (rc (HRC_EXELIB_PE_NO_DD));
    }
  }

  EXE_PE_ImageDataDirectory* pExport      = aDDH.GetEntry (EXE_PE_DIRECTORY_EXPORT);
  EXE_PE_ImageDataDirectory* pImport      = aDDH.GetEntry (EXE_PE_DIRECTORY_IMPORT);
  EXE_PE_ImageDataDirectory* pResource    = aDDH.GetEntry (EXE_PE_DIRECTORY_RESOURCE);
  EXE_PE_ImageDataDirectory* pCertificate = aDDH.GetEntry (EXE_PE_DIRECTORY_SECURITY);
  EXE_PE_ImageDataDirectory* pFixups      = aDDH.GetEntry (EXE_PE_DIRECTORY_FIXUPS);
  EXE_PE_ImageDataDirectory* pTLS         = aDDH.GetEntry (EXE_PE_DIRECTORY_TLSORT);

  // list exports

  // if commandline switch /peexp was used
  if (CmdlineParam_GetBool (GT_FLAG_PEEXP) &&
      pExport &&
      pExport->nRVA != rva_t (0))
  {
    EXE_PE_ExportAnalyzer aAnalyzer (m_pBuffer, m_pST, pExport);
    aAnalyzer.List ();
  }

  // list imports

  // if commandline switch /peimp was used
  if (CmdlineParam_GetBool (GT_FLAG_PEIMP) &&
      pImport &&
      pImport->nRVA != rva_t (0))
  {
    EXE_PE_ImportAnalyzer aAnalyzer (m_pBuffer, m_pST, pImport);
    aAnalyzer.List ();
  }

  // list resources

  if (pResource && pResource->nRVA != rva_t (0))
  {
    EXE_PE_ResourceAnalyzer aAnalyzer (m_pBuffer, m_pST, pResource);
    aAnalyzer.List ();
  }

  // list certificates

  if (pCertificate && pCertificate->nRVA != rva_t (0))
  {
    EXE_PE_CertificateAnalyzer aAnalyzer (m_pBuffer, m_pST, pCertificate);
    aAnalyzer.List ();
  }

  // list fixups

  // if commandline switch /pefixup was used
  if (CmdlineParam_GetBool (GT_FLAG_PEFIXUP) &&
      pFixups &&
      pFixups->nRVA != rva_t (0))
  {
    EXE_PE_FixupsAnalyzer aAnalyzer (m_pBuffer, m_pST, pFixups);
    aAnalyzer.List ();
  }

  // list TLS

  if (pTLS && pTLS->nRVA != rva_t (0))
  {
    EXE_PE_TLSAnalyzer aAnalyzer (m_pBuffer, m_pST, pTLS);
    aAnalyzer.List ();
  }
}

//--------------------------------------------------------------------
bool GT_CALL EXE_PE_Lister::_AnalyzeAsDOTNET ()
//--------------------------------------------------------------------
{
  ASSERT (m_pST);

  //------------------------------------------------------------------
  struct DOTNET_StreamHeader
  //------------------------------------------------------------------
  {
    gtuint32 nOffset;
    gtuint32 nSize;
    pstring sName;
  };

  // read 4 bytes at the offset of the first section + 0x10
  // which is an RVA to the MetaData section
  EXE_PE_Section *pSection = m_pST->GetSectionOfPos (0);
  rva_t nMetaDataRVA (m_pBuffer->GetInt32 (pSection->nPhysicalOffset + 0x10));

  // yeah, we've got the MetaData offset
  const gtuint32 nMetaDataOffset = m_pST->r2o (nMetaDataRVA);

  // if offset is out of file bounds -> do nothing
  if (nMetaDataOffset < 0 || nMetaDataOffset >= m_pBuffer->GetFileSize ())
  {
    // no error message - used to differentiate between .NET EXEs and
    // old fashioned PE EXEs
    return false;
  }

  // read header information
  DOTNET_MetaDataHeader aHdr;
  m_pBuffer->GetBufferX (nMetaDataOffset, &aHdr, EXE_NET_METADATA_HDR);

  // not a DOT NET executable
  if (aHdr.nID != 0x424a5342)
    return false;

  out_append (_T ("\n"));

  out_info_format (rc (HRC_EXELIB_NET_AT_OFFSET),
                   m_aIFH.nFlags & 0x2000
                     ? _T ("DLL")
                     : _T ("EXE"),
                   aHdr.nMajorVer,
                   aHdr.nMinorVer,
                   m_nNEOffset,
                   m_nNEOffset);
  out_incindent ();

  out_format (rc (HRC_EXELIB_NET_METADATA_OFFSET),
              nMetaDataOffset);

  {
    const gtuint32 nVersionStringLen = m_pBuffer->GetInt32 ();

    pstring sVersion;
    m_pBuffer->GetFixedSizeAString (sVersion, nVersionStringLen);
    out_format (rc (HRC_EXELIB_NET_VERSION),
                sVersion.c_str ());
  }

  // align to 4 byte boundary
  m_pBuffer->SetActFilePos (ALIGN_4 (m_pBuffer->GetActFilePos ()));

  // skip flags
  m_pBuffer->IncActFilePos (2);

  // get stream header information
  {
    const gtuint16 nStreamCount = m_pBuffer->GetUint16 ();

    if (nStreamCount > 5)
    {
      out_error_format (rc (HRC_EXELIB_NET_TOO_MANY_STREAM_HEADER),
                        nStreamCount);
    }

    out_append (rc (HRC_EXELIB_NET_STREAM_HEADER));

    Table aTable;
    aTable.AddColumn (rc (HRC_EXELIB_NET_STREAM_POS),    NUM_SPACE, 4,  2);
    aTable.AddColumn (rc (HRC_EXELIB_NET_STREAM_OFFSET), HEX,       11, 8);
    aTable.AddColumn (rc (HRC_EXELIB_NET_STREAM_SIZE),   HEX,       11, 8);
    aTable.AddColumn (rc (HRC_EXELIB_NET_STREAM_NAME),   STR,       2,  ALIGN_LEFT);

    pstring sStreamName;
    for (gtuint16 n = 0; n < nStreamCount; ++n)
    {
      gtuint32 nOfs  = m_pBuffer->GetInt32 ();
      gtuint32 nSize = m_pBuffer->GetInt32 ();
      m_pBuffer->GetASCIIZ_AString (sStreamName, 16);

      // align to 4 byte boundary
      m_pBuffer->SetActFilePos (ALIGN_4 (m_pBuffer->GetActFilePos ()));

      // append description string
           if (sStreamName == _T ("#Strings")) str_append_from_res (sStreamName, HRC_EXELIB_NET_STREAM_STRING_HEAP);
      else if (sStreamName == _T ("#US"))      str_append_from_res (sStreamName, HRC_EXELIB_NET_STREAM_USER_STRING_HEAP);
      else if (sStreamName == _T ("#Blob"))    str_append_from_res (sStreamName, HRC_EXELIB_NET_STREAM_BLOB_HEAP);
      else if (sStreamName == _T ("#GUID"))    str_append_from_res (sStreamName, HRC_EXELIB_NET_STREAM_GUID_HEAP);
      else if (sStreamName == _T ("#~"))       str_append_from_res (sStreamName, HRC_EXELIB_NET_STREAM_SET_OF_TABLES);

      aTable.AddInt (0, gtuint32 (n + 1));
      aTable.AddInt (1, nOfs);
      aTable.AddInt (2, nSize);
      aTable.AddStr (3, sStreamName);
    }

    out_incindent ();
    out_table (&aTable);
    out_decindent ();
  }

  out_decindent ();

  return true;
}

/*! Some progs (e.g. WinZIP) have a special content in a
      section with special name. E.g. Winzip has a ZIP like
      structure in the data of a section called "_winzip_".
 */
//--------------------------------------------------------------------
void GT_CALL EXE_PE_Lister::_CheckForSpecialSections ()
//--------------------------------------------------------------------
{
  // for all sections
  for (int i = 0; i < m_aIFH.nNumberOfSections; i++)
  {
    EXE_PE_Section *pSection = m_pST->GetSectionOfPos (i);

    // special check for WinZIP sections ;-)
    if (strncmp (pSection->sName, "_winzip_", 8) == 0)
    {
      // handle as overlay, because of the correct
      // indentation handling
      m_nOverlayOffset = pSection->nPhysicalOffset;
/*
      // call listproc with a special offset
      // -> do not handle as Overlay!
      m_pBuffer->CallListProc (pSection->nPhysicalOffset);
*/
    }
  }
}

//--------------------------------------------------------------------
bool GT_CALL EXE_PE_Lister::_AnalyzeAsPE ()
//--------------------------------------------------------------------
{
  ASSERT (m_pST);
  ASSERT (m_nOverlayOffset == 0);

  const gtuint32 nEntryPoint = m_pST->r2o (m_aOFH.nAddressOfEntryPoint);
  const gtuint32 nEXESize = m_pST->GetEXESize ();
  m_nOverlayOffset = nEXESize;
  const file_t nOverlaySize = m_pBuffer->GetFileSize () - m_nOverlayOffset;
  pstring sTemp;

  if (CmdlineParam_GetBool (GT_FLAG_EXEMOD))
  {
    // check for modifier only
    _CheckForCode (nEntryPoint, true);
  }
  else
  {
    out_append (_T ("\n"));

    const bool bIsDLL = (m_aIFH.nFlags & 0x2000);

    // print header incl. offset
    out_info_format (rc (HRC_EXELIB_PE_AT_OFFSET),
                     bIsDLL
                       ? _T ("DLL")
                       : _T ("EXE"));
    out_info_format (OUTPUT_HEX_DEC_32, m_nNEOffset, m_nNEOffset);
    out_append (_T ("\n"));

    out_incindent ();
    {
      {
        Table aTable;
        aTable.EnableHeadline (false);
        aTable.AddColumn (_T ("_key_"), STR, TABLE_AUTO_WIDTH, ALIGN_LEFT);
        aTable.AddColumn (_T ("_val_"), STR, TABLE_AUTO_WIDTH, ALIGN_LEFT);

        const size_t KEY_INDEX = 0;
        const size_t VAL_INDEX = 1;

        // entrypoint
        aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_EP);
        aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_32, nEntryPoint, nEntryPoint));

        if (nEntryPoint == 0)
        {
          // no entrypoint present
          //#f#out_incindent ();
          TableCell* pCell = aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_RESOURCE_DLL);
          pCell->SetColSpan (2);
          pCell->SetColor (eColorINTERESTING);
          //#f#out_decindent ();
        }
        else
        {
          // print entrypoint RVA
          aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_EP_RVA);
          aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_32, RVA_VAL (m_aOFH.nAddressOfEntryPoint)));

          // print entrypoint section
          aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_EP_SECTION);
          aTable.AddStr (VAL_INDEX, m_pST->GetSectionNameOfRVA (m_aOFH.nAddressOfEntryPoint));
        }

        // print calculated EXE size
        aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_CALCED_EXE_SIZE);
        aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_32, nEXESize, nEXESize));

        // overlay present?
        if (nOverlaySize > 0)
        {
          aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_OVERLAY_SIZE);
          aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_64, nOverlaySize, nOverlaySize));
        }

        // print image base
        aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_IMAGE_BASE);
        aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_32, m_aOFH.nImageBase, m_aOFH.nImageBase));

        // typical DLL image base is 0x00400000
        // typical EXE image base is 0x10000000
        if ( ((m_aIFH.nFlags & 0x2000) && (m_aOFH.nImageBase != 0x10000000)) ||
            (!(m_aIFH.nFlags & 0x2000) && (m_aOFH.nImageBase != 0x00400000)))
        {
          //#f#out_incindent ();
          TableCell* pCell = aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_REBASED);
          pCell->SetColSpan (2);
          pCell->SetColor (eColorINTERESTING);
          //#f#out_decindent ();
        }

        {
          const resid_t nCPUResId =
            m_aIFH.nCPUType == 0x0000 ? HRC_EXELIB_PE_CPU_INDEPENDENT :
            m_aIFH.nCPUType == 0x014C ? HRC_EXELIB_PE_CPU_80386 :
            m_aIFH.nCPUType == 0x014D ? HRC_EXELIB_PE_CPU_80486 :
            m_aIFH.nCPUType == 0x014E ? HRC_EXELIB_PE_CPU_PENTIUM :
            m_aIFH.nCPUType == 0x0160 ? HRC_EXELIB_PE_CPU_MIPS_R2000_BIG :
            m_aIFH.nCPUType == 0x0162 ? HRC_EXELIB_PE_CPU_MIPS_R2000_LITTLE :
            m_aIFH.nCPUType == 0x0163 ? HRC_EXELIB_PE_CPU_MIPS_R6000 :
            m_aIFH.nCPUType == 0x0166 ? HRC_EXELIB_PE_CPU_MIPS_R4000 :
            m_aIFH.nCPUType == 0x0168 ? HRC_EXELIB_PE_CPU_MIPS_R10000 :
            m_aIFH.nCPUType == 0x0169 ? HRC_EXELIB_PE_CPU_MIPS_WCE :
            m_aIFH.nCPUType == 0x0184 ? HRC_EXELIB_PE_CPU_DEC_ALPHA_AXP :
            m_aIFH.nCPUType == 0x01A2 ? HRC_EXELIB_PE_CPU_HITACHI_SH3 :
            m_aIFH.nCPUType == 0x01A3 ? HRC_EXELIB_PE_CPU_HITACHI_SH3DSP :
            m_aIFH.nCPUType == 0x01A4 ? HRC_EXELIB_PE_CPU_HITACHI_SH3E :
            m_aIFH.nCPUType == 0x01A6 ? HRC_EXELIB_PE_CPU_HITACHI_SH4 :
            m_aIFH.nCPUType == 0x01A8 ? HRC_EXELIB_PE_CPU_HITACHI_SH5 :
            m_aIFH.nCPUType == 0x01C0 ? HRC_EXELIB_PE_CPU_ARM :
            m_aIFH.nCPUType == 0x01C2 ? HRC_EXELIB_PE_CPU_THUMB :
            m_aIFH.nCPUType == 0x01D3 ? HRC_EXELIB_PE_CPU_AM33 :
            m_aIFH.nCPUType == 0x01F0 ? HRC_EXELIB_PE_CPU_IBM_POWERPC :
            m_aIFH.nCPUType == 0x01F0 ? HRC_EXELIB_PE_CPU_IBM_POWERPCFP :
            m_aIFH.nCPUType == 0x0200 ? HRC_EXELIB_PE_CPU_INTEL_IA64 :
            m_aIFH.nCPUType == 0x0266 ? HRC_EXELIB_PE_CPU_MIPS16 :
            m_aIFH.nCPUType == 0x0268 ? HRC_EXELIB_PE_CPU_MOTOROLA :
            m_aIFH.nCPUType == 0x0284 ? HRC_EXELIB_PE_CPU_DEC_ALPHA_64 :
            m_aIFH.nCPUType == 0x0366 ? HRC_EXELIB_PE_CPU_MIPS_FPU :
            m_aIFH.nCPUType == 0x0466 ? HRC_EXELIB_PE_CPU_MIPS16_FPU :
            m_aIFH.nCPUType == 0x0520 ? HRC_EXELIB_PE_CPU_INFINEON :
            m_aIFH.nCPUType == 0x0CEF ? HRC_EXELIB_PE_CPU_CEF :
            m_aIFH.nCPUType == 0x0EBC ? HRC_EXELIB_PE_CPU_EFI :
            m_aIFH.nCPUType == 0x8664 ? HRC_EXELIB_PE_CPU_AMD64 :
            m_aIFH.nCPUType == 0x9041 ? HRC_EXELIB_PE_CPU_M32R :
            m_aIFH.nCPUType == 0xC0EE ? HRC_EXELIB_PE_CPU_CEE :
                                        HRC_UNKNOWN;

          // unknown type?
          if (nCPUResId == HRC_UNKNOWN)
            ShowErrorMessage (rc (HRC_EXELIB_PE_CPU_VERY_UNKNOWN), m_aIFH.nCPUType);

          // resolve and print
          aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_CPU_TYPE);
          aTable.AddStr (VAL_INDEX, nCPUResId);
        }

        {
          const resid_t nOSResId =
            m_aOFH.nOSMajor == 0 ? HRC_EXELIB_PE_OS_V0 :
            m_aOFH.nOSMajor == 1 ? HRC_EXELIB_PE_OS_ANY :
            m_aOFH.nOSMajor == 3 ? HRC_EXELIB_PE_OS_NT351 :
            m_aOFH.nOSMajor == 4 ? HRC_EXELIB_PE_OS_95_NT4 :
            m_aOFH.nOSMajor == 5 ? HRC_EXELIB_PE_OS_98_2K :
            m_aOFH.nOSMajor == 6 ? HRC_EXELIB_PE_OS_XP :
                                   HRC_UNKNOWN;

          // resolve and print
          str_assign_from_res (sTemp, nOSResId);
          aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_OS);
          aTable.AddStr (VAL_INDEX, str_get_formatted (_T ("%d.%02d - %s"), m_aOFH.nOSMajor, m_aOFH.nOSMinor, sTemp.c_str ()));
        }

        {
          // Check only the lower byte because in file
          // "gt2\_received_data\2004_05_14 Serge Semashko\00048588.BIN"
          // a sub system of 0xfc03
          const int nSubSystemToCheck = m_aOFH.nSubSystem & 0x00ff;

          // Source: http://msdn.microsoft.com/en-us/library/ms680339(VS.85).aspx
          const resid_t nSSResId =
            nSubSystemToCheck ==  0 ? HRC_EXELIB_PE_SS_UNKNOWN :
            nSubSystemToCheck ==  1 ? HRC_EXELIB_PE_SS_DEVICE_DRIVER :
            nSubSystemToCheck ==  2 ? HRC_EXELIB_PE_SS_WIN_GUI :
            nSubSystemToCheck ==  3 ? HRC_EXELIB_PE_SS_WIN_CUI :
            // no 4
            nSubSystemToCheck ==  5 ? HRC_EXELIB_PE_SS_OS2_CHAR :
            // no 6
            nSubSystemToCheck ==  7 ? HRC_EXELIB_PE_SS_POSIX :
            nSubSystemToCheck ==  8 ? HRC_EXELIB_PE_SS_NATIVE_9X :
            nSubSystemToCheck ==  9 ? HRC_EXELIB_PE_SS_WIN_CE_GUI :
            nSubSystemToCheck == 10 ? HRC_EXELIB_PE_SS_EFI_APP :
            nSubSystemToCheck == 11 ? HRC_EXELIB_PE_SS_EFI_DRIVER_BOOT :
            nSubSystemToCheck == 12 ? HRC_EXELIB_PE_SS_EFI_DRIVER_RUNTIME :
            nSubSystemToCheck == 13 ? HRC_EXELIB_PE_SS_EFI_ROM :
            nSubSystemToCheck == 14 ? HRC_EXELIB_PE_SS_XBOX :
            nSubSystemToCheck == 16 ? HRC_EXELIB_PE_SS_WINDOWS_BOOT_APPLICATION :
                                      HRC_UNKNOWN;

          // unknown type?
          if (nSSResId == HRC_UNKNOWN)
          {
            // No resource required for internal error messages
            ShowErrorMessage (_T ("The PE EXE contains an unknown subsystem %04Xh"), m_aOFH.nSubSystem);
          }

          if ((m_aOFH.nSubSystem & 0xff00) > 0)
          {
            TableCell* pCell = aTable.AddStr (KEY_INDEX, str_get_formatted (rc (HRC_EXELIB_PE_SS_STRANGE_HIGHBIT), m_aOFH.nSubSystem >> 8));
            pCell->SetColSpan (2);
            pCell->SetColor (eColorINTERESTING);
          }

          // resolve and print
          aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_SS);
          aTable.AddStr (VAL_INDEX, nSSResId);
        }

        aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_LINKER_VERSION);
        aTable.AddStr (VAL_INDEX, str_get_formatted (_T ("%d.%02d"), m_aOFH.nLinkerMajor, m_aOFH.nLinkerMinor));

        aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_STACK_RESERVE);
        aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_32, m_aOFH.nSizeOfStackReserve, m_aOFH.nSizeOfStackReserve));

        aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_STACK_COMMIT);
        aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_32, m_aOFH.nSizeOfStackCommit, m_aOFH.nSizeOfStackCommit));

        aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_HEAP_RESERVE);
        aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_32, m_aOFH.nSizeOfHeapReserve, m_aOFH.nSizeOfHeapReserve));

        aTable.AddStr (KEY_INDEX, HRC_EXELIB_PE_HEAP_COMMIT);
        aTable.AddStr (VAL_INDEX, str_get_formatted (OUTPUT_HEX_DEC_32, m_aOFH.nSizeOfHeapCommit, m_aOFH.nSizeOfHeapCommit));

        out_table (&aTable);
      }

      {
        Table aTable;
        aTable.EnableHeadline (false);
        aTable.AddColumn (_T ("_bla_"), STR, TABLE_AUTO_WIDTH, ALIGN_LEFT);

        if (m_aIFH.nFlags & 0x0001) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_RELOC_STRIPPED);
        if (m_aIFH.nFlags & 0x0002) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_EXECUTABLE);
        if (m_aIFH.nFlags & 0x0004) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_LINENUM_STRIPPED);
        if (m_aIFH.nFlags & 0x0008) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_LOCALSYM_STRIPPED);
        if (m_aIFH.nFlags & 0x0010) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_TRIM);
        if (m_aIFH.nFlags & 0x0020) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_CANHANDLE_2GB);
        // 0x0040 is reserved
        if (m_aIFH.nFlags & 0x0080) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_LITTLE_ENDIAN);  /*undoc*/
        if (m_aIFH.nFlags & 0x0100) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_32BIT_WORD);
        if (m_aIFH.nFlags & 0x0200) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_DBGINFO_STRIPPED);
        if (m_aIFH.nFlags & 0x0400) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_REMOVEABLE_COPY);
        if (m_aIFH.nFlags & 0x0800) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_NETWORK_COPY);
        if (m_aIFH.nFlags & 0x1000) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_SYSTEM_FILE);
        if (m_aIFH.nFlags & 0x2000) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_DLL);
        if (m_aIFH.nFlags & 0x4000) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_UP);
        if (m_aIFH.nFlags & 0x8000) aTable.AddStr (0, HRC_EXELIB_PE_FLAGS_BIG_ENDIAN);  /*undoc*/

        out_append (rc (HRC_EXELIB_PE_FLAGS));
        out_incindent ();
        out_table (&aTable);
        out_decindent ();
      }

      if (bIsDLL && m_aOFH.nDLLCharacteristics != 0)
      {
        Table aTable;
        aTable.EnableHeadline (false);
        aTable.AddColumn (_T ("_bla_"), STR, TABLE_AUTO_WIDTH, ALIGN_LEFT);

        if (m_aOFH.nDLLCharacteristics & 0x0001) aTable.AddStr (0, HRC_EXELIB_PE_DLL_CHAR_PROCESS_INIT);
        if (m_aOFH.nDLLCharacteristics & 0x0002) aTable.AddStr (0, HRC_EXELIB_PE_DLL_CHAR_PROCESS_TERM);
        if (m_aOFH.nDLLCharacteristics & 0x0004) aTable.AddStr (0, HRC_EXELIB_PE_DLL_CHAR_THREAD_INIT);
        if (m_aOFH.nDLLCharacteristics & 0x0008) aTable.AddStr (0, HRC_EXELIB_PE_DLL_CHAR_THREAD_TERM);
        if (m_aOFH.nDLLCharacteristics & 0x0200) aTable.AddStr (0, HRC_EXELIB_PE_DLL_CHAR_NO_ISOLATION);
        if (m_aOFH.nDLLCharacteristics & 0x0400) aTable.AddStr (0, HRC_EXELIB_PE_DLL_CHAR_NO_SEH);
        if (m_aOFH.nDLLCharacteristics & 0x0800) aTable.AddStr (0, HRC_EXELIB_PE_DLL_CHAR_NO_BIND);
        if (m_aOFH.nDLLCharacteristics & 0x2000) aTable.AddStr (0, HRC_EXELIB_PE_DLL_CHAR_WDM_DRIVER);
        if (m_aOFH.nDLLCharacteristics & 0x8000) aTable.AddStr (0, HRC_EXELIB_PE_DLL_CHAR_TERMINAL_SERVER_AWARE);

        out_append (rc (HRC_EXELIB_PE_DLL_CHAR));
        out_incindent ();
        out_table (&aTable);
        out_decindent ();
      }

      _PrintSections ();
      _CheckDataDirectory ();
      _CheckForCode (nEntryPoint, false);
      _CheckForSpecialSections ();
    }
    out_decindent ();
  }

  // handle overlays
  return true;
}

//--------------------------------------------------------------------
bool GT_CALL EXE_PE_Lister::_Check_PE_0J
                                        (const gtuint32 nStartPos)
//--------------------------------------------------------------------
{
  // fill the buffer
  gtuint8 aBuffer[EXE_BUFFER_SIZE];
  m_pBuffer->GetBuffer (nStartPos, aBuffer, EXE_BUFFER_SIZE);

  SignatureCmp aCompare (aBuffer,
                         GT_COMPARE_MAGIC_PE,
                         CmdlineParam_GetLong (GT_FLAG_FTP),
                         m_pBuffer,
                         NULL);

  // run it through the signature comparer
  for (int i = 0; i < GT_PE_0J_Count (); i++)
    if (aCompare.Matches (GT_PE_0J_Index (i)) == GTCOMP_MATCH)
      return true;

  // not found
  return false;
}

//--------------------------------------------------------------------
bool GT_CALL EXE_PE_Lister::_Check_EXT_PE
                                        (const gtuint32 nStartPos)
//--------------------------------------------------------------------
{
  // buffer for target operation
  gtuint8 aBuffer[EXE_BUFFER_SIZE];

  // init macro manager
  EXE_MacroManager aMacroMan (m_pBuffer, m_pST, &m_aOFH);

  // get tolerance
  const size_t nTolerance = CmdlineParam_GetLong (GT_FLAG_FTP);

  // loop over all EXT_PE modifier
  for (int i = 0; i < GT_PE_Ext_Count (); i++)
  {
    const ExtEXEEntry *pEntry = GT_PE_Ext_Index (i);
    ASSERT (pEntry);

    // check if macro can be resolved
    file_t nTarget = nStartPos;
    if (!aMacroMan.GetMacroPos (nTarget, pEntry->sMacro, false))  // false = no COM ;-)
      continue;

    // yes it could be resolved - nTarget contains the target position
    if (!m_pBuffer->GetBuffer (nTarget, aBuffer, EXE_BUFFER_SIZE))
    {
      // not enough bytes read
/*
      out_info_format (rc (HRC_EXELIB_PE_COMPARE_READ_ERROR),
                       nTarget,
                       pEntry->sMacro);
*/
      continue;
    }

    // compare signature
    SignatureCmp aCompare (aBuffer,
                           GT_COMPARE_MAGIC_PE,
                           nTolerance,
                           m_pBuffer,
                           NULL);

    // match?
    if (aCompare.Matches ((EXEEntry*) pEntry) == GTCOMP_MATCH)
      return true;
  }

  return false;
}

//--------------------------------------------------------------------
void GT_CALL EXE_PE_Lister::_CheckForCode
                                        (const gtuint32 nEntryPoint,
                                         const bool     bExeMod)
//--------------------------------------------------------------------
{
  ASSERT (m_pST);
  ASSERT (nEntryPoint >= 0);

  bool bFound = false;
  pstring sSeemsToBe;

  // check if entrypoint is within the file
  if (file_t (nEntryPoint) >= m_pBuffer->GetFileSize ())
  {
    out_error_append (rc (HRC_EXELIB_PE_EP_OUT_OF_BOUNDS));
    return;
  }

  // do not print this when in /exemod mode
  if (!bExeMod)
  {
    out_append (_T ("\n"));
    out_append (rc (HRC_EXELIB_PE_PROCESSED_WITH));
    out_incindent ();
  }

  // only check for code, if we have an entrypoint
  // else, we only do heuristics :|
  if (nEntryPoint > 0)
  {
    // scan for signatures (output is done inside)
    // [ph]: 2004/05/01 changed order so that extended signatures
    //       (with macros) are scanned first!
    if (_Check_EXT_PE (nEntryPoint) ||
        _Check_PE_0J (nEntryPoint))
    {
      bFound = true;
    }
  }

  if (!bFound)
  {
    const EXE_PE_Section *pFirstSection = m_pST->GetSectionOfPos (0);
    if (strcmp (pFirstSection->sName, ".text") == 0)
    {
      str_assign_from_res (sSeemsToBe, HRC_EXELIB_PE_MS_LINKER);

      // simply check the linker version numbers
      if (m_aOFH.nLinkerMajor == 2 && m_aOFH.nLinkerMinor == 56)
        str_assign_from_res (sSeemsToBe, HRC_EXELIB_PE_GCC);
      else
      if (m_aOFH.nLinkerMajor == 3 && m_aOFH.nLinkerMinor == 10)
        sSeemsToBe += _T ("3.1");
      else
      if (m_aOFH.nLinkerMajor == 4 && m_aOFH.nLinkerMinor == 20)
        sSeemsToBe += _T ("4.2");
      else
      if (m_aOFH.nLinkerMajor == 5 && m_aOFH.nLinkerMinor == 0)
        sSeemsToBe += _T ("5.0");
      else
      if (m_aOFH.nLinkerMajor == 5 && m_aOFH.nLinkerMinor == 10)
        sSeemsToBe += _T ("5.0 SP3");
      else
      if (m_aOFH.nLinkerMajor == 6 && m_aOFH.nLinkerMinor == 0)
        sSeemsToBe += _T ("6.0");
      else
      if (m_aOFH.nLinkerMajor == 7 && m_aOFH.nLinkerMinor == 0)
        sSeemsToBe += _T ("7.0 / .NET 2002");
      else
      if (m_aOFH.nLinkerMajor == 7 && m_aOFH.nLinkerMinor == 10)
        sSeemsToBe += _T ("7.1 / .NET 2003");
      else
      if (m_aOFH.nLinkerMajor == 8 && m_aOFH.nLinkerMinor == 0)
        sSeemsToBe += _T ("8.0 / 2005");
      else
      if (m_aOFH.nLinkerMajor == 9 && m_aOFH.nLinkerMinor == 0)
        sSeemsToBe += _T ("9.0 / 2008");
      else
      if (m_aOFH.nLinkerMajor == 10 && m_aOFH.nLinkerMinor == 0)
        sSeemsToBe += _T ("10.0 / 2010");
      else
      if (m_aOFH.nLinkerMajor == 11 && m_aOFH.nLinkerMinor == 0)
        sSeemsToBe += _T ("11.0 / 2012");
      else
      {
        // unknown
        str_append_format (sSeemsToBe, _T ("%u.%u (\?\?)"),
                           m_aOFH.nLinkerMajor,
                           m_aOFH.nLinkerMinor);
      }
    }
    else
    if (strncmp (pFirstSection->sName, "CODE", 4) == 0)
    {
      if (m_aOFH.nLinkerMajor == 2 && m_aOFH.nLinkerMinor == 25)
      {
        str_assign_from_res (sSeemsToBe, HRC_EXELIB_PE_BORLAND_LINKER);
      }
    }

    {
      // special checks go here

      // 1. check for UPX string at 0x3E0 (new version)
      char s[5];
      if ((m_pBuffer->GetBuffer (0x3e0, &s[0], 3) && strncmp (s, "UPX", 3) == 0))
      {
        if (m_pBuffer->GetBuffer (0x3db, &s[0], 4))
        {
          s[4] = 0;
          str_assign_from_res (sSeemsToBe, HRC_EXELIB_PE_NEW_UPX);
          str_append_format (sSeemsToBe, _T ("%hs"), s);
        }
      }

      // 2. check for UPX string at 0x3C7 (old version)
      if ((m_pBuffer->GetBuffer (0x3c7, &s[0], 3) && strncmp (s, "UPX", 3) == 0))
      {
        if (m_pBuffer->GetBuffer (0x3cb, &s[0], 4))
        {
          s[4] = 0;
          str_assign_from_res (sSeemsToBe, HRC_EXELIB_PE_OLD_UPX);
          str_append_format (sSeemsToBe, _T ("%hs"), s);
        }
      }
    }

    // if not detected, use "not processed by any known app" message
    if (sSeemsToBe.empty ())
      str_assign_from_res (sSeemsToBe, HRC_EXELIB_PE_NOT_PROCESSED);

    out_format (_T ("%s\n"), sSeemsToBe.c_str ());
  }

  if (!bExeMod)
    out_decindent ();
}

/*! Entrypoint for PE EXE lister
 */
//--------------------------------------------------------------------
void GT_CALL EXE_PE_Lister::Execute ()
//--------------------------------------------------------------------
{
  ASSERT (!m_pST);

  // read both header!
  m_pBuffer->SetActFilePos (m_nNEOffset);
  m_pBuffer->GetBuffer (&m_aIFH, EXE_PE_IMAGEHEADER_SIZE);

  // internal error message
  if (m_aIFH.nOptionalHeaderSize < EXE_PE_OPTIONALHEADER_SIZE)
    out_error_format (_T ("m_aIFH.nOptionalHeaderSize (%lu) < EXE_PE_OPTIONALHEADER_SIZE (%lu)"),
                      m_aIFH.nOptionalHeaderSize,
                      EXE_PE_OPTIONALHEADER_SIZE);

  m_pBuffer->GetBuffer (&m_aOFH, EXE_PE_OPTIONALHEADER_SIZE);

  // the magic must always be 0x010B
  // Win64 executables seem to have 0x020B (e.g. copyx64.exe from Avast Antivir 4.8)
  if (m_aOFH.nMagic != 0x010B)
    if (m_aOFH.nMagic == 0x020B)
      out_error_append (rc (HRC_EXELIB_PE_INVALID_MAGIC_64));
    else
      out_error_format (rc (HRC_EXELIB_PE_INVALID_MAGIC), m_aOFH.nMagic);

  // #f# if it is Big endian? What to do???
  //     The header always seems to be in Little endian!

  // init section table handler - needs pointer to IFH!
  m_pST = new EXE_PE_SectionTableAnalyzer (m_pBuffer,
                                           &m_aIFH,
                                           m_nNEOffset,
                                           true);

  if (_AnalyzeAsDOTNET ())
    return;

  if (_AnalyzeAsPE ())
    return;
}

}  // namespace
