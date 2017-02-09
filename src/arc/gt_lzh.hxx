//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (LZH)
//--------------------------------------------------------------------
{
  pstring sDirName, sFilename, sFullPath;
  LZH_Header aH;
  file_t nReadOffset;
  bool bCanBeAwardFile;
  pstring sAwardSegmentName;

  // check if a date is present
  if (m_pBuffer->GetUint8 (m_nLocalOffset + 20) == 1)
  {
    // m_bHasDateTime = false;
  }

  // start at the beginning
  m_pBuffer->SetActFilePos (m_nLocalOffset + 0);

  for (;;)
  {
    // remember where we are
    nReadOffset = m_pBuffer->GetActFilePos ();

    // read the current header
    m_pBuffer->GetBufferX (&aH, GT_LZH_H_SIZE);

    // if the size is 0 -> Pech gehabt
    if (aH.nHeaderSize == 0)
    {
      if (m_pBuffer->GetActFilePos () < m_pBuffer->GetFileSize ())
        m_pBuffer->IncActFilePos (-int (GT_LZH_H_SIZE));

      break;
    }

    // thanx to Veit Kannegieser for this code!!!
    bCanBeAwardFile = (aH.aAward.n00 == 0x00) &&
                      (aH.aAward.n00_or_80 == 0x00 || aH.aAward.n00_or_80 == 0x80) &&
                      (aH.aAward.n20 == 0x20) &&
                      (aH.aAward.n01 == 0x01) &&
                      (aH.aAward.nTargetSegment < 0xA000) &&
                      (aH.nOriginalSize < 512 * 1024);

    if (bCanBeAwardFile)
    {
#define GT_LZH_SET_AWARD(__n,__id) case __n: str_assign_from_res (sAwardSegmentName, __id); break;
      switch (aH.aAward.nTargetSegment)
      {
        GT_LZH_SET_AWARD (0x0800, HRC_ARCLIB_LZH_AWARD_NCR_ROM)
        GT_LZH_SET_AWARD (0x1000, HRC_ARCLIB_LZH_AWARD_BUSLOGIC_RAM)
        GT_LZH_SET_AWARD (0x4000, HRC_ARCLIB_LZH_AWARD_LOGO_BITMAP)
        GT_LZH_SET_AWARD (0x4001, HRC_ARCLIB_LZH_AWARD_CPU_MICRO_CODE)
        GT_LZH_SET_AWARD (0x4002, HRC_ARCLIB_LZH_AWARD_EPA_PATTERN)
        GT_LZH_SET_AWARD (0x4003, HRC_ARCLIB_LZH_AWARD_ACPI_TABLE)
        GT_LZH_SET_AWARD (0x4004, HRC_ARCLIB_LZH_AWARD_VSA_DRIVER)
        GT_LZH_SET_AWARD (0x4100, HRC_ARCLIB_LZH_AWARD_EXT_SYSTEM_BIOS)
        GT_LZH_SET_AWARD (0x5000, HRC_ARCLIB_LZH_AWARD_SYSTEM_BIOS)
        GT_LZH_SET_AWARD (0x7000, HRC_ARCLIB_LZH_AWARD_VGA_ROM)
        default:
          bCanBeAwardFile = false;
          break;
      }
    }

    // different versions require different handling

    if (aH.aNorm.nHeaderVersion == 0)
    {
      // read the filename
      m_pBuffer->GetPascalString (sFilename);

      if (debug)
        sFilename += _T (" [dbg:HeaderVersion0]");

      _SetArchiveFlag (ARC_DT_DOS);

      ArchiveFile* pArcFile = new ArchiveFile (sFilename);
      pArcFile->SetPackedSize (aH.nPackedSize);
      pArcFile->SetUnpackedSize (aH.nOriginalSize);
      pArcFile->SetDT (aH.aNorm.nDateTime);
      _AddFile (pArcFile);

      // CRC 16 + data
      m_pBuffer->IncActFilePos (2 + aH.nPackedSize);
    }
    else
    if (aH.aNorm.nHeaderVersion == 1)
    {
      // read the current file
      m_pBuffer->GetPascalString (sFilename);

      if (bCanBeAwardFile)
      {
        sFilename += _T (" (");
        sFilename += sAwardSegmentName;
        str_append_format (sFilename, _T (" %04Xh:%04Xh)"), aH.aAward.nTargetSegment, 0);
      }

      if (debug)
        sFilename += _T (" [dbg:HeaderVersion1]");

      _SetArchiveFlag (ARC_DT_DOS);

      ArchiveFile* pArcFile = new ArchiveFile (sFilename);
      pArcFile->SetPackedSize (aH.nPackedSize);
      pArcFile->SetUnpackedSize (aH.nOriginalSize);
      pArcFile->SetDT (aH.aNorm.nDateTime);
      _AddFile (pArcFile);

      // CRC 16 + OS ID + data + next header size
      m_pBuffer->IncActFilePos (2 + 1 + aH.nPackedSize + 2);
    }
    else
    if (aH.aNorm.nHeaderVersion == 2)
    {
      // ... difficult ...

      // skip 3 bytes
      m_pBuffer->IncActFilePos (3);

      // read next header size
      gtuint16 nNextHeaderSize = m_pBuffer->GetUint16 ();

      gtuint8 nExtType;
      do
      {
        nExtType = m_pBuffer->GetUint8 ();
        switch (nExtType)
        {
          case 0:
          {
            // common
            m_pBuffer->IncActFilePos (2);
            break;
          }
          case 1:
          {
            // the -3 is for the nExtType and the nNextHeaderSize
            const int nFileNameLen = nNextHeaderSize - 3;

            // size is fixed...
            m_pBuffer->GetFixedSizeAString (sFilename, nFileNameLen);

            if (!sFilename.empty ())
            {
              if (debug)
                sFilename += _T (" [dbg:HeaderVersion2|1]");

              sFullPath = sDirName;
              sFullPath += sFilename;

              _SetArchiveFlag (ARC_DT_UNIX);

              ArchiveFile* pArcFile = new ArchiveFile (sFullPath);
              pArcFile->SetPackedSize (aH.nPackedSize);
              pArcFile->SetUnpackedSize (aH.nOriginalSize);
              pArcFile->SetDT (aH.aNorm.nDateTime);
              _AddFile (pArcFile);
            }

            break;
          }
          case 2:
          {
            m_pBuffer->GetFixedSizeAString (sDirName, nNextHeaderSize - 3);

            // \0377 is 0xff is 255!!
            str_replace_all (sDirName, TCHAR (0xff), _T ('\\'));
            break;
          }
          case 0x3F:
          {
            // comment
            m_pBuffer->IncActFilePos (nNextHeaderSize - 3);
            break;
          }
          case 0x40:
          {
            // MS-DOS attributes
            if (debug)
              out_format (_T ("[dbg] LZH MS DOS attribute - please send me the file\n"));
            m_pBuffer->IncActFilePos (2);
            break;
          }
          case 0x41:
          {
            // unknown but occured in E_SI08E3.EXE (Epson Spooler Driver)
            m_pBuffer->IncActFilePos (nNextHeaderSize - 3);
            break;
          }
          default:
          {
            if (debug)
              out_format (_T ("[dbg] Unknown extended type %02Xh\n"), nExtType);
            m_pBuffer->IncActFilePos (nNextHeaderSize - 3);
            break;
          }
        }
        nNextHeaderSize = m_pBuffer->GetUint16 ();
      } while (nExtType != 0);

      m_pBuffer->IncActFilePos (aH.nPackedSize);
      break;
    }
    else
    {
      // unknown header format
      SetErrorCode (ARCERR_UNKNOWN_METHOD);
      break;
    }

    // at the end of the file??
    if (m_pBuffer->GetActFilePos () >= m_pBuffer->GetFileSize () - 1)
    {
      m_pBuffer->SetToEOF ();
      break;
    }
  }
}
