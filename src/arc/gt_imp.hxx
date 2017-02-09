#define IMP_DIRBLOCK       8192
#define IMP_MAXOVERLAP     524288
#define IMP_MAXFILECOMMENT 78

/*IMP_DIR_ENTRY flags*/
const int IMP_DFLAG_DIR     = 1;
const int IMP_DFLAG_E8SHORT = 2;
const int IMP_DFLAG_E8LONG  = 4;

/*IMP_HEADER flags*/
#define IMP_HFLAG_MULTIVOL 1
#define IMP_HFLAG_LASTVOL  2
#define IMP_HFLAG_BLOWFISH 4

/*methods*/
#define IMP_METHOD_STORE   0
#define IMP_METHOD_LZ77    1
#define IMP_METHOD_BWT     2
#define IMP_METHOD_LZ77_MM 3

/*LZ77*/
#define IMP_NLLCODES      288 /* Number of literal-length codes */
#define IMP_NDCODES       42   /* Number of distance codes */
#define IMP_ND2CODES      14  /* Number of distance codes for length==2 */
#define IMP_ND3CODES      28  /* Number of distance codes for length==3 */
#define IMP_MIN_LZ_CODES  (IMP_NLLCODES + IMP_NDCODES + IMP_ND2CODES)
#define IMP_LONGMATCHBITS 15  /*extra bits for long match*/
#define IMP_LONGMATCHMASK ((1 << IMP_LONGMATCHBITS) - 1)
#define IMP_LONGMATCHCODE (IMP_NLLCODES-2)
#define IMP_MMCODE        (IMP_NLLCODES-1) /*multimedia code*/
#define IMP_LLTTBITS      9
#define IMP_DTTBITS       7
#define IMP_LLTTSIZE      (1 << IMP_LLTTBITS)
#define IMP_DTTSIZE       (1 << IMP_DTTBITS)
#define IMP_MAX_MM_CODES  1024

/*BWT*/
#define IMP_NCODES   258
#define IMP_WINDOW   50
#define IMP_BWTABLES 6
#define IMP_BWTTBITS 9
#define IMP_BWTTSIZE (1 << IMP_BWTTBITS)

#define IMP_MAX_NCODES IMP_NLLCODES
#define IMP_MAXCODELEN 16
#define IMP_NLENCODES  (IMP_MAXCODELEN + 5) /* no. of code length codes */
#define IMP_ZRUN       (IMP_MAXCODELEN + 1) /* run of zeros */
#define IMP_DZRUN      (IMP_MAXCODELEN + 3)/* run of delta-zeros*/
#define IMP_HC_MCL     14    /* max code length for storing Huffman codes */
#define IMP_HCTTBITS   6
#define IMP_HCTTSIZE   (1 << IMP_HCTTBITS)

//--------------------------------------------------------------------
IMPArchiveLister::IMPArchiveLister
                                        (      FileBuffer* pBuffer,
                                         const file_t      nOffset)
//--------------------------------------------------------------------
  : BasicArchiveLister (pBuffer, nOffset)
{
  g_ncodes[0] = IMP_NDCODES;
  g_ncodes[1] = IMP_ND2CODES;
  g_ncodes[2] = IMP_ND3CODES;

  g_ncoffset[0] = IMP_NLLCODES;
  g_ncoffset[1] = IMP_NLLCODES + IMP_NDCODES;
  g_ncoffset[2] = IMP_NLLCODES + IMP_NDCODES + IMP_ND2CODES;
}

//--------------------------------------------------------------------
void IMPArchiveLister::_InitBits ()
//--------------------------------------------------------------------
{
  g_nBitPos = 0;
  g_nBitBuf = 0;
}

//--------------------------------------------------------------------
gtuint32 IMPArchiveLister::_GetBits (const int nBits)
//--------------------------------------------------------------------
{
  if (nBits > g_nBitPos)
    while (g_nBitPos <= 24)
    {
      if (m_pBuffer->GetActFilePos () < m_pBuffer->GetFileSize ())
        g_nBitBuf += m_pBuffer->GetUint8 () << g_nBitPos;
      else
        g_nBitBuf += 0 << g_nBitPos;
      g_nBitPos += 8;
    }

  return g_nBitBuf;
}

