MAKE_ANALYZER (JPG);

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (JPG)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetUint16 (0) == 0xD8FF;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (JPG)
//--------------------------------------------------------------------
{
  const gtuint8 JPG_SOF0 = 0xC0;  // start of image
  const gtuint8 JPG_SOF1 = 0xC1;  // start of image
  const gtuint8 JPG_SOF2 = 0xC2;  // start of image
//  const gtuint8 JPG_DHT  = 0xC4;  // define Huffman table
//  const gtuint8 JPG_DAC  = 0xCC;  // define arithmetic table
//  const gtuint8 JPG_RST0 = 0xD0;  // RSTn are used for resync
//  const gtuint8 JPG_RST1 = 0xD1;  // RSTn are used for resync
//  const gtuint8 JPG_RST2 = 0xD2;  // RSTn are used for resync
//  const gtuint8 JPG_RST3 = 0xD3;  // RSTn are used for resync
//  const gtuint8 JPG_RST4 = 0xD4;  // RSTn are used for resync
//  const gtuint8 JPG_RST5 = 0xD5;  // RSTn are used for resync
//  const gtuint8 JPG_RST6 = 0xD6;  // RSTn are used for resync
//  const gtuint8 JPG_RST7 = 0xD7;  // RSTn are used for resync
//  const gtuint8 JPG_SOI  = 0xD8;  // start of image
//  const gtuint8 JPG_EOI  = 0xD9;  // end of image
//  const gtuint8 JPG_SOS  = 0xDA;  // start of scan
//  const gtuint8 JPG_DQT  = 0xDB;  // define quantization table
//  const gtuint8 JPG_DNL  = 0xDC;
//  const gtuint8 JPG_DRI  = 0xDD;  // define restart interval
//  const gtuint8 JPG_DHP  = 0xDE;
//  const gtuint8 JPG_EXP  = 0xDF;
//  const gtuint8 JPG_APP0 = 0xE0;  // JFIF APP0 segment marker
//  const gtuint8 JPG_APPF = 0xEF;
//  const gtuint8 JPG_JPG0 = 0xF0;
//  const gtuint8 JPG_JPGD = 0xFD;
  const gtuint8 JPG_COM  = 0xFE;  // comment

  file_t nActPos = 2;
  bool bFound = false;
  pstring sComment;
  JPG_Block aBlock;
  JPG_SOF aSOF;

  for (;;)
  {
    if (!m_pBuffer->GetBuffer (nActPos, &aBlock, JPG_B_SIZE))
      break;

    // valid block?
    if (aBlock.nID != 0xFF)
    {
      out_error_append (rc (HRC_GENLIB_JPG_INVALID_ID));
      break;
    }

    switch (aBlock.nType)
    {
      case JPG_SOF0:
      case JPG_SOF1:
      case JPG_SOF2:
      {
        m_pBuffer->GetBufferX (nActPos, &aSOF, JPG_S_SIZE);

        out_format (rc (HRC_GENLIB_JPG_HEADER),
                    WORD_SWAP_BYTE (aSOF.nWidth),
                    WORD_SWAP_BYTE (aSOF.nHeight));
        if (aSOF.nComponents == 1)
          out_append (rc (HRC_GENLIB_JPG_COL_GREYSCALED));
        else
        if (aSOF.nComponents == 3)
          out_append (rc (HRC_GENLIB_JPG_COL_TRUE24));
        else
        if (aSOF.nComponents == 4)
          out_append (rc (HRC_GENLIB_JPG_COL_TRUE32));
        else
          out_append (rc (HRC_GENLIB_JPG_COL_UNKNOWN));

        out_append (_T ("\n"));
        bFound = true;
        break;
      }
      case JPG_COM:
      {
        const gtuint16 nCommentLen = WORD_SWAP_BYTE (aBlock.nSize) - 1;

        // read comment string
        m_pBuffer->GetFixedSizeAString (nActPos + JPG_B_SIZE, sComment, nCommentLen);
        break;
      }
    }

    if (bFound)
      break;

    nActPos += (2 + WORD_SWAP_BYTE (aBlock.nSize));
  }

  if (!bFound)
    out_error_append (rc (HRC_GENLIB_JPG_NO_SOF));

  if (!m_bListMode)
  {
    out_incindent ();

    if (!sComment.empty ())
    {
      // search byte 0xA8
      size_t n = sComment.find_first_of (gtuint8 (0xA8));
      if (n != pstring::npos)
      {
        // append name for n bytes
        pstring sTemp (sComment, n);
        // add blank
        sTemp.push_back (_T (' '));
        // skip 0xa8 and append version (3 chars)
        pstring::iterator it = sComment.begin () + n + 2;
        sTemp.append (it, it + 3);
        // copy back
        sComment = sTemp;
      }
      else
      {
        // if the sring contains no \n -> replace all '\r' with '\n'
        if (sComment.find_first_of (_T ('\n')) == pstring::npos)
          str_replace_all (sComment, _T ('\r'), _T ('\n'));

        // 0xff means "end of comment"
        n = sComment.find_first_of (TCHAR (0xFF));
        if (n != pstring::npos)
          sComment.erase (n);
      }

      out_format (rc (HRC_GENLIB_JPG_COMMENT),
                  sComment.c_str ());
    }

    out_decindent ();
  }
}

