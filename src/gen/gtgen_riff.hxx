MAKE_ANALYZER (RIFF);

namespace RIFF {

//--------------------------------------------------------------------
static void _SubAnalyze
                                        (      FileBuffer* pBuffer,
                                         const file_t      nStartPos,
                                         const file_t      nMaxSize,
                                               pstring&    sPrefix)
//--------------------------------------------------------------------
{
  RIFF_Header aH;
  file_t nHandledBytes = 0;
  gtuint32 nSubType = gtuint32 (-1);
  bool bHasType, bHasNoType, bAggregate;
  bool bEndOfBlock;
  bool bPrintedDots = false;
  TCHAR s5[5];

  do
  {
    // try to avoid exception in WAVE files like
    // C:\Programme\Canon\ZoomBrowser EX\MyCameraFiles\RL_WAV\ANIMAL003_RL01.WAV
    // -> the last header consists only of a single NULL-byte
    if (nHandledBytes + 1 == nMaxSize)
      break;

    // read current RIFF header (8 bytes)
    pBuffer->GetBufferX (nStartPos + nHandledBytes, &aH, RIFF_H_SIZE);
    nHandledBytes += RIFF_H_SIZE;

    if (0 && debug)
    {
      out_format (_T ("[dbg] nStartPos=%I64d; nHandledBytes=%I64d; aH.nSize=%lu; nMaxSize=%I64d;\n"),
                  nStartPos, nHandledBytes, aH.nSize, nMaxSize);
    }

    // Assumption [ph]: an element with the ID 0x00000000 means EO Block
    if (aH.aID.n == 0x00000000)
    {
      if (0 && debug)
        out_info_append (_T ("[dbg] Info: found element with the ID 0x00000000\n"));
      break;
    }

/*
    DISP <type> <data>
    JUNK <filler>
    PAD <filler>  "aligns data on 2k boundary"
*/

    /* The following type have been checked on several different files.
       The strh token is important for AVIs
     */
    bHasType = (aH.aID.n == 0x50534944) || /*DISP*/
               (aH.aID.n == 0x5453494C) || /*LIST*/
               (aH.aID.n == 0x46464952) || /*RIFF*/
               (aH.aID.n == 0x68727473) || /*strh*/
               0; // dummy

    // definitly no type at:
    bHasNoType = (aH.aID.n == 0x54465349) || /*ISFT*/
                 (aH.aID.n == 0x4B4E554A) || /*JUNK*/
                 (aH.aID.n == 0x63643030) || /*00dc*/
                 (aH.aID.n == 0x62773130) || /*01wb*/
                 (aH.aID.n == 0x68697661) || /*avih*/
                 (aH.aID.n == 0x686C6D64) || /*dmlh*/
                 (aH.aID.n == 0x31786469) || /*idx1*/
                 (aH.aID.n == 0x66727473) || /*strf*/
                 0; // dummy

    // the following elements can be aggregated
    bAggregate = (aH.aID.n == 0x63643030) || /*00dc*/
                 (aH.aID.n == 0x62773130) || /*01wb*/
                 0; // dummy

    // 0xffffffff means: invalid size
    if (bHasType && aH.nSize != 0xffffffff)
    {
      // if it is one of the special types, read the subtype
      nSubType = pBuffer->GetInt32 (nStartPos + nHandledBytes);
      nHandledBytes += 4;
      aH.nSize      -= 4;
    }
    else
    {
      // default to an invalid subtype
      // Exception: in debug we are curious!
      if (1 && debug && gtint32 (aH.nSize) >= 4 && !bHasNoType)
      {
        nSubType = pBuffer->GetInt32 (nStartPos + nHandledBytes);
        bHasType = true;
        nHandledBytes += 4;
        aH.nSize      -= 4;
      }
      else
        nSubType = gtuint32 (-1);
    }

    bEndOfBlock = (nHandledBytes + aH.nSize >= nMaxSize);

    if (bAggregate)
    {
      // aggregate elements to minimize output
      if (!bPrintedDots)
      {
        out_format (_T ("%s%s [...]\n"),
                    sPrefix.c_str (),
                    bEndOfBlock
                      ? _T ("\\-")
                      : _T ("+-"));
        bPrintedDots = true;
      }
    }
    else
    {
      bPrintedDots = false;

      // print prefix, tree element and ID
      out_format (_T ("%s%s%c%c%c%c"),
                  sPrefix.c_str (),
                  bEndOfBlock
                    ? _T ("\\-")
                    : _T ("+-"),
                  aH.aID.s[0],
                  aH.aID.s[1],
                  aH.aID.s[2],
                  aH.aID.s[3]);

      // nSubType == -1 means no subtype
      if (bHasType && nSubType != gtuint32 (-1))
      {
        // convert subtype to string
        _ltoc4 (nSubType, s5);

        // and add
        out_format (_T (" [%c%c%c%c]"),
                    s5[0],
                    s5[1],
                    s5[2],
                    s5[3]);
      }

      // print size
      out_format (_T (" (%08Xh/%d bytes)\n"),
                  aH.nSize, aH.nSize);
    }

    // Assumption [ph]: a size of 0xffffffff means: EO Block but print block
    if (aH.nSize == 0xffffffff)
    {
      break;
    }

    // only uppercase stuff (except JUNK)
    if (bHasType)
    {
      if (aH.aID.n == 0x5453494C && nSubType == 0x72706D63) // "LIST" && "cmpr"
      {
        ; // do not go into the recursion
      }
      else
      {
        // append prefix
        sPrefix += (bEndOfBlock ? _T ("  ") : _T ("| "));

        // recursive call!!
        _SubAnalyze (pBuffer,
                     nStartPos + nHandledBytes,
                     aH.nSize,
                     sPrefix);

        // remove prefix
        sPrefix.erase (sPrefix.end () - 2, sPrefix.end ());
      }
    }

    // is it the rest?
    if (0 && bEndOfBlock)
    {
      out_format (_T ("%sData (%I64d bytes)"), sPrefix.c_str (), nMaxSize - nHandledBytes);
      if (debug)
        out_format (_T (" [dbg] at %I64d for %d"), nStartPos + nHandledBytes, aH.nSize);
      out_append (_T ("\n"));
      return;
    }

    // Assumption [ph]: size must be word-aligned
    // See E:\games\hl_14\dmc\sound\player\land2.wav
    nHandledBytes += ALIGN_2 (aH.nSize);
  } while (nHandledBytes < nMaxSize);
}

//--------------------------------------------------------------------
struct WAV_Info
//--------------------------------------------------------------------
{
  gtuint32 m_nTotalSize;            // all
  bool     m_bCompressed;           // "fmt "
  gtint16  m_nCompressionType;      // "fmt "
  gtuint16 m_nChannels;             // "fmt "
  gtuint32 m_nSamplesPerSec;        // "fmt "
  gtuint32 m_nAverageBytesPerSec;   // "fmt "
  gtuint16 m_nBlockAlign;           // "fmt "
  gtuint16 m_nBitsPerSample;        // "fmt "
  gtuint32 m_nDecompressedSize;     // "fact" or "data"
  gtuint32 m_nDataSize;             // "data"

  WAV_Info ()
    : m_nTotalSize          (12),
      m_bCompressed         (false),
      m_nCompressionType    (0),
      m_nChannels           (0),
      m_nSamplesPerSec      (0),
      m_nAverageBytesPerSec (0),
      m_nBlockAlign         (0),
      m_nBitsPerSample      (0),
      m_nDecompressedSize   (0),
      m_nDataSize           (0)
  {}
};

//--------------------------------------------------------------------
static bool _WAVE_GetChunkInfo
                                        (FileBuffer* pBuffer,
                                         file_t&     nOffset,
                                         WAV_Info&   aInfo)
//--------------------------------------------------------------------
{
  // local variables
  RIFF_Header aBasicHeader;
  WAV_fmt fmt;
  gtuint32 nAddSize;

  // try to get the next chunk info
  if (!pBuffer->GetBuffer (nOffset, &aBasicHeader, RIFF_H_SIZE))
    return false;

  switch (aBasicHeader.aID.n)
  {
    case 0x20746d66: // "fmt "
    {
      pBuffer->GetBufferX (&fmt, WAV_FMT_SIZE);

      aInfo.m_nCompressionType    = fmt.nTyp;
      aInfo.m_bCompressed         = (aInfo.m_nCompressionType != 1);
      aInfo.m_nChannels           = fmt.nChannels;
      aInfo.m_nSamplesPerSec      = fmt.nSamplesPerSec;
      aInfo.m_nAverageBytesPerSec = fmt.nAverageBytesPerSec;
      aInfo.m_nBlockAlign         = fmt.nBlockAlign;
      aInfo.m_nBitsPerSample      = fmt.nBitsPerSample;
      break;
    }
    case 0x74636166: // "fact"
    {
      ASSERT (aBasicHeader.nSize == 4);
      if (aInfo.m_bCompressed)
      {
        // get the number of decompressed sample points
        // sample frame = sample point * nChannels
        aInfo.m_nDecompressedSize = (pBuffer->GetInt32 () *
                                     aInfo.m_nChannels *
                                     aInfo.m_nBitsPerSample)
                                     / 1;
      }
      break;
    }
    case 0x61746164: // "data"
    {
      if (!aInfo.m_bCompressed)
        aInfo.m_nDecompressedSize = aBasicHeader.nSize;
      aInfo.m_nDataSize = aBasicHeader.nSize;
      break;
    }
    case 0x4F464E49: // "INFO"
    case 0x50534944: // "DISP"
    case 0x5453494c: // "LIST"
    case 0x61696c61: // "alia"
    case 0x74736e69: // "inst"
    case 0x6b72616d: // "mark"
    case 0x6c706d73: // "smpl"
    {
      // do nothing (yet)
      break;
    }
    default:
    {
      // found an unknown chunk
      if (debug)
      {
        out_error_format (_T ("[dbg] Error: found unknown wave chunk %c%c%c%c\n"),
                          aBasicHeader.aID.s[0],
                          aBasicHeader.aID.s[1],
                          aBasicHeader.aID.s[2],
                          aBasicHeader.aID.s[3]);
      }
      return false;
    }
  }

  // only add even number of bytes!
  nAddSize = RIFF_H_SIZE + aBasicHeader.nSize;
  if (aBasicHeader.nSize & 1)
    nAddSize++;

  // add the size of the current chunk to nTotalSize
  aInfo.m_nTotalSize += nAddSize;
  nOffset += nAddSize;

  return true;
}

//--------------------------------------------------------------------
static void _WAVE_List (const WAV_Info& aInfo)
//--------------------------------------------------------------------
{
  out_incindent ();

  // write channel
  out_format (rc (HRC_GENLIB_RIFF_WAVE_CHANNELS),
              aInfo.m_nChannels);
  switch (aInfo.m_nChannels)
  {
    case 1: out_append (rc (HRC_GENLIB_RIFF_WAVE_MONO)); break;
    case 2: out_append (rc (HRC_GENLIB_RIFF_WAVE_STEREO)); break;
    case 4: out_append (rc (HRC_GENLIB_RIFF_WAVE_QUADRO)); break;
  }
  out_append (_T ("\n"));

  // write sampling rate
  out_format (rc (HRC_GENLIB_RIFF_WAVE_SAMPLING),
              aInfo.m_nSamplesPerSec);
  if (aInfo.m_nSamplesPerSec != 11025 &&
      aInfo.m_nSamplesPerSec != 22050 &&
      aInfo.m_nSamplesPerSec != 44100)
    out_append (rc (HRC_GENLIB_RIFF_WAVE_NON_STD));
  out_append (_T ("\n"));

  // write data size
  out_append (rc (HRC_GENLIB_RIFF_WAVE_DATASIZE));

  if (aInfo.m_bCompressed)
  {
    if (aInfo.m_nBitsPerSample == 0)
    {
      out_error_append (rc (HRC_GENLIB_RIFF_WAVE_0BITS_SMPL));
    }
    else
    {
      out_format (rc (HRC_GENLIB_RIFF_WAVE_SIZE_COMPR),
                  aInfo.m_nDataSize,
                  aInfo.m_nDecompressedSize);
//            sRes += int ((double) 16 / aInfo.m_nBitsPerSample * aInfo.m_nDataSize);
    }
  }
  else
  {
    out_format (rc (HRC_GENLIB_RIFF_WAVE_SIZE),
                aInfo.m_nDataSize);
    if (aInfo.m_nBlockAlign > 1)
    {
      out_format (rc (HRC_GENLIB_RIFF_WAVE_FRAMES),
                  aInfo.m_nDataSize / aInfo.m_nBlockAlign);
    }
    out_append (_T ("\n"));
  }

  out_format (rc (HRC_GENLIB_RIFF_WAVE_BYTES_SEC),
              aInfo.m_nAverageBytesPerSec);

  out_format (rc (HRC_GENLIB_RIFF_WAVE_BITS),
              aInfo.m_nBitsPerSample);
  if (aInfo.m_bCompressed)
  {
    if (aInfo.m_nAverageBytesPerSec == 0)
    {
      // we would have an division by zero
      out_error_append (rc (HRC_GENLIB_RIFF_WAVE_DIVBY0));
    }
    else
    {
      out_format (rc (HRC_GENLIB_RIFF_WAVE_DECOMP_BITS),
                  int (double (4) * aInfo.m_nBitsPerSample * aInfo.m_nSamplesPerSec / aInfo.m_nAverageBytesPerSec));
    }
  }
  out_append (_T ("\n"));

  const double dPlayTime = (double) aInfo.m_nDataSize / aInfo.m_nAverageBytesPerSec;
  const gtuint16 min  = (dPlayTime > 60 ? gtuint16 (dPlayTime / 60) : gtuint16 (0));
  const gtuint16 sec  = (dPlayTime > 60 ? gtuint16 (dPlayTime - (min * 60)) : gtuint16 (dPlayTime));
  const gtuint16 hsec = (dPlayTime > 60 ? gtuint16 ((dPlayTime - (min * 60) - sec) * 100) : gtuint16 ((dPlayTime - sec) * 100));

  out_append (rc (HRC_GENLIB_RIFF_WAVE_PLAYTIME));
  if (min > 0)
  {
    out_format (rc (HRC_GENLIB_RIFF_WAVE_TIME_MIN),
                min, sec, hsec);
  }
  else
  {
    out_format (rc (HRC_GENLIB_RIFF_WAVE_TIME_SEC),
                sec, hsec);
  }
  out_append (_T ("\n"));

  out_decindent ();
}

}  // namespace RIFF

