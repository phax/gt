const gtuint8 GT_RAR_MARK     = 0x72;
const gtuint8 GT_RAR_MAIN     = 0x73;
const gtuint8 GT_RAR_FILEHEAD = 0x74;
const gtuint8 GT_RAR_COMMENT  = 0x75;
const gtuint8 GT_RAR_AVHEAD   = 0x76;
const gtuint8 GT_RAR_SUB      = 0x77;
const gtuint8 GT_RAR_RECOVERY = 0x78;
const gtuint8 GT_RAR_SIGN     = 0x79;
const gtuint8 GT_RAR_NEW_SUB  = 0x7A;
const gtuint8 GT_RAR_ENDARC   = 0x7B;

const gtuint16 GT_RAR_FLAG_ARCCOMMENT  = 0x0002;
const gtuint16 GT_RAR_FLAG_ENCRYPTED   = 0x0004;
const gtuint16 GT_RAR_FLAG_FILECOMMENT = 0x0008;
const gtuint16 GT_RAR_FLAG_SALT        = 0x0400;
const gtuint16 GT_RAR_FLAG_64BITSIZE   = 0x0100;
const gtuint16 GT_RAR_FLAG_LHD_EXTTIME = 0x1000;
const gtuint16 GT_RAR_FLAG_HASADDSIZE  = 0x8000;

const gtuint32 GT_RAR_SALT_SIZE = 8;

/*! This class is ripped from unrar.
 */
//--------------------------------------------------------------------
class EncodeFileName
//--------------------------------------------------------------------
{
private:
  gtuint8 Flags;
  int FlagBits;
  int FlagsPos;
  int DestSize;

public:
  EncodeFileName()
    : Flags    (0),
      FlagBits (0),
      FlagsPos (0),
      DestSize (0)
  {}

  void Decode (      char*    Name,
               const gtuint8* EncName,
               const size_t   EncSize,
                     wchar_t* NameW,
               const size_t   MaxDecSize)
  {
    size_t EncPos = 0, DecPos = 0;
    gtuint8 HighByte = EncName[EncPos++];
    while (EncPos < EncSize && DecPos < MaxDecSize)
    {
      if (FlagBits == 0)
      {
        Flags = EncName[EncPos++];
        FlagBits = 8;
      }
      switch (Flags >> 6)
      {
        case 0:
          NameW[DecPos++] = EncName[EncPos++];
          break;
        case 1:
          NameW[DecPos++] = EncName[EncPos++] + (HighByte<<8);
          break;
        case 2:
          NameW[DecPos++] = EncName[EncPos] + (EncName[EncPos + 1] << 8);
          EncPos += 2;
          break;
        case 3:
        {
          int Length = EncName[EncPos++];
          if (Length & 0x80)
          {
            gtuint8 Correction = EncName[EncPos++];
            for (Length = (Length & 0x7f) + 2; Length > 0 && DecPos < MaxDecSize; Length--, DecPos++)
              NameW [DecPos] = ((Name [DecPos] + Correction) & 0xff) + (HighByte << 8);
          }
          else
          {
            for (Length += 2; Length > 0 && DecPos < MaxDecSize; Length--, DecPos++)
              NameW [DecPos] = Name [DecPos];
          }
          break;
        }
      }
      Flags <<= 2;
      FlagBits -= 2;
    }
    NameW [DecPos < MaxDecSize ? DecPos : MaxDecSize - 1] = 0;
  }
};