//--------------------------------------------------------------------
void IMPArchiveLister::_DelBits (const int nBits)
//--------------------------------------------------------------------
{
  ASSERT (g_nBitPos >= nBits);

  g_nBitPos -= nBits;
  g_nBitBuf >>= nBits;
}

//--------------------------------------------------------------------
void IMPArchiveLister::load_fixed_codes
                                        (gtuint8* hclens,
                                         long     exsize)
//--------------------------------------------------------------------
{
  memset (hclens, 8, 244);
  memset (hclens + 244, 9, 17);
  hclens [256] = 0;
  memset (hclens + 261, 10, 5);
  memset (hclens + 266, 11, IMP_NLLCODES - 266);
  if (exsize <= 8192)
  {
    memset (hclens + IMP_NLLCODES, 4, 4);
    memset (hclens + IMP_NLLCODES + 4, 5, 24);
    memset (hclens + IMP_NLLCODES + 28, 0, IMP_NDCODES - 28);
  }
  else
  {
    memset (hclens + IMP_NLLCODES, 5, 22);
    memset (hclens + IMP_NLLCODES + 22, 6, IMP_NDCODES - 22);
  }
  memset (hclens + IMP_NLLCODES + IMP_NDCODES, 4, IMP_ND2CODES);
  hclens [IMP_NLLCODES + IMP_NDCODES] = 3;
  hclens [IMP_NLLCODES + IMP_NDCODES + 1] = 3;
}

//--------------------------------------------------------------------
void IMPArchiveLister::make_codes
                                        (gtuint32* huffcodes,
                                         gtuint8*  hclengths,
                                         int       ncodes)
//--------------------------------------------------------------------
{
  gtuint32 i, k,count[IMP_MAXCODELEN + 1], nextcode [IMP_MAXCODELEN + 1];

  memset (count, 0, sizeof(count));

  for (i = 0; i < (gtuint32) ncodes; i++)
    count[hclengths[i]]++;
  count[0] = 0;

  for (i = 1, k = 0; i <= IMP_MAXCODELEN; i++)
  {
    k = (k + count [i - 1]) << 1;
    nextcode[i] = k;
  }
  for (i = 0; i < (gtuint32) ncodes; i++)
  {
    gtuint32 j = hclengths[i];
    if (j > 0)
    {
      gtuint32 l = 0;
      k = nextcode[j]++;
      while (j > 0)
      {
        l <<= 1;
        l += k & 1;
        k >>= 1;
        j--;
      }
      huffcodes[i] = l;
    }
  }
}

//--------------------------------------------------------------------
int IMPArchiveLister::make_decode_tables
                                        (int*      ttable,
                                         int       hdchain[][2],
                                         gtuint32* huffcodes,
                                         gtuint8*  hclengths,
                                         int       ttbits,
                                         int       ncodes)
//--------------------------------------------------------------------
{
  int hdnode,hdndest,i,ttsize;
  gtuint32 base,n,hcode;

  ttsize = 1 << ttbits;
  if (hdchain)
    for (i = 0; i < ncodes; i++)
      hdchain[i][0] = hdchain[i][1] = -IMP_MAX_NCODES;

  for (i = 0; i < ttsize; i++)
    ttable [i] = -IMP_MAX_NCODES;
  hdndest = 1;

  for (i = 0; i < ncodes; i++)
    if (hclengths[i])
    {
      base = 1 << hclengths[i];
      n = huffcodes[i];
      if (base <= (gtuint32) ttsize)
      {
        while (n < (gtuint32) ttsize) /*n should be less than ttsize but it might be invalid*/
        {
          ttable [n] = i;
          n += base;
        }
      }
      else
        if (hdchain)
        {
          base = hclengths[i] - ttbits - 1;
          n &= ttsize - 1;
          hcode = huffcodes[i] >> ttbits;
          if (ttable [n] == -IMP_MAX_NCODES)
          {
            ttable [n] =- hdndest;
            hdnode = hdndest++;
          }
          else
          {
            hdnode = -ttable[n];
            while (hdchain[hdnode][hcode & 1] >= 0)
            {
              hdnode = hdchain [hdnode] [hcode & 1];
              hcode >>= 1;
              base--;
            }
          }

          while (base--)
          {
            if (hdndest == ncodes)
              return -1;
            hdchain [hdnode] [hcode & 1] = hdndest;
            hdnode = hdndest++;
            hcode >>= 1;
          }
          hdchain[hdnode] [hcode & 1] = -i;
        }
    }
  return 0;
}

