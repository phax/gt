#ifndef _GT_DIRENTRY_HXX_
#define _GT_DIRENTRY_HXX_

#ifdef _MSC_VER
#pragma once
#endif

namespace GT {

//--------------------------------------------------------------------
class GT_EXPORT_BASIC DirEntry
//--------------------------------------------------------------------
{
private:
  DirEntry ();
  DirEntry (const DirEntry&);
  DirEntry& operator = (const DirEntry&);

protected:
  pstring m_sName;

public:
  explicit DirEntry (const pstring& sName);

  //! get path separator char
  static TCHAR GT_CALL GetPathSep ();

  //! get the directory (including the last path separator)
  void GT_CALL GetDirectoryName (      pstring& sDirName,
                                 const bool     bTrailingSep) const;

  //! get the basename (no directory and not extension)
  void GT_CALL GetBaseName (pstring& sBaseName) const;

  //! get the filename without the directory (incl. extension)
  void GT_CALL GetFileName (pstring& sFilename) const;

  //! get the filename extension (incl. the dot)
  void GT_CALL GetFileNameExtension (pstring& sExtension) const;

  //! does the file have this special extension (exkl. the dot)
  bool GT_CALL HasFileNameExtension (LPCTSTR pExt) const;

  //! get the complete filename without the extension
  void GT_CALL GetFullNameWithoutExt (pstring& sName) const;
};

}  // namespace

#endif
