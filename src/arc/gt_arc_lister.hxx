#ifndef _GT_ARCHIVELISTER_HXX_
#define _GT_ARCHIVELISTER_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include <list>

namespace GT {

class FileBuffer;

//--------------------------------------------------------------------
enum EArchiveEC
//--------------------------------------------------------------------
{
  ARCERR_SUCCESS,
  ARCERR_UNEXPECTED_EOF,
  ARCERR_UNEXPECTED_VALUE,
  ARCERR_MISSING_DATA,
  ARCERR_UNKNOWN_METHOD,
  ARCERR_EXCEPTION,
};

//--------------------------------------------------------------------
enum EArchiveFileFlags
//--------------------------------------------------------------------
{
  ARCFILE_PASSWORD            = 0x0001,
  ARCFILE_DIR                 = 0x0002,
  ARCFILE_CONTINUED_FROM_LAST = 0x0004,
  ARCFILE_CONTINUED_ON_NEXT   = 0x0008,
  ARCFILE_COMMENT             = 0x0010,
};

//--------------------------------------------------------------------
class ArchiveFile
//--------------------------------------------------------------------
{
private:
  pstring  m_sFilename;
  file_t   m_nPackedSize;
  file_t   m_nUnpackedSize;
  gtuint64 m_nDT;
  gtuint32 m_nFlags;

  ArchiveFile ();
  ArchiveFile (const ArchiveFile&);
  ArchiveFile& operator = (const ArchiveFile&);

  //------------------------------------------------------------------
  void _SetFlag (const gtuint32 nFlag, const int bEnable)
  //------------------------------------------------------------------
  {
    if (bEnable)
      m_nFlags |= nFlag;
    else
      m_nFlags &= ~nFlag;
  }

public:
  //------------------------------------------------------------------
  explicit ArchiveFile (LPCTSTR pFilename)
  //------------------------------------------------------------------
    : m_sFilename     (pFilename),
      m_nPackedSize   (-1),
      m_nUnpackedSize (-1),
      m_nDT           (gtuint64 (-1)),
      m_nFlags        (0)
  {}

  //------------------------------------------------------------------
  explicit ArchiveFile (const pstring& sFilename)
  //------------------------------------------------------------------
    : m_sFilename     (sFilename),
      m_nPackedSize   (-1),
      m_nUnpackedSize (-1),
      m_nDT           (gtuint64 (-1)),
      m_nFlags        (0)
  {}

  //! get filename
  //------------------------------------------------------------------
  const pstring& GetFilename () const
  //------------------------------------------------------------------
  {
    return m_sFilename;
  }

  //! set packed size
  //------------------------------------------------------------------
  void SetPackedSize (const file_t nPackedSize)
  //------------------------------------------------------------------
  {
    ASSERT (m_nPackedSize == -1);
    m_nPackedSize = nPackedSize;
  }

  //! get packed size
  //------------------------------------------------------------------
  file_t GetPackedSize () const
  //------------------------------------------------------------------
  {
    // packed size is always present!
    ASSERT (m_nPackedSize != -1);
    return m_nPackedSize;
  }

  //! set unpacked size
  //------------------------------------------------------------------
  void SetUnpackedSize (const file_t nUnpackedSize)
  //------------------------------------------------------------------
  {
    ASSERT (m_nUnpackedSize == -1);
    m_nUnpackedSize = nUnpackedSize;
  }

  //! get unpacked size
  //------------------------------------------------------------------
  file_t GetUnpackedSize () const
  //------------------------------------------------------------------
  {
    // unpacked size should not be queried if 0
    ASSERT (m_nUnpackedSize != -1);
    return m_nUnpackedSize;
  }

  //! set date/time
  //------------------------------------------------------------------
  void SetDT (const gtuint64 nDT)
  //------------------------------------------------------------------
  {
    ASSERT (m_nDT == gtuint64 (-1));
    m_nDT = nDT;
  }

  //! get date/time
  //------------------------------------------------------------------
  gtuint64 GetDT () const
  //------------------------------------------------------------------
  {
    // date/time should not be queried if 0
    ASSERT (m_nDT != gtuint64 (-1));
    return m_nDT;
  }

  //! mark the file as password protected
  //------------------------------------------------------------------
  void SetPWProtected (const int bEnabled)
  //------------------------------------------------------------------
  {
    _SetFlag (ARCFILE_PASSWORD, bEnabled);
  }

  //! is the file password protected?
  //------------------------------------------------------------------
  int IsPWProtected () const
  //------------------------------------------------------------------
  {
    return m_nFlags & ARCFILE_PASSWORD;
  }

  //! mark the file as directory
  //------------------------------------------------------------------
  void SetDirectory (const int bEnabled)
  //------------------------------------------------------------------
  {
    _SetFlag (ARCFILE_DIR, bEnabled);
  }

  //! is the file a directory?
  //------------------------------------------------------------------
  int IsDirectory () const
  //------------------------------------------------------------------
  {
    return m_nFlags & ARCFILE_DIR;
  }

  //! mark the file as continued from last volume
  //------------------------------------------------------------------
  void SetContinuedFromLast (const int bEnabled)
  //------------------------------------------------------------------
  {
    _SetFlag (ARCFILE_CONTINUED_FROM_LAST, bEnabled);
  }

  //! is the file continued from last volume?
  //------------------------------------------------------------------
  int IsContinuedFromLast () const
  //------------------------------------------------------------------
  {
    return m_nFlags & ARCFILE_CONTINUED_FROM_LAST;
  }

  //! mark the file as continued on next volume
  //------------------------------------------------------------------
  void SetContinuedOnNext (const int bEnabled)
  //------------------------------------------------------------------
  {
    _SetFlag (ARCFILE_CONTINUED_ON_NEXT, bEnabled);
  }

  //! is the file continued on next volume?
  //------------------------------------------------------------------
  int IsContinuedOnNext () const
  //------------------------------------------------------------------
  {
    return m_nFlags & ARCFILE_CONTINUED_ON_NEXT;
  }

  //! does the file have a comment
  //------------------------------------------------------------------
  void SetComment (const int bEnabled)
  //------------------------------------------------------------------
  {
    _SetFlag (ARCFILE_COMMENT, bEnabled);
  }

  //! has the file a comment
  //------------------------------------------------------------------
  int HasComment () const
  //------------------------------------------------------------------
  {
    return m_nFlags & ARCFILE_COMMENT;
  }
};

//--------------------------------------------------------------------
class ArchiveFileList : public std::list <ArchiveFile*>
//--------------------------------------------------------------------
{
public:
  //------------------------------------------------------------------
  virtual ~ArchiveFileList ()
  //------------------------------------------------------------------
  {
    iterator it = begin ();
    for (; !(it == end ()); ++it)
      delete *it;
  }
};

/*! Flags that apply to a complete archive.
 */
//--------------------------------------------------------------------
enum EArchiveFlags
//--------------------------------------------------------------------
{
  // first 2 bits (0 -> no DT)
  ARC_DT_DOS           = 0x0001,  // flags & 3 = 1
  ARC_DT_WIN           = 0x0002,  // flags & 3 = 2
  ARC_DT_UNIX          = 0x0003,  // flags & 3 = 3
  ARC_ENCRYPTED        = 0x0004,  //!< the archive is completely encrypted
  ARC_NO_PACKED_SIZE   = 0x0008,  //!< the packed size of a file is not stored
  ARC_NO_UNPACKED_SIZE = 0x0010,  //!< the unpacked size of a file is not stored
  ARC_SOLID            = 0x0040,  //!< solid archive
  ARC_COMMENT          = 0x0080,  //!< archive comment present
  ARC_MULTI_VOLUME     = 0x0100,  //!< it's a multi volume archive
  ARC_LOCKED           = 0x0200,  //!< is the archive locked?
  ARC_AUTHENTIFICATION = 0x0400,  //!< authentification info present?
  ARC_RECOVERY_RECORD  = 0x0800,  //!< recovery record present?
};

//--------------------------------------------------------------------
struct ArchiveVersion
//--------------------------------------------------------------------
{
  gtuint32 m_nMajor;
  gtuint32 m_nMinor;

  ArchiveVersion ()
    : m_nMajor (gtuint32 (-1)),
      m_nMinor (gtuint32 (-1))
  {}

  bool Use () const
  {
    return m_nMajor != gtuint32 (-1) &&
           m_nMinor != gtuint32 (-1);
  }
};

//--------------------------------------------------------------------
class ArchiveContent
//--------------------------------------------------------------------
{
private:
  ArchiveVersion  m_aVersion;      //!< archive version
  gtuint32        m_nFlags;        //!< archive flags
  ArchiveFileList m_aFiles;        //!< file list
  pstring         m_sAddOnString;
  EArchiveEC      m_eErrorCode;    //!< listing error

protected:
  //------------------------------------------------------------------
  explicit ArchiveContent ()
  //------------------------------------------------------------------
    : m_nFlags     (0),
      m_eErrorCode (ARCERR_SUCCESS)
  {}

  //! set any flag
  //------------------------------------------------------------------
  void _SetArchiveFlag (int nFlag, int bEnable = true)
  //------------------------------------------------------------------
  {
    if (bEnable)
      m_nFlags |= nFlag;
    else
      m_nFlags &= ~nFlag;
  }

  //! check any of the flags
  //------------------------------------------------------------------
  int _IsArchiveFlag (const int nFlag) const
  //------------------------------------------------------------------
  {
    return m_nFlags & nFlag;
  }

  //! add a file
  //------------------------------------------------------------------
  void _AddFile (ArchiveFile* pArcFile)
  //------------------------------------------------------------------
  {
    m_aFiles.push_back (pArcFile);
  }

  //! do we have date/time per file?
  //------------------------------------------------------------------
  int _HasDT () const
  //------------------------------------------------------------------
  {
    return _IsArchiveFlag (3);
  }

  //! set optional archive version
  //------------------------------------------------------------------
  void _SetArchiveVersion (const gtuint32 nMajor, const gtuint32 nMinor)
  //------------------------------------------------------------------
  {
    ASSERT (!m_aVersion.Use ());
    m_aVersion.m_nMajor = nMajor;
    m_aVersion.m_nMinor = nMinor;
  }

  //! set special AddOn string
  //------------------------------------------------------------------
  void _SetAddOnString (const pstring& sAddOnString)
  //------------------------------------------------------------------
  {
    ASSERT (m_sAddOnString.empty ());
    m_sAddOnString = sAddOnString;
  }

public:
  EArchiveEC GetErrorCode () const { return m_eErrorCode; }
  void       SetErrorCode (const EArchiveEC eErrorCode) { m_eErrorCode = eErrorCode; }

  // display result
  void Display ();
};

//--------------------------------------------------------------------
class BasicArchiveLister : public ArchiveContent
//--------------------------------------------------------------------
{
private:
  BasicArchiveLister ();
  BasicArchiveLister (const BasicArchiveLister&);
  BasicArchiveLister& operator = (const BasicArchiveLister&);

protected:
        FileBuffer* m_pBuffer;       //!< basic buffer
  const file_t      m_nLocalOffset;  //!< start offset within FileBuffer offset

public:
  BasicArchiveLister (      FileBuffer* pBuffer,
                      const file_t      nOffset);

  // virtual dtor recommended because virtual methods are present
  virtual ~BasicArchiveLister ()
  {}

  // overridden by each lister
  virtual void ListFiles ();
};

// virtual dtor needed by GCC
#define ARC_CREATE_CLASS(__class) \
             class __class##ArchiveLister : public BasicArchiveLister \
             { \
             private: \
               __class##ArchiveLister (); \
               __class##ArchiveLister (const __class##ArchiveLister&); \
               __class##ArchiveLister& operator = (const __class##ArchiveLister&); \
             public: \
               __class##ArchiveLister (FileBuffer *p, const file_t n) \
                 : BasicArchiveLister (p, n) {} \
               virtual ~__class##ArchiveLister () {} \
               virtual void ListFiles (); \
             };

#define ARC_DEFINE_LISTMETHOD(__class) \
             void __class##ArchiveLister::ListFiles ()

ARC_CREATE_CLASS (_777);
ARC_CREATE_CLASS (ACE);
ARC_CREATE_CLASS (AIN);
ARC_CREATE_CLASS (AKT7);
ARC_CREATE_CLASS (AKT9);
ARC_CREATE_CLASS (AMG);
ARC_CREATE_CLASS (ARC);
ARC_CREATE_CLASS (ARG);
ARC_CREATE_CLASS (ARJ);
ARC_CREATE_CLASS (ARQ);
ARC_CREATE_CLASS (ARX);
ARC_CREATE_CLASS (ASD);
ARC_CREATE_CLASS (BIX);
ARC_CREATE_CLASS (BLI);
ARC_CREATE_CLASS (BOA);
ARC_CREATE_CLASS (BSA);
ARC_CREATE_CLASS (BSN);
ARC_CREATE_CLASS (CAB);
ARC_CREATE_CLASS (DPA);
ARC_CREATE_CLASS (GZ);
ARC_CREATE_CLASS (HA);
ARC_CREATE_CLASS (INNO);

//--------------------------------------------------------------------
class IMPArchiveLister : public BasicArchiveLister
//--------------------------------------------------------------------
{
protected:
  int      g_ncodes [3];
  int      g_ncoffset [3];
  int      g_nBitPos;
  gtuint32 g_nBitBuf;

  void     _InitBits ();
  gtuint32 _GetBits (const int nBits);
  void     _DelBits (const int nBits);

  void load_fixed_codes (gtuint8* hclens,
                         long     exsize);

  void make_codes (gtuint32* huffcodes,
                   gtuint8*  hclengths,
                   int       ncodes);

  int make_decode_tables (int*      ttable,
                          int       hdchain[][2],
                          gtuint32* huffcodes,
                          gtuint8 * hclengths,
                          int       ttbits,
                          int       ncodes);

  int load_hcodes (gtuint8* hclengths,
                   int      ncodes,
                   int      ntrees,
                   long     exsize);

  int expand_lz (gtuint8* output,
                 int      start,
                 int      exsize);

public:
  IMPArchiveLister (      FileBuffer* pBuffer,
                    const file_t      nOffset);

  void ListFiles ();
};

ARC_CREATE_CLASS (INS);
ARC_CREATE_CLASS (ISC);
ARC_CREATE_CLASS (ISO);
ARC_CREATE_CLASS (LG);
ARC_CREATE_CLASS (LZH);
ARC_CREATE_CLASS (LZO);
ARC_CREATE_CLASS (NERO5);
ARC_CREATE_CLASS (NERO);
ARC_CREATE_CLASS (NSIS13);
ARC_CREATE_CLASS (NSIS16);
ARC_CREATE_CLASS (PAQ);
ARC_CREATE_CLASS (PAQ8);
ARC_CREATE_CLASS (PPZ);
ARC_CREATE_CLASS (RAR);
ARC_CREATE_CLASS (SBC);
ARC_CREATE_CLASS (SONY);
ARC_CREATE_CLASS (STI);
ARC_CREATE_CLASS (SYN);
ARC_CREATE_CLASS (TAR);
ARC_CREATE_CLASS (XPA);
ARC_CREATE_CLASS (YC);
ARC_CREATE_CLASS (ZET);
ARC_CREATE_CLASS (ZIP);
ARC_CREATE_CLASS (ZOO);

}  // namespace

#endif