//--------------------------------------------------------------------
int IMPArchiveLister::load_hcodes
                                        (gtuint8* hclengths,
                                         int      ncodes,
                                         int      ntrees,
                                         long     exsize)
//--------------------------------------------------------------------
{
  int i, j, dzrpt, zrpt, len, hcode, code;
  gtuint8 hlclens [IMP_NLENCODES];
  gtuint8 *hcldest, *hclend, *hclrpt;
  gtuint32 huffcodes [IMP_NLENCODES];
  int ttable [IMP_HCTTSIZE];
  int hdchain [IMP_NLENCODES] [2];

  if (!ntrees)
  {
    load_fixed_codes (hclengths, exsize);
    return 0;
  }
  memset (hlclens, 0, IMP_NLENCODES);
  for (i = 0, j = 1; i < IMP_NLENCODES;)
  {
    code = _GetBits  (5) & 31;
    _DelBits (5);
    if (code < 2)
    {
      i += j << code;
      j <<= 1;
    }
    else
    {
      if (code > IMP_MAXCODELEN + 1)
        return -1;
      hlclens [i++] = gtuint8 (code - 1);
      j = 1;
    }
  }
  make_codes (huffcodes, hlclens, IMP_NLENCODES);
  make_decode_tables (ttable,
                      hdchain,
                      huffcodes,
                      hlclens,
                      IMP_HCTTBITS,
                      IMP_NLENCODES);
  hcldest = hclengths;
  hclend = hclengths + ncodes*ntrees;
  hclrpt = hclengths + ncodes;
  len = 0;
  dzrpt = 1;
  zrpt = 1;
  while (hcldest < hclend)
  {
    code = ttable [_GetBits (IMP_HCTTBITS)& (IMP_HCTTSIZE - 1)];
    if (code < 0)
    {
      if (code == -IMP_MAX_NCODES)
        return -1;
      hcode = _GetBits (IMP_MAXCODELEN) >> IMP_HCTTBITS;
      code = -code;
      do
      {
        code = hdchain [code] [hcode&1];
        hcode >>= 1;
      }
      while (code > 0);
      if (code == -IMP_MAX_NCODES)
        return -1;
      code = -code;
    }
    _DelBits (hlclens [code]);
    if (code >= IMP_DZRUN)
    {
      code = dzrpt << (code - IMP_DZRUN);
      while (code-- && hcldest < hclend)
      {
        if (hcldest >= hclrpt && hcldest [-ncodes])
          len = hcldest [-ncodes];
        *hcldest++ = (gtuint8) len;
      }
      dzrpt <<= 1;
      zrpt = 1;
    }
    else
    if (code >= IMP_ZRUN)
    {
      code = zrpt << (code - IMP_ZRUN);
      while (code-- && hcldest < hclend)
        *hcldest++ = 0;
      zrpt <<= 1;
      dzrpt = 1;
    }
    else
    {
      if (hcldest >= hclrpt && hcldest [-ncodes])
        len = code + hcldest [-ncodes];
      else
        len+=code;
      if (len > IMP_MAXCODELEN)
        len -= IMP_MAXCODELEN + 1;
      *hcldest++ = (gtuint8) len;
      dzrpt = 1;
      zrpt = 1;
    }
  }
  for (hcldest = hclengths; hcldest < hclend; hcldest++)
    if (*hcldest > IMP_MAXCODELEN)
      return -1;
  return 0;
}

//--------------------------------------------------------------------
int IMPArchiveLister::expand_lz
                                        (gtuint8* output,
                                         int      start,
                                         int      exsize)
