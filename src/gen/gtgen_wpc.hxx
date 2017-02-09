MAKE_ANALYZER (WPC);

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (WPC)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x435057FF;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (WPC)
//--------------------------------------------------------------------
{
  WPC_Header aH;
  m_pBuffer->GetBufferX (0, &aH, WPC_H_SIZE);

  pstring sType;
  switch (aH.nFileType)
  {
    case 0x01: str_assign_from_res (sType, HRC_GENLIB_WPC_01); break;
    case 0x02: str_assign_from_res (sType, HRC_GENLIB_WPC_02); break;
    case 0x03: str_assign_from_res (sType, HRC_GENLIB_WPC_03); break;
    case 0x0A: str_assign_from_res (sType, HRC_GENLIB_WPC_0A); break;
    case 0x0B: str_assign_from_res (sType, HRC_GENLIB_WPC_0B); break;
    case 0x0C: str_assign_from_res (sType, HRC_GENLIB_WPC_0C); break;
    case 0x0D: str_assign_from_res (sType, HRC_GENLIB_WPC_0D); break;
    case 0x0E: str_assign_from_res (sType, HRC_GENLIB_WPC_0E); break;
    case 0x0F: str_assign_from_res (sType, HRC_GENLIB_WPC_0F); break;
    case 0x10: str_assign_from_res (sType, HRC_GENLIB_WPC_10); break;
    case 0x11: str_assign_from_res (sType, HRC_GENLIB_WPC_11); break;
    case 0x12: str_assign_from_res (sType, HRC_GENLIB_WPC_12); break;
    case 0x13: str_assign_from_res (sType, HRC_GENLIB_WPC_13); break;
    case 0x14: str_assign_from_res (sType, HRC_GENLIB_WPC_14); break;
    case 0x15: str_assign_from_res (sType, HRC_GENLIB_WPC_15); break;
    case 0x16: str_assign_from_res (sType, HRC_GENLIB_WPC_16); break;  /* WPG */
    case 0x17: str_assign_from_res (sType, HRC_GENLIB_WPC_17); break;
    case 0x18: str_assign_from_res (sType, HRC_GENLIB_WPC_18); break;
    case 0x19: str_assign_from_res (sType, HRC_GENLIB_WPC_19); break;
    case 0x1A: str_assign_from_res (sType, HRC_GENLIB_WPC_1A); break;
    case 0x1B: str_assign_from_res (sType, HRC_GENLIB_WPC_1B); break;
    case 0x46: str_assign_from_res (sType, HRC_GENLIB_WPC_46); break;
    default:
    {
      str_assign_from_res (sType, HRC_GENLIB_WPC_UNK);
      str_append_format (sType, _T (" (%02Xh)"), aH.nFileType);
      break;
    }
  }

  // write WPC header
  out_format (rc (HRC_GENLIB_WPC_HEADER),
              sType.c_str ());
  if (aH.nEncryptionKey != 0)
    out_append (rc (HRC_GENLIB_WPC_ENCRYPTED));
  out_append (_T ("\n"));

  // is it a WPG graphic file?
  if (!m_bListMode && aH.nFileType == 0x16)
  {
    out_incindent ();

    while (!m_pBuffer->IsEOF ())
    {
      file_t nFilePos = m_pBuffer->GetActFilePos ();
      gtuint8 nRecordType = m_pBuffer->GetUint8 ();
      gtint32 nRecordLength = m_pBuffer->GetUint8 ();
      int nLengthBytes = 1;
      if (nRecordLength == 0x000000FF)
      {
        nRecordLength = m_pBuffer->GetUint16 ();
        nLengthBytes = 2;
        if (HI_BYTE (gtuint16 (nRecordLength)) == 0x80)
        {
          nRecordLength &= 0x00FF;
          nRecordLength <<= 16;
          nRecordLength += m_pBuffer->GetUint16 ();
          nLengthBytes = 4;
        }
      }

      gtuint32 nDiff = 0;
      if (nRecordType >= 0x01 && nRecordType <= 0x1B)
      {
        // print WPG type (in the resource in ascending order)
#ifdef _MSC_VER
#pragma warning (disable:4312)
#endif
        out_append (rc (HRC_GENLIB_WPC_WPG_01 + nRecordType - 1));
#ifdef _MSC_VER
#pragma warning (default:4312)
#endif
        out_format (rc (HRC_GENLIB_WPC_WPG_OFFSET),
                    nFilePos);

        switch (nRecordType)
        {
          // size record
          case 0x0B:
          {
            gtuint16 nWidth = m_pBuffer->GetUint16 ();
            gtuint16 nHeight = m_pBuffer->GetUint16 ();
            gtuint16 nBitsPerPixel = m_pBuffer->GetUint16 ();
            nDiff = 6;

            out_format (rc (HRC_GENLIB_WPC_WPG_SIZE),
                        nWidth,
                        nHeight,
                        gtint32 (1) << nBitsPerPixel);
            break;
          }
          // Color map record
          case 0x0E:
          {
            gtuint16 nColorCount = m_pBuffer->GetUint16 (m_pBuffer->GetActFilePos () + 2);
            nDiff = 4;

            out_format (rc (HRC_GENLIB_WPC_WPG_COLORS),
                        nColorCount);
            break;
          }
          // Start of WPG data (Type 1)
          case 0x0F:
          {
            gtuint8 nVersion = m_pBuffer->GetUint8 ();
            nDiff = 1;

            out_format (rc (HRC_GENLIB_WPC_WPG_VERSION),
                        nVersion);
            break;
          }
        }
      }
      else
      {
        out_info_format (rc (HRC_GENLIB_WPC_WPG_UNKNOWN),
                         nRecordType);
      }

      out_append (_T ("\n"));

      nRecordLength -= nDiff;

      // check whether IncActFilePos will work!
      if (m_pBuffer->GetActFilePos () + nRecordLength > m_pBuffer->GetFileSize ())
      {
        out_error_append (rc (HRC_GENLIB_WPC_WPG_ERROR));
        m_pBuffer->SetToEOF ();
      }
      else
      {
        m_pBuffer->IncActFilePos (nRecordLength - nDiff);
      }
    }

    out_decindent ();
  }
}
