MAKE_ANALYZER (XPT);

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (XPT)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->CompareA (0, 16, "XPCOM\nTypeLib\r\n\032");
}

//--------------------------------------------------------------------
MAKE_DISPLAY (XPT)
//--------------------------------------------------------------------
{
  // get file header from start
  XPT_Header aH;
  m_pBuffer->GetBufferX (0, &aH, XPT_H_SIZE);

  // headline
  out_format (rc (HRC_GENLIB_XPT_HEADER),
              aH.nMajorVersion,
              aH.nMinorVersion);

  if (!m_bListMode)
  {
    out_incindent ();

    // big endian...
    aH.nInterfaces = WORD_SWAP_BYTE (aH.nInterfaces);
    aH.nFileLength = LONG_SWAP_BYTE (aH.nFileLength);
    aH.nIDEOffset  = LONG_SWAP_BYTE (aH.nIDEOffset);

    if (file_t (aH.nFileLength) != m_pBuffer->GetFileSize ())
    {
      out_error_format (rc (HRC_GENLIB_XPT_SIZE_MISMATCH),
                        aH.nFileLength,
                        m_pBuffer->GetFileSize ());
    }
    else
    {
      out_format (rc (HRC_GENLIB_XPT_INTERFACES),
                  aH.nInterfaces);

      // size matches - read the whole file into memory!!!
      gtuint8* pXPT = new gtuint8 [aH.nFileLength];
      m_pBuffer->GetBufferX (0, pXPT, aH.nFileLength);

      // start working
      gtuint8* p = pXPT + XPT_H_SIZE;

      if (0) {
      // read annotations
      gtuint8 nFlags;
      do
      {
        nFlags = *p++;
        if (nFlags & 0x40)
        {
          // private flag is set
          // -> read 2 strings
          gtuint16 nLen = WORD_SWAP_BYTE (*((gtuint16*) p));
          p += 2;
          p += nLen;
          out_format (_T ("private string for %d bytes\n"), nLen);
          nLen = WORD_SWAP_BYTE (*((gtuint16*) p));
          p += 2;
          p += nLen;
          out_format (_T ("private string for %d bytes\n"), nLen);
        }
      } while (!(nFlags & 0x80));

      // read interface directory entries
      p = pXPT + aH.nIDEOffset;
      }

      // free memory again
      delete [] pXPT;
    }

    out_decindent ();
  }
}