//--------------------------------------------------------------------
{
  gtuint32 huffcodes [IMP_MAX_NCODES];
  int      llcode_table [IMP_LLTTSIZE];
  int      distcode_table [3] [IMP_DTTSIZE];
  int      llcode_chain [IMP_NLLCODES] [2];
  int      distcode_chain [3] [IMP_NDCODES] [2];
  gtuint32 dist_base [IMP_NDCODES-6];
  gtuint32 dist_mask [IMP_NDCODES-6];
  gtuint8  dist_extra [IMP_NDCODES-6];
  gtuint8  len_base [IMP_NLLCODES-268];
  gtuint8  len_mask [IMP_NLLCODES-268];
  gtuint8  len_extra [IMP_NLLCODES-268];
  gtuint8* dest;
  gtuint8* output_end;
  int      i,j,treenum,code,num_dist_trees,dist_split,numcodes,numtrees, len;
  gtuint32 hcode,dist,last_dist,last_dist2;
  gtuint8* code_lengths,*codelen_src;

  num_dist_trees = _GetBits (2) & 3;
  _DelBits (2);
  if (!num_dist_trees)
    return -1;

  dist_split = num_dist_trees > 2;
  numcodes = IMP_NLLCODES + IMP_NDCODES + IMP_ND2CODES;
  if (dist_split)
    numcodes += IMP_ND3CODES;
  dest = output + start;
  output_end = dest + exsize;

  numtrees = _GetBits (6) & 63;
  _DelBits (6);
  if (numtrees)
    code_lengths = (gtuint8*) malloc (numcodes * numtrees);
  else
    code_lengths = (gtuint8*) malloc (IMP_MIN_LZ_CODES);

  if (!code_lengths)
    return -1;

  i = load_hcodes (code_lengths, numcodes, numtrees, start + exsize);
  if (i < 0)
  {
    free (code_lengths);
    return i;
  }

  for (i = 2, len = 11, code = 0; code < IMP_NLLCODES - 268; i <<= 1)
    for (j = code + 4; code < j; len += i)
      len_base[code++] = (gtuint8) len;

  for (i = 1, code = 0; code < IMP_NLLCODES - 268; i++)
    for (j = code + 4; code < j; code++)
      len_extra[code] = (gtuint8) i;

  for (code = 0, len = 1; code < IMP_NLLCODES - 268; len = (len << 1) + 1)
    for (j = code + 4; code < j; code++)
      len_mask[code] = (gtuint8) len;

  for (hcode = 2, dist = 5, code = 0; code < IMP_NDCODES - 6; hcode <<= 1)
  {
    dist_base[code] = dist;
    dist += hcode;
    dist_base[code + 1] = dist;
    dist += hcode;
    code += 2;
  }

  for (i = 1, code = 0; code < IMP_NDCODES - 6; i++, code += 2)
    dist_extra[code] = dist_extra[code + 1] = (gtuint8) i;

  for (code = 0, dist = 1; code < IMP_NDCODES - 6; dist = (dist << 1) + 1, code += 2)
    dist_mask[code] = dist_mask[code + 1] = dist;

  codelen_src = code_lengths - numcodes;
  last_dist = 0;
  last_dist2 = 0;
  code = 256;
  numtrees += !numtrees;
  while (dest < output_end)
  {
    if (code == 256)
    {
      if(!numtrees)
        break;
      numtrees--;
      codelen_src += numcodes;
      make_codes (huffcodes, codelen_src, IMP_NLLCODES);
      make_decode_tables (llcode_table,
                          llcode_chain,
                          huffcodes,
                          codelen_src,
                          IMP_LLTTBITS,
                          IMP_NLLCODES);
      codelen_src += IMP_NLLCODES;
      for (i = 0; i < num_dist_trees; i++)
      {
        make_codes (huffcodes, codelen_src, g_ncodes[i]);
        make_decode_tables (distcode_table[i],
                            distcode_chain[i],
                            huffcodes,
                            codelen_src,
                            IMP_DTTBITS,
                            g_ncodes[i]);
        codelen_src += g_ncodes[i];
      }
      codelen_src -= numcodes;
    }
    code = llcode_table[_GetBits (IMP_LLTTBITS) & (IMP_LLTTSIZE - 1)];
    if (code < 0)
    {
      if (code == -IMP_MAX_NCODES)
        break;
      hcode = _GetBits (IMP_MAXCODELEN)>>IMP_LLTTBITS;
      code = -code;
      do
      {
        code = llcode_chain [code] [hcode & 1];
        hcode >>= 1;
      } while(code > 0);

      if (code == -IMP_MAX_NCODES)
        break;
      code = -code;
    }
    _DelBits (codelen_src[code]);
    if (code < 256)
      *dest++ = (gtuint8) code;
    else
    if (code > 256)
    {
      if (code < 266)
        len = code - 255;
      else
      if (code < IMP_LONGMATCHCODE)
      {
        len = _GetBits(5);
        _DelBits (len_extra [code - 266]);
        len = (len & len_mask [code - 266]) + len_base [code - 266];
      }
      else
      if (code == IMP_LONGMATCHCODE)
      {
        len = (_GetBits (IMP_LONGMATCHBITS) & IMP_LONGMATCHMASK) + 259;
        _DelBits (IMP_LONGMATCHBITS);
      }
      else
      {
        _GetBits (3);
        _DelBits (3);
        continue;
      }

      treenum = (len == 2) + ((dist_split & (len == 3)) << 1);
      code = distcode_table[treenum][_GetBits (IMP_DTTBITS) & (IMP_DTTSIZE - 1)];
      if (code < 0)
      {
        if (code == -IMP_MAX_NCODES)
          break;
        hcode = _GetBits (IMP_MAXCODELEN) >> IMP_DTTBITS;
        code = -code;
        do
        {
          code = distcode_chain[treenum][code][hcode&1];
          hcode >>= 1;
        } while (code > 0);

        if (code == -IMP_MAX_NCODES)
          break;
        code =- code;
      }
      _DelBits (codelen_src [code + g_ncoffset [treenum]]);
      if (!code)
        dist=last_dist;
      else
      if (code == 1)
      {
        dist = last_dist2;
        last_dist2 = last_dist;
        last_dist = dist;
      }
      else
      {
        if (code < 6)
          dist = code - 1;
        else
        {
          dist = _GetBits (18);
          _DelBits (dist_extra [code - 6]);
          dist = (dist & dist_mask[code - 6]) + dist_base [code - 6];
          last_dist2 = last_dist;
          last_dist = dist;
        }
      }

      if (dist <= gtuint32 (dest - output))
        while (len-- && dest < output_end)
          *dest++ = dest [-dist];
    }
  }
  free (code_lengths);
  return dest - output - start;
}

