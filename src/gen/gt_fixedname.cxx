#include "gt_fixedname.hxx"

#include "gtc_data.hxx"
#include "gt_registry.hxx"
#include "gt_stringhelper.hxx"
#include "gt_utils.hxx"  // rc

namespace GT {

/*! check if the filename sFilename in the directory sDirectory is special
    The passed directory can be empty if we're in root, else
      it contains only the last directory!
 */
//--------------------------------------------------------------------
GT_EXPORT_GEN bool GT_CALL GetSpecialNameDescription
                                        (LPCTSTR  sDirectory,
                                         LPCTSTR  sFilename,
                                         pstring& sDescription)
//--------------------------------------------------------------------
{
  ASSERT (sDirectory);
  ASSERT (sFilename);

  const int nCount = GT_FixedName_Count ();
  const FixedNameEntry* pEntry;

  for (int i = 0; i < nCount; i++)
  {
    pEntry = GT_FixedName_Index (i);
    ASSERT (pEntry);
    ASSERT (pEntry->m_sFilename);
    ASSERT (pEntry->m_sDescription);

    // also compare directory?
    if (pEntry->m_sDirectory == NULL || _tcsicmp (pEntry->m_sDirectory, sDirectory) == 0)
    {
      // compare filename
      if (_tcsicmp (pEntry->m_sFilename, sFilename) == 0)
      {
        // found -> resolve string ID
        if (str_isresptr (rc (pEntry->m_sDescription)))
          str_assign_from_res (sDescription, pEntry->m_sDescription);
        else
          str_assign (sDescription, rc (pEntry->m_sDescription));

        return true;
      }
    }
  }

  sDescription.clear ();
  return false;
}

//--------------------------------------------------------------------
static void _FindMatchingExtension
                                        (LPCTSTR               sExtension,
                                         SpecialExtensionList& aMatchingEntries,
                                         int (GT_CALL* FunCount) (),
                                         const FixedExtensionEntry* (GT_CALL* FunGet) (unsigned int))
//--------------------------------------------------------------------
{
  const int nCount = FunCount ();
  const FixedExtensionEntry *pEntry;
  int nCmp;

  for (int i = 0; i < nCount; i++)
  {
    pEntry = FunGet (i);
    ASSERT (pEntry);
    ASSERT (pEntry->m_sExtension);
    ASSERT (pEntry->m_sDescription);

    // compare
    nCmp = _tcsicmp (pEntry->m_sExtension, sExtension);
    if (nCmp == 0)
    {
      // match -> append
      aMatchingEntries.push_back (pEntry);
    }
    else
    if (nCmp > 0)
    {
      // since the extension list should be sorted alphabetically
      // we don't need to scan any further!
      break;
    }
  }
}

/*! check if the extension sExtension is special
 */
//--------------------------------------------------------------------
GT_EXPORT_GEN bool GT_CALL GetSpecialExtensionDescription
                                        (LPCTSTR               sExtension,
                                         SpecialExtensionList& aMatchingEntries)
//--------------------------------------------------------------------
{
  ASSERT (aMatchingEntries.empty ());

  // file has an extension ...
  if (sExtension && *sExtension)
  {
    // ignore the dot!
    ASSERT (sExtension[0] == '.');

    // Scan internal extensions first
    _FindMatchingExtension (sExtension + 1, aMatchingEntries, GT_FixedExtension_Count, GT_FixedExtension_Index);

    // Scan FilExt after internal extensions!
    _FindMatchingExtension (sExtension + 1, aMatchingEntries, GT_FilextDotCom_Count, GT_FilextDotCom_Index);
  }

  return !aMatchingEntries.empty ();
}

/*! check if the extension sExtension is known in the registry
 */
//--------------------------------------------------------------------
GT_EXPORT_GEN bool GT_CALL GetSpecialRegistryDescription
                                        (LPCTSTR  sExtension,
                                         pstring& sRegDescription,
                                         pstring& sRegClass,
                                         pstring& sProgramPath)
//--------------------------------------------------------------------
{
  const int BUFFERLEN = 512;
  TCHAR sBufferShortcut[BUFFERLEN];
  TCHAR sBufferDescription[BUFFERLEN];
  TCHAR sBufferMisc[BUFFERLEN];
  pstring s;

  sRegDescription.clear ();
  sRegClass.clear ();
  sProgramPath.clear ();

  // file has no extension ...
  if (!sExtension || !*sExtension)
    return false;

  // get the shortcut for this extension
  // e.g. ".386" => "vxdfile"
  if (!Registry::GetClassesRootString (sExtension,
                                       _T (""),
                                       BUFFERLEN,
                                       sBufferShortcut))
  {
    return false;
  }

  // get the description from the extension
  // e.g. "vxdfile" => "Virtueller Gerätetreiber"
  if (!Registry::GetClassesRootString (sBufferShortcut,
                                       _T (""),
                                       BUFFERLEN,
                                       sBufferDescription))
  {
    return false;
  }

  // build main string
  sRegDescription = sBufferDescription;
  sRegClass = sBufferShortcut;

  {
    // is there an open command?
    s = sBufferShortcut;
    s += _T ("\\Shell\\Open\\Command");
    if (Registry::GetClassesRootString (s.c_str (),
                                        _T (""),
                                        BUFFERLEN,
                                        sBufferMisc) &&
        *sBufferMisc)
    {
      sProgramPath = sBufferMisc;
    }
  }

  return true;
}

}  // namespace
