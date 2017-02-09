MAKE_ANALYZER (TIFF);

/*! Binary only
    @see http://www.awaresystems.be/imaging/tiff/faq.html
    @see http://local.wasp.uwa.edu.au/~pbourke/dataformats/tiff/
    @see http://www.atalasoft.com/kb/article.aspx?id=10010
    @see tiff.h from LibTIFF
 */
//--------------------------------------------------------------------
MAKE_IS_A (TIFF)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  // 0x4949 Byte order indicator
  // 0x002A Version number (always 42)
  return aGI.m_pBuffer->GetInt32 (0) == 0x002A4949 &&
         aGI.m_pBuffer->GetInt32 (4) <= aGI.m_pBuffer->GetTotalFileSize ();
}

//--------------------------------------------------------------------
MAKE_DISPLAY (TIFF)
//--------------------------------------------------------------------
{
  const gtuint32 nFirstIFDPos = m_pBuffer->GetInt32 (4);
  m_pBuffer->SetActFilePos (nFirstIFDPos);

  gtuint32 nWidth = 0;
  gtuint32 nHeight = 0;
  gtuint32 nBitsPerSample = 0;
  gtuint32 nCompressionMethod = 0;
  gtuint32 nPhotometricInterpretation = 0;

  // read IFD
  const gtuint16 nTagCount = m_pBuffer->GetUint16 ();
  TIFF_Tag aTag;
  for (gtuint16 i = 0; i < nTagCount; ++i)
  {
    m_pBuffer->GetBuffer (&aTag, TIFF_TAG_SIZE);
    switch (aTag.nIdentifyingCode)
    {
      case 0x0100:
        nWidth = aTag.nTagData;
        break;
      case 0x0101:
        nHeight = aTag.nTagData;
        break;
      case 0x0102:
        nBitsPerSample = aTag.nTagData;
        break;
      case 0x0103:
        nCompressionMethod = aTag.nTagData;
        break;
      case 0x0106:
        nPhotometricInterpretation = aTag.nTagData;
        break;
      default:
        if (false)
          out_format (_T ("[dbg] Tag %02Xh\n"), aTag.nIdentifyingCode);
    }
  }

  out_format (rc (HRC_GENLIB_TIFF_HEADER), nWidth, nHeight, 1 << nBitsPerSample);
  out_incindent ();
  out_append (_T ("Compression method: "));
  switch (nCompressionMethod)
  {
    case 1:
      out_append (_T ("Uncompressed\n"));
      break;
    case 2:
      out_append (_T ("CCITT modified Huffman RLE\n"));
      break;
    case 3:
      out_append (_T ("CCITT Group 3 FAX Encoding\n"));
      break;
    case 4:
      out_append (_T ("CCITT Group 4 FAX Encoding\n"));
      break;
    case 5:
      out_append (_T ("LZW\n"));
      break;
    case 6:
      out_append (_T ("!6.0 JPEG\n"));
      break;
    case 7:
      out_append (_T ("%JPEG DCT compression\n"));
      break;
    case 8:
      out_append (_T ("Deflate compression, as recognized by Adobe\n"));
      break;
    case 23773:
      out_append (_T ("Macintosh Packbits\n"));
      break;
    case 32766:
      out_append (_T ("NeXT 2-bit RLE\n"));
      break;
    case 32771:
      out_append (_T ("#1 w/ word alignment\n"));
      break;
    case 32773:
      out_append (_T ("Macintosh RLE\n"));
      break;
    case 32809:
      out_append (_T ("ThunderScan RLE\n"));
      break;
    case 32895:
      out_append (_T ("IT8 CT w/padding\n"));
      break;
    case 32896:
      out_append (_T ("IT8 Linework RLE\n"));
      break;
    case 32897:
      out_append (_T ("IT8 Monochrome picture\n"));
      break;
    case 32898:
      out_append (_T ("IT8 Binary line art\n"));
      break;
    case 32908:
      out_append (_T ("Pixar companded 10bit LZW\n"));
      break;
    case 32909:
      out_append (_T ("Pixar companded 11bit ZIP\n"));
      break;
    case 32946:
      out_append (_T ("Deflate compression\n"));
      break;
    case 32947:
      out_append (_T ("Kodak DCS encoding\n"));
      break;
    case 34661:
      out_append (_T ("ISO JBIG\n"));
      break;
    case 34676:
      out_append (_T ("SGI Log Luminance RLE\n"));
      break;
    case 34677:
      out_append (_T ("SGI Log 24-bit packed\n"));
      break;
    case 34712:
      out_append (_T ("Leadtools JPEG2000\n"));
      break;
    default:
      out_format (_T ("unknown %u\n"), nCompressionMethod);
      break;
  }
  out_append (_T ("Photometric Interpretation: "));
  switch (nPhotometricInterpretation)
  {
    case 0:
      out_append (_T ("min value is white\n"));
      break;
    case 1:
      out_append (_T ("min value is black\n"));
      break;
    case 2:
      out_append (_T ("RGB color model\n"));
      break;
    case 3:
      out_append (_T ("color map indexed\n"));
      break;
    case 4:
      out_append (_T ("$holdout mask\n"));
      break;
    case 5:
      out_append (_T ("!color separations\n"));
      break;
    case 6:
      out_append (_T ("!CCIR 601\n"));
      break;
    case 8:
      out_append (_T ("!1976 CIE L*a*b*\n"));
      break;
    case 9:
      out_append (_T ("ICC L*a*b* [Adobe TIFF Technote 4]\n"));
      break;
    case 10:
      out_append (_T ("ITU L*a*b*\n"));
      break;
    case 32844:
      out_append (_T ("CIE Log2(L)\n"));
      break;
    case 32845:
      out_append (_T ("CIE Log2(L) (u',v')\n"));
      break;
    default:
      out_format (_T ("unknown %u\n"), nPhotometricInterpretation);
      break;
  }
  out_decindent ();
}
