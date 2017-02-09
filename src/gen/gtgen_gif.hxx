MAKE_ANALYZER (GIF);

/*! Binary or text
 */
//--------------------------------------------------------------------
MAKE_IS_A (GIF)
//--------------------------------------------------------------------
{
  return aGI.m_pBuffer->CompareA (0, 6, "GIF87a") ||
         aGI.m_pBuffer->CompareA (0, 6, "GIF89a");
}

//--------------------------------------------------------------------
MAKE_DISPLAY (GIF)
//--------------------------------------------------------------------
{
  unsigned int nImageCount = 0;
  gtuint8 nID, nLen;
  GIF_Header aH;
  GIF_ImageDescriptor aID;

  // read the global header
  m_pBuffer->GetBufferX (0, &aH, GIF_H_SIZE);

  const bool bVer87 = (memcmp (aH.sID, "GIF87a", 6) == 0);
  const int  nGlobalColors = (1 << ((aH.nFlags & 7) + 1));

  // skip the global color map (if any)
  const bool bGlobalColorMapFollowsDescriptor = aH.nFlags & BITS[7];
  if (bGlobalColorMapFollowsDescriptor)
  {
    try
    {
      m_pBuffer->IncActFilePos (nGlobalColors * 3);
    }
    catch (const Excpt&)
    {
      // occurs when IncActFilePos goes out of file bounds
      // add descriptive text and throw to next level
      out_append (rc (HRC_GENLIB_GIF_SEEMS_TO_BE));
      out_error_append (rc (HRC_GENLIB_GIF_ERROR_COL_TABLE));
      return;
    }
  }

  do
  {
    nID = m_pBuffer->GetUint8 ();
    switch (nID)
    {
      case 0x21:
      {
        // extension block
        const gtuint8 nFunctionCode = m_pBuffer->GetUint8 ();

        // print extension blocks only in non-list-mode
        if (!m_bListMode)
        {
          const bool bGIFLITE = nFunctionCode == 0xFF && m_pBuffer->CompareA (m_pBuffer->GetActFilePos () + 1, 7, "GIFLITE");

          out_format (rc (HRC_GENLIB_GIF_EXT_BLOCK),
                      bVer87
                        ? _T ("87")
                        : _T ("89"),
                      nFunctionCode);

          if (bGIFLITE)
            out_append (rc (HRC_GENLIB_GIF_GIFLITE));
          out_append (_T ("\n"));
        }

        // and now skip all the data
        try
        {
          while ((nLen = m_pBuffer->GetUint8 ()) != 0x00)
          {
//            out_format (_T ("Skipping %d bytes\n"), nLen);
            m_pBuffer->IncActFilePos (nLen);
          }
        }
        catch (const Excpt&)
        {
          // occurs when IncActFilePos goes out of file bounds
          out_error_append (rc (HRC_GENLIB_GIF_EXT_BLOCK_ERR));
          return;
        }

        break;
      }
      case 0x2C:
      {
        // image descriptor
        nImageCount++;

        m_pBuffer->GetBufferX (m_pBuffer->GetActFilePos () - 1, &aID, GIF_ID_SIZE);

        const bool bUseLocalColorMap = (aID.nFlags & BITS[7]);
        const bool bInterlaced = (aID.nFlags & BITS[6]);
        const int nLocalColors = (1 << ((aID.nFlags & 7) + 1));

        // print this only when we're not in list-mode or when it is ths first image in list-mode
        if (nImageCount == 1 || !m_bListMode)
        {
          out_format (rc (HRC_GENLIB_GIF_HEADER),
                      bVer87
                        ? _T ("87")
                        : _T ("89"),
                      aID.nImageWidth,
                      aID.nImageHeight,
                      nGlobalColors);
          if (bInterlaced)
            out_append (rc (HRC_GENLIB_GIF_INTERLACED));
          out_append (_T ("\n"));
        }

        if (bUseLocalColorMap)
        {
          try
          {
            m_pBuffer->IncActFilePos (nLocalColors * 3);
          }
          catch (const Excpt&)
          {
            // occurs when IncActFilePos goes out of file bounds
            // add descriptive text and throw to next level
            out_error_append (rc (HRC_GENLIB_GIF_ERR_LOCAL_COL_TAB));
            return;
          }
        }

        // skip the "code size"
        try
        {
          m_pBuffer->IncActFilePos (1);
          while ((nLen = m_pBuffer->GetUint8 ()) != 0x00)
            m_pBuffer->IncActFilePos (nLen);
        }
        catch (const Excpt&)
        {
          // occurs when IncActFilePos goes out of file bounds
          out_error_append (rc (HRC_GENLIB_GIF_ERR_DESC_BLOCK));
          return;
        }

        break;
      }
      case 0x3B:
      {
        // EOF
        break;
      }
      default:
      {
        // the GIF file seems to be corrupt
        out_error_format (rc (HRC_GENLIB_GIF_ERR_IMG_DESC),
                          nID);
        return;
      }
    }

    // avoid assertion in GetUint8
    if (nID != 0x3B && m_pBuffer->IsEOF ())
    {
      out_error_append (rc (HRC_GENLIB_GIF_UNEXP_EOF));
      break;
    }
  } while (nID != 0x3B);
}
