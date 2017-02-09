MAKE_ANALYZER (INI);

namespace INI
{
file_t g_nStart;
file_t g_nEnd;
} // namespace INI

/*! Text only
 */
//--------------------------------------------------------------------
MAKE_IS_A (INI)
//--------------------------------------------------------------------
{
  if (!aGI.m_pBuffer->IsText ())
    return false;

  // start at the beginning :)
  INI::g_nStart = 0;

  // read as byte and not as char because if a value >= 0x80
  // the int value would be 0xffffffdb -> assertion is isspace!
  int c = aGI.m_pBuffer->GetUint8 (INI::g_nStart);
  while (!aGI.m_pBuffer->IsEOF () && isspace (c))
    c = aGI.m_pBuffer->GetUint8 (++INI::g_nStart);

  if (c != '[')
    return false;

  // search only the next 512 bytes (any number)
  INI::g_nEnd = aGI.m_pBuffer->Find (']', INI::g_nStart, INI::g_nStart + 512);
  return (INI::g_nEnd != GT_NOT_FOUND);
}

//--------------------------------------------------------------------
MAKE_DISPLAY (INI)
//--------------------------------------------------------------------
{
  DirEntry aDE (m_pBuffer->GetFileName ());

  // must work
  pstring sName;
  m_pBuffer->GetFixedSizeAString (INI::g_nStart + 1, sName, size_t (INI::g_nEnd - INI::g_nStart - 1));

  // special names...
  if (sName == _T ("CloneCD"))
    str_assign_from_res (sName, HRC_GENLIB_INI_CLONECD);
  else
  if (sName == _T ("InternetShortcut"))
    str_assign_from_res (sName, HRC_GENLIB_INI_URL);

  out_format (rc (HRC_GENLIB_INI_HEADER),
              aDE.HasFileNameExtension (_T ("inf"))
                ? _T ("INF")
                : aDE.HasFileNameExtension (_T ("url"))
                  ? _T ("URL")
                  : _T ("INI"),
              sName.c_str ());
}
