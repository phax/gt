#include "gt_cmdline_params.hxx"
#include "gt_direntry.hxx"
#include "gt_filebuffer.hxx"
#include "gt_fixedname.hxx"
#include "gt_output.hxx"
#include "gt_utils.hxx"

// must be extern "C"!
START_EXTERN_C
GT_EXPORT_ARC GT_ListResult GT_CALL GT_RunDefaultPlugin_ARC (GT::FileBuffer *pBuffer);
GT_EXPORT_EXE GT_ListResult GT_CALL GT_RunDefaultPlugin_EXE (GT::FileBuffer *pBuffer);
GT_EXPORT_GEN GT_ListResult GT_CALL GT_RunDefaultPlugin_GEN (GT::FileBuffer *pBuffer);
END_EXTERN_C

namespace GT {

//--------------------------------------------------------------------
static void __out_filestart (FileBuffer* pBuffer)
//--------------------------------------------------------------------
{
  // fill structure from buffer
  Output_FilenameStruct aFNStruct;
  pBuffer->FillStruct (&aFNStruct);

  // print filename
  out_filestart (&aFNStruct);
}

//--------------------------------------------------------------------
static GT_ListResult __DoNameDetection
                                        (FileBuffer* pBuffer,
                                         bool        bPrintedFileStart)
//--------------------------------------------------------------------
{
  ASSERT (pBuffer);

  int nFound = 0;
  pstring sFixedName, sRegDescription, sRegClass, sProgramPath;
  SpecialExtensionList aSpecialExtensions;

  // get the absolute path of the file
  DirEntry aDE (pBuffer->GetFileName ());

  // split into directory, name and extensiopn
  pstring sDir, sFilename, sExt;
  aDE.GetDirectoryName (sDir, false);  // no trailing path sep
  aDE.GetFileName (sFilename);
  aDE.GetFileNameExtension (sExt);

  // directory present?
  if (!sDir.empty ())
  {
    // use only the last directory
    pstring::size_type n = sDir.find_last_of (DirEntry::GetPathSep ());
    if (n != pstring::npos)
      sDir.erase (0, n + 1);
  }

  // check for special names (e.g. "autoexec.bat")
  if (!CmdlineParam_GetBool (GT_FLAG_NOSCANNAME) &&
      GetSpecialNameDescription (sDir.c_str (), sFilename.c_str (), sFixedName))
  {
    nFound++;
  }

  // check for special extension (e.g. ".doc", ".xls", etc)
  if (!CmdlineParam_GetBool (GT_FLAG_NOSCANEXT) &&
      GetSpecialExtensionDescription (sExt.c_str (), aSpecialExtensions))
  {
    nFound++;
  }

  // check for registry entries!
  if (!CmdlineParam_GetBool (GT_FLAG_NOSCANEXT) &&
      GetSpecialRegistryDescription (sExt.c_str (), sRegDescription, sRegClass, sProgramPath))
  {
    nFound++;
  }

  // found nothing?
  if (nFound == 0)
    return GT_LIST_NOT_IDENTIFIED;

  // shall we list identified files?
  if (!CmdlineParam_GetBool (GT_FLAG_NOTIDENTIFIED))
  {
    gtuint32 nOutputLines = 0, nNewLines = 0;

    // print fileheader
    if (!bPrintedFileStart)
      __out_filestart (pBuffer);

    // special filename?
    if (!sFixedName.empty ())
    {
      out_format (rc (HRC_NAMELIB_SPECIAL_FILENAME),
                  sFixedName.c_str ());
      ++nOutputLines;
    }

    // special extension?
    if (!aSpecialExtensions.empty ())
    {
      pstring sSepcialExtDesc;
      bool bPrintedFilExtStuff = false;

      SpecialExtensionList::const_iterator cit = aSpecialExtensions.begin (), cite = aSpecialExtensions.end ();
      for (; cit != cite; ++cit)
      {
        const FixedExtensionEntry *pEntry = *cit;

        // add spacing
        if (nNewLines < nOutputLines++ && !bPrintedFilExtStuff)
        {
          out_append (_T ("\n"));
          ++nNewLines;
        }

        // Source: filext.com?
        // If yes, all following entries are from FilExt.com
        if (pEntry->m_nFlags & GT_FIXED_EXT_FLAG_FILEXT_COM && !bPrintedFilExtStuff)
        {
          out_append (rc (HRC_NAMELIB_SPECIAL_FILEXT_COM));
          out_incindent ();
          bPrintedFilExtStuff = true;
        }


        // get description
        if (str_isresptr (pEntry->m_sDescription))
          str_assign_from_res (sSepcialExtDesc, resid_t (pEntry->m_sDescription));
        else
          str_assign (sSepcialExtDesc, pEntry->m_sDescription);

        // print
        out_format (rc (HRC_NAMELIB_SPECIAL_EXTENSION),
                    sSepcialExtDesc.c_str ());

        // handle flags
        if (pEntry->m_nFlags > 0)
        {
          out_incindent ();

          // can detect file?
          if (pEntry->m_nFlags & GT_FIXED_EXT_FLAG_CAN_DETECT)
            out_append (rc (HRC_NAMELIB_SPECIAL_CAN_DETECT));

          // should detect file?
          if (pEntry->m_nFlags & GT_FIXED_EXT_FLAG_SHOULD_DETECT)
            out_append (rc (HRC_NAMELIB_SPECIAL_SHOULD_DETECT));

          out_decindent ();
        }
      }

      if (bPrintedFilExtStuff)
      {
        out_decindent ();
      }
    }

    // special registry extension?
    if (!sRegDescription.empty ())
    {
      // add spacing
      if (nNewLines < nOutputLines++)
      {
        out_append (_T ("\n"));
        ++nNewLines;
      }

      // description from registry and registry class
      out_format (rc (HRC_NAMELIB_SPECIAL_WIN_EXT),
                  sRegDescription.c_str (),
                  sRegClass.c_str ());

      if (!sProgramPath.empty ())
      {
        // program to open with ...
        out_incindent ();
        out_format (rc (HRC_NAMELIB_OPEN_WITH),
                    sProgramPath.c_str ());
        out_decindent ();
      }
    }

    // write file done
    out_filedone ();

    // no overlays can be contained
    out_fileend ();
  }

  // and tschühüs
  return GT_LIST_IDENTIFIED_BREAK;
}

//--------------------------------------------------------------------
static int __WriteUnknownFile
                                        (FileBuffer* pBuffer,
                                         bool        bPrintedFileStart)
//--------------------------------------------------------------------
{
  // shall we print info on unknown files?
  if (!CmdlineParam_GetBool (GT_FLAG_NOTUNKNOWN))
  {
    // eventually start the output for this file
    if (!bPrintedFileStart)
      __out_filestart (pBuffer);

    // if the filesize is 0 and we shall list unknown files
    if (pBuffer->GetFileSize () == 0)
    {
      // cannot analyze 0-byte files
      out_append (rc (HRC_APP_0_BYTE_FILE));
    }
    else
    {
      // filesize > 0!
      if (pBuffer->IsText ())
      {
        out_append (rc (HRC_APP_UNKNOWN_TEXT_FILE));
      }
      else
      {
        out_append (rc (HRC_APP_UNKNOWN_BINARY_FILE));

        if (pBuffer->GetFileSize () >= 4)
        {
          gtuint8 aFirst[4];
          pBuffer->GetBuffer (0, &aFirst, 4);
          out_format (_T (" (0x%02X 0x%02X 0x%02X 0x%02X)"),
                      aFirst[0], aFirst[1], aFirst[2], aFirst[3]);
        }

        // manually add newline
        out_append (_T ("\n"));
      }
    }

    // done with this 'file'
    out_filedone ();

    // no overlays -> finish file output
    out_fileend ();
  }

  return GT_LIST_NOT_IDENTIFIED;
}

//--------------------------------------------------------------------
int GT_CALL DefaultCallback
                                        (      LPCTSTR     sFilename,
                                         const file_t      nOffset,
                                               File *const pParent)
//--------------------------------------------------------------------
{
  ASSERT (sFilename);
  ASSERT (*sFilename);
  ASSERT (nOffset >= 0);

  // if we're in an overlay, but overlays should not be handled -> return
  if (nOffset > 0 && CmdlineParam_GetBool (GT_FLAG_NOOVR))
  {
    return GT_LIST_NOT_ALLOWED;
  }

  // init file cache
  FileBuffer aFileBuf (sFilename,
                       nOffset,            // our current offset within the file
                       DefaultCallback,    // indirect recursion possible!
                       pParent);

  // try to open the file
  const EOpenError eOpen = aFileBuf.Init ();
  const bool bScanNameOnError = (eOpen != OPEN_SUCCESS);
  bool bPrintedFileStart = false;

  switch (eOpen)
  {
    case OPEN_SUCCESS:
      // everything worked fine
      break;
    case OPEN_ERROR:
      // failed to find the file -> do nothing
      return GT_LIST_OPEN_ERROR;
    case OPEN_EACCES:
      // access denied -> do only name detection
      bPrintedFileStart = true;
      __out_filestart (&aFileBuf);
      out_error_append (rc (HRC_APP_ERROR_OPEN_EACCES));
      break;
    case OPEN_SHARING_VIOLATION:
      // access denied -> do only name detection
      bPrintedFileStart = true;
      __out_filestart (&aFileBuf);
      out_error_append (rc (HRC_APP_ERROR_OPEN_SHARING_VIOLATION));
      break;
    default:
      // failed to open the file for any reason
      break;
  }

  // special handling for 0 byte files...
  if (aFileBuf.GetFileSize () == 0 || bScanNameOnError)
  {
    // do the name detection
    if (__DoNameDetection (&aFileBuf, bPrintedFileStart) == GT_LIST_IDENTIFIED_BREAK)
      return GT_LIST_IDENTIFIED_BREAK;

    // if we do only name detection -> don't call __WriteUnknownFile!
    return bScanNameOnError
             ? GT_LIST_NOT_IDENTIFIED
             : __WriteUnknownFile (&aFileBuf, bPrintedFileStart);
  }
  else
  {
    // these are the main detection routines:

    // check for archives
    if (GT_RunDefaultPlugin_ARC (&aFileBuf) == GT_LIST_IDENTIFIED_BREAK)
      return GT_LIST_IDENTIFIED_BREAK;

    // check for executables
    if (GT_RunDefaultPlugin_EXE (&aFileBuf) == GT_LIST_IDENTIFIED_BREAK)
      return GT_LIST_IDENTIFIED_BREAK;

    // check all the rest
    if (GT_RunDefaultPlugin_GEN (&aFileBuf) == GT_LIST_IDENTIFIED_BREAK)
      return GT_LIST_IDENTIFIED_BREAK;

    // Nothing known right now -> do name detection if we're not in an overlay
    // If we're in an overlay, the file has already been detected an we need
    //   no more name detection
    if (nOffset == 0)
    {
      if (__DoNameDetection (&aFileBuf, bPrintedFileStart) == GT_LIST_IDENTIFIED_BREAK)
        return GT_LIST_IDENTIFIED_BREAK;
    }

    // Unknown file and unknown name -> absolutely unknown
    // This is possible for overlays and non-overlays.
    return __WriteUnknownFile (&aFileBuf, bPrintedFileStart);
  }
}

}  // namespace
