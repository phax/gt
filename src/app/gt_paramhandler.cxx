#include "gt_paramhandler.hxx"

#include "gt_direntry.hxx"
#include "gt_driveinfo.hxx"
#include "gt_filesearch.hxx"
#include "gt_output.hxx"
#include "gt_timer.hxx"
#include "gt_utils.hxx"

namespace GT {

// this global variable resides in gt_app.cxx!
extern Timer g_aFileTimer;

/*! This function calls the specified callback function, but with
      enabled timer measuring.
    \param pCallback the callback function to be called
    \param pFilename the filename to handle
 */
//--------------------------------------------------------------------
static void GT_CALL __CallCallback
                                        (FileHandleProc pCallback,
                                         LPCTSTR        pFilename)
//--------------------------------------------------------------------
{
  g_aFileTimer.Start ();
  (*pCallback) (pFilename);
  g_aFileTimer.Stop ();
}

//--------------------------------------------------------------------
static size_t GT_CALL __ScanDirectory
                                        (const pstring&       sPath,
                                         const pstring&       sMask,
                                               FileHandleProc pCallback,
                                         const bool           bSubDirs)
//--------------------------------------------------------------------
{
  ASSERT (!sMask.empty ());

  size_t nFilesFound = 0;
  pstring sSearchMask, sFilename;

  // build search mask
  sSearchMask = sPath;
  sSearchMask += sMask;

  // scan for files:
  FileSearch aSearch;
  aSearch.FindFirst (sSearchMask.c_str ());
  while (!aSearch.IsError ())
  {
    // is it a file and not a directory??
    if (aSearch.IsValidFile ())
    {
      ++nFilesFound;

      // build filename
      sFilename = sPath;
      sFilename += aSearch.GetFilename ();

      // call for current file
      __CallCallback (pCallback, sFilename.c_str ());
    }

    aSearch.FindNext ();
  }
  aSearch.FindClose ();

  // scan subfolder??
  if (bSubDirs)
  {
    // yes, scan subfolder
    sSearchMask = sPath;
    sSearchMask += _T ('*');

    // search sub directories
    aSearch.FindFirst (sSearchMask.c_str ());
    while (!aSearch.IsError ())
    {
      if (aSearch.IsValidDirectory ())
      {
        // build filename
        sFilename = sPath;
        sFilename += aSearch.GetFilename ();
        sFilename += DirEntry::GetPathSep ();

        // recursive call
        nFilesFound += __ScanDirectory (sFilename, sMask, pCallback, true);
      }

      aSearch.FindNext ();
    }
    aSearch.FindClose ();
  }

  return nFilesFound;
}

/*! Start a directory search.
    \param pParam the parameter passed on the commandline that contains
                  the path/filemask to search
    \param pCallBack the callback function to be called when a file matches
    \param bSubDirs if true, the directories are scanned recursively
    \retval the number of matched files
 */
//--------------------------------------------------------------------
static size_t GT_CALL __StartDirectorySearch
                                        (      LPCTSTR        pParam,
                                               FileHandleProc pCallback,
                                         const bool           bSubDirs)
//--------------------------------------------------------------------
{
  ASSERT (pParam);

  // split into path and filemask
  pstring sDir, sMask;
  DirEntry aDE (pParam);
  aDE.GetDirectoryName (sDir, true);
  aDE.GetFileName (sMask);

  return __ScanDirectory (sDir, sMask, pCallback, bSubDirs);
}

/*! Handle the given commandline parameter according to the settings.
    This function handles the /all.... switches that forces a search
     on multiple drives.
    \param pParam the parameter passed on the commandline that contains
                  the path/filemask to search
    \param pCallBack the callback function to be called when a file matches
    \param bSubDirs if true, the directories are scanned recursively
 */
//--------------------------------------------------------------------
void GT_CALL __EvaluateCmdlineParameter
                                        (      LPCTSTR        sParam,
                                               FileHandleProc pCallback,
                                         const bool           bSubDirs)
//--------------------------------------------------------------------
{
  ASSERT (sParam);

  size_t nResultCount = 0;

  // check whether the name is valid! - should never happen!
  if (_tcspbrk (sParam, _T ("<>|")))
  {
    // first flush necessary if listmode is aenabled!!!
    out_flush ();
    out_error_format (rc (HRC_APP_INVALID_CHAR_IN_PARAM), sParam);
    out_flush ();
    return;
  }

  if (!DriveInfo::Instance ().empty ())
  {
    // if "/alldrives" is used no pathes are allowed
    if (_tcspbrk (sParam, _T ("/\\")))
    {
      out_flush ();
      out_error_format (rc (HRC_APP_NO_REL_PATHES_ALLOWED), sParam);
      out_flush ();
      return;
    }

    // if "/alldrives" is used no drives are allowed
    if (_tcschr (sParam, _T (':')))
    {
      out_flush ();
      out_error_format (rc (HRC_APP_NO_DRIVE_LETTERS_ALLOWED), sParam);
      out_flush ();
      return;
    }

    // for all drives
    DriveInfo::const_iterator cit = DriveInfo::Instance ().begin ();
    for (; !(cit == DriveInfo::Instance ().end ()); ++cit)
    {
      // scan dir
      nResultCount += __ScanDirectory ((*cit).m_sRoot, sParam, pCallback, bSubDirs);
    }
  }
  else
  {
    // if the parameter contains no filenames - call list function
    nResultCount += __StartDirectorySearch (sParam, pCallback, bSubDirs);
  }

  if (nResultCount == 0)
  {
    // no files found!
    out_flush ();
    out_error_format (rc (HRC_APP_NO_FILE_FOUND),
                      sParam);
    out_flush ();
  }
}

}  // namespace