//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (RAR)
//--------------------------------------------------------------------
{
  RAR_MarkHead aMarkHead;
  RAR_MainHead aMainHead;
  RAR_FileHead aFileHead;
  RAR_FileCommentHead aCommentHead;
  pstring sFilename;
  file_t nOriginalSize, nPackedSize;
  wchar_t sFilenameW[1024];

  _SetArchiveFlag (ARC_DT_DOS);

  // read and check marker block
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aMarkHead, GT_RAR_MARKHEAD_SIZE);

  for (;;)
  {
    // no GetBufferX here because it is executed as the last action in a file
    if (!m_pBuffer->GetBuffer (&aMarkHead, GT_RAR_MARKHEAD_SIZE))
    {
      // end of file
      break;
    }

    if (aMarkHead.nHeadType == 0)
    {
      // end of archive?
      break;
    }

    // bit for additional size set (except file header?)
    gtuint32 nHeaderSize = aMarkHead._nHeadSize;
    // fileheader has a special flag for additional size!
    if (aMarkHead.nHeadType != GT_RAR_FILEHEAD &&
        aMarkHead.nHeadType != GT_RAR_NEW_SUB &&
        (aMarkHead.nHeadFlags & GT_RAR_FLAG_HASADDSIZE))
    {
      // add ADD_SIZE to headersize
      nHeaderSize += m_pBuffer->GetInt32 ();
    }

    switch (aMarkHead.nHeadType)
    {
      case GT_RAR_MAIN:  /* 0x73 */
      {
        // check header flags
        if (aMarkHead.nHeadFlags & BITS[0])
          _SetArchiveFlag (ARC_MULTI_VOLUME);
        if (aMarkHead.nHeadFlags & GT_RAR_FLAG_ARCCOMMENT)
          _SetArchiveFlag (ARC_COMMENT);
        if (aMarkHead.nHeadFlags & BITS[2])
          _SetArchiveFlag (ARC_LOCKED);
        if (aMarkHead.nHeadFlags & BITS[3])
          _SetArchiveFlag (ARC_SOLID);
        if (aMarkHead.nHeadFlags & BITS[5])
          _SetArchiveFlag (ARC_AUTHENTIFICATION);
        if (aMarkHead.nHeadFlags & BITS[6])
          _SetArchiveFlag (ARC_RECOVERY_RECORD);
        if (aMarkHead.nHeadFlags & BITS[7])
          _SetArchiveFlag (ARC_ENCRYPTED);

        // read additional main header
        m_pBuffer->GetBufferX (&aMainHead, GT_RAR_ADD_MAINHEAD_SIZE);

        if (aMarkHead.nHeadFlags & GT_RAR_FLAG_ARCCOMMENT)
        {
          // archive comment..
          _SetArchiveFlag (ARC_COMMENT);

          const size_t nCommentSize = nHeaderSize -
                                      GT_RAR_MARKHEAD_SIZE -
                                      GT_RAR_ADD_MAINHEAD_SIZE;
          m_pBuffer->IncActFilePos (nCommentSize);
        }
        break;
      }
      case GT_RAR_FILEHEAD:  /* 0x74 */
      case GT_RAR_NEW_SUB:   /* 0x7a */
      {
        // read additional file header fields
        m_pBuffer->GetBufferX (&aFileHead, GT_RAR_ADD_FILEHEAD_SIZE);

        if (aMarkHead.nHeadFlags & GT_RAR_FLAG_64BITSIZE)
        {
          // 64-bit filesize
          // get high long of packed and unpacked size
          gtuint32 nHighPackedSize   = m_pBuffer->GetInt32 ();
          gtuint32 nHighOriginalSize = m_pBuffer->GetInt32 ();
          nOriginalSize = (nHighOriginalSize * file_t (0x100000000LL)) + aFileHead.nOriginalSize;
          nPackedSize   = (nHighPackedSize   * file_t (0x100000000LL)) + aFileHead.nPackedSize;
        }
        else
        {
          // 32-bit filesize
          nOriginalSize = aFileHead.nOriginalSize;
          nPackedSize   = aFileHead.nPackedSize;
        }

        // get filename (as charbuf)
        // Note: using _alloca here causes an stack overflow on large
        //       archives. it seems that the stack is freed after
        //       the routine is left!
        char* sTemp = new char [aFileHead.nFilenameLen + 1];
        sTemp[aFileHead.nFilenameLen] = 0;
        m_pBuffer->GetBuffer (sTemp, aFileHead.nFilenameLen);

        // Special for Unicode filenames
        if (aMarkHead.nHeadFlags & BITS[9])
        {
          size_t Length = strlen (sTemp) + 1;
          EncodeFileName NameCoder;
          NameCoder.Decode (sTemp,
                            (gtuint8*)sTemp + Length,
                            aFileHead.nFilenameLen - Length,
                            sFilenameW,
                            GT_ARRAY_SIZE (sFilenameW));
          str_assign (sFilename, sFilenameW);
        }
        else
        {
          str_assign (sFilename, sTemp);
        }

        delete sTemp;

        // show file
        if (aMarkHead.nHeadType == GT_RAR_FILEHEAD)
        {
          ArchiveFile* pArcFile = new ArchiveFile (sFilename);
          pArcFile->SetPackedSize (nPackedSize);
          pArcFile->SetUnpackedSize (nOriginalSize);
          pArcFile->SetDT (aFileHead.nDateTime);
          pArcFile->SetPWProtected (aMarkHead.nHeadFlags & GT_RAR_FLAG_ENCRYPTED);
          pArcFile->SetContinuedFromLast (aMarkHead.nHeadFlags & BITS[0]);
          pArcFile->SetContinuedOnNext (aMarkHead.nHeadFlags & BITS[0]);
          pArcFile->SetComment (aMarkHead.nHeadFlags & GT_RAR_FLAG_FILECOMMENT);
          _AddFile (pArcFile);

          // it's a file
          if (aMarkHead.nHeadFlags & GT_RAR_FLAG_FILECOMMENT)
          {
            // file comment...

            // read comment header!
            m_pBuffer->GetBufferX (&aMarkHead, GT_RAR_MARKHEAD_SIZE);
            m_pBuffer->GetBufferX (&aCommentHead, GT_RAR_ADD_COMMENTH_SIZE);
            m_pBuffer->IncActFilePos (aMarkHead._nHeadSize - GT_RAR_MARKHEAD_SIZE - GT_RAR_ADD_COMMENTH_SIZE);
          }
        }
        else
        {
          if (debug)
            // listings according to unrar 3.2.4:
            if (sFilename != _T ("CMT") &&      // Comment
                sFilename != _T ("ACL") &&      // Access Control List
                sFilename != _T ("STM") &&      // Stream
                sFilename != _T ("UOW") &&      // UOwner
                sFilename != _T ("AV") &&       // Anti Virus
                sFilename != _T ("RR") &&       // Recovery Record
                sFilename != _T ("EA2") &&      // extended attributes OS2
                sFilename != _T ("EABE"))       // extended attributes BEOS
            {
              out_error_format (_T ("Error [dbg] [unknown new sub type '%s']\n"), sFilename.c_str ());
            }

          // this stuff is copied from unrar (arcread.hpp)
          size_t nDataSize = nHeaderSize - aFileHead.nFilenameLen  - 32;
          if (aMarkHead.nHeadFlags & GT_RAR_FLAG_SALT)
            nDataSize -= GT_RAR_SALT_SIZE;
          if (nDataSize > 0)
            m_pBuffer->IncActFilePos (nDataSize);
        }

        // salt present? (from arcread.hpp)
        if (aMarkHead.nHeadFlags & GT_RAR_FLAG_SALT)
          m_pBuffer->IncActFilePos (GT_RAR_SALT_SIZE);

        // read extended time (from arcread.hpp)
        if (aMarkHead.nHeadFlags & GT_RAR_FLAG_LHD_EXTTIME)
        {
          // 4 times 64bit
          const gtuint16 nFlags = m_pBuffer->GetUint16 ();
          for (int i = 0; i < 4; ++i)
          {
            gtuint16 rmode = nFlags >> (3 - i) * 4;
            if ((rmode & 8) == 0)
              continue;
            if (i != 0)
            {
              // read dos time
              m_pBuffer->IncActFilePos (4);
            }
            m_pBuffer->IncActFilePos (rmode & 3);
          }
        }

        // skip packed size (after reading comment)
        m_pBuffer->IncActFilePos (aFileHead.nPackedSize);
        break;
      }
      case GT_RAR_RECOVERY:
      {
        // recovery record..
        m_pBuffer->IncActFilePos (nHeaderSize - GT_RAR_MARKHEAD_SIZE - 4);
        break;
      }
      case GT_RAR_AVHEAD:
      case GT_RAR_COMMENT:
      case GT_RAR_SUB:
      case GT_RAR_SIGN:
      case GT_RAR_ENDARC:
      {
        // skip comment...
        m_pBuffer->IncActFilePos (nHeaderSize - GT_RAR_MARKHEAD_SIZE);
        break;
      }
      default:
      {
        if (debug)
          out_error_format (_T ("Error [dbg] nHeadType unknown %02Xh\n"), aMarkHead.nHeadType);

        // unknown token
        SetErrorCode (ARCERR_UNKNOWN_METHOD);
        break;
      }
    }  // switch

    if (GetErrorCode () != ARCERR_SUCCESS)
      break;
  }
}
