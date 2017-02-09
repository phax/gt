#ifndef _GT_FILESEARCH_HXX_
#define _GT_FILESEARCH_HXX_

#ifndef _GT_STDAFX_HXX_
#include "gt_stdafx.hxx"
#include "gt_dll.hxx"
#endif

namespace GT {

#define FILESEARCH_ERROR  (-1)
#define FA_NORMAL    (0x00)
#define FA_READONLY  (0x01)
#define FA_HIDDEN    (0x02)
#define FA_SYSTEM    (0x04)
#define FA_SUBDIR    (0x10)
#define FA_ARCHIVE   (0x20)
#define FA_ANY       (0x3F)

struct FileSearchData;

/*! File searching class.
 */
//--------------------------------------------------------------------
class GT_EXPORT_BASIC FileSearch
//--------------------------------------------------------------------
{
private:
  bool  operator ! ();  //!< avoid usage
  operator void* ();    //!< avoid usage

protected:
  bool            m_bError;
  bool            m_bOpened;
  bool            m_bClosed;
  FileSearchData* m_pData;

public:
  FileSearch ();

  virtual ~FileSearch ();

  bool     GT_CALL IsError () const { return m_bError; }

  void     GT_CALL FindFirst (LPCTSTR sFilename);
  void     GT_CALL FindNext ();
  void     GT_CALL FindClose ();

  LPCTSTR  GT_CALL GetFilename () const;
  file_t   GT_CALL GetFilesize () const;
  gtuint32 GT_CALL GetAttr () const;

  bool     GT_CALL IsDirectory () const;

  //! is it a valid (== scannable) directory?
  //------------------------------------------------------------------
  bool GT_CALL IsValidDirectory () const
  //------------------------------------------------------------------
  {
    return IsDirectory () && (*GetFilename ()) != _T ('.');
  }

  //! is it a valid file?
  //------------------------------------------------------------------
  bool GT_CALL IsValidFile () const
  //------------------------------------------------------------------
  {
    return !IsDirectory ();
  }
};

}  // namespace

#endif
