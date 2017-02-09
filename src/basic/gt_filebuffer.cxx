#include "gt_filebuffer.hxx"

#include "gt_exception.hxx"
#include "gt_utils.hxx"
#include "gt_output.hxx"

namespace GT  {

#define MAX_FIND_BUF (1024 * 1024)

//--------------------------------------------------------------------
void GT_CALL FileBuffer::_DetermineIfText ()
//--------------------------------------------------------------------
{
  const size_t BUFSIZE = m_nFileSize <= 4096 ? size_t (m_nFileSize) : 4096;

  size_t nMaxNonASCII = BUFSIZE < 100
                          ? 1
                          : BUFSIZE < 1000
                            ? 5
                            : 15;
  m_bIsUnix = true;
  m_bIsMac  = true;
  m_bIs7Bit = true;

  size_t nMax = BUFSIZE;

  gtuint8* p = _GetDataPtr (0);
  for (size_t i = 0; i < nMax; ++i, ++p)
  {
    // non-ASCII char?
    if (*p == 13)
      // special handling for '\r'
      m_bIsUnix = false;
    else
    if (*p == 10)
      // special handling for '\n'
      m_bIsMac = false;
    else
    // default non-printable chars
    // Note: 10 and 13 already handled above!
    // 9:\t 10:\n 12:? 13:\r 26:EOF
    if (*p < 32 && (*p != 9 &&* p != 12 &&* p != 26))
    {
      // break even if nMaxNonASCII is 0 because then it doesn't matter
      // whether it is unix or 7bit since it is no text file!
      if (--nMaxNonASCII == 0)
        break;
    }
    else
    if (*p > 127)
      m_bIs7Bit = false;
    // else *p >= 32 und *p <=127 -> ASCII
  }

  // 0-byte files are binary per definition!
  m_bIsText = BUFSIZE > 0
                ? (nMaxNonASCII > 0)
                : false;
  if (!m_bIsText)
  {
    m_bIsUnix = false;
    m_bIsMac  = false;
    m_bIs7Bit = false;
  }
}

// checked version of memcpy in case of dynamic mapping
//--------------------------------------------------------------------
void GT_CALL FileBuffer::_memcpy (void* pDst, file_t nIndex, size_t nLen)
//--------------------------------------------------------------------
{
  // check the copy action is within the file
  ASSERT (m_nScanOffset + nIndex + nLen < m_nScanOffset);

  const gtuint8* p = _GetDataPtr (nIndex);

  if (m_bDynamicMapping)
  {
    // check if the buffer is within the current mapping size
    ASSERT (nIndex - m_nDynamicMapPos >= 0);
    ASSERT (nIndex - m_nDynamicMapPos < m_nDynamicMapSize);
    ASSERT (nIndex - m_nDynamicMapPos + nLen >= 0);
    ASSERT (nIndex - m_nDynamicMapPos + nLen < m_nDynamicMapSize);
  }
}

//--------------------------------------------------------------------
FileBuffer::FileBuffer
                                        (      LPCTSTR        sFilename,
                                         const file_t         nOffset,
                                               FileListerProc aListProc,
                                               File *const    pParent)
//--------------------------------------------------------------------
  : File        (sFilename, nOffset, pParent),
    m_aListProc (aListProc),
    m_bIsText   (false),
    m_bIsUnix   (false),
    m_bIsMac    (false),
    m_bIs7Bit   (false)
{}

//--------------------------------------------------------------------
EOpenError GT_CALL FileBuffer::Init ()
//--------------------------------------------------------------------
{
  EOpenError eOpen = File::Init ();
  if (eOpen != OPEN_SUCCESS)
    return eOpen;

  _DetermineIfText ();
  return OPEN_SUCCESS;
}

//--------------------------------------------------------------------
void GT_CALL FileBuffer::CallListProc
                                        (const file_t nNewOffset) const
//--------------------------------------------------------------------
{
  // overlays are handled anytime; the cmdline parameter are checked in m_aListProc
  if (m_aListProc)
  {
    ASSERT (nNewOffset > 0);  // if 0 -> endless loop
    if (nNewOffset > 0)
      m_aListProc (m_sFilename.c_str (), m_nScanOffset + nNewOffset, const_cast <FileBuffer*> (this));
  }
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::ReadUint8
                                        (const file_t   nIndex,
                                               gtuint8& nByte)
//--------------------------------------------------------------------
{
  // ensure that the index is not totally invalid ;-)
  if (nIndex < 0 || (nIndex + 1) > m_nFileSize)
    return false;

  // get byte
  nByte = *_GetDataPtr (nIndex);

  // remember the current fileposition
  m_nActFilePos = nIndex + 1;

  return true;
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::ReadUint16
                                        (const file_t    nIndex,
                                               gtuint16& nWord)
//--------------------------------------------------------------------
{
  if (nIndex < 0 || (nIndex + 2) > m_nFileSize)
    return false;

  // get as word
  nWord = *((gtuint16*) _GetDataPtr (nIndex));

  // remember the current fileposition
  m_nActFilePos = nIndex + 2;

  return true;
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::ReadInt32
                                        (const file_t   nIndex,
                                               gtint32& nLong)
//--------------------------------------------------------------------
{
  if (nIndex < 0 || (nIndex + 4) > m_nFileSize)
    return false;

  // get long
  nLong = *((gtint32*) _GetDataPtr (nIndex));

  // remember the current fileposition
  m_nActFilePos = nIndex + 4;

  return true;
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::ReadInt64
                                        (const file_t   nIndex,
                                               gtint64& nLongLong)
//--------------------------------------------------------------------
{
  if (nIndex < 0 || (nIndex + 8) > m_nFileSize)
    return false;

  // get long
  nLongLong = *((gtint64*) _GetDataPtr (nIndex));

  // remember the current fileposition
  m_nActFilePos = nIndex + 8;

  return true;
}

//--------------------------------------------------------------------
void GT_CALL FileBuffer::GetBufferX
                                        (const file_t nIndex,
                                               void*  pTargetBuffer,
                                         const size_t nLen)
//--------------------------------------------------------------------
{
  ASSERT (pTargetBuffer);

  // is the buffer partly out of file bounds??
  if (nIndex + nLen > m_nFileSize)
  {
    // tried to read over EOF
    pstring sText;
    str_assign_from_res (sText, HRC_GTLIB_FILEBUFFER_EXCEPTION_GETBUFFERX_EOF);
    throw Excpt (_T (__FILE__), __LINE__,
                 sText.c_str (),
                 m_sFilename.c_str (),
                 nIndex + nLen - m_nFileSize);
  }

  if (nIndex < 0 || nIndex + nLen < 0)
  {
    // tried to read before SOF
    pstring sText;
    str_assign_from_res (sText, HRC_GTLIB_FILEBUFFER_EXCEPTION_GETBUFFERX_SOF);
    throw Excpt (_T (__FILE__), __LINE__,
                 sText.c_str (),
                 m_sFilename.c_str (),
                 nIndex + nLen);
  }

  if (nLen == 0)
  {
    m_nActFilePos = nIndex;
    return;
  }

  // copy to target buffer
  memcpy (pTargetBuffer, _GetDataPtr (nIndex), nLen);

  // set file pos
  m_nActFilePos = nIndex + nLen;
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::GetBuffer
                                        (const file_t nIndex,
                                               void*  pTargetBuffer,
                                         const size_t nLen)
//--------------------------------------------------------------------
{
  ASSERT (pTargetBuffer);

//  out_format ("GB: %08xh + %08xh (=%08xh) >= %08xh\n", nIndex, nLen, nIndex + nLen, m_nFileSize);

  // is the buffer partly out of file bounds??
  if (nIndex + nLen > m_nFileSize)
  {
//    out_format ("GetBuffer: Index is out of bounds (%d >= %d)\n", nIndex + nLen, m_nFileSize);
    return false;
  }

  if (nIndex < 0)
  {
    // happened in _AnalyzeWaveChunk
    return false;
  }

  if (nLen == 0)
  {
    m_nActFilePos = nIndex;
    return true;
  }

  // copy to target buffer
  memcpy (pTargetBuffer, _GetDataPtr (nIndex), nLen);

  // set file pos
  m_nActFilePos = nIndex + nLen;

  return true;
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::GetFixedSizeAString
                                        (const file_t  nIndex,
                                               string& sTarget,
                                         const size_t  nStringLen)
//--------------------------------------------------------------------
{
  bool bReturn;

  // _alloca: allcoate on stack -> no need to delete
  char* sTemp = (char*) _alloca (nStringLen + 1);
  sTemp[nStringLen] = 0;
  bReturn = GetBuffer (nIndex, sTemp, nStringLen);

  if (bReturn)
    sTarget.assign (sTemp, int (nStringLen));
  else
    sTarget.clear ();

  return bReturn;
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::GetFixedSizeAString
                                        (const file_t   nIndex,
                                               wstring& sTarget,
                                         const size_t   nStringLen)
//--------------------------------------------------------------------
{
  string sTemp;
  if (!GetFixedSizeAString (nIndex, sTemp, nStringLen))
  {
    sTarget.clear ();
    return false;
  }
  str_assign (sTarget, sTemp);
  return true;
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::GetFixedSizeWString
                                        (const file_t   nIndex,
                                               wstring& sTarget,
                                         const size_t   nStringLen)
//--------------------------------------------------------------------
{
  bool bReturn;

  // read as single-byte string and assign (with conversion)
  // _alloca: allcoate on stack -> no need to delete
  wchar_t* sTemp = (wchar_t*) _alloca ((nStringLen + 1) * sizeof (wchar_t));
  sTemp[nStringLen] = 0;
  bReturn = GetBuffer (nIndex, sTemp, nStringLen * sizeof (wchar_t));

  if (bReturn)
    sTarget.assign (sTemp, nStringLen);
  else
    sTarget.clear ();

  return bReturn;
}

//! read an Unicode string with the given length at the current index
//------------------------------------------------------------------
bool GT_CALL FileBuffer::GetFixedSizeWString
                                        (const file_t  nIndex,
                                               string& sTarget,
                                         const size_t  nStringLen)
//------------------------------------------------------------------
{
  wstring sTemp;
  if (!GetFixedSizeWString (nIndex, sTemp, nStringLen))
  {
    sTarget.clear ();
    return false;
  }
  // convert to string
  str_assign (sTarget, sTemp);
  return true;
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::GetFixedSizeWStringBigEndian
                                        (const file_t   nIndex,
                                               wstring& sTarget,
                                         const size_t   nStringLen)
//--------------------------------------------------------------------
{
  bool bReturn;

  // read as single-byte string and assign (with conversion)
  // _alloca: allcoate on stack -> no need to delete
  wchar_t* sTemp = (wchar_t*) _alloca ((nStringLen + 1) * sizeof (wchar_t));
  sTemp[nStringLen] = 0;
  bReturn = GetBuffer (nIndex, sTemp, nStringLen * sizeof (wchar_t));

  if (bReturn)
  {
    // convert from big endian to little endian!
    for (size_t i = 0; i < nStringLen; ++i)
      sTemp[i] = WORD_SWAP_BYTE (sTemp[i]);
    sTarget.assign (sTemp, nStringLen);
  }
  else
    sTarget.clear ();

  return bReturn;
}

//! read an Unicode string with the given length at the current index
//------------------------------------------------------------------
bool GT_CALL FileBuffer::GetFixedSizeWStringBigEndian
                                        (const file_t  nIndex,
                                               string& sTarget,
                                         const size_t  nStringLen)
//------------------------------------------------------------------
{
  wstring sTemp;
  if (!GetFixedSizeWStringBigEndian (nIndex, sTemp, nStringLen))
  {
    sTarget.clear ();
    return false;
  }
  // convert to string
  str_assign (sTarget, sTemp);
  return true;
}

//--------------------------------------------------------------------
void GT_CALL FileBuffer::GetASCIIZ_AString
                                        (const file_t   nIndex,
                                               pstring& sTarget,
                                         const size_t   nMaxTargetLen)
//--------------------------------------------------------------------
{
  char* sTemp = (char*) _alloca (nMaxTargetLen + 1);
  sTemp [nMaxTargetLen] = 0;
  // allocate on the stack
  if (!GetUntil (nIndex, 0, sTemp, nMaxTargetLen))
  {
    sTarget.clear ();

    pstring sText;
    str_assign_from_res (sText, HRC_GTLIB_FILEBUFFER_EXCEPTION_GETCSTRING);
    throw Excpt (_T (__FILE__), __LINE__,
                 sText.c_str (),
                 m_sFilename.c_str (),
                 nIndex);
  }

  // don't use max len - let the find out the length themselves!!!
  str_assign (sTarget, sTemp);
}

/*! A pascal string is different to a C-string
    because it contains the length as the first byte.
    -> the string can be a maximum of 255 byte!
 */
//--------------------------------------------------------------------
void GT_CALL FileBuffer::GetPascalString
                                        (const file_t   nIndex,
                                               pstring& sTarget)
//--------------------------------------------------------------------
{
  // get the length of the string
  const int nLen = GetUint8 (nIndex);

  // read as ANSI and convert to unicode
  char* sTemp = (char*) _alloca (nLen + 1);
  sTemp[nLen] = '\0';

  // read after length byte
  if (!GetBuffer (nIndex + 1, sTemp, nLen))
  {
    sTarget.clear ();

    pstring sText;
    str_assign_from_res (sText, HRC_GTLIB_FILEBUFFER_EXCEPTION_GETPASCALSTRING);
    throw Excpt (_T (__FILE__), __LINE__,
                 sText.c_str (),
                 m_sFilename.c_str (),
                 nIndex);
  }

  // and assign to result
  str_assign (sTarget, sTemp, nLen);
}

//--------------------------------------------------------------------
void GT_CALL FileBuffer::GetLine
                                        (const file_t        nIndex,
                                         const LineDelimiter eDelimiter,
                                               pstring&      sTarget)
//--------------------------------------------------------------------
{
  file_t nPos;
  if (eDelimiter == DELIMITER_CRLF)
    nPos = Find ("\r\n", nIndex);
  else
  if (eDelimiter == DELIMITER_CR)
    nPos = Find ('\r', nIndex);
  else
    nPos = Find ('\n', nIndex);

  if (nPos != GT_NOT_FOUND)
  {
    // get line without delimiter
    size_t nLen = size_t (nPos);

    // read as ANSI and convert to unicode
    char* sTemp = (char*) _alloca (nLen + 1);
    sTemp[nLen] = '\0';

    // read after length byte
    if (GetBuffer (nIndex, sTemp, nLen))
    {
      str_assign (sTarget, sTemp, nLen);

      // skip line end
      m_nActFilePos += (eDelimiter == DELIMITER_CRLF ? 2 : 1);
      return;
    }
  }

  // failed ...
  sTarget.clear ();

  pstring sText;
  str_assign_from_res (sText, HRC_GTLIB_FILEBUFFER_EXCEPTION_GETLINE);
  throw Excpt (_T (__FILE__), __LINE__,
               sText.c_str (),
               m_sFilename.c_str (),
               nIndex);
}


//--------------------------------------------------------------------
bool GT_CALL FileBuffer::GetUntil
                                        (const file_t  nIndex,
                                         const gtuint8 nStopByte,
                                               void*   pTargetBuffer,
                                         const size_t  nMaxTargetBufferSize,
                                               size_t& nTargetBufferSize)
//--------------------------------------------------------------------
{
  ASSERT (nIndex >= 0);
  ASSERT (pTargetBuffer);

  nTargetBufferSize = 0;
  if (nIndex > m_nFileSize)
  {
//    out_append ("GetUntil: index is out of bounds\n");
    return false;
  }

  // check if less than 1024 bytes are left from the current position of the file until EOF
  const file_t nMaxFileBufferSize = m_nFileSize - nIndex;
  const size_t nBufferSize = nMaxFileBufferSize > nMaxTargetBufferSize
                                 ? nMaxTargetBufferSize
                                 : size_t (nMaxFileBufferSize);

  // read the maximum into the temporary buffer - must fit!!
  VERIFY (GetBuffer (nIndex, pTargetBuffer, nBufferSize));

  gtuint8* p = (gtuint8*) pTargetBuffer;
  nTargetBufferSize = 0;
  while (*p != nStopByte && nTargetBufferSize < nBufferSize)
  {
    nTargetBufferSize++;
    p++;
  }

  // and set the new fileposition
  // has to be done manually because I read a maximum buffer and the filepos would be on the
  //   end of this maximum buffer
  // the + 1 is to skip the trailing nStopByte also
  m_nActFilePos = nIndex + nTargetBufferSize + 1;

  // finish it (if it is a string)
  ((gtuint8*) pTargetBuffer) [nTargetBufferSize] = '\0';

  return (nTargetBufferSize >= nBufferSize ? false : true);
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::SkipUntil
                                        (const file_t  nIndex,
                                         const gtuint8 nStopByte)
//--------------------------------------------------------------------
{
  ASSERT (nIndex >= 0);

  if (nIndex > m_nFileSize)
  {
//    out_append ("SkipUntil: index out of bounds\n");
    return false;
  }

  gtuint8 temp[GT_MAX_SIZE];
  bool bDone = false;
  file_t nCurrentIndex = nIndex;
  int nSkippedBytes = 0;

  // maybe the endbyte is not in the first buffer, so must be in a loop
  size_t nRealBufferSize;
  do
  {
    // check if less than 1024 bytes are left from the current position of the file until EOF
    const file_t nMaxFileBufferSize = m_nFileSize - nCurrentIndex;
    if (nMaxFileBufferSize <= 0)
      break;

    nRealBufferSize = nMaxFileBufferSize > GT_MAX_SIZE
                        ? GT_MAX_SIZE
                        : size_t (nMaxFileBufferSize);
    VERIFY (GetBuffer (nCurrentIndex, temp, nRealBufferSize));
    nCurrentIndex += nRealBufferSize;

    for (size_t i = 0; i < nRealBufferSize; i++)
    {
      ++nSkippedBytes;
      if (temp[i] == nStopByte)
      {
        bDone = true;
        break;  // exit FOR loop
      }
    }
  } while (nRealBufferSize == GT_MAX_SIZE && !bDone);

  // now: if m_nBufferSize != BUF_SIZE there must be an EOF :-|

  // manually set m_nActFilePos
  m_nActFilePos = nIndex + nSkippedBytes;

  return true;
}

//--------------------------------------------------------------------
file_t GT_CALL FileBuffer::Find
                                        (const gtuint8 nByte,
                                         const file_t  nStartPos,
                                         const file_t  nEndPos /* = GT_FILEBUFFER_MAX_SIZE */)
//--------------------------------------------------------------------
{
  const file_t nRealEndPos = (nEndPos == INVALID_FILE_T || nEndPos >= m_nFileSize
                               ? m_nFileSize - 1
                               : nEndPos);

  // can happen in a loop -> special check
  if (nStartPos == m_nFileSize)
    return GT_NOT_FOUND;

  ASSERT (nRealEndPos >= 0);
  ASSERT (nRealEndPos >= nStartPos);
  ASSERT (nStartPos < m_nFileSize);
  ASSERT (nRealEndPos < m_nFileSize);

  if (nRealEndPos < nStartPos)
    return GT_NOT_FOUND;

  ASSERT (nRealEndPos - nStartPos <= GT_FILEBUFFER_MAX_SIZE);

  if (m_bDynamicMapping)
  {
    // We may not cache the data ptr!
    for (file_t i = nStartPos; i <= nRealEndPos; ++i)
      if (*_GetDataPtr (i) == nByte)
        return i;
  }
  else
  {
    // cache data ptr and read linear
    gtuint8* p = _GetDataPtr (nStartPos);
    for (file_t i = nStartPos; i <= nRealEndPos; ++i, ++p)
      if (*p == nByte)
        return i;
  }

  return GT_NOT_FOUND;
}

//--------------------------------------------------------------------
file_t GT_CALL FileBuffer::Find
                                        (const char*  sString,
                                         const file_t nStartPos,
                                         const file_t nEndPos /* = GT_FILEBUFFER_MAX_SIZE */)
//--------------------------------------------------------------------
{
  const file_t nRealEndPos = (nEndPos == INVALID_FILE_T || nEndPos >= m_nFileSize
                                ? m_nFileSize - 1
                                : nEndPos);

  ASSERT (nStartPos >= 0);
  ASSERT (nRealEndPos >= 0);
  ASSERT (nRealEndPos >= nStartPos);
  ASSERT (nStartPos < m_nFileSize);
  ASSERT (nRealEndPos < m_nFileSize);

  ASSERT (nRealEndPos - nStartPos <= GT_FILEBUFFER_MAX_SIZE);

  gtuint8* pSource = _GetDataPtr (nStartPos);
  const size_t nSourceLength = size_t (nRealEndPos - nStartPos + 1);

  gtuint8* pPattern = (gtuint8*) sString;
  const size_t nPatternLength = sString ? strlen (sString) : 0;

  // init BMT table
  file_t BMT[256];
  file_t z;
  for (z = 0; z < 256; z++)
    BMT[z] = nPatternLength;

  for (z = 0; z < nPatternLength; z++)
    BMT[pPattern[z]] = nPatternLength - z - 1;

  // start searching
  file_t nSourceIndex  = nPatternLength - 1;
  file_t nPatternIndex = nPatternLength - 1;

  while (nPatternIndex >= 0 && nSourceIndex < nSourceLength)
  {
    if (pSource[nSourceIndex] == pPattern[nPatternIndex])
    {
      nSourceIndex--;
      nPatternIndex--;
    }
    else
    {
      if (nPatternLength - nPatternIndex > BMT[pSource[nSourceIndex]])
        nSourceIndex += nPatternLength - nPatternIndex;
      else
        nSourceIndex += BMT[pSource[nSourceIndex]];

      nPatternIndex = nPatternLength - 1;
    }
  }

  return nPatternIndex < 0
           ? nSourceIndex + 1
           : GT_NOT_FOUND;
}

//--------------------------------------------------------------------
file_t GT_CALL FileBuffer::iFind
                                        (const char*  sString,
                                         const file_t nStartPos,
                                         const file_t nEndPos /* = GT_FILEBUFFER_MAX_SIZE */)
//--------------------------------------------------------------------
{
  const file_t nRealEndPos = (nEndPos == INVALID_FILE_T || nEndPos >= m_nFileSize
                                ? m_nFileSize - 1
                                : nEndPos);

  ASSERT (sString);
  ASSERT (nStartPos >= 0);
  ASSERT (nRealEndPos >= 0);
  ASSERT (nRealEndPos >= nStartPos);
  ASSERT (nStartPos < m_nFileSize);
  ASSERT (nRealEndPos < m_nFileSize);

  // if an invalid or empty string is passed return NOT FOUND
  if (!sString || !*sString)
    return GT_NOT_FOUND;

  ASSERT (nRealEndPos - nStartPos <= GT_FILEBUFFER_MAX_SIZE);

  const size_t nSourceLength = size_t (nRealEndPos - nStartPos + 1);  // n
  gtuint8* pSource = new gtuint8 [nSourceLength + 1];          // a
  memcpy (pSource, _GetDataPtr (nStartPos), nSourceLength);
  pSource [nSourceLength] = 0;
  // lowercase string...
  CharLowerA ((char*) pSource);

  const size_t nPatternLength = sString ? strlen (sString) : 0;           // m
  gtuint8* pPattern = new gtuint8[nPatternLength + 1];         // b
  memcpy (pPattern, sString, nPatternLength + 1);
  // lowercase string...
  CharLowerA ((char*) pPattern);

  // init BMT table
  file_t BMT[256];
  file_t z;
  for (z = 0; z < 256; z++)
    BMT[z] = int (nPatternLength);

  for (z = 0; z < nPatternLength; z++)
    BMT[pPattern[z]] = int (nPatternLength - z - 1);

  // start scanning
  file_t nSourceIndex  = nPatternLength - 1;
  file_t nPatternIndex = nPatternLength - 1;

  while (nPatternIndex >= 0 && nSourceIndex < nSourceLength)
  {
    // get a lowercase character from the buffer...
    gtuint8 c = pSource[nSourceIndex];

    if (c == pPattern[nPatternIndex])
    {
      nSourceIndex--;
      nPatternIndex--;
    }
    else
    {
      if (nPatternLength - nPatternIndex > BMT[c])
        nSourceIndex += nPatternLength - nPatternIndex;
      else
        nSourceIndex += BMT[c];

      nPatternIndex = nPatternLength - 1;
    }
  }

  delete [] pSource;
  delete [] pPattern;

  return (nPatternIndex < 0
            ? nSourceIndex + 1
            : GT_NOT_FOUND);
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::EqualBytes
                                        (const file_t  nIndex,
                                         const size_t  nLen,
                                         const gtuint8 n)
//--------------------------------------------------------------------
{
  ASSERT (nIndex >= 0);
  ASSERT (nLen > 0);

  // comparison only inside the buffer
  if (nIndex + nLen > m_nFileSize)
    return false;

  gtuint8* p = _GetDataPtr (nIndex);
  for (size_t i = 0; i < nLen; i++, p++)
    if (*p != n)
      return false;

  return true;
}

//--------------------------------------------------------------------
bool GT_CALL FileBuffer::CompareA
                                        (const file_t nIndex,
                                         const size_t nLen,
                                         const char*  sText,
                                         const bool   bCaseSensitive /* = true */)
//--------------------------------------------------------------------
{
  ASSERT (nLen > 0);
  ASSERT (sText);
  // ASSERT (nLen == strlen (sText)); - no longer valid for L"<?xml"

  // do compare - use memcmp to handle '\0' as well!
  return bCaseSensitive
           ? (memcmp   ((char*) _GetDataPtr (nIndex), sText, nLen) == 0)
           : (_memicmp ((char*) _GetDataPtr (nIndex), sText, nLen) == 0);
}

//--------------------------------------------------------------------
void GT_CALL FileBuffer::FillStruct
                                        (Output_FilenameStruct* p)
//--------------------------------------------------------------------
{
  p->m_nSizeOfStruct  = sizeof (Output_FilenameStruct);
  p->m_sFilename      = const_cast <LPTSTR> (m_sFilename.c_str ());
  p->m_nTotalFileSize = m_nTotalFileSize;
  p->m_nScanOffset    = m_nScanOffset;

  // order is important if it is text, determine if unix or not
  p->m_nFlags         = m_bIsText
                          ? m_bIsUnix
                            ? GT_OUTPUT_FILENAME_UNIXTEXT
                            : m_bIsMac
                              ? GT_OUTPUT_FILENAME_MACTEXT
                              : GT_OUTPUT_FILENAME_DOSTEXT
                          : GT_OUTPUT_FILENAME_BINARY;
}

//! search on disk until the next gtuint32
//--------------------------------------------------------------------
bool GT_CALL FileBuffer::GotoPosOfUint32 (gtuint32 nData)
//--------------------------------------------------------------------
{
  const size_t BUFFER_SIZE = 4096;
  const size_t ROUNDTRIP_SIZE = sizeof (gtuint32) - 1;

  file_t nRestBytes = m_nFileSize - m_nActFilePos;
  gtuint8 aBuf[BUFFER_SIZE + ROUNDTRIP_SIZE];
  bool bFirstRound = true;
  file_t nLastReadPos;

  while (nRestBytes >= BUFFER_SIZE)
  {
    // remember where we are
    nLastReadPos = m_nActFilePos;

    // fill buffer (after the first round, don't overwrite old buffer)
    VERIFY (GetBuffer (aBuf + (bFirstRound ? 0 : ROUNDTRIP_SIZE), BUFFER_SIZE));

    // search buffer for parameter
    // on the first round, we have no roundtrip so don't search for it
    for (size_t i = 0; i < BUFFER_SIZE - (bFirstRound ? ROUNDTRIP_SIZE : 0); ++i)
      if (*(gtuint32*) (aBuf + i) == nData)
      {
        // found!
        SetActFilePos (nLastReadPos + i - (bFirstRound ? 0 : ROUNDTRIP_SIZE));
        return true;
      }

    // move last ROUNDTRIP_SIZE to the beginning
    memmove (aBuf, aBuf + BUFFER_SIZE - (bFirstRound ? ROUNDTRIP_SIZE : 0), ROUNDTRIP_SIZE);

    // dec rest size
    nRestBytes -= BUFFER_SIZE;

    // the first round is finished!
    bFirstRound = false;
  }

  // anything left?
  // 0 <= nRestSize < BUFFER_SIZE
  if (nRestBytes > 0)
  {
    // remember where we are
    nLastReadPos = m_nActFilePos;

    // fill buffer (after the first round, don't overwrite old buffer)
    VERIFY (GetBuffer (aBuf + (bFirstRound ? 0 : ROUNDTRIP_SIZE), gtuint32 (nRestBytes)));

    // search buffer for parameter
    // on the first round, we have no roundtrip so don't search for it
    for (size_t i = 0; i < nRestBytes - (bFirstRound ? ROUNDTRIP_SIZE : 0); ++i)
      if (*(gtuint32*) (aBuf + i) == nData)
      {
        // found!
        SetActFilePos (nLastReadPos + i - (bFirstRound ? 0 : ROUNDTRIP_SIZE));
        return true;
      }
  }

  return false;
}

}  // namespace