/*! Text or binary
 */
//--------------------------------------------------------------------
MAKE_IS_A (RIFF)
//--------------------------------------------------------------------
{
  return aGI.m_pBuffer->GetInt32 (0) == 0x46464952;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (RIFF)
//--------------------------------------------------------------------
{
  RIFF::WAV_Info aInfo;

  /* goto start position
     read 8 byte
     if extended ID, then
       read 4 byte
       analyse next level
   */
//  const gtuint32 nRIFFSize = m_pBuffer->GetInt32 (4) + 8; // 8 == 2x gtuint32
  const gtuint32 nSubTYPE  = m_pBuffer->GetInt32 (8);

  switch (nSubTYPE)
  {
    case 0x20495641: /*AVI */ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_AVI)); break;
    case 0x36524443: /*CDR6*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDR6)); break;
    case 0x36544443: /*CDT6*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDT6)); break;
    case 0x37524443: /*CDR7*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDR7)); break;
    case 0x37544443: /*CDT7*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDT7)); break;
    case 0x38524443: /*CDR8*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDR8)); break;
    case 0x38544443: /*CDT8*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDT8)); break;
    case 0x39524443: /*CDR9*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDR9)); break;
    case 0x39544443: /*CDT9*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDT9)); break;
    case 0x3A524443: /*CDRA*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDRA)); break;
    case 0x3A544443: /*CDTA*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDTA)); break;
    case 0x3B524443: /*CDRB*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDRB)); break;
    case 0x3B544443: /*CDTB*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDTB)); break;
    case 0x3C524443: /*CDRC*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDRC)); break;
    case 0x3C544443: /*CDTC*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_CDTC)); break;
    case 0x45564157: /*WAVE*/
    {
      out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_WAVE));

      // and finally scan the sub parts
      nOverlayOffset = 12;
      while (RIFF::_WAVE_GetChunkInfo (m_pBuffer, nOverlayOffset, aInfo))
      {
        /* empty */
      }
      nOverlayOffset = aInfo.m_nTotalSize;

      if (aInfo.m_bCompressed)
      {
        aInfo.m_nDecompressedSize = int ((double) 16 / aInfo.m_nBitsPerSample * aInfo.m_nDataSize);
//          aInfo.m_nDecompressedSize = 4.0 * aInfo.m_nBitsPerSample * aInfo.m_nSamplesPerSec / aInfo.m_nAverageBytesPerSec;

        out_format (rc (HRC_GENLIB_RIFF_WAVE_COMPRESSED),
                    aInfo.m_nCompressionType);
      }
      out_append (_T ("\n"));

      // further info
      RIFF::_WAVE_List (aInfo);
      break;
    }
    case 0x6B626673: /*sfbk*/ out_append (rc (HRC_GENLIB_RIFF_SUBTYPE_sfbk)); break;
    default:
    {
      // read 4 bytes at position 8
      pstring s4;
      m_pBuffer->GetFixedSizeAString (8, s4, 4);

      // always char!!
      out_format (rc (HRC_GENLIB_RIFF_SUBTYPE_UNK),
                  s4.c_str ());
      break;
    }
  }  // switch subtype

  // in listmode, we don't need to go any deeper (save time)
  // PH: disabled for 0.25 - too buggy - #f#
  if (0 && !m_bListMode)
  {
    pstring sPrefix;
    RIFF::_SubAnalyze (m_pBuffer,
                       0,                          // offset
                       m_pBuffer->GetFileSize (),  // maxsize
                       sPrefix);                   // currently ""
  }
}
