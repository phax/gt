#include "gt_file.hxx"

#include "gt_utils.hxx"  // _abs
#include "gt_errormsg.hxx"
#include "gt_exception.hxx"

namespace GT  {

//--------------------------------------------------------------------
EOpenError GT_CALL File::_Open ()
//--------------------------------------------------------------------
{
  DWORD nLastError;
  DWORD nHighDWordFileSize;
  TCHAR sMessage[1024];

  // open file
  m_hFile = ::CreateFile (m_sFilename.c_str (),
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_EXISTING,
                          FILE_FLAG_RANDOM_ACCESS,
                          0);
  if (m_hFile == INVALID_HANDLE_VALUE)
  {
    // error opening file
    nLastError = ::GetLastError ();

    // get error message
    pstring sRes;
    str_assign_from_res (sRes, HRC_GTLIB_FILEBUFFER_FAILED_TO_OPEN);
    _stprintf (sMessage, sRes.c_str (), m_sFilename.c_str ());

    switch (nLastError)
    {
      case ERROR_ACCESS_DENIED:
        // don't show a message if access is denied because
        // this may happen frequently
        return OPEN_EACCES;
      case ERROR_SHARING_VIOLATION:
        // don't show a message if access is denied because
        // this may happen frequently
        return OPEN_SHARING_VIOLATION;
      default:
        ShowWin32Error (nLastError, sMessage);
        return OPEN_ERROR;
    }
  }

  // build filesize
  m_nTotalFileSize = ::GetFileSize (m_hFile, &nHighDWordFileSize);
  ASSERT (m_nTotalFileSize != INVALID_FILE_SIZE);
  m_nTotalFileSize |= (file_t (nHighDWordFileSize) << 32);

  // create file mapping
  // If the function succeeds, the return value is a handle to the file mapping
  // object. If the object existed before the function call, the function returns
  // a handle to the existing object (with its current size, not the specified size)
  // and GetLastError returns ERROR_ALREADY_EXISTS.
  //
  // If the function fails, the return value is NULL. To get extended error
  // information, call GetLastError.
  //
  // An attempt to map a file with a length of zero in this manner fails with an
  // error code of ERROR_FILE_INVALID. Applications should test for files with a length
  // of zero and reject such files.
  //
  // For geew we need a dynamic FileMapping name because more files are opened at once

  if (m_nTotalFileSize == 0)
    return OPEN_SUCCESS;

  {
    static int i = 0;

    // 32 is the number of chars
    LPTSTR pFileMappingName = (LPTSTR) alloca (32 * sizeof (TCHAR));
    _stprintf (pFileMappingName, _T ("GT_FileMapping_%d"), ++i);

    m_hMapOfFile = CreateFileMapping (m_hFile,            // current file handle
                                      NULL,               // default security
                                      PAGE_READONLY,      // read permission
                                      0,                  // size high of mapping (of 64 bit)
                                      0,                  // size low of mapping
                                      pFileMappingName);  // name of mapping object
    nLastError = GetLastError ();
    if (nLastError != ERROR_SUCCESS && nLastError != ERROR_ALREADY_EXISTS)  // in case 2 instances of gt* are running
      ShowWin32Error (nLastError, NULL);
    if (m_hMapOfFile == NULL || nLastError == ERROR_ALREADY_EXISTS)
    {
      CloseHandle (m_hMapOfFile);
      m_hMapOfFile = NULL;
      return OPEN_CREATE_MAPPING_ERROR;
    }
  }

  // map view of file
  {
    // Here we have a problem with very large files 1.xGB!
    // we have to dynamically map the view!
    m_pFileData = MapViewOfFile (m_hMapOfFile,    // handle to mapping object
                                 FILE_MAP_READ,   // read/write permission
                                 0,               // dwFileOffsetHigh
                                 0,               // dwFileOffsetLow
                                 0);              // dwNumberOfBytesToMap (0 == map entire file)
    nLastError = GetLastError ();
    if (nLastError != ERROR_SUCCESS)
    {
//_ftprintf (stderr, _T ("Debug: static mapping failed because of EC %u\n"), nLastError);
      // Should be either ERROR_ACCESS_DENIED or ERROR_NOT_ENOUGH_MEMORY
      // here we need a dynamic mapping
      // On an AVI I had an ERROR_NO_SYSTEM_RESOURCES (1450)
      m_bDynamicMapping = true;
      nLastError = _MapPortionAtPos (m_nDynamicMapPos);
    }
    if (nLastError != ERROR_SUCCESS)
      ShowWin32Error (nLastError, NULL);
    if (m_pFileData == NULL)
      return OPEN_MAP_VIEW_ERROR;
  }

  // done
  return OPEN_SUCCESS;
}

//--------------------------------------------------------------------
DWORD GT_CALL File::_MapPortionAtPos
                                        (const file_t nMappingPos)
//--------------------------------------------------------------------
{
  ASSERT (m_bDynamicMapping);
  ASSERT (m_hMapOfFile != NULL);

  // release previous view
  if (m_pFileData != NULL)
  {
    if (!UnmapViewOfFile (m_pFileData))
      ShowWin32Error (GetLastError (), NULL);
    m_pFileData = NULL;
  }

  // determine size of new view
  m_nDynamicMapSize = (DWORD) _min (FILE_MAPPING_SIZE, m_nTotalFileSize - nMappingPos);

//_ftprintf (stderr, _T ("Debug: mapping %lu bytes at offset %I64d\n"), m_nDynamicMapSize, nMappingPos);

  // map new view
  m_pFileData = MapViewOfFile (m_hMapOfFile,            // handle to mapping object
                               FILE_MAP_READ,           // read/write permission
                               HI_DWORD (nMappingPos),  // dwFileOffsetHigh
                               LO_DWORD (nMappingPos),  // dwFileOffsetLow
                               m_nDynamicMapSize);      // dwNumberOfBytesToMap (0 == map entire file)

  // success, if ptr != NULL
  return m_pFileData != NULL ? ERROR_SUCCESS : GetLastError ();
}

//--------------------------------------------------------------------
gtuint8* GT_CALL File::_GetDataPtr
                                        (const file_t nOffset)
//--------------------------------------------------------------------
{
  if (nOffset < 0 ||
      (!m_bDynamicMapping && nOffset > m_nFileSize))
  {
    pstring sText;
    str_assign_from_res (sText, HRC_GTLIB_FILEBUFFER_EXCEPTION_SETACTFILEPOS);
    throw Excpt (_T (__FILE__), __LINE__,
                 sText.c_str (),
                 m_sFilename.c_str (),
                 nOffset);
  }

  // get absolute offset
  file_t nAbsOffset = m_nScanOffset + nOffset;

  // handle dynamic file mapping?
  if (m_bDynamicMapping)
  {
    // change dynamically mapping?
    if (nOffset < m_nDynamicMapPos || nOffset > m_nDynamicMapPos + m_nDynamicMapSize)
    {
      // Important: mapping position needs to be aligned!
      SYSTEM_INFO aSI;
      GetSystemInfo (&aSI);
      m_nDynamicMapPos = (nAbsOffset / aSI.dwAllocationGranularity) * aSI.dwAllocationGranularity;

      // now we can map
      DWORD nLastError = _MapPortionAtPos (m_nDynamicMapPos);
      if (nLastError != ERROR_SUCCESS)
        ShowWin32Error (nLastError, NULL);
    }

    // adopt absolute offset anyway!
    nAbsOffset -= m_nDynamicMapPos;
  }

  // get ptr to data
  return ((gtuint8*) m_pFileData) + nAbsOffset;
}

//--------------------------------------------------------------------
File::File
                                        (      LPCTSTR     sFilename,
                                         const file_t      nScanOffset,
                                               File *const pParent)
//--------------------------------------------------------------------
  : m_hFile           (INVALID_HANDLE_VALUE),
    m_hMapOfFile      (NULL),
    m_pFileData       (NULL),
    m_pParent         (pParent),
    m_sFilename       (sFilename),
    m_nScanOffset     (nScanOffset),
    m_nFileSize       (INVALID_FILE_T),
    m_nTotalFileSize  (INVALID_FILE_T),
    m_nActFilePos     (INVALID_FILE_T),
    m_bDynamicMapping (false),
    m_nDynamicMapPos  (0),
    m_nDynamicMapSize (0),
    m_nParentMapPos   (0)
{
  ASSERT (sFilename);
  ASSERT (nScanOffset >= 0);
  ASSERT (!pParent || pParent->m_sFilename == this->m_sFilename);
}

//--------------------------------------------------------------------
File::~File ()
//--------------------------------------------------------------------
{
  // release on highest level!
  if (m_pParent == NULL)
  {
    if (m_pFileData != NULL)
      if (!UnmapViewOfFile (m_pFileData))
        ShowWin32Error (GetLastError (), NULL);

    // close file mapping
    if (m_hMapOfFile != NULL)
      if (!CloseHandle (m_hMapOfFile))
        ShowWin32Error (GetLastError (), NULL);

    // close file
    if (m_hFile != INVALID_HANDLE_VALUE)
      if (!CloseHandle (m_hFile))
        ShowWin32Error (GetLastError (), NULL);
  }
  else
  {
    // Do we need to restore the mapping point?
    if (m_nDynamicMapPos != m_nParentMapPos)
    {
      // restore previous positiomn
      DWORD nLastError = _MapPortionAtPos (m_nParentMapPos);
      if (nLastError != ERROR_SUCCESS)
        ShowWin32Error (nLastError, NULL);

      // restore ptr
      m_pParent->m_pFileData = m_pFileData;
    }
  }
}

//--------------------------------------------------------------------
EOpenError GT_CALL File::Init ()
//--------------------------------------------------------------------
{
  if (m_pParent == NULL)
  {
    // open file
    EOpenError eOpen = _Open ();
    if (eOpen != OPEN_SUCCESS)
      return eOpen;
  }
  else
  {
    // set from parent should be enough
    m_hMapOfFile      = m_pParent->m_hMapOfFile;
    m_pFileData       = m_pParent->m_pFileData;
    m_nTotalFileSize  = m_pParent->m_nTotalFileSize;
    m_bDynamicMapping = m_pParent->m_bDynamicMapping;
    m_nDynamicMapPos  = m_pParent->m_nDynamicMapPos;
    m_nDynamicMapSize = m_pParent->m_nDynamicMapSize;
    m_nParentMapPos   = m_pParent->m_nDynamicMapPos;  // save!
  }

  // init vars
  m_nActFilePos = 0;
  m_nFileSize = m_nTotalFileSize - m_nScanOffset;

  // check whether the offset was greater than the filesize ;-)
  ASSERT (m_nFileSize >= 0);

  return OPEN_SUCCESS;
}

//--------------------------------------------------------------------
void GT_CALL File::SetActFilePos
                                        (const file_t nNewActFilePos)
//--------------------------------------------------------------------
{
  if (nNewActFilePos < 0 || nNewActFilePos > m_nFileSize)
  {
    pstring sText;
    str_assign_from_res (sText, HRC_GTLIB_FILEBUFFER_EXCEPTION_SETACTFILEPOS);
    throw Excpt (_T (__FILE__), __LINE__,
                 sText.c_str (),
                 m_sFilename.c_str (),
                 nNewActFilePos);
  }

  m_nActFilePos = nNewActFilePos;
}

//--------------------------------------------------------------------
void GT_CALL File::IncActFilePos
                                        (const file_t n)
//--------------------------------------------------------------------
{
  if (!IncActFilePosNoThrow (n))
  {
    pstring sText;
    str_assign_from_res (sText, HRC_GTLIB_FILEBUFFER_EXCEPTION_INCACTFILEPOS);
    throw Excpt (_T (__FILE__), __LINE__,
                 sText.c_str (),
                 m_sFilename.c_str (),
                 m_nActFilePos + n);
  }
}

//--------------------------------------------------------------------
bool GT_CALL File::IncActFilePosNoThrow (const file_t n)
//--------------------------------------------------------------------
{
//  ASSERT (m_nActFilePos + n >= 0 && m_nActFilePos + n <= m_nFileSize);

  if ((n < 0 && _abs (n) > m_nActFilePos) || (n > 0 && m_nActFilePos + n > m_nFileSize))
    return false;

//  out_format ("Incing from %08Xh to %08Xh\n", m_nActFilePos, m_nActFilePos + n);

  m_nActFilePos += n;
  return true;
}


}  // namespace