//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (IMP)
//--------------------------------------------------------------------
{
  const gtuint8 IMP_DIR_SIGNATURE [6] = { 0x49, 0x4D, 0x50, 0x44, 0x45, 0 };
  IMP_GlobalHeader aGH;
  pstring sFilename;

  _SetArchiveFlag (ARC_DT_DOS | ARC_NO_PACKED_SIZE);

  // read global header...
  m_pBuffer->GetBuffer (m_nLocalOffset + 0, &aGH, GT_IMP_GH_SIZE);

  {
    // to calc the CRC the CRC field must be zerored!!
    const gtuint16 nSavedCRC16 = aGH.nCRC16;
    aGH.nCRC16 = 0x0000;

    // check the header CRC
    const gtuint32 nCRC32 = CRC32::Get (&aGH, GT_IMP_GH_SIZE);
    if (gtuint16 (~(nCRC32 & 0xFFFF)) != nSavedCRC16)
    {
      // CRC16 on header failed...
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);      return;
    }
  }

  if (aGH.nCommentOffset)
    _SetArchiveFlag (ARC_COMMENT);

  if (aGH.nFlags & IMP_HFLAG_MULTIVOL)
    _SetArchiveFlag (ARC_MULTI_VOLUME);

  if (aGH.nFlags & IMP_HFLAG_BLOWFISH)
    _SetArchiveFlag (ARC_ENCRYPTED);

  // get the directory structure
  char sDirectorySignature[6];
  m_pBuffer->GetBufferX (m_nLocalOffset + aGH.nDataOffset, sDirectorySignature, 6);

  // compare it to the saved value
  if (memcmp (sDirectorySignature, IMP_DIR_SIGNATURE, 6) != 0)
  {
    SetErrorCode (ARCERR_UNEXPECTED_VALUE);    return;
  }

