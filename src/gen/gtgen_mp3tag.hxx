MAKE_ANALYZER (MP3TAG);

/*! Binary or text
 */
//--------------------------------------------------------------------
MAKE_IS_A (MP3TAG)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->GetFileSize () < MP3TAG_SIZE)
    return false;

  return aGI.m_pBuffer->CompareA (0, 3, "TAG");
}

//--------------------------------------------------------------------
MAKE_DISPLAY (MP3TAG)
//--------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_MP3TAG_HEADER));

  if (!m_bListMode)
  {
    out_incindent ();

    // read MP3 TAG from file
    MP3TAG aTag;
    m_pBuffer->GetBufferX (0, &aTag, MP3TAG_SIZE);

    // check song title...
    pstring s;
    str_assign (s, aTag.sTitle, 30);
    str_erase_trailing (s, _T (' '));
    if (!s.empty ())
      out_format (rc (HRC_GENLIB_MP3TAG_TITLE), s.c_str ());

    // check artist...
    str_assign (s, aTag.sArtist, 30);
    str_erase_trailing (s, _T (' '));
    if (!s.empty ())
      out_format (rc (HRC_GENLIB_MP3TAG_ARTIST), s.c_str ());

    // check album...
    str_assign (s, aTag.sAlbum, 30);
    str_erase_trailing (s, _T (' '));
    if (!s.empty ())
      out_format (rc (HRC_GENLIB_MP3TAG_ALBUM), s.c_str ());

    // check year...
    str_assign (s, aTag.sYear, 4);
    str_erase_trailing (s, _T (' '));
    if (!s.empty ())
      out_format (rc (HRC_GENLIB_MP3TAG_YEAR), s.c_str ());

    // check comment ...
    str_assign (s, aTag.sComment, 30);
    str_erase_trailing (s, _T (' '));
    if (!s.empty ())
      out_format (rc (HRC_GENLIB_MP3TAG_COMMENT), s.c_str ());

    out_decindent ();
  }
}
