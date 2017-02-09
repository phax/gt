MAKE_ANALYZER (HTML);

/*! Textfiles only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (HTML)
//--------------------------------------------------------------------
{
  return aGI.m_pBuffer->IsText () &&
         (aGI.m_pBuffer->iFind ("<HTML>", 0) >= 0 ||
          aGI.m_pBuffer->iFind ("<HEAD>", 0) >= 0 ||
          aGI.m_pBuffer->iFind ("<BODY>", 0) >= 0);
}

//--------------------------------------------------------------------
MAKE_DISPLAY (HTML)
//--------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_HTML_HEADER));

  if (!m_bListMode)
  {
    out_incindent ();

    // the title should ne within the first 256 chars!
    // This is just a "random" number, nothing fixed
    file_t nStart = m_pBuffer->iFind ("<TITLE>", 0, 256);
    if (nStart != GT_NOT_FOUND)
    {
      nStart += 7;  // == _tcslen ("<TITLE>");
      // to avoid too long titles (e.g. source code)
      // This is just a "random" number, nothing fixed
      file_t nEnd = m_pBuffer->iFind ("</TITLE>", nStart, nStart + 1024);
      if (nEnd != GT_NOT_FOUND)
      {
        // nEnd contains the relative offset!!
        nEnd += nStart;

        // read and write the title tag - can contain weird characters if HTML page is in chinese (etc.)
        pstring sTitle;
        m_pBuffer->GetFixedSizeAString (nStart, sTitle, size_t (nEnd - nStart));

        out_format (rc (HRC_GENLIB_HTML_TITLE),
                    sTitle.c_str ());
      }
    }

    out_decindent ();
  }
}
