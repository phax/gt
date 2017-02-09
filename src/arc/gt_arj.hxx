//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ARJ)
//--------------------------------------------------------------------
{
  ARJ_GlobalHeader aGH;
  ARJ_LocalHeader aLH;
  pstring sFilename;

  _SetArchiveFlag (ARC_DT_DOS);

  // get global header..
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aGH, GT_ARJ_GH_SIZE);

  /* if version >= 2.63 an additional flag was added to
   * the global header (a long)
   */
  if ((aGH.nPackVersion >= 10 && aGH.nPackVersion < 50) ||
      (aGH.nPackVersion >= 101))
  {
    m_pBuffer->IncActFilePos (4);
  }

  // read the original filename
  m_pBuffer->GetASCIIZ_AString (sFilename, GT_MAX_SIZE);

  // bild special AddOn string
  {
#define _ARJ_CASE(__n,__s)  case __n: str_assign_format (sAddOnStr, rc (HRC_ARCLIB_ARJ_VERSION), _T (__s)); break;
    pstring sAddOnStr;
    switch (aGH.nPackVersion)
    {
      case 1:
      _ARJ_CASE (  2, "< 2.21");
      _ARJ_CASE (  3, "2.21");
      _ARJ_CASE (  4, "2.30");
      _ARJ_CASE (  5, "2.39");
      _ARJ_CASE (  6, "2.41");
      _ARJ_CASE (  7, "2.42(c) - 2.50(a)");
      _ARJ_CASE (  8, "2.55 - 2.60");
      _ARJ_CASE (  9, "2.62");
      _ARJ_CASE ( 10, "2.63 - 2.76");
      _ARJ_CASE ( 51, "ARJZ");
      _ARJ_CASE (100, "3.00a");
      _ARJ_CASE (101, "3.03 - 3.09");
      default:
        str_assign_format (sAddOnStr,
                           rc (HRC_ARCLIB_ARJ_VERSION),
                           _T ("[unknown]"));
        break;
#undef _ARJ_CASE
    }

    // append internal archive filename
    str_append_format (sAddOnStr, rc (HRC_ARCLIB_ARJ_ADDONSTRING), sFilename.c_str ());

    // special addon information
    _SetAddOnString (sAddOnStr);
  }

  // skip the archive comment...
  if (!m_pBuffer->SkipUntil (gtuint8 (0)))
  {
    // cannot find end of archive comment...
    SetErrorCode (ARCERR_UNEXPECTED_EOF);    return;
  }

  // and now lets skip CRC32
  m_pBuffer->IncActFilePos (4);

  // now check if there is a first extended header...
  gtuint16 nFirstExtHeaderSize = m_pBuffer->GetUint16 ();
  if (nFirstExtHeaderSize > 0)
  {
    m_pBuffer->IncActFilePos (nFirstExtHeaderSize + 4);  // data + CRC
  }

  for(;;)
  {
    const file_t nSavedPos = m_pBuffer->GetActFilePos ();

    if (!m_pBuffer->GetBuffer (&aLH, GT_ARJ_LH_SIZE))
    {
      // if you are 4 before the EOF - it's the end of the archive..
      m_pBuffer->IncActFilePos (4);
      if (!m_pBuffer->IsEOF ())
      {
        SetErrorCode (ARCERR_UNEXPECTED_EOF);
      }
      break;
    }

    // last entry has headersize 0
    if (aLH.nHeaderSize == 0)
      break;

    // check the header ID
    if (aLH.nID != 60000)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    // check for invalid values
    if (aLH.nOriginalSize < 0 || aLH.nPackedSize < 0)
    {
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }

    // goto filename offset
    m_pBuffer->SetActFilePos (nSavedPos + 4 + aLH.nFirstHeaderSize);

    //try to determine the filename
    m_pBuffer->GetASCIIZ_AString (sFilename, GT_MAX_SIZE);

    ArchiveFile* pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (aLH.nPackedSize);
    pArcFile->SetUnpackedSize (aLH.nOriginalSize);
    pArcFile->SetDT (aLH.nDT);
    pArcFile->SetPWProtected (aLH.nFlags & 1);
    _AddFile (pArcFile);

    m_pBuffer->SetActFilePos (nSavedPos +         // old position
                              4 +                 // header start
                              aLH.nHeaderSize +   // header
                              4);                 // local CRC32

    nFirstExtHeaderSize = m_pBuffer->GetUint16 ();
    if (nFirstExtHeaderSize > 0)
    {
      m_pBuffer->IncActFilePos (nFirstExtHeaderSize + 4);  // data + CRC
    }

    m_pBuffer->IncActFilePos (aLH.nPackedSize);  // packed data
  }
}
