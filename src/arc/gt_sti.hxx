//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (STI)
//--------------------------------------------------------------------
{
  int nFilesFound = 0;
  int nDiskNr = 0;
  pstring sDirName, sOutputDirectory, sOutputFilename;
  STI_GlobalHeader aGH;
  STI_DirHeader aDH;
  STI_FileHeader aFH;
  std::vector<pstring> aDirectories;

  // read global header
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_STI_GH_SIZE);

  // is it a multi disk archive?
  if (aGH.nCurrentDisk > 0)
  {
/*
    if (aGH.nTotalDisks > 0)
      out_format (_T ("  This is the master disk %d of %d\n\n"), aGH.nCurrentDisk, aGH.nTotalDisks);
    else
      out_format (_T ("  This is disk %d\n\n"), aGH.nCurrentDisk);
*/
  }

  // goto start of data offset
  m_pBuffer->SetActFilePos (m_nLocalOffset + aGH.nDataOffsetWithDirs);

  for (;;)
  {
    const file_t nOldFilePos = m_pBuffer->GetActFilePos ();

    if (m_pBuffer->GetUint8 (nOldFilePos + 2) == 0x00)
    {
      // read file header ...
      m_pBuffer->GetBufferX (nOldFilePos, &aFH, GT_STI_FH_SIZE);

      // found a new file!
      nFilesFound++;

      if (aFH.nDiskID != nDiskNr)
      {
        nDiskNr = aFH.nDiskID;

        // Disk %d
//        out_format (_T ("Disk %u"), nDiskNr);
      }

      // check if a directory name for this directory id exists...
      if (aFH.nDirID < aDirectories.size ())
      {
        sOutputDirectory = aDirectories[aFH.nDirID];
        if (!sOutputDirectory.empty ())  // to avoid '\foo.txt'
          sOutputDirectory += _T ('\\');
      }

      // directory is changed only every nth file!
      sOutputFilename = sOutputDirectory;

      // read the filename from the file and append it...
      {
        char *sFilename = (char*) _alloca (aFH.nFilenameLen + 1);
        sFilename[aFH.nFilenameLen] = '\0';
        m_pBuffer->GetBufferX (sFilename, aFH.nFilenameLen);

        str_append (sOutputFilename, sFilename, aFH.nFilenameLen);
      }

      ArchiveFile *pArcFile = new ArchiveFile (sOutputFilename);
      pArcFile->SetPackedSize (aFH.nPackedSize);
      pArcFile->SetUnpackedSize (aFH.nOriginalSize);
      _AddFile (pArcFile);

      m_pBuffer->SetActFilePos (nOldFilePos + aFH.nHeaderSize);
    }
    else
    {
      // directory entry

      // read directory header
      m_pBuffer->GetBufferX (nOldFilePos, &aDH, GT_STI_DH_SIZE);

      if (aDH.nFilenameLen > 0)
      {
        if (!m_pBuffer->GetFixedSizeAString (sDirName, aDH.nFilenameLen))
        {
          SetErrorCode (ARCERR_UNEXPECTED_EOF);
          break;
        }
        aDirectories.push_back (sDirName);
      }
      else
      {
        // error: invalid dirname len
        SetErrorCode (ARCERR_UNEXPECTED_VALUE);
        break;
      }

      // and goto the next entry...
      m_pBuffer->SetActFilePos (nOldFilePos + aDH.nHeaderSize);
    }

    if ((m_pBuffer->GetActFilePos () - m_nLocalOffset) >= aGH.nArchiveSize ||
        nFilesFound >= aGH.nFilesInArchive)
    {
      break;
    }
  }
}