// extract_dir
  char g_dirbuf [IMP_DIRBLOCK];
  char *g_dirbufsrc = g_dirbuf;
  char *g_dirbufend = g_dirbuf;

  for (gtuint32 x = 0; x < aGH.nEntriesInDataOffset; x++)
  {
    if (g_dirbufsrc >= g_dirbufend)
    {
      g_dirbufsrc = g_dirbuf;
      g_dirbufend = g_dirbuf;

      file_t nTargetFilePos = m_pBuffer->GetActFilePos ();

      _InitBits ();

      int nMethod = _GetBits (5) & 15;
      _DelBits (5);

      int nExtractedSize = _GetBits (20) & ((1L << 20) - 1);
      _DelBits (20);

      int nCompressedSize = _GetBits (20) & ((1L << 20) - 1);
      _DelBits (20);

      if (nMethod == IMP_METHOD_STORE)
      {
        m_pBuffer->GetBuffer (g_dirbuf, nExtractedSize);
        g_dirbufend += nExtractedSize;
      }
      else
      if (nMethod == IMP_METHOD_LZ77)
      {
        _DelBits (5);
        g_dirbufend += expand_lz ((gtuint8*) g_dirbuf, 0, nExtractedSize);
      }
      else
      {
        // IMP_METHOD_BWT || IMP_METHOD_LZ77_MM || ???
        // unknown/unsupported method ...
        SetErrorCode (ARCERR_UNKNOWN_METHOD);
        break;
      }

      nTargetFilePos += nCompressedSize;
      if (nTargetFilePos < m_pBuffer->GetFileSize ())
        nTargetFilePos += 6;
      m_pBuffer->SetActFilePos (nTargetFilePos);
    }

    // now get the current dir entry
    IMP_LocalHeader aLH;
    memcpy (&aLH, g_dirbufsrc, GT_IMP_LH_SIZE);

    // calcualte the CRC16 size
    const int nLHSize = GT_IMP_LH_SIZE + aLH.nNameLen + aLH.nCommentLen + aLH.extra;

/*
    // mark the CRC16 fields as 0
    g_dirbufsrc[36] = 0;
    g_dirbufsrc[37] = 0;

    // and check the CRC
    nCRC32 = CRC32::Get (g_dirbufsrc, nLHSize);
    if((~nCRC32 & 0xFFFF) != aLH.entcrc)
    {
      out_format ("  CRC16 check on local header failed (%04Xh - %04Xh)\n", ~nCRC32 & 0xFFFF, aLH.entcrc);
      SetErrorCode (ARCERR_UNEXPECTED_VALUE);
      break;
    }
*/

    // if it is no directory, list it....
    // I just commented the following line to test
    //   the dynamic directory output
//    if (!(aLH.nFlags & IMP_DFLAG_DIR))
    {
      // get the name and convert it
      char sName [_MAX_PATH];
      memcpy (sName, g_dirbufsrc + GT_IMP_LH_SIZE, aLH.nNameLen);
      sName [aLH.nNameLen] = '\0';

      str_assign (sFilename, sName);

/*
      // get the comment - if any
      char sComment [IMP_MAXFILECOMMENT + 1];
      memcpy (sComment, g_dirbufsrc + GT_IMP_LH_SIZE + aLH.nNameLen, aLH.nCommentLen);
      sComment [aLH.nCommentLen] = '\0';
*/

      ArchiveFile *pArcFile = new ArchiveFile (sFilename);
      pArcFile->SetUnpackedSize (aLH.nOrigSize);
      pArcFile->SetDT (aLH.nFileDT);
      pArcFile->SetDirectory (aLH.nFlags & IMP_DFLAG_DIR);
      pArcFile->SetComment (aLH.nCommentLen > 0);
      _AddFile (pArcFile);
    }

//    m_pBuffer->IncActFilePos (nLHSize);
    g_dirbufsrc += nLHSize;
  }
}
