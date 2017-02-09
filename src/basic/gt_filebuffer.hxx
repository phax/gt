#ifndef _GT_FILEBUFFER_HXX_
#define _GT_FILEBUFFER_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gt_file.hxx"

namespace GT {

struct Output_FilenameStruct;

// callback function type
typedef int (GT_CALL* FileListerProc) (LPCTSTR, const file_t, File *const);

// default return on error
const int GT_NOT_FOUND = -1;

// should be the maximum search size
const file_t GT_FILEBUFFER_MAX_SIZE = file_t (1024 * 1024);

/**
 * Creates access methods like:
 *
 * bool ReadUint8 (gtuint8&)
 * gtuint8 GetUint8 (const file_t)
 * gtuint8 GetUint8 ()
 * gtuint8 LookAheadUint8 ()
 */
#define FILEBUFFER_DECLARE_READ(_type,_name) \
  bool GT_CALL Read##_name (_type& nByte) { \
    return Read##_name (m_nActFilePos, nByte); \
  } \
  _type GT_CALL Get##_name (const file_t nIndex) { \
    _type tmp; \
    Read##_name (nIndex, tmp); \
    return tmp; \
  } \
  _type GT_CALL Get##_name () { \
    return Get##_name (m_nActFilePos); \
  } \
  _type GT_CALL LookAhead##_name () { \
    const file_t oldpos = m_nActFilePos; \
    _type tmp; \
    Read##_name (m_nActFilePos, tmp); \
    m_nActFilePos = oldpos; \
    return tmp; \
  }

#define FILEBUFFER_DECLARE_READ_UINT(_x) FILEBUFFER_DECLARE_READ (gtuint##_x, Uint##_x)
#define FILEBUFFER_DECLARE_READ_INT(_x)  FILEBUFFER_DECLARE_READ (gtint##_x, Int##_x)

//--------------------------------------------------------------------
enum LineDelimiter
//--------------------------------------------------------------------
{
  DELIMITER_CRLF,  // \r\n
  DELIMITER_CR,    // \r
  DELIMITER_LF,    // \n
};

