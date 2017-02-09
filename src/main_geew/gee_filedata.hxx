#ifndef _GEE_FILEDATA_HXX_
#define _GEE_FILEDATA_HXX_

#include <list>

#include "gt_filebuffer.hxx"
#include "gt_exe_pe.hxx"
using namespace GT;

namespace GEE {

/*! Must be in bit format because I use it in 'WriteByteLookup'
 */
//--------------------------------------------------------------------
enum GEEFormat
//--------------------------------------------------------------------
{
  UNKNOWN = 0x0001,
  ANYFILE = 0x0002,
  EXE_DOS = 0x0004,
  EXE_NE  = 0x0008,
  EXE_LE  = 0x0010,
  EXE_PE  = 0x0020,
};

// must be < 0!!!
const int GEE_EOF = -1;
const size_t MAX_READ = 1024;

/**
 * Represents the data of a single EXE file.
 */
//--------------------------------------------------------------------
class GEEFileData
//--------------------------------------------------------------------
{
private:
  FileBuffer* m_pBuffer;
  GEEFormat   m_eFormat;
  file_t      m_nNewExeOffset;
  file_t      m_nEntryPoint;

  gtuint8     m_aBuf[MAX_READ];
  gtuint8*    m_pCur;
  gtuint8*    m_pEnd;

  // needed for macro manager:
  EXE_PE_ImageFileHeader       m_aIFH;
  EXE_PE_OptionalFileHeader    m_aOFH;
  EXE_PE_SectionTableAnalyzer* m_pSTA;

  void _Read ();

public:
  explicit GEEFileData (LPCTSTR sFilename);
  virtual ~GEEFileData ();

  // helper stuff
  file_t            getEntryPoint () const { return m_nEntryPoint; }
  const FileBuffer* getBuffer     () const { return m_pBuffer; }
  GEEFormat         getFormat     () const { return m_eFormat; }

  // init stuff
  bool   isOkay () const;
  bool   determineFileFormat ();
  bool   evaluateMacro (LPCTSTR pMacro);
  void   resetToStart ();

  // compare stuff
  int    getNextChar ();

  // opcode helper stuff
  bool   getTargetPosition (file_t  nStartPos,
                            LPCTSTR pMacro,
                            file_t& nTargetPos);
};

/**
 * This is a wrapper around of a list of GEEFileData objects.
 */
//--------------------------------------------------------------------
class FileDataSeq : public std::list <GEEFileData*>
//--------------------------------------------------------------------
{
private:
  FileDataSeq () {}

public:
  static FileDataSeq& Instance ();

  /*! Delete all elements in the dtor.
   */
  //------------------------------------------------------------------
  virtual ~FileDataSeq ()
  //------------------------------------------------------------------
  {
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
      delete (*cit);
  }

  /*! Check if the filename pName is already in the list.
   */
  //------------------------------------------------------------------
  bool containsFilename (LPCTSTR pName)
  //------------------------------------------------------------------
  {
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
      if (_tcsicmp ((*cit)->getBuffer ()->GetpFileName (), pName) == 0)
        return true;
    return false;
  }

  /*! Get the comparison format (EXE_DOS, EXE_PE, ...)
      If the format is not the same for all entries, false is returned.
   */
  //------------------------------------------------------------------
  bool getFormat (GEEFormat& eFormat)
  //------------------------------------------------------------------
  {
    if (empty ())
      return false;

    // get the format of the first entry
    const_iterator cit = begin ();
    eFormat = (*cit)->getFormat ();

    for (; !(cit == end ()); ++cit)
      if ((*cit)->getFormat () != eFormat)
        return false;
    return true;
  }

  /*!
   * Returns -1 for unexpected EOF
   * Returns 0 if the bytes are totally different
   * Returns >0 if a matching byte was found
   */
  //------------------------------------------------------------------
  int getNextComparedByte () const
  //------------------------------------------------------------------
  {
    const_iterator cit = begin ();

    // get byte of first file
    GEEFileData *pFD = *cit;
    int n1 = pFD->getNextChar (), n2;
    if (n1 == GEE_EOF)
      return GEE_EOF;

    for (++cit; !(cit == end ()); ++cit)
    {
      pFD = *cit;

      // get byte of the other files
      n2 = pFD->getNextChar ();
      if (n2 == GEE_EOF)
        return GEE_EOF;

      // 0 means: different
      // -> continue to scan all files because
      // otherwise the relative indices would be different!
      if (n1 != n2)
        n1 = 0;
    }

    return n1;
  }

  //------------------------------------------------------------------
  bool canResolveAllMacros (LPCTSTR pMacro) const
  //------------------------------------------------------------------
  {
    GEEFileData *pFD;
    file_t nTargetPos;
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
    {
      pFD = *cit;
      if (!pFD->getTargetPosition (pFD->getEntryPoint (), pMacro, nTargetPos))
        return false;
    }

    return true;
  }

  //------------------------------------------------------------------
  void resetToStart () const
  //------------------------------------------------------------------
  {
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
      (*cit)->resetToStart ();
  }
};

}  // namespace

#endif
