MAKE_ANALYZER (HeadText);

namespace HeadText {

pstring g_sHeadText;

//--------------------------------------------------------------------
static bool _IsTextChar (const gtuint8 c)
//--------------------------------------------------------------------
{
  // \xa9 is the copyright character
  static const char TEXT_CHARS[] = "+-*/ _.,;!?()[]<>\r\n\t\xA9";

  // Note: strchr (x, '\0') returns something != NULL!!
  return c != 0 && (isalnum (c) || strchr (TEXT_CHARS, c) != NULL);
}

}  // namespace HeadText

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (HeadText)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  // local buffer
  const size_t HEADTEXT_BUFFER = 256;
  gtuint8 aBuffer[HEADTEXT_BUFFER + 1];
  memset (aBuffer, 0, sizeof (aBuffer));

  // determine number of bytes to check
  const size_t nBufferSize = _min (HEADTEXT_BUFFER, size_t (aGI.m_pBuffer->GetFileSize ()));

  // read the bytes into the buffer
  aGI.m_pBuffer->GetBuffer (0, aBuffer, nBufferSize);

  // start at the beginning ...
  gtuint8 *pCur = aBuffer;

  while (pCur < aBuffer + HEADTEXT_BUFFER)
  {
    if (!HeadText::_IsTextChar (*pCur))
      break;

    pCur++;
  }

  // remove all trailing newlines
  while (pCur > aBuffer && (pCur[-1] == '\r' || pCur[-1] == '\n'))
    --pCur;

  // determine real start (no leading \r or \n)
  gtuint8* pRealStart = aBuffer;
  while (pRealStart < pCur && (*pRealStart == '\r' || *pRealStart == '\n'))
    ++pRealStart;

  // there should be at least 5 printable chars!
  if ((pCur - pRealStart) < 5)
  {
    HeadText::g_sHeadText.clear ();
  }
  else
  {
    str_assign (HeadText::g_sHeadText, (char*) pRealStart, int (pCur - pRealStart));
  }

  return !HeadText::g_sHeadText.empty ();
}

//--------------------------------------------------------------------
MAKE_DISPLAY (HeadText)
//--------------------------------------------------------------------
{
  out_format (rc (HRC_GENLIB_HEADTEXT_HEADER),
              HeadText::g_sHeadText.c_str ());
}
