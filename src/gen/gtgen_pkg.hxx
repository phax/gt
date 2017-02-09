MAKE_ANALYZER (PKG);

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (PKG)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetUint16 (0) == 0xA34A;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (PKG)
//--------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_PKG_HEADER));

  if (!m_bListMode)
  {
    PKG_GlobalHeader aGH;
    PKG_ArchiveHeader1 aAH1;
    PKG_ArchiveHeader2 aAH2;
    gtuint16 nNameLen;
    pstring sName, sDisplayName;
    int i, nDiskPos = 0, nNextReadDiskPos = 0;
    gtuint32 nTotalSize, nDirectoryPos, nFileSize;
    gtuint16 nDirectoryCount, nFileCount;
    gtuint8 nGuardByte;
    std::vector<pstring> aDirectories;

    out_incindent ();

    // read global header
    m_pBuffer->GetBufferX (0, &aGH, PKG_GH_SIZE);
    if (aGH.nVersionNumber != 2)
    {
      out_error_format (rc (HRC_GENLIB_PKG_INV_VERSION),
                        aGH.nVersionNumber);
    }

    // goto archive directory (at end of archive)
    m_pBuffer->SetActFilePos (aGH.nOffsetToArchiveList);

    while (!m_pBuffer->IsEOF ())
    {
      if (nDiskPos++ == nNextReadDiskPos)
      {
        // uint16, uint16 diskStart, uint16 diskEnd
        m_pBuffer->GetBufferX (&aAH1, PKG_AH1_SIZE);
        nNextReadDiskPos += aAH1.nDiskCount;
      }

      // read name of archive
      nNameLen = m_pBuffer->GetUint16 ();
      if (!m_pBuffer->GetFixedSizeAString (sName, nNameLen))
      {
        out_error_format (rc (HRC_GENLIB_PKG_ERR_READ_1),
                          nNameLen, m_pBuffer->GetActFilePos ());
      }

      // and read some nice flags (uint16, uint32 ofsToFiles)
      m_pBuffer->GetBufferX (&aAH2, PKG_AH2_SIZE);

      // format ouput
      out_format (rc (HRC_GENLIB_PKG_ITEM),
                  sName.c_str (),
                  aAH2.nOffsetToFiles,
                  aAH2.nOffsetToFiles,
                  nDiskPos,
                  nNextReadDiskPos);

      // can happen:
      if (file_t (aAH2.nOffsetToFiles) < m_pBuffer->GetFileSize ())
      {
        // save position ...
        const file_t nSavedPos = m_pBuffer->GetActFilePos ();

        // goto list of files in package
        m_pBuffer->SetActFilePos (aAH2.nOffsetToFiles);

        // read total size of this package content (without these 4 bytes!!)
        nTotalSize = m_pBuffer->GetInt32 ();

        // dir count
        nDirectoryCount = m_pBuffer->GetUint16 ();

        // read name of directory
        for (i = 0; i < nDirectoryCount; i++)
        {
          // read dir name (uint16 len, _len name)
          nNameLen = m_pBuffer->GetUint16 ();
          if (!m_pBuffer->GetFixedSizeAString (sName, nNameLen))
          {
            // happens on Netscape 4.72 setup.pkg :(
            out_error_format (rc (HRC_GENLIB_PKG_ERR_READ_2),
                              nNameLen, m_pBuffer->GetActFilePos ());
            break;
          }

          // there's one byte to skip (
          nGuardByte = m_pBuffer->GetUint8 ();
          if (nGuardByte != 0)
          {
            out_info_format (rc (HRC_GENLIB_PKG_GUARD_BYTE),
                             nGuardByte);
          }

          // add a trailing backslash if directory name != ""
          if (nNameLen > 0)
            sName += _T ('\\');
          aDirectories.push_back (sName);
        }

        // follow-up error (Netscape 4.72 setup.pkg)
        if (!aDirectories.empty ())
        {
          // read number of files in this package
          nFileCount = m_pBuffer->GetUint16 ();

          // read file data...
          out_incindent ();
          for (i = 0; i < nFileCount; i++)
          {
            nDirectoryPos = m_pBuffer->GetUint16 ();   // index in list (0 based)
            nFileSize = m_pBuffer->GetInt32 ();       // read file size (4 byte)
            nNameLen = m_pBuffer->GetUint8 ();        // read filename
            if (!m_pBuffer->GetFixedSizeAString (sName, nNameLen))
            {
              // can happen in Netscape 4.72 setup.pkg
              out_error_format (rc (HRC_GENLIB_PKG_ERR_READ_3),
                                nNameLen, m_pBuffer->GetActFilePos ());
              break;
            }

            // sometimes we cannot increment if we are at the last entry!
  //          if (i < nFileCount - 1 || !m_pBuffer->IsEOF ())
            m_pBuffer->IncActFilePos (1);            // and skip an additional byte

            if (nDirectoryPos < aDirectories.size ())
              sName.insert (0, aDirectories[nDirectoryPos]);
            else
            {
              out_error_format (rc (HRC_GENLIB_PKG_ERR_INV_INDEX),
                                nDirectoryPos, aDirectories.size ());
            }

            out_format (rc (HRC_GENLIB_PKG_FILE),
                        sName.c_str (),
                        nFileSize);
          }
          out_decindent ();
        }

        // ... and restore previous pos
        m_pBuffer->SetActFilePos (nSavedPos);
      }
    }

    out_decindent ();
  }
}
