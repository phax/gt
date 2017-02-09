MAKE_ANALYZER (CHM);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (CHM)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x46535449;
}

//--------------------------------------------------------------------
static gtuint32 _get_encint (FileBuffer* pBuffer)
//--------------------------------------------------------------------
{
  gtuint8 n;
  gtuint32 x = 0;
  do
  {
    n = pBuffer->GetUint8 ();
    x <<= 7;
    x |= (n & 0x7f);
  } while (n & 0x80);
  return x;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (CHM)
//--------------------------------------------------------------------
{
  // read header + section data
  CHM_Header aH;
  m_pBuffer->GetBufferX (0, &aH, CHM_H_SIZE);    // @0
  CHM_HeaderSection aHS[2];
  m_pBuffer->GetBufferX (&aHS[0], CHM_HS_SIZE);  // @56
  m_pBuffer->GetBufferX (&aHS[1], CHM_HS_SIZE);  // @72

  // write header
  out_append (rc (HRC_GENLIB_CHM_HEADER));

  if (!m_bListMode)
  {
    out_incindent ();
    out_format (rc (HRC_GENLIB_CHM_VERSION), aH.nVersion, aH.nLangID);

    {
      static const gtuint8 aGUID1 [16] = { 0x10, 0xFD, 0x01, 0x7C, 0xAA, 0x7B, 0xD0, 0x11, 0x9E, 0x0C, 0x00, 0xA0, 0xC9, 0x22, 0xE6, 0xEC };
      static const gtuint8 aGUID2 [16] = { 0x11, 0xFD, 0x01, 0x7C, 0xAA, 0x7B, 0xD0, 0x11, 0x9E, 0x0C, 0x00, 0xA0, 0xC9, 0x22, 0xE6, 0xEC };
      if (memcmp (aH.aGUID1, aGUID1, 16) != 0)
        out_error_append (rc (HRC_GENLIB_CHM_INVALID_GUID_1));
      if (memcmp (aH.aGUID2, aGUID2, 16) != 0)
        out_error_append (rc (HRC_GENLIB_CHM_INVALID_GUID_2));
    }

    // Following the header section table is 8 bytes of additional header data.
    // In Version 2 files, this data is not there and the content section starts
    //   immediately after the directory.
    if (aH.nVersion >= 3)
      m_pBuffer->IncActFilePos (8);

    // read section 0
    CHM_HeaderSection0 aHS0;
    m_pBuffer->GetBufferX (&aHS0, CHM_HS0_SIZE);

    if (aHS0.nID != 0x000001fe)
      out_error_format (rc (HRC_GENLIB_CHM_INVALID_SECTION_0_ID), aHS0.nID);
    if (aHS0.nFileSize != m_pBuffer->GetFileSize ())
      out_error_append (rc (HRC_GENLIB_CHM_FILESIZE_MISMATCH));

    // read directory header
    CHM_DirectoryHeader aDH;
    m_pBuffer->GetBufferX (&aDH, CHM_DH_SIZE);

    if (aDH.nID != 0x50535449)
      out_error_format (rc (HRC_GENLIB_CHM_INVALID_DIR_HEADER_ID), aDH.nID);
    if (aDH.nVersion != 1)
      out_error_format (rc (HRC_GENLIB_CHM_INVALID_DIR_HEADER_VERSION), aDH.nVersion);
    if (aDH.nSizeOfHeader != CHM_DH_SIZE)
      out_error_format (rc (HRC_GENLIB_CHM_INVALID_DIR_HEADER_SIZE), aDH.nSizeOfHeader);

    if (debug)
    {
      out_format (_T ("Dir Chunk Size: %Xh\n"), aDH.nDirChunkSize);
      out_format (_T ("PMGL Numbers: %u - %u [%u]\n"),
                  aDH.nFirstPMGLChunkNumber,
                  aDH.nLastPMGLChunkNumber,
                  aDH.nChunkCount);

      const gtuint32 nQuickRefCount = 1 + (1 << aDH.nQuickRefDensity);
      out_format (_T ("QRCount: %u\n"), nQuickRefCount);
    }

/*
    // read an ID, any identify, whether Listing or Directory Chunk
    gtuint32 nID = m_pBuffer->GetInt32 ();
    m_pBuffer->IncActFilePos (-4);
    out_format (_T ("  Now at %I64Xh (ID = %08Xh)\n"), m_pBuffer->GetActFilePos (), nID);
*/

    gtuint32 nFileCount = 0;
    CHM_ListingChunk aLC;
    CHM_IndexChunk aIC;

    for (gtuint32 nChunkIndex = 0; nChunkIndex < aDH.nChunkCount; ++nChunkIndex)
    {
      file_t nDesiredEndPos = m_pBuffer->GetActFilePos () + aDH.nDirChunkSize;

      if (nChunkIndex >= aDH.nFirstPMGLChunkNumber &&
          nChunkIndex <= aDH.nLastPMGLChunkNumber)
      {
        // read listing chunk
        m_pBuffer->GetBufferX (&aLC, CHM_LC_SIZE);
        if (aLC.nID != 0x4c474d50)    // "PMGL"
          out_error_format (rc (HRC_GENLIB_CHM_INVALID_LIST_CHUNK_ID), aLC.nID);

        if (debug)
          out_info_format (_T ("Free space afterwards: %Xh\n"), aLC.nFreeSpaceAfterwards);

        nDesiredEndPos -= aLC.nFreeSpaceAfterwards;

        string sFilename;
        gtuint32 nFilenameLen, nContentSection, nOffset, nLength;
        do
        {
          ++nFileCount;

          // read filename length
          nFilenameLen = _get_encint (m_pBuffer);
          m_pBuffer->GetFixedSizeAString (sFilename, nFilenameLen);

          // read length and size
          nContentSection = _get_encint (m_pBuffer);
          nOffset = _get_encint (m_pBuffer);
          nLength = _get_encint (m_pBuffer);

          out_format (rc (HRC_GENLIB_CHM_ENTRY),
                      nFileCount,
                      sFilename.c_str (),
                      nContentSection,
                      nOffset,
                      nLength);

        } while (m_pBuffer->GetActFilePos () < nDesiredEndPos);
        m_pBuffer->IncActFilePos (aLC.nFreeSpaceAfterwards);
      }
      else
      {
        // read index chunk
        m_pBuffer->GetBufferX (&aIC, CHM_IC_SIZE);
        if (aIC.nID != 0x49474d50)    // "PMGI"
          out_error_format (rc (HRC_GENLIB_CHM_INVALID_CHUNK_INDEX_ID), aIC.nID);

//        m_pBuffer->IncActFilePos (aIC.nFreeSpaceAfterwards);
        m_pBuffer->SetActFilePos (nDesiredEndPos);
      }
    }

    out_decindent ();
  }
}
