// Central Directory Structure
const gtuint32 GT_ZIP_CDS_ID   = 0x02014B50;
const gtuint32 GT_ZIP_CDS_ID2  = 0x02014B4F;

// Central Directory Structure End
const gtuint32 GT_ZIP_CDSE_ID  = 0x06054B50;

// Local File Header
const gtuint32 GT_ZIP_LFH_ID   = 0x04034B50;
const gtuint32 GT_ZIP_LFH_ID2  = 0x04034B4F;
const gtuint32 GT_ZIP_LFH_TIP  = 0x04034B54;

// flags
const gtuint32 GT_ZIP_PASSWORD    = BITS[0];
const gtuint32 GT_ZIP_STREAM_FLAG = BITS[3];

//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ZIP)
//--------------------------------------------------------------------
{
  pstring sFilename;
  ZIP_CDS aCDS;
  ZIP_CDSE aCDSE;
  ZIP_LFH aLFH;
  gtint32 nFileCount = 0;  // inc in ZIP file, dec in ZIP dir (signed!)
  gtuint32 nLocalLocalOffset = 0;

  _SetArchiveFlag (ARC_DT_DOS);

  // if it is a multi volume ZIP skip the first header
  if (m_pBuffer->GetInt32 (0) == 0x30304B50 ||
      m_pBuffer->GetInt32 (0) == 0x08074B50)
  {
    nLocalLocalOffset = 4;
  }

  // start at offset 0
  m_pBuffer->SetActFilePos (m_nLocalOffset + nLocalLocalOffset);

  for (;;)
  {
    // read the ID and go back to the original filepos ...
    const gtuint32 nID = m_pBuffer->GetInt32 ();
    m_pBuffer->IncActFilePos (-4);

    // do not use a swicth statement since we use 'break' to
    // leave the 'for (;;)' loop
    if (nID == GT_ZIP_LFH_ID ||
        nID == GT_ZIP_LFH_ID2 ||
        nID == GT_ZIP_LFH_TIP)
    {
      // read a new header record...
      m_pBuffer->GetBufferX (&aLFH, GT_ZIP_LFH_SIZE);

      // check if the values are valid...
      if (aLFH.nPackedSize > m_pBuffer->GetFileSize ())
      {
        SetErrorCode (ARCERR_UNEXPECTED_VALUE);
        break;
      }

      // valid filename len?
      if (aLFH.nFilenameLen <= 0)
      {
        SetErrorCode (ARCERR_UNEXPECTED_VALUE);
        break;
      }

      // extralen must be a multiple of 4
      // Not the case for Xerces 2.5.0 source :(
//      ASSERT ((aLFH.nExtraLen % 4) == 0);

      ++nFileCount;

      // read the filename
      if (!m_pBuffer->GetFixedSizeAString (sFilename, aLFH.nFilenameLen))
      {
        SetErrorCode (ARCERR_UNEXPECTED_EOF);
        break;
      }

      // skip extra bytes
      // the Extra-data has the following layout (n-times)
      // {
      //   gtuint16:header
      //   gtuint16:datasize
      // }
      m_pBuffer->IncActFilePos (aLFH.nExtraLen);

      // The aLFH.nCRC32 == 0 is a workaround for the file "testking 350-001 cisco CCIE written v.21.zip"
      if ((aLFH.nFlags & GT_ZIP_STREAM_FLAG) && (aLFH.nCRC32 == 0))
      {
        // the archive data is appended after the compressed stream
        // nPackedSize, nOriginalSize and CRC32 are set to 0
        ASSERT (aLFH.nCRC32 == 0);
        ASSERT (aLFH.nPackedSize == 0 || aLFH.nPackedSize == 12);
        ASSERT (aLFH.nOriginalSize == 0);

        // search the byte with the data on the disk
        if (!m_pBuffer->GotoPosOfUint32 (0x08074B50))
        {
          // not found :(
          SetErrorCode (ARCERR_UNEXPECTED_EOF);
          break;
        }

        // skip ID
        m_pBuffer->IncActFilePos (4);
        aLFH.nCRC32        = m_pBuffer->GetInt32 ();
        aLFH.nPackedSize   = m_pBuffer->GetInt32 ();
        aLFH.nOriginalSize = m_pBuffer->GetInt32 ();
      }
      else
      {
        // skip packed size
        m_pBuffer->IncActFilePos (aLFH.nPackedSize);
      }

      ArchiveFile *pArcFile = new ArchiveFile (sFilename);
      pArcFile->SetPackedSize (aLFH.nPackedSize);
      pArcFile->SetUnpackedSize (aLFH.nOriginalSize);
      pArcFile->SetDT (aLFH.nDT);
      pArcFile->SetPWProtected (aLFH.nFlags & GT_ZIP_PASSWORD);
      _AddFile (pArcFile);
    }
    else
    if (nID == GT_ZIP_CDS_ID || nID == GT_ZIP_CDS_ID2)
    {
      if (!m_pBuffer->GetBuffer (&aCDS, GT_ZIP_CDS_SIZE))
      {
        SetErrorCode (ARCERR_UNEXPECTED_EOF);
        break;
      }

      m_pBuffer->IncActFilePos (aCDS.nFilenameLen +
                                aCDS.nExtraLen +
                                aCDS.nCommentLen);
      --nFileCount;
    }
    else
    if (nID == GT_ZIP_CDSE_ID)
    {
      // check for ZIP CDSE...
      m_pBuffer->GetBufferX (&aCDSE, GT_ZIP_CDSE_SIZE);

      // skip comment
      m_pBuffer->IncActFilePos (aCDSE.nCommentLen);

      // should be EOF!
      break;
    }
    else
    {
      if (nID == 0x06054141)
      {
        // HKI archiver (http://www.winhki.com) ...
        // -> skip (4 + 18) bytes at eof
        m_pBuffer->IncActFilePos (22);
      }
      else
      {
        if (debug)
          out_info_format (_T ("[dbg] unknown ZIP ID %08Xh\n"), nID);
        SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      }

      // should be EOF or error
      break;
    }
  }  // endless loop

  if (nFileCount > 0)
  {
    // n files are not in the ZIP directory...
    SetErrorCode (ARCERR_UNEXPECTED_EOF);
  }
  else
  if (nFileCount < 0)
  {
    // n files are too much in the ZIP directory...
    // don't set the error code here - special archives..
  }
}
