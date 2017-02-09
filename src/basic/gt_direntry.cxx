#include "gt_direntry.hxx"

#include "gt_stringhelper.hxx"

namespace GT {

  // #f# We have a problem if the commandline looks like "c:*.dll" -> in this case the path is not resolved correctly!

const TCHAR PATH_SEP = _T ('\\');

//--------------------------------------------------------------------
DirEntry::DirEntry (const pstring& sName)
//--------------------------------------------------------------------
  : m_sName (sName)
{
  // ensure we have only '\' chars
  str_replace_all (m_sName, _T ('/'), PATH_SEP);
}

//--------------------------------------------------------------------
TCHAR GT_CALL DirEntry::GetPathSep ()
//--------------------------------------------------------------------
{
  return PATH_SEP;
}

/*! get the directory (including the last path separator)
 */
//--------------------------------------------------------------------
void GT_CALL DirEntry::GetDirectoryName
                                        (      pstring& sDirName,
                                         const bool     bTrailingSep) const
//--------------------------------------------------------------------
{
  // find last path separator
  const pstring::size_type n = m_sName.find_last_of (PATH_SEP);

  // backslash found?
  if (n != pstring::npos)
    sDirName.assign (m_sName, 0, n + (bTrailingSep ? 1 : 0));
  else
    sDirName.clear ();
}

/*! get the basename (no directory and not extension)
 */
//--------------------------------------------------------------------
void GT_CALL DirEntry::GetBaseName
                                        (pstring& sBaseName) const
//--------------------------------------------------------------------
{
  // find last path separator
  pstring::size_type nPS  = m_sName.find_last_of (PATH_SEP);
  pstring::size_type nDot = m_sName.find_last_of (_T ('.'));

  if (nPS == pstring::npos)
  {
    // no path
    nPS = 0;
    nDot = nDot == pstring::npos ? _MAX_PATH : nDot;
  }
  else
  {
    // with path
    nPS++;
    nDot = nDot < nPS ? _MAX_PATH : nDot - nPS;
  }

  sBaseName.assign (m_sName, nPS, nDot);
}

/*! get the filename (incl. extension)
 */
//--------------------------------------------------------------------
void GT_CALL DirEntry::GetFileName
                                        (pstring& sFileName) const
//--------------------------------------------------------------------
{
  // find last path separator
  const pstring::size_type n = m_sName.find_last_of (PATH_SEP);

  // backslash found?
  if (n != pstring::npos)
    sFileName.assign (m_sName, n + 1, _MAX_PATH);
  else
    sFileName.assign (m_sName);
}

/*! get the filename extension (incl. the dot)
 */
//--------------------------------------------------------------------
void GT_CALL DirEntry::GetFileNameExtension
                                        (pstring& sExtension) const
//--------------------------------------------------------------------
{
  // find last path separator
  pstring::size_type nPS  = m_sName.find_last_of (PATH_SEP);
  pstring::size_type nDot = m_sName.find_last_of (_T ('.'));

  // get real path end pos
  nPS = nPS == pstring::npos ? 0 : nPS + 1;

  if (nDot != pstring::npos && nDot >= nPS)
    sExtension.assign (m_sName, nDot, _MAX_PATH);
  else
    sExtension.clear ();
}

/*! does the file have this special extension (exkl. the dot)
 */
//--------------------------------------------------------------------
bool GT_CALL DirEntry::HasFileNameExtension
                                        (LPCTSTR pExt) const
//--------------------------------------------------------------------
{
  /* do it to avoid invalid extensions on files like
       c:\path.1\filename
     -> the last dot would be in the path and not in the file!
   */

  // find last backslash
  LPCTSTR p = _tcsrchr (m_sName.c_str (), PATH_SEP);
  // find last dot (after BS if any)
  p = _tcsrchr (p ? p : m_sName.c_str (), _T ('.'));

  // case insensitive on Win!
  return p ? _tcsicmp (p + 1, pExt) == 0 : false;
}

/*! get the complete filename without the (last) extension.
    This method is specially for GZ archives.
 */
//--------------------------------------------------------------------
void GT_CALL DirEntry::GetFullNameWithoutExt
                                        (pstring& sName) const
//--------------------------------------------------------------------
{
  // like HasFileNameExtension but basic_string code
  pstring::size_type nPS  = m_sName.find_last_of (PATH_SEP);
  pstring::size_type nDot = m_sName.find_last_of (_T ('.'));

  // get real path end pos
  nPS = nPS == pstring::npos ? 0 : nPS + 1;

  if (nDot != pstring::npos && nDot >= nPS)
    sName.assign (m_sName, 0, nDot);
  else
    sName.assign (m_sName);
}

}  // namespace
