MAKE_ANALYZER (MP3);

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (MP3)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  const gtuint8 b0 = aGI.m_pBuffer->GetUint8 (0);
  const gtuint8 b1 = aGI.m_pBuffer->GetUint8 (1);
  const gtuint32 l0 = aGI.m_pBuffer->GetInt32 (0);

  // [ph] 2004/03/06 Added Textpad check for imageviewer.tws
  return b0 == 0xFF &&
         (b1 >> 4) == 0x0F &&
         l0 != 0xFFFFFFFF &&
         !aGI.m_pBuffer->CompareA (6, 7, "TextPad");
}

//--------------------------------------------------------------------
MAKE_DISPLAY (MP3)
//--------------------------------------------------------------------
{
  // ripped from Typ by Veit Kannegieser - thanx man!
  const size_t MP3Table[2][3][15] = {
    {
      { 0, 4, 8,12,16,20,24,28,32,36,40,44,48,52,56 },
      { 0, 4, 6, 7, 8,10,12,14,16,20,24,28,32,40,48 },
      { 0, 4, 5, 6, 7, 8,10,12,14,16,20,24,28,32,40 }
    },
    {
      { 0, 4, 6, 7, 8,10,12,14,16,18,20,22,24,28,32 },
      { 0, 1, 2, 3, 4, 5, 6, 7, 8,10,12,14,16,18,20 },
      { 0, 1, 2, 3, 4, 5, 6, 7, 8,10,12,14,16,18,20 }
    }
  };

  /*

  Byte 0: 01234567
          11111111 [ID]
  Byte 1: 01234567
          1111     [ID]
           11      [layer]
             1     [version]
                 1 [error protection]
  Byte 2: 01234567
              1111 [table index]
  Byte 3: 01234567
                11 [mode]

  */

  const int    nLayer      = 4 - ((m_pBuffer->GetUint8 (1) >> 1) & 3);  // 4 - [0..3]
  const int    nVersion    = 2 - ((m_pBuffer->GetUint8 (1) >> 3) & 1);  // 2 - [0..1]
  const bool   bErrorProt  = m_pBuffer->GetUint8 (1) & BITS[7];         // [0..1]
  const size_t nTableIndex = m_pBuffer->GetUint8 (2) >> 4;              // [0..15]
  const size_t nMode       = m_pBuffer->GetUint8 (3) >> 6;              // [0..3]

  out_format (rc (HRC_GENLIB_MP3_HEADER),
              nVersion, nLayer);

  if (!m_bListMode)
  {
    out_incindent ();

    // bitrate
    out_format (rc (HRC_GENLIB_MP3_BITRATE),
                MP3Table[nVersion - 1][nLayer - 1][nTableIndex] * 8);

    // error protection: [yes|no]
    out_append (rc (HRC_GENLIB_MP3_ERROR_PROT));
    out_append (bErrorProt
                 ? rc (HRC_YES)
                 : rc (HRC_NO));
    out_append (_T ("\n"));

    // audio mode
    out_append (nMode == 0 ? rc (HRC_GENLIB_MP3_STEREO) :
                nMode == 1 ? rc (HRC_GENLIB_MP3_JOINT_STEREO) :
                nMode == 2 ? rc (HRC_GENLIB_MP3_TWO_CHANNELS) :
                             rc (HRC_GENLIB_MP3_MONO));
    out_append (_T ("\n"));

    out_decindent ();
  }

  // MP3 tag present?
  if (!m_bListMode && m_pBuffer->GetFileSize () > MP3TAG_SIZE)
  {
    char sID[3];
    m_pBuffer->GetBufferX (m_pBuffer->GetFileSize () - MP3TAG_SIZE, sID, 3);
    if (memcmp (sID, "TAG", 3) == 0)
      nOverlayOffset = m_pBuffer->GetFileSize () - MP3TAG_SIZE;
  }
}
