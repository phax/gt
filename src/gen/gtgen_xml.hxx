MAKE_ANALYZER (XML);

static size_t  g_nBOMSize;
static LPCTSTR g_sXMLEncoding;

/*! Text or binary
 */
//--------------------------------------------------------------------
MAKE_IS_A (XML)
//--------------------------------------------------------------------
{
  // may be identified as binary for small files!

  if (aGI.m_pBuffer->CompareA (0, 5, "<?xml")) // ANSI // >
  {
    g_nBOMSize = 0;
    g_sXMLEncoding = _T ("ANSI/UTF-8");
    return true;
  }

  if (aGI.m_pBuffer->CompareA (0, 8, "\xef\xbb\xbf<?xml")) // with BOM  // >
  {
    g_nBOMSize = 3;
    g_sXMLEncoding = _T ("UTF-8");
    return true;
  }

  if (aGI.m_pBuffer->CompareA (0, 10, "\x00\x3c\x00\x3f\x00\x78\x00\x6d\x00\x6c"))
  {
    g_nBOMSize = 0;
    g_sXMLEncoding = _T ("UTF-16 Big Endian");
    return true;
  }

  if (aGI.m_pBuffer->CompareA (0, 10, "\x3c\x00\x3f\x00\x78\x00\x6d\x00\x6c\x00"))
  {
    g_nBOMSize = 0;
    g_sXMLEncoding = _T ("UTF-16 Little Endian");
    return true;
  }

  if (aGI.m_pBuffer->CompareA (0, 12, "\xfe\xff\x00\x3c\x00\x3f\x00\x78\x00\x6d\x00\x6c"))
  {
    g_nBOMSize = 2;
    g_sXMLEncoding = _T ("UTF-16 Big Endian");
    return true;
  }

  if (aGI.m_pBuffer->CompareA (0, 12, "\xff\xfe\x3c\x00\x3f\x00\x78\x00\x6d\x00\x6c\x00"))
  {
    g_nBOMSize = 2;
    g_sXMLEncoding = _T ("UTF-16 Little Endian");
    return true;
  }

  if (aGI.m_pBuffer->CompareA (0, 20, "\x00\x00\x00\x3c\x00\x00\x00\x3f\x00\x00\x00\x78\x00\x00\x00\x6d\x00\x00\x00\x6c"))
  {
    g_nBOMSize = 0;
    g_sXMLEncoding = _T ("UCS-4 Big Endian");
    return true;
  }

  if (aGI.m_pBuffer->CompareA (0, 20, "\x3c\x00\x00\x00\x3f\x00\x00\x00\x78\x00\x00\x00\x6d\x00\x00\x00\x6c\x00\x00\x00"))
  {
    g_nBOMSize = 0;
    g_sXMLEncoding = _T ("UCS-4 Little Endian");
    return true;
  }

  if (aGI.m_pBuffer->CompareA (0, 24, "\x00\x00\xfe\xff\x00\x00\x00\x3c\x00\x00\x00\x3f\x00\x00\x00\x78\x00\x00\x00\x6d\x00\x00\x00\x6c"))
  {
    g_nBOMSize = 4;
    g_sXMLEncoding = _T ("UCS-4 Big Endian");
    return true;
  }

  if (aGI.m_pBuffer->CompareA (0, 24, "\xff\xfe\x00\x00\x3c\x00\x00\x00\x3f\x00\x00\x00\x78\x00\x00\x00\x6d\x00\x00\x00\x6c\x00\x00\x00"))
  {
    g_nBOMSize = 4;
    g_sXMLEncoding = _T ("UCS-4 Little Endian");
    return true;
  }

  return false;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (XML)
//--------------------------------------------------------------------
{
  out_format (rc (HRC_GENLIB_XML_HEADER), g_sXMLEncoding);
  out_incindent ();
  if (g_nBOMSize > 0)
    out_format (_T ("BOM size: %u bytes\n"), g_nBOMSize);
  out_decindent ();
}
