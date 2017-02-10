#include "gee_filedata.hxx"
#include "gee_output.hxx"
#include "gee_settings.hxx"
#include "gee_permgen.hxx"

#include "gt_app.hxx"
#include "gt_cmdline_params.hxx"
#include "gt_output.hxx"
#include "gt_version.hrc"

using namespace GEE;

namespace GT {

//--------------------------------------------------------------------
tParamBool const* GT_CALL GetBoolParam (const size_t nIndex)
//--------------------------------------------------------------------
{
  static const tParamBool _BOOL_FLAGS [] = {
    { GEEFLAG_RESLOOKUP, 0, false },
  };

  return nIndex < GT_ARRAY_SIZE (_BOOL_FLAGS)
           ? &_BOOL_FLAGS[nIndex]
           : NULL;
}

//--------------------------------------------------------------------
tParamLong const* GT_CALL GetLongParam (const size_t nIndex)
//--------------------------------------------------------------------
{
  static const tParamLong _LONG_FLAGS [] = {
    { GEEFLAG_CMPLEN,       0, DEFAULT_CMPLEN },
    { GEEFLAG_MAXOPENFILES, 0, DEFAULT_MAXOPENFILES },
  };

  return nIndex < GT_ARRAY_SIZE (_LONG_FLAGS)
           ? &_LONG_FLAGS[nIndex]
           : NULL;
}

//--------------------------------------------------------------------
tParamString const* GT_CALL GetStringParam (const size_t nIndex)
//--------------------------------------------------------------------
{
  static const tParamString _STRING_FLAGS [] = {
    { GEEFLAG_MACRO, 0, _T ("") },
  };

  return nIndex < GT_ARRAY_SIZE (_STRING_FLAGS)
           ? &_STRING_FLAGS[nIndex]
           : NULL;
}


/* Used in the programs headline. The name should only be
     the name and the version should only be the version
     (but inlcuding any suffix like beta etc.)
 */
LPCTSTR GT_CALL GetProgramName    () { return _T ("geew2"); }
LPCTSTR GT_CALL GetProgramVersion () { return GT_VERSION_STR; }
LPCTSTR GT_CALL GetConfigfileName () { return _T ("geew.cfg"); }

void GT_CALL HandleCommandlineDependencies ()
{
  // always disable listmode
  CmdlineParam_ApplyBool (GT_FLAG_LISTMODE, false);
}

bool GT_CALL CanStartSearching () { return true; }

/*! This is the callback function called for each files passed
    on the commandline!
 */
//--------------------------------------------------------------------
void GT_CALL HandleFilename (LPCTSTR pFilename)
//--------------------------------------------------------------------
{
  // check that only the maximum number of files are opened!
  static int nMaxOpenFiles = 0;
  if (nMaxOpenFiles == 0)
    nMaxOpenFiles = CmdlineParam_GetLong (GEEFLAG_MAXOPENFILES);
  if ((int) FileDataSeq::Instance ().size () > nMaxOpenFiles)
    return;

  pstring sStatus;
  str_assign_format (sStatus, _T ("Opening file '%s'"), pFilename);
  out_status (sStatus.c_str ());

  // is name unique?
  if (FileDataSeq::Instance ().containsFilename (pFilename))
  {
    out_error_format (_T ("The file '%s' is already in the comparison list\n"), pFilename);
    return;
  }

  // new object
  GEEFileData *pFileData = new GEEFileData (pFilename);

  // opened file?
  if (!pFileData->isOkay ())
  {
    out_error_format (_T ("Failed to open file '%s'\n"), pFilename);
    delete pFileData;
    return;
  }

  // determine file format
  if (!pFileData->determineFileFormat ())
  {
    out_error_format (_T ("'%s' is no supported file format\n"), pFilename);
    delete pFileData;
    return;
  }

  // any macro present?
  pstring sMacro = CmdlineParam_GetString (GEEFLAG_MACRO);
  if (!sMacro.empty ())
  {
    if (!pFileData->evaluateMacro (sMacro.c_str ()))
    {
      out_error_format (_T ("Could not resolve macro '%s' in file '%s'\n"), sMacro.c_str (), pFilename);
      delete pFileData;
      return;
    }
  }

  // okay - done
  FileDataSeq::Instance ().push_back (pFileData);
}

//--------------------------------------------------------------------
static int _getEqualCount (gtuint8* pResBuf, const int nCmpLen)
//--------------------------------------------------------------------
{
  // fill with default byte
  memset (pResBuf, 0, nCmpLen);

  // start at -1 to differentiate from match at first byte
  int nEqualCount = -1;

  int i, n;
  for (i = 0; i < nCmpLen; i++)
  {
    // Get next compared byte (0 == different)
    n = FileDataSeq::Instance ().getNextComparedByte ();

    // unexpected EOF?
    if (n == GEE_EOF)
      break;

    // if n == 0 -> difference
    pResBuf[i] = n;

    // remember last equal byte
    if (n > 0)
      nEqualCount = i;
  }

  // were all bytes written??
  if (i < nCmpLen)
    out_info_format (_T ("Unexpected end of comparison at %d/%d\n"), i, nCmpLen);

  // for index handling etc
  return nEqualCount + 1;
}

//--------------------------------------------------------------------
static int _evalPermutation (const int* aIndices, const size_t nIndexCount)
//--------------------------------------------------------------------
{
  // reset all positions to the start
  FileDataSeq::Instance ().resetToStart ();

  for (size_t i = 0; i < nIndexCount; ++i)
    out_format (_T ("%d,"), aIndices[i]);
  out_append (_T ("\n"));

  // FIXME
  return 0;
}

//--------------------------------------------------------------------
void GT_CALL Run ()
//--------------------------------------------------------------------
{
  // no files present??
  if (FileDataSeq::Instance ().empty ())
  {
    out_error_append (_T ("Found no files to compare.\n"));
    return;
  }

  // list the names of the files we compare
  {
    out_append (_T ("Comparing:\n"));

    FileDataSeq::const_iterator cit = FileDataSeq::Instance ().begin ();
    for (; !(cit == FileDataSeq::Instance ().end ()); ++cit)
      if (0)
        out_format (_T (" %s"), (*cit)->getBuffer ()->GetpFileName ());
      else
        out_format (_T ("  '%s'\n"), (*cit)->getBuffer ()->GetpFileName ());
    out_append (_T ("\n"));
  }

  // only 1 file present??
  if (FileDataSeq::Instance ().size () == 1)
  {
    out_error_append (_T ("I need at least a second file to compare to.\n"));
    return;
  }

  // get comparison format
  GEEFormat eFormat;
  if (!FileDataSeq::Instance ().getFormat (eFormat))
  {
    out_error_append (_T ("The files don't have the same EXE format (DOS, NE, PE).\n"));
    return;
  }

  // get number of bytes to compare
  int nCmpLen = CmdlineParam_GetLong (GEEFLAG_CMPLEN);
  if (nCmpLen < 1)
  {
    out_error_append (_T ("Invalid compare length - defaulting to 80.\n"));
    nCmpLen = DEFAULT_CMPLEN;
  }

  // FIXME permutation generation does not work - disabled!
  PermutationGenerator *pPermGen = NULL;
  if (FileDataSeq::Instance ().size () < 10 && false)
    pPermGen = new PermutationGenerator (FileDataSeq::Instance ().size ());

  gtuint8* pResBuf = new gtuint8 [nCmpLen];
  int nEqualCount;
  if (!pPermGen)
  {
    // no permutation generator required
    nEqualCount = _getEqualCount (pResBuf, nCmpLen);
  }
  else
  {
    // use a permutation generator
    const int *pBestMatch = NULL;
    int nBestEqualCount = -1;

    while (pPermGen->hasMore ())
    {
      int* pNextPerm = pPermGen->getNext ();
      // pPermGen->getWorkItemCount () == FileDataSeq::Instance ().size ()
      int nNextEqualCount = _evalPermutation (pNextPerm, pPermGen->getWorkItemCount ());

      if (pBestMatch == NULL || nNextEqualCount > nBestEqualCount)
      {
        pBestMatch = pNextPerm;
        nBestEqualCount = nNextEqualCount;
      }
    }

    // eval best one!
    nEqualCount = _evalPermutation (pBestMatch, pPermGen->getWorkItemCount ());
    delete pPermGen;
  }

  if (nEqualCount == 0)
  {
    // all different!
    out_error_append (_T ("Files are totally different!\n"));
  }
  else
  {
    // scan for special bytes
    if (CmdlineParam_GetBool (GEEFLAG_RESLOOKUP))
      WriteByteLookup (pResBuf, nEqualCount, eFormat);
    else
      WriteCStruct (pResBuf, nEqualCount);
  }

  delete pResBuf;
}

void GT_CALL HandleCleanup (const int nTotalNonFlagsParameters) {}

}  // namespace
