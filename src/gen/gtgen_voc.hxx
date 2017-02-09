MAKE_ANALYZER (VOC);

/*! Binary or text.
 */
//--------------------------------------------------------------------
MAKE_IS_A (VOC)
//--------------------------------------------------------------------
{
  return aGI.m_pBuffer->CompareA (0, 19, "Creative Voice File");
}

//--------------------------------------------------------------------
MAKE_DISPLAY (VOC)
//--------------------------------------------------------------------
{
#define VOC_BYTE2SR(b)       (1000000 / (256 - (b)))

  typedef gtuint32 VOC_DataRecord;
  const size_t VOC_DATA_SIZE = sizeof (VOC_DataRecord);

  VOC_Header aHeader;
  VOC_DataRecord aData;

  m_pBuffer->GetBufferX (0, &aHeader, VOC_H_SIZE);

  out_format (rc (HRC_GENLIB_VOC_HEADER),
              aHeader.nVersionMajor,
              aHeader.nVersionMinor);

  if (!m_bListMode)
  {
    out_incindent ();

    out_append (rc (HRC_GENLIB_VOC_LISTING));

    m_pBuffer->SetActFilePos (aHeader.nDataOfs);

    int nDataID;
    do
    {
      m_pBuffer->GetBuffer (&aData, VOC_DATA_SIZE);

      const gtint32 nDataLen = (aData >> 8);
      nDataID = gtuint8 (aData & 0xFF);

      switch (nDataID)
      {
        case 0:
        {
          if (nDataLen != 0)
            out_info_append (rc (HRC_GENLIB_VOC_ERROR_IN_DATA));
          break;
        }
        case 1:
        {
          const gtuint8 b1 = m_pBuffer->GetUint8 ();
          const gtuint8 b2 = m_pBuffer->GetUint8 ();

          out_format (rc (HRC_GENLIB_VOC_SOUND_DATA),
                      VOC_BYTE2SR (b1));

          switch (b2)
          {
            case 0:  out_format (rc (HRC_GENLIB_VOC_8BIT)); break;
            case 1:  out_format (rc (HRC_GENLIB_VOC_4BIT)); break;
            case 2:  out_format (rc (HRC_GENLIB_VOC_2_6BIT)); break;
            case 3:  out_format (rc (HRC_GENLIB_VOC_2BIT)); break;
            default: out_format (rc (HRC_GENLIB_VOC_MULTI_DAC), b2 - 3); break;
          }
          out_append (_T ("\n"));

          m_pBuffer->IncActFilePos (nDataLen - 2);
          break;
        }
        case 2:
        {
          out_append (rc (HRC_GENLIB_VOC_SOUND_CONTINUE));
          m_pBuffer->IncActFilePos (nDataLen);
          break;
        }
        case 3:
        {
          const gtuint16 w = m_pBuffer->GetUint16 ();
          m_pBuffer->IncActFilePos (1);

          DoublePair aSecs;
          _splitdouble (double (w) / 1000, aSecs, 1);
          out_format (rc (HRC_GENLIB_VOC_SILENCE),
                      aSecs.h, aSecs.l);

          m_pBuffer->IncActFilePos (nDataLen - 3);
          break;
        }
        case 4:
        {
          const gtuint16 w = m_pBuffer->GetUint16 ();

          out_format (rc (HRC_GENLIB_VOC_MARKER),
                      w);
          break;
        }
        case 5:
        {
          out_append (rc (HRC_GENLIB_VOC_ASCII));
          m_pBuffer->IncActFilePos (nDataLen);
          break;
        }
        case 6:
        {
          const gtuint16 w = m_pBuffer->GetUint16 ();
          if (w == 0xFFFF)
            out_append (rc (HRC_GENLIB_VOC_REPEAT_INFINITE));
          else
          {
            out_format (rc (HRC_GENLIB_VOC_REPEAT_N_TIMES),
                        w - 1);
          }
          break;
        }
        case 7:
        {
          out_append (rc (HRC_GENLIB_VOC_END_REPEAT));
          break;
        }
        case 8:
        {
          out_append (rc (HRC_GENLIB_VOC_EXTENDED));
          m_pBuffer->IncActFilePos (nDataLen);
          break;
        }
        default:
        {
          out_format (rc (HRC_GENLIB_VOC_UNKNOWN),
                      nDataID);
          break;
        }
      }
    } while (nDataID != 0 && m_pBuffer->GetActFilePos () > m_pBuffer->GetFileSize ());

    out_decindent ();
  }
}

