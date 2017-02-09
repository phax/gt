MAKE_ANALYZER (RES);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (RES)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0)  == 0x00000000 &&
         aGI.m_pBuffer->GetInt32 (4)  == 0x00000020 &&
         aGI.m_pBuffer->GetInt32 (8)  == 0x0000FFFF &&
         aGI.m_pBuffer->GetInt32 (12) == 0x0000FFFF;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (RES)
//--------------------------------------------------------------------
{
#define RES_GET(__T,__n) *(reinterpret_cast <__T*> (pHeader + __n))

  out_append (rc (HRC_GENLIB_RES_HEADER));

  if (!m_bListMode)
  {
    gtuint32 nDataSize, nHeaderSize;
    gtuint16 nResType, nResName;
    file_t   nFilePos;
    size_t   nHeaderIndex;
    pstring  sTemp;

    out_incindent ();

    //! skip the first empty res header!
    m_pBuffer->SetActFilePos (32);

    while (!m_pBuffer->IsEOF ())
    {
      nFilePos = m_pBuffer->GetActFilePos ();

      // read the first two values
      // The header size is including the first 2 DWORD fields
      //   and is directly after the 2 fields. The data is directly
      //   append afterwards.
      nDataSize   = m_pBuffer->GetInt32 ();
      nHeaderSize = m_pBuffer->GetInt32 ();

      // check for max size (e.g. error in testembed.aps from Mozilla)
      // check for min size (e.g. error in xml4com.aps from Xerces 1.5.0)
      if (nHeaderSize >= m_pBuffer->GetFileSize () ||
          nDataSize   >= m_pBuffer->GetFileSize () ||
          nHeaderSize < 8)
      {
        out_error_append (rc (HRC_GENLIB_RES_ERR_ALIGN));
        break;
      }

      // and now read the remaining header!
      gtuint8 *pHeader = (gtuint8*) _alloca (nHeaderSize - 8);
      m_pBuffer->GetBufferX (pHeader, nHeaderSize - 8);

      // get the first uint16 to identify the type of the resource
      nResType = RES_GET (gtuint16, 0);

      if (nResType == 0xFFFF)
      {
        // resource type identified by ID (the next gtuint16!)
        nResType = RES_GET (gtuint16, 2);

        // default: 2 USHORT - 0xffff and the resource type
        nHeaderIndex = 4;

        switch (nResType)
        {
          case 0x0001: out_append (rc (HRC_GENLIB_RES_TYPE_0001)); break;
          case 0x0002: out_append (rc (HRC_GENLIB_RES_TYPE_0002)); break;
          case 0x0003: out_append (rc (HRC_GENLIB_RES_TYPE_0003)); break;
          case 0x0004: out_append (rc (HRC_GENLIB_RES_TYPE_0004)); break;
          case 0x0005: out_append (rc (HRC_GENLIB_RES_TYPE_0005)); break;
          case 0x0006: out_append (rc (HRC_GENLIB_RES_TYPE_0006)); break;
          case 0x0007: out_append (rc (HRC_GENLIB_RES_TYPE_0007)); break;
          case 0x0008: out_append (rc (HRC_GENLIB_RES_TYPE_0008)); break;
          case 0x0009: out_append (rc (HRC_GENLIB_RES_TYPE_0009)); break;
          case 0x000A: out_append (rc (HRC_GENLIB_RES_TYPE_000A)); break;
          case 0x000B: out_append (rc (HRC_GENLIB_RES_TYPE_000B)); break;
          case 0x000C: out_append (rc (HRC_GENLIB_RES_TYPE_000C)); break;

          case 0x000E: out_append (rc (HRC_GENLIB_RES_TYPE_000E)); break;

          case 0x0010: out_append (rc (HRC_GENLIB_RES_TYPE_0010)); break;
          case 0x0011: out_append (rc (HRC_GENLIB_RES_TYPE_0011)); break;

          case 0x0013: out_append (rc (HRC_GENLIB_RES_TYPE_0013)); break;
          case 0x0014: out_append (rc (HRC_GENLIB_RES_TYPE_0014)); break;
          case 0x0015: out_append (rc (HRC_GENLIB_RES_TYPE_0015)); break;

          case 0x2002: out_append (rc (HRC_GENLIB_RES_TYPE_2002)); break;
          case 0x2004: out_append (rc (HRC_GENLIB_RES_TYPE_2004)); break;
          case 0x2005: out_append (rc (HRC_GENLIB_RES_TYPE_2005)); break;
          default:     out_info_format (rc (HRC_GENLIB_RES_TYPE_UNK), nResType); break;
        }
      }
      else
      {
        // it's a resource name

        // The name is always stored in Unicode
        str_assign (sTemp, LPCWSTR (pHeader));

        // print it
        out_format (rc (HRC_GENLIB_RES_TYPE_AS_NAME),
                    sTemp.c_str ());

        // skip name and
        // align the index of the main header to a 4-byte boundary
        nHeaderIndex = ALIGN_4 ((sTemp.length () + 1) * 2);
      }

      nResName = RES_GET (gtuint16, nHeaderIndex);
      if (nResName == 0xFFFF)
      {
        // identified by ID - the next gtuint16!
        nResName = RES_GET (gtuint16, nHeaderIndex + 2);
        out_format (rc (HRC_GENLIB_RES_NAME_AS_ID),
                    nResName);
      }
      else
      {
        // identified by name
        str_assign (sTemp, LPCWSTR (pHeader + nHeaderIndex));
        out_format (rc (HRC_GENLIB_RES_NAME_AS_TEXT),
                    sTemp.c_str ());
      }

      out_format (rc (HRC_GENLIB_RES_DATASIZE),
                  nDataSize);

      // skip resource data :) - it is always aligned to DWORD boundaries!
      nFilePos += nHeaderSize + nDataSize;
      nFilePos = ALIGN_4 (nFilePos);

      // must be signed!
      const file_t nBytesFromEOF = nFilePos - m_pBuffer->GetFileSize ();
      if (nBytesFromEOF > 0)
      {
        out_error_format (rc (HRC_GENLIB_RES_TOO_LARGE),
                          nBytesFromEOF);
        break;
      }

      // goto next entry
      m_pBuffer->SetActFilePos (nFilePos);
    }

    out_decindent ();
  }
}

