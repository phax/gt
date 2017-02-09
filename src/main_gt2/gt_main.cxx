#include "gt_app.hxx"
#include "gt_cmdline_params.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_timer.hxx"
#include "gt_utils.hxx"

namespace GT {

extern Timer g_aAlloverTimer;
extern Timer g_aFileTimer;

gtuint32 g_nTotalFilesFound = 0;
gtuint32 g_nTotalFilesIdentified = 0;

// in gt_mainlister.cxx
extern int GT_CALL DefaultCallback
                                        (      LPCTSTR     sFilename,
                                         const file_t      nOffset,
                                               File *const pParent);

//--------------------------------------------------------------------
static void __UpdateStatusBefore (LPCTSTR pFilename)
//--------------------------------------------------------------------
{
  if (debug)
    out_status (pFilename);
}

//--------------------------------------------------------------------
class StatusString : public pstring
//--------------------------------------------------------------------
{
public:
  explicit StatusString ()
  {
    assign (CmdlineParam_GetString (GT_FLAG_STATUS));
  }
};

//--------------------------------------------------------------------
static void __UpdateStatus (LPCTSTR pFilename)
//--------------------------------------------------------------------
{
  static const StatusString sStatus;

  pstring sCurStatus = sStatus;
  if (sCurStatus.empty ())
  {
    TCHAR sConsoleTitle[1024];

    // default behaviour
    if (g_nTotalFilesFound == 0)
    {
      // no file found yet
      _tcscpy (sConsoleTitle, _T ("GT [PHaX]"));
    }
    else
    {
      if (g_nTotalFilesIdentified == g_nTotalFilesFound)
      {
        // all files identified
        _stprintf (sConsoleTitle, _T ("GT rate: all %ld"),
                   g_nTotalFilesFound);
      }
      else
      {
        DoublePair aRate;
        _splitdouble (PERCENTAGE (g_nTotalFilesIdentified, g_nTotalFilesFound), aRate, 2);

        _stprintf (sConsoleTitle, _T ("GT rate: %d.%02d%%"),
                   aRate.h, aRate.l);
      }
    }

    out_status (sConsoleTitle);
  }
  else
  {
    // user defined stuff
    TCHAR sBuf[32];

    // detected files
    _stprintf (sBuf, _T ("%lu"), g_nTotalFilesIdentified);
    str_replace_all (sCurStatus, _T ("$D"), sBuf);

    // total files
    _stprintf (sBuf, _T ("%lu"), g_nTotalFilesFound);
    str_replace_all (sCurStatus, _T ("$T"), sBuf);

    // percentage
    if (g_nTotalFilesFound > 0)
    {
      DoublePair aRate;
      _splitdouble (PERCENTAGE (g_nTotalFilesIdentified, g_nTotalFilesFound), aRate, 2);
      _stprintf (sBuf, _T ("%lu.%02d%%"), aRate.h, aRate.l);
      str_replace_all (sCurStatus, _T ("$P"), sBuf);
    }
    else
      str_replace_all (sCurStatus, _T ("$P"), _T ("0.00%"));

    // filename
    str_replace_all (sCurStatus, _T ("$F"), pFilename);

    // show headline
    out_status (sCurStatus.c_str ());
  }
}

//--------------------------------------------------------------------
tParamBool const* GT_CALL GetBoolParam (const size_t nIndex)
//--------------------------------------------------------------------
{
  /* IMPORTANT:
   * if you have parameter which are partly equal, add the longer one
   * first! e.g. put "listopts" before "l"!!!!
   */
  static const tParamBool _BOOL_FLAGS [] = {
    { GT_FLAG_EXEMOD,         HRC_APP_FLAG_EXEMOD,         false },
#ifdef _DEBUG
    { GT_FLAG_GTUI2,          HRC_APP_FLAG_GTUI2,          false },
#endif
    { GT_FLAG_NOTIDENTIFIED,  HRC_APP_FLAG_NOTIDENTIFIED,  false },
    { GT_FLAG_NOARCS,         HRC_APP_FLAG_NOARCS,         false },
    { GT_FLAG_NOFOOTER,       HRC_APP_FLAG_NOFOOTER,       false },
    { GT_FLAG_NOOVR,          HRC_APP_FLAG_NOOVR,          false },
    { GT_FLAG_NOSCANEXT,      HRC_APP_FLAG_NOSCANEXT,      false },
    { GT_FLAG_NOSCANNAME,     HRC_APP_FLAG_NOSCANNAME,     false },
    { GT_FLAG_NOTUNKNOWN,     HRC_APP_FLAG_NOTUNKNOWN,     false },
    { GT_FLAG_PEALL,          HRC_APP_FLAG_PEALL,          false },
    { GT_FLAG_PEDD,           HRC_APP_FLAG_PEDD,           false },
    { GT_FLAG_PEEXP,          HRC_APP_FLAG_PEEXP,          false },
    { GT_FLAG_PEFIXUP,        HRC_APP_FLAG_PEFIXUP,        false },
    { GT_FLAG_PEIMP2,         HRC_APP_FLAG_PEIMP2,         false },
    { GT_FLAG_PEIMP,          HRC_APP_FLAG_PEIMP,          false },
    { GT_FLAG_PERES2,         HRC_APP_FLAG_PERES2,         false },
    { GT_FLAG_PERES,          HRC_APP_FLAG_PERES,          false },
    { GT_FLAG_PESECT,         HRC_APP_FLAG_PESECT,         false },
    { GT_FLAG_PEVER,          HRC_APP_FLAG_PEVER,          false },
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
    { GT_FLAG_FTE,     HRC_APP_FLAG_FTE,     3 },
    { GT_FLAG_FTN,     HRC_APP_FLAG_FTN,     3 },
    { GT_FLAG_FTP,     HRC_APP_FLAG_FTP,     3 },
    { GT_FLAG_FTS,     HRC_APP_FLAG_FTS,     1 },
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
    { GT_FLAG_STATUS, HRC_APP_FLAG_STATUS, _T ("") },
  };

  return nIndex < GT_ARRAY_SIZE (_STRING_FLAGS)
           ? &_STRING_FLAGS[nIndex]
           : NULL;
}

//--------------------------------------------------------------------
LPCTSTR GT_CALL GetProgramName ()
//--------------------------------------------------------------------
{
  return _T ("gt2");
}

//--------------------------------------------------------------------
LPCTSTR GT_CALL GetProgramVersion ()
//--------------------------------------------------------------------
{
  return GT_VERSION_STR;
}

//--------------------------------------------------------------------
LPCTSTR GT_CALL GetConfigfileName ()
//--------------------------------------------------------------------
{
  return _T ("gt2.cfg");
}

//--------------------------------------------------------------------
void GT_CALL HandleCommandlineDependencies ()
//--------------------------------------------------------------------
{
  // if nothing should be displayed (except summary)
  // -> set listmode so that as little information as
  // possible is used
  if (CmdlineParam_GetBool (GT_FLAG_NOTIDENTIFIED) &&
      CmdlineParam_GetBool (GT_FLAG_NOTUNKNOWN))
  {
    CmdlineParam_Apply (GT_FLAG_LISTMODE);
  }

  // if listmode is enabled, or if the user only wants to
  // see not identified files, disable all extensions!
  if (CmdlineParam_GetBool (GT_FLAG_LISTMODE) ||
      CmdlineParam_GetBool (GT_FLAG_NOTIDENTIFIED))
  {
    CmdlineParam_ApplyBool (GT_FLAG_PEEXP,   false);
    CmdlineParam_ApplyBool (GT_FLAG_PEFIXUP, false);
    CmdlineParam_ApplyBool (GT_FLAG_PEIMP2,  false);
    CmdlineParam_ApplyBool (GT_FLAG_PEIMP,   false);
    CmdlineParam_ApplyBool (GT_FLAG_PERES2,  false);
    CmdlineParam_ApplyBool (GT_FLAG_PERES,   false);
    CmdlineParam_ApplyBool (GT_FLAG_PEDD,    false);
    CmdlineParam_ApplyBool (GT_FLAG_PESECT,  false);
    CmdlineParam_ApplyBool (GT_FLAG_PEVER,   false);
    CmdlineParam_ApplyBool (GT_FLAG_PEALL,   false);
    CmdlineParam_ApplyBool (GT_FLAG_NOARCS,  true);
    CmdlineParam_ApplyBool (GT_FLAG_NOOVR,   true);
  }

  // if the user wants extended PE import listing
  // the basic import listing must be activated too!
  if (CmdlineParam_GetBool (GT_FLAG_PEIMP2))
  {
    CmdlineParam_ApplyBool (GT_FLAG_PEIMP, true);
  }

  // if the user wants extended PE resource table listing
  // the basic resource table listing must be activated too!
  if (CmdlineParam_GetBool (GT_FLAG_PERES2))
  {
    CmdlineParam_ApplyBool (GT_FLAG_PERES, true);
  }

  // shortcut for most PE flags
  if (CmdlineParam_GetBool (GT_FLAG_PEALL))
  {
    CmdlineParam_ApplyBool (GT_FLAG_PEDD,    true);
    CmdlineParam_ApplyBool (GT_FLAG_PESECT,  true);
    CmdlineParam_ApplyBool (GT_FLAG_PEVER,   true);
    CmdlineParam_ApplyBool (GT_FLAG_PEEXP,   true);
    CmdlineParam_ApplyBool (GT_FLAG_PERES,   true);
    // Do not add fixups - too much output!
  }

/*
#ifdef _MSC_VER
  // Now we're ready to start GTUI2!
  if (CmdlineParam_GetBool (GT_FLAG_GTUI2))
  {
    StartGTUI2 ();
  }
#endif
*/
}

//--------------------------------------------------------------------
bool GT_CALL CanStartSearching ()
//--------------------------------------------------------------------
{
  // no further actions to take after initialization
  return true;
}

//--------------------------------------------------------------------
void GT_CALL HandleFilename (LPCTSTR pFilename)
//--------------------------------------------------------------------
{
  __UpdateStatusBefore (pFilename);
  switch (DefaultCallback (pFilename, 0, NULL))
  {
    case GT_LIST_IDENTIFIED_BREAK:
    case GT_LIST_IDENTIFIED_CONTINUE:
    {
      g_nTotalFilesFound++;
      g_nTotalFilesIdentified++;
      break;
    }
    case GT_LIST_NOT_IDENTIFIED:
    {
      g_nTotalFilesFound++;
      break;
    }
    case GT_LIST_OPEN_ERROR:
    case GT_LIST_NOT_ALLOWED:
    {
      break;
    }
  }

  out_flush ();
  __UpdateStatus (pFilename);
}

//--------------------------------------------------------------------
void GT_CALL Run ()
//--------------------------------------------------------------------
{
  /* nothing to do here */
}

//--------------------------------------------------------------------
void __PrintStatistics (int nTotalNonFlagsParameters)
//--------------------------------------------------------------------
{
  DoublePair aHighLow;

  out_append (_T ("\n// --- [done] ---\n"));

  // if at least one file was given as a parameter, do the statistics ;-)
  if (nTotalNonFlagsParameters > 0)
  {
    out_format (_T ("Found %d file%s"),
                g_nTotalFilesFound,
                g_nTotalFilesFound != 1
                  ? _T ("s")
                  : _T (""));

    // if any file was found that matches -> print percentage
    if (g_nTotalFilesFound > 0)
    {
      _splitdouble (PERCENTAGE (g_nTotalFilesIdentified, g_nTotalFilesFound), aHighLow, 2);
      out_format (_T (" and identified %d (= %d.%02d%%)"),
                  g_nTotalFilesIdentified, aHighLow.h, aHighLow.l);
    }

    // and finally end the line
    out_append (_T ("\n"));

    g_aAlloverTimer.Stop ();

    if (g_aAlloverTimer.GetSeconds () > 0 || g_nTotalFilesFound > 0)
    {
      double dAllOverSecs = g_aAlloverTimer.GetSeconds ();
      size_t nHours = 0, nMins = 0;
      if (dAllOverSecs > 3600)
      {
        // at least one hour
        nHours = size_t (dAllOverSecs / 3600);
        dAllOverSecs -= (nHours * 3600);
      }
      if (dAllOverSecs > 60)
      {
        // at least one minute
        nMins = size_t (dAllOverSecs / 60);
        dAllOverSecs -= (nMins * 60);
      }
      if (nHours > 0)
      {
        // show hours and minutes
        out_format (_T ("Time needed: %d:%02d:%02d hours"), nHours, nMins, size_t (dAllOverSecs));
      }
      else
      if (nMins > 0)
      {
        // show minutes and seconds
        out_format (_T ("Time needed: %d:%02d mins"), nMins, size_t (dAllOverSecs));
      }
      else
      {
        // show secs and millisecs
        _splitdouble (dAllOverSecs, aHighLow, 2);
        out_format (_T ("Time needed: %d.%02d seconds"), aHighLow.h, aHighLow.l);
      }
    }

    // if no time was used it makes no sense
    if (g_nTotalFilesFound > 0 && g_aAlloverTimer.GetSeconds () > 0)
    {
      const double dFileTimerSecs = g_aFileTimer.GetSeconds ();

      _splitdouble (g_nTotalFilesFound > 0
                       ? dFileTimerSecs / g_nTotalFilesFound
                       : 0.0,
                     aHighLow, 3);
      out_format (_T (" (%d.%03d secs/file, "), aHighLow.h, aHighLow.l);

      _splitdouble (g_aAlloverTimer.GetSeconds () - dFileTimerSecs, aHighLow, 2);
      out_format (_T ("%d.%02d secs lost)\n"), aHighLow.h, aHighLow.l);
    }
    else
    {
      out_append (_T ("\n"));
    }
  }
}

//--------------------------------------------------------------------
void GT_CALL HandleCleanup (const int nTotalNonFlagsParameters)
//--------------------------------------------------------------------
{
  // shall we print the statistics?
  if (!CmdlineParam_GetBool (GT_FLAG_NOFOOTER))
    __PrintStatistics (nTotalNonFlagsParameters);
}

}  // namespace
