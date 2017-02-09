#ifndef _GT_FILE_HXX_
#define _GT_FILE_HXX_

#ifdef _MSC_VER
#pragma once
#endif

namespace GT {

const file_t INVALID_FILE_T = file_t (-1);
const file_t FILE_MAPPING_SIZE = file_t (1024 * 1024);

//--------------------------------------------------------------------
enum EOpenError
//--------------------------------------------------------------------
{
  OPEN_SUCCESS,
  OPEN_ERROR,
  OPEN_EACCES,
  OPEN_SHARING_VIOLATION,
  OPEN_CREATE_MAPPING_ERROR,
  OPEN_MAP_VIEW_ERROR,
};

//--------------------------------------------------------------------
class GT_EXPORT_BASIC File
//--------------------------------------------------------------------
{
protected:
  HANDLE      m_hFile;        //!< file handle
  HANDLE      m_hMapOfFile;   //!< memory file mapping handle
  LPVOID      m_pFileData;    //!< pointer to the beginning of the data
  File *const m_pParent;      //!< the parent file; != NULL in nested files

  pstring  m_sFilename;       //!< the filename
  file_t   m_nScanOffset;     //!< total offset
  file_t   m_nFileSize;       //!< file size from offset
  file_t   m_nTotalFileSize;  //!< file size without start offset
  file_t   m_nActFilePos;     //!< current position within file
  bool     m_bDynamicMapping; //!< a dynamic MapViewOfFile is required (only for large files)
  file_t   m_nDynamicMapPos;  //!< the current position of the dynamic mapping
  DWORD    m_nDynamicMapSize; //!< the current size of the dynamic mapping
  file_t   m_nParentMapPos;   //!< the current position of the dynamic mapping of the parent

private:
  //! open the file
  EOpenError GT_CALL _Open ();

  //! next dynamic mapping
  DWORD GT_CALL _MapPortionAtPos (const file_t nMappingPos);

protected:
  gtuint8* GT_CALL _GetDataPtr (const file_t nOffset);

public:
  File (      LPCTSTR     sFilename,
        const file_t      nScanOffset,
              File *const pParent);

  virtual ~File ();

  // main call to open
  EOpenError GT_CALL Init ();

  LPCTSTR        GT_CALL GetpFileName () const { return m_sFilename.c_str (); }
  const pstring& GT_CALL GetFileName  () const { return m_sFilename; }

  file_t  GT_CALL GetActFilePos () const { return m_nActFilePos; }
  void    GT_CALL SetActFilePos (const file_t nNewActFilePos);   /* throws Excpt */
  void    GT_CALL IncActFilePos (const file_t n);   /* throws Excpt */
  bool    GT_CALL IncActFilePosNoThrow (const file_t n);

  void    GT_CALL SetToEOF  ()                   { m_nActFilePos = m_nFileSize; }
  file_t  GT_CALL GetScanOffset () const         { return m_nScanOffset; }
  void    GT_CALL IncScanOffset (const file_t n) { m_nScanOffset += n; m_nFileSize -= n; }

  file_t GT_CALL GetFileSize      () const { return m_nFileSize; }
  file_t GT_CALL GetTotalFileSize () const { return m_nTotalFileSize; }
};

} // namespace

#endif
