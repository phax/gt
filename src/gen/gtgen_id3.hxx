MAKE_ANALYZER (ID3);

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (ID3)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  // Version or revision will never be $FF
  // the header "UUU" seems to be equal to "ID3"
  return (aGI.m_pBuffer->CompareA (0, 3, "ID3") ||
          aGI.m_pBuffer->CompareA (0, 3, "UUU")) &&
         aGI.m_pBuffer->GetUint8 (3) != 0xff &&
         aGI.m_pBuffer->GetUint8 (4) == 0x00;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (ID3)
//--------------------------------------------------------------------
{
  ID3_GlobalHeader aGH;
  ID3_FrameHeader aFH;

  // read 10 bytes header
  m_pBuffer->GetBufferX (0, &aGH, ID3_GH_SIZE);

  // all data is stored in big endian
  aGH.nSize = LONG_SWAP_BYTE (aGH.nSize);

/*
   The ID3v2 tag size is encoded with four bytes where the most
   significant bit (bit 7) is set to zero in every byte, making a total
   of 28 bits. The zeroed bits are ignored, so a 257 bytes long tag is
   represented as $00 00 02 01.

   The ID3v2 tag size is the size of the complete tag after
   unsychronisation, including padding, excluding the header but not
   excluding the extended header (total tag size - 10). Only 28 bits
   (representing up to 256MB) are used in the size description to avoid
   the introducuction of 'false syncsignals'.
*/
  // aGH.nFields is swapped when aGH.nSize is swapped
  const file_t nSize = ID3_GH_SIZE +
                        (aGH.nFields[0] & 0x7f) +
                       ((aGH.nFields[1] & 0x7f) << 7) +
                       ((aGH.nFields[2] & 0x7f) << 14) +
                       ((aGH.nFields[3] & 0x7f) << 21);

  out_format (rc (HRC_GENLIB_ID3_HEADER),
              aGH.nVersion,
              aGH.nRevision,
              nSize);

  // set overlay offset
  nOverlayOffset = nSize;

  if (!m_bListMode)
  {
    out_incindent ();

    // write header flags
    if (aGH.nFlags & BITS[0])
      out_error_append (rc (HRC_GENLIB_ID3_FLAG_UNSYNC));
    if (aGH.nFlags & BITS[1])
      out_append (rc (HRC_GENLIB_ID3_FLAG_EXT_HEAD));
    if (aGH.nFlags & BITS[2])
      out_append (rc (HRC_GENLIB_ID3_FLAG_EXPERIMENTAL));
    if (aGH.nFlags & BITS[3])
      out_append (rc (HRC_GENLIB_ID3_FLAG_FOOTER));

    // list all frames
    file_t nCurPos = ID3_GH_SIZE;
    while (nCurPos < nSize)
    {
      // read 10 bytes
      m_pBuffer->GetBufferX (nCurPos, &aFH, ID3_FH_SIZE);

      // if the first ID byte is zero - lets stop
      if (aFH.sID[0] == 0)
        break;

      // big endian!
      // The frame header size is not packed!!
      aFH.nSize = LONG_SWAP_BYTE (aFH.nSize);

      // print it
      out_format (rc (HRC_GENLIB_ID3_FRAME),
                  aFH.sID[0],
                  aFH.sID[1],
                  aFH.sID[2],
                  aFH.sID[3],
                  aFH.nSize);

      // goto next
      nCurPos += ID3_FH_SIZE + aFH.nSize;
    }

    out_decindent ();
  }
}