//--------------------------------------------------------------------
class GT_EXPORT_BASIC FileBuffer : public File
//--------------------------------------------------------------------
{
private:
  FileListerProc m_aListProc;

  bool m_bIsText;
  bool m_bIsUnix;
  bool m_bIsMac;
  bool m_bIs7Bit;

private:
  // check what kind of file it is
  void GT_CALL _DetermineIfText ();

  // checked version of memcpy in case of dynamic mapping
  void GT_CALL _memcpy (void* pDst, file_t nIndex, size_t nLen);

public:
  FileBuffer (      LPCTSTR        sFilename,
              const file_t         nOffset,
                    FileListerProc aListProc,
                    File *const    pParent);

  EOpenError GT_CALL Init ();

  gtuint8* GT_CALL GetLastBuffer (size_t n) { return _GetDataPtr (m_nFileSize - n); }

  bool GT_CALL IsText     () const { return m_bIsText; }
  bool GT_CALL IsUnixText () const { return m_bIsUnix; }
  bool GT_CALL IsMacText  () const { return m_bIsMac; }
  bool GT_CALL Is7Bit     () const { return m_bIs7Bit; }
  bool GT_CALL IsEOF      () const { return m_nActFilePos >= m_nFileSize; }

  void GT_CALL CallListProc (const file_t nNewOffset) const;

  //! read an uint8 at the given index
  bool GT_CALL ReadUint8 (const file_t nIndex, gtuint8& nByte);

  //! read an uint16 at the given index
  bool GT_CALL ReadUint16 (const file_t nIndex, gtuint16& nWord);

  //! read an int32 at the given index
  bool GT_CALL ReadInt32 (const file_t nIndex, gtint32& nLong);

  //! read an int64 at the given index
  bool GT_CALL ReadInt64 (const file_t nIndex, gtint64& nLongLong);

  FILEBUFFER_DECLARE_READ_UINT (8)
  FILEBUFFER_DECLARE_READ_UINT (16)
  FILEBUFFER_DECLARE_READ_INT (32)
  FILEBUFFER_DECLARE_READ_INT (64)

  //! get a buffer at the desired position
  void GT_CALL GetBufferX (const file_t nIndex,
                                 void*  pTargetBuffer,
                           const size_t nLen);   /* throws GT_BufferException */

  //! get a buffer at the current position
  //------------------------------------------------------------------
  void GT_CALL GetBufferX (      void*  pTargetBuffer,
                           const size_t nLen)
  //------------------------------------------------------------------
  {
    GetBufferX (m_nActFilePos, pTargetBuffer, nLen);
  }

  //! get a buffer at the desired position
  bool GT_CALL GetBuffer (const file_t nIndex,
                                void*  pTargetBuffer,
                          const size_t nLen);

  //! get a buffer at the current position
  //------------------------------------------------------------------
  bool GT_CALL GetBuffer (      void*  pTargetBuffer,
                          const size_t nLen)
  //------------------------------------------------------------------
  {
    return GetBuffer (m_nActFilePos, pTargetBuffer, nLen);
  }

  //! read an ASCIIZ string with the given length at the given index
  bool GT_CALL GetFixedSizeAString (const file_t  nIndex,
                                          string& sTarget,
                                    const size_t  nStringLen);

  //! read an ASCIIZ string with the given length at the current index
  //------------------------------------------------------------------
  bool GT_CALL GetFixedSizeAString (      string& sTarget,
                                    const size_t  nStringLen)
  //------------------------------------------------------------------
  {
    return GetFixedSizeAString (m_nActFilePos, sTarget, nStringLen);
  }

  //! read an ASCIIZ string with the given length at the given index
  bool GT_CALL GetFixedSizeAString (const file_t   nIndex,
                                          wstring& sTarget,
                                    const size_t   nStringLen);

  //! read an ASCIIZ string with the given length at the current index
  //------------------------------------------------------------------
  bool GT_CALL GetFixedSizeAString (      wstring& sTarget,
                                    const size_t   nStringLen)
  //------------------------------------------------------------------
  {
    return GetFixedSizeAString (m_nActFilePos, sTarget, nStringLen);
  }

  //! read an Unicode string with the given length at the given index
  bool GT_CALL GetFixedSizeWString (const file_t   nIndex,
                                          wstring& sTarget,
                                    const size_t   nStringLen);

  //! read an Unicode string with the given length at the current index
  //------------------------------------------------------------------
  bool GT_CALL GetFixedSizeWString (      wstring& sTarget,
                                    const size_t   nStringLen)
  //------------------------------------------------------------------
  {
    return GetFixedSizeWString (m_nActFilePos, sTarget, nStringLen);
  }

  //! read an Unicode string with the given length at the current index
  bool GT_CALL GetFixedSizeWString (const file_t  nIndex,
                                          string& sTarget,
                                    const size_t  nStringLen);

  //! read an Unicode string with the given length at the given index
  //------------------------------------------------------------------
  bool GT_CALL GetFixedSizeWString (      string& sTarget,
                                    const size_t  nStringLen)
  //------------------------------------------------------------------
  {
    return GetFixedSizeWString (m_nActFilePos, sTarget, nStringLen);
  }

  //! read an Unicode string with the given length at the given index
  bool GT_CALL GetFixedSizeWStringBigEndian (const file_t   nIndex,
                                                   wstring& sTarget,
                                             const size_t   nStringLen);

  //! read an Unicode string with the given length at the current index
  //------------------------------------------------------------------
  bool GT_CALL GetFixedSizeWStringBigEndian (      wstring& sTarget,
                                             const size_t   nStringLen)
  //------------------------------------------------------------------
  {
    return GetFixedSizeWStringBigEndian (m_nActFilePos, sTarget, nStringLen);
  }

  //! read an Unicode string with the given length at the current index
  bool GT_CALL GetFixedSizeWStringBigEndian (const file_t  nIndex,
                                                   string& sTarget,
                                             const size_t  nStringLen);

  //! read an Unicode string with the given length at the given index
  //------------------------------------------------------------------
  bool GT_CALL GetFixedSizeWStringBigEndian (      string& sTarget,
                                             const size_t  nStringLen)
  //------------------------------------------------------------------
  {
    return GetFixedSizeWStringBigEndian (m_nActFilePos, sTarget, nStringLen);
  }

  //! read a string until the next 0 Byte, but with a max length
  void GT_CALL GetASCIIZ_AString (const file_t   nIndex,
                                        pstring& sTarget,
                                  const size_t   nMaxLen);   /* throws GT_BufferException */

  //! read a string until the next 0 Byte, but with a max length at the current position
  //------------------------------------------------------------------
  void GT_CALL GetASCIIZ_AString (      pstring& sTarget,
                                  const size_t   nMaxLen)
  //------------------------------------------------------------------
  {
    return GetASCIIZ_AString (m_nActFilePos, sTarget, nMaxLen);
  }

  // read a string at the given position (first byte = len)
  void GT_CALL GetPascalString (const file_t   nIndex,
                                      pstring& sTarget); /* throws GT_BufferException */

  //------------------------------------------------------------------
  void GT_CALL GetPascalString (pstring& sTarget)
  //------------------------------------------------------------------
  {
    return GetPascalString (m_nActFilePos, sTarget);
  }

  // read a line at the given position until the specified delimiter
  // Does not return the line end, although the line end is skipped
  void GT_CALL GetLine (const file_t        nIndex,
                        const LineDelimiter eDelimiter,
                              pstring&      sTarget); /* throws GT_BufferException */

  //------------------------------------------------------------------
  void GT_CALL GetLine (const LineDelimiter eDelimiter, pstring& sTarget)
  //------------------------------------------------------------------
  {
    return GetLine (m_nActFilePos, eDelimiter, sTarget);
  }

  // get all chars until the specified stopbyte
  bool GT_CALL GetUntil (const file_t  nIndex,
                         const gtuint8 nStopByte,
                               void*   pTargetBuffer,
                         const size_t  nMaxTargetBufferSize,
                               size_t& nTargetBufferSize);


  //------------------------------------------------------------------
  bool GT_CALL GetUntil (const gtuint8 nStopByte,
                               void*   pTargetBuffer,
                         const size_t  nMaxTargetBufferSize,
                               size_t& nTargetBufferSize)
  //------------------------------------------------------------------
  {
    return GetUntil (m_nActFilePos,
                     nStopByte,
                     pTargetBuffer,
                     nMaxTargetBufferSize,
                     nTargetBufferSize);
  }

  //------------------------------------------------------------------
  bool GT_CALL GetUntil (const file_t  nIndex,
                         const gtuint8 nStopByte,
                               void*   pTargetBuffer,
                         const size_t  nMaxTargetBufferSize)
  //------------------------------------------------------------------
  {
    size_t junk;
    return GetUntil (nIndex, nStopByte, pTargetBuffer, nMaxTargetBufferSize, junk);
  }


  //------------------------------------------------------------------
  bool GT_CALL GetUntil (const gtuint8 nStopByte,
                               void*   pTargetBuffer,
                         const size_t  nMaxTargetBufferSize)
  //------------------------------------------------------------------
  {
    size_t junk;
    return GetUntil (m_nActFilePos, nStopByte, pTargetBuffer, nMaxTargetBufferSize, junk);
  }

  // skip all bytes until you reach the selected
  bool GT_CALL SkipUntil (const file_t  nIndex,
                          const gtuint8 nStopByte);

  //------------------------------------------------------------------
  bool SkipUntil (const gtuint8 nStopByte)
  //------------------------------------------------------------------
  {
    return SkipUntil (m_nActFilePos, nStopByte);
  }

  // returns GT_NOT_FOUND on error or if not found
  // Delivers a position relative to the start position
  file_t GT_CALL Find (const gtuint8 nByte,
                       const file_t  nStartPos,
                       const file_t  nEndPos = GT_FILEBUFFER_MAX_SIZE);

  // returns GT_NOT_FOUND on error or if not found
  // Delivers a position relative to the start position
  file_t GT_CALL Find (const char*  sString,
                       const file_t nStartPos,
                       const file_t nEndPos = GT_FILEBUFFER_MAX_SIZE);

  // returns GT_NOT_FOUND on error or if not found
  // Delivers a position relative to the start position
  file_t GT_CALL iFind (const char*  sString,
                        const file_t nStartPos,
                        const file_t nEndPos = GT_FILEBUFFER_MAX_SIZE);

  // case sensitive compare
  bool GT_CALL EqualBytes (const file_t  nIndex,
                           const size_t  nLen,
                           const gtuint8 n);

  // compare with string data
  bool GT_CALL CompareA (const file_t nIndex,
                         const size_t nLen,
                         const char*  sText,
                         const bool   bCaseSensitive = true);

  void GT_CALL FillStruct (Output_FilenameStruct* p);

  //! search on disk until the next gtuint32
  bool GT_CALL GotoPosOfUint32 (gtuint32 nData);
};

} // namespace

#endif
