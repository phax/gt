#include "gt_app.hxx"

#include "gt_cmdline_args.hxx"
#include "gt_cmdline_params.hxx"
#include "gt_configfile.hxx"
#include "gt_crashtracker.hxx"
#include "gt_driveinfo.hxx"
#include "gt_errormsg.hxx"
#include "gt_output.hxx"
#include "gt_output_html.hxx"
#include "gt_output_stdout.hxx"
#include "gt_output_xml.hxx"
#include "gt_paramhandler.hxx"
#include "gt_timer.hxx"
#include "gt_utils.hxx"

#include <algorithm>

// common commandline parameter
#define GT_FLAG_ALLDRIVES       _T ("alldrives")
#define GT_FLAG_ALLFIXED        _T ("allfixed")
#define GT_FLAG_ALLNETWORK      _T ("allnetwork")
#define GT_FLAG_FLUSHOUTPUT     _T ("flushoutput")
#define GT_FLAG_HELP            _T ("?")
#define GT_FLAG_LISTOPTS        _T ("listopts")
#define GT_FLAG_NOCOLOR         _T ("nocolor")
#define GT_FLAG_NOERRBOX        _T ("noerrbox")
#define GT_FLAG_NOCONFIGGLOBAL  _T ("ncg")
#define GT_FLAG_NOCONFIGLOCAL   _T ("ncl")
#define GT_FLAG_NOLOGO          _T ("nologo")
#define GT_FLAG_OUTHTML         _T ("outhtml")
#define GT_FLAG_OUTLINENUMS     _T ("outlinenums")
#define GT_FLAG_OUTPAUSE        _T ("outpause")
#define GT_FLAG_OUTXML          _T ("outxml")
#define GT_FLAG_SUBDIRS         _T ("s")

namespace GT {

// global variables
Timer g_aAlloverTimer;
Timer g_aFileTimer;

/* Set the output file format to stdout.
   Set the parameter bInit to disable all dynamic descisions.
   The fallback should always be to call this function with the
     parameter value 'true'
 */
//--------------------------------------------------------------------
static void __SetStdoutOutput ()
//--------------------------------------------------------------------
{
  // set function pointer
  SetOutputFuncPtrs (GetStdoutOutputFuncPtrs ());

  // init output
  Output_Settings aOutSettings;
  aOutSettings.m_nSizeOfStruct    = sizeof (Output_Settings);
  aOutSettings.m_nFlagUsage       = GT_OUTPUT_SETTINGS_USE_ALL;
  aOutSettings.m_nFlagValues      = 0;
  aOutSettings.m_nPauseAfterLines = 0;
  aOutSettings.m_hInstance        = 0;
  aOutSettings.m_nCmdShow         = 0;
  out_init (&aOutSettings);
}

/*! Register all commandline parameters that are common for all
      applications.
 */
//--------------------------------------------------------------------
static void __RegisterDefaultCommandlineParams ()
//--------------------------------------------------------------------
{
  static const tParamBool _BOOL_FLAGS [] = {
    { GT_FLAG_HELP,           HRC_APP_FLAG_HELP,           false },
    { GT_FLAG_ALLDRIVES,      HRC_APP_FLAG_ALLDRIVES,      false },
    { GT_FLAG_ALLFIXED,       HRC_APP_FLAG_ALLFIXED,       false },
    { GT_FLAG_ALLNETWORK,     HRC_APP_FLAG_ALLNETWORK,     false },
    { GT_FLAG_FLUSHOUTPUT,    HRC_APP_FLAG_FLUSHOUTPUT,    false },
    { GT_FLAG_LISTOPTS,       HRC_APP_FLAG_LISTOPTS,       false },
    { GT_FLAG_LISTMODE,       HRC_APP_FLAG_LISTMODE,       false },
    { GT_FLAG_NOCONFIGGLOBAL, HRC_APP_FLAG_NOCONFIGGLOBAL, false },
    { GT_FLAG_NOCONFIGLOCAL,  HRC_APP_FLAG_NOCONFIGLOCAL,  false },
    { GT_FLAG_NOCOLOR,        HRC_APP_FLAG_NOCOLOR,        false },
    { GT_FLAG_NOERRBOX,       HRC_APP_FLAG_NOERRBOX,       false },
    { GT_FLAG_NOLOGO,         HRC_APP_FLAG_NOLOGO,         false },
    { GT_FLAG_OUTHTML,        HRC_APP_FLAG_OUTHTML,        false },
    { GT_FLAG_OUTLINENUMS,    HRC_APP_FLAG_OUTLINENUMS,    false },
    { GT_FLAG_OUTXML,         HRC_APP_FLAG_OUTXML,         false },
    { GT_FLAG_SUBDIRS,        HRC_APP_FLAG_SUBDIRS,        false },
    { GT_FLAG_WAITFORKEY,     HRC_APP_FLAG_WAITFORKEY,     false },
  };

  static const tParamLong _LONG_FLAGS [] = {
    { GT_FLAG_OUTPAUSE, HRC_APP_FLAG_OUTPAUSE, 0 },
  };

  size_t i;

  for (i = 0; i < GT_ARRAY_SIZE (_BOOL_FLAGS); ++i)
    CmdlineParam_Register (new CmdlineParamBool (_BOOL_FLAGS[i]));

  for (i = 0; i < GT_ARRAY_SIZE (_LONG_FLAGS); ++i)
    CmdlineParam_Register (new CmdlineParamLong (_LONG_FLAGS[i]));
};

/*! Register the default commandline options and the application
      specific stuff as well
 */
//--------------------------------------------------------------------
static void __RegisterCommandlineParams ()
//--------------------------------------------------------------------
{
  size_t i;
  tParamBool const *pBool;
  tParamLong const *pLong;
  tParamString const *pString;

  // register default stuff
  __RegisterDefaultCommandlineParams ();

  // register all bool switches
  i = 0;
  while ((pBool = GetBoolParam (i++)) != NULL)
    CmdlineParam_Register (new CmdlineParamBool (*pBool));

  // register all long switches
  i = 0;
  while ((pLong = GetLongParam (i++)) != NULL)
    CmdlineParam_Register (new CmdlineParamLong (*pLong));

  // register all string switches
  i = 0;
  while ((pString = GetStringParam (i++)) != NULL)
    CmdlineParam_Register (new CmdlineParamString (*pString));

  // ... and set them to default values
  CmdlineParam_SetToDefault ();
}

/*! Helper function to determine whether a commandline parameter
      string is a switch or anything else
 */
//--------------------------------------------------------------------
static bool __IsSwitch (LPCTSTR s)
//--------------------------------------------------------------------
{
  ASSERT (s && *s);
  return (*s == _T ('-') || *s == _T ('/')) &&
         (_tcslen (s) > 1);
}

/*! After all cmdline swicthes are registered, it's time to apply them.
    Both the ones from the cmdline and the one from the (optional)
      config files are read.
 */
//--------------------------------------------------------------------
static bool __ApplyCommandlineFlags
                                        (int     nCmdCountStart,
                                         int     argc,
                                         TCHAR** argv)
//--------------------------------------------------------------------
{
  int i;

  // apply all switches passed in argc/argv
  for (i = nCmdCountStart; i < argc; i++)
    if (__IsSwitch (argv[i]))
      CmdlineParam_Apply (_tcsinc (argv[i]));  //< skip the first char

  // should the config file should be read?
  const bool bReadLocalConfigFile  = !CmdlineParam_GetBool (GT_FLAG_NOCONFIGLOCAL);
  const bool bReadGlobalConfigFile = !CmdlineParam_GetBool (GT_FLAG_NOCONFIGGLOBAL);
  if (bReadLocalConfigFile || bReadGlobalConfigFile)
  {
    // read the config file
    ConfigFile aConfigFile (GetConfigfileName (),
                            bReadLocalConfigFile,
                            bReadGlobalConfigFile);
    if (!aConfigFile.Read ())
      return false;

    // overwrite the parameter from the config file with those from the commandline!
    // therefore the commandline is parsed another time after any config file was read!
    for (i = nCmdCountStart; i < argc; i++)
      if (__IsSwitch (argv[i]))
        CmdlineParam_Apply (_tcsinc (argv[i]));  //< skip the first char
  }

  return true;
}

/*! Handle any dependencies between parameters.
 */
//--------------------------------------------------------------------
static void __HandleCommandlineDependencies ()
//--------------------------------------------------------------------
{
  // if any of the "all drives" flags is used, subdir
  // listing must be enabled to!
  if (CmdlineParam_GetBool (GT_FLAG_ALLDRIVES) ||
      CmdlineParam_GetBool (GT_FLAG_ALLFIXED) ||
      CmdlineParam_GetBool (GT_FLAG_ALLNETWORK))
  {
    CmdlineParam_ApplyBool (GT_FLAG_SUBDIRS, true);
  }

  // if we're in any other than STDOUT mode, disable pause mode
  if (CmdlineParam_GetBool (GT_FLAG_OUTHTML) ||
      CmdlineParam_GetBool (GT_FLAG_OUTXML))
  {
    CmdlineParam_ApplyLong (GT_FLAG_OUTPAUSE, 0);
  }

  if (CmdlineParam_GetBool (GT_FLAG_OUTXML))
  {
    // disable colors when printing XML
    CmdlineParam_ApplyBool (GT_FLAG_NOCOLOR, true);
    CmdlineParam_ApplyBool (GT_FLAG_NOLOGO, true);
  }

  // call external function
  HandleCommandlineDependencies ();
}

/*! After all cmdline switches are applied we can finally determine
     what output format we weant to use.
 */
//--------------------------------------------------------------------
static void __InitOutput (HINSTANCE hInstance, int nCmdShow)
//--------------------------------------------------------------------
{
  // flush all old output before reiniting
  out_flush ();

  if (CmdlineParam_GetBool (GT_FLAG_OUTHTML))
  {
    // switch to HTML
    SetOutputFuncPtrs (GetHTMLOutputFuncPtrs ());
  }
  else
  if (CmdlineParam_GetBool (GT_FLAG_OUTXML))
  {
    // switch to XML
    SetOutputFuncPtrs (GetXMLOutputFuncPtrs ());
  }
  // else the default STDOUT is used

  // init output
  Output_Settings aOutSettings;
  aOutSettings.m_nSizeOfStruct    = sizeof (Output_Settings);
  aOutSettings.m_nFlagUsage       = GT_OUTPUT_SETTINGS_USE_ALL;

  // the flags depend on the cmdline-switches
  aOutSettings.m_nFlagValues      = 0;
  if (CmdlineParam_GetBool (GT_FLAG_LISTMODE))
    aOutSettings.m_nFlagValues |= GT_OUTPUT_SETTINGS_USE_LISTMODE;
  if (CmdlineParam_GetBool (GT_FLAG_NOCOLOR))
    aOutSettings.m_nFlagValues |= GT_OUTPUT_SETTINGS_USE_NOCOLOR;
  if (CmdlineParam_GetBool (GT_FLAG_FLUSHOUTPUT))
    aOutSettings.m_nFlagValues |= GT_OUTPUT_SETTINGS_USE_FLUSH;
  if (CmdlineParam_GetBool (GT_FLAG_OUTLINENUMS))
    aOutSettings.m_nFlagValues |= GT_OUTPUT_SETTINGS_USE_LINENUMS;

  aOutSettings.m_nPauseAfterLines = CmdlineParam_GetLong (GT_FLAG_OUTPAUSE);
  aOutSettings.m_hInstance        = hInstance;
  aOutSettings.m_nCmdShow         = nCmdShow;
  out_init (&aOutSettings);

  // Shall we show Error Box or not?
  EnableInlineErrorMessages (CmdlineParam_GetBool (GT_FLAG_NOERRBOX));
}

/*! Unless the switch "-nologo" is passedm the copyright line is
      shown.
 */
//--------------------------------------------------------------------
static void __ShowHeadline ()
//--------------------------------------------------------------------
{
  if (!CmdlineParam_GetBool (GT_FLAG_NOLOGO))
  {
    // write name and version
    out_format (_T ("%s %s"),
                GetProgramName (),
                GetProgramVersion ());

    // and append copyright tag
    out_append (rc (HRC_APP_HEADER));
    out_append (_T ("\n"));

    // flush is necessary if we're in listmode to avoid cutting
    // of the first file's info
    out_flush ();
  }
}

/*! Sometimes the listmode needs to be disabled.
    E.g. for showing the syntax or when cleaning up and showing
      the stats.
 */
//--------------------------------------------------------------------
static void __DisableListmode ()
//--------------------------------------------------------------------
{
  if (CmdlineParam_GetBool (GT_FLAG_LISTMODE))
  {
    // reinit without listmode (flush is done inside)
    Output_Settings x;
    x.m_nSizeOfStruct = sizeof (Output_Settings);
    x.m_nFlagUsage    = GT_OUTPUT_SETTINGS_USE_LISTMODE;
    x.m_nFlagValues   = 0;
    out_init (&x);
  }
}

/*! Helper function to sort cmdline switches according
      to their names.
 */
//--------------------------------------------------------------------
static bool __CmdlineSorter (CmdlineParam* p1, CmdlineParam* p2)
//--------------------------------------------------------------------
{
  return _tcscmp (p1->m_sName, p2->m_sName) < 0;
}

/*! Show the syntax and the parameters of the proggy.
 */
//--------------------------------------------------------------------
static void __ShowSyntax (const bool bWithParams)
//--------------------------------------------------------------------
{
  pstring sText;

  // Avoid showing the first line only
  __DisableListmode ();

  out_format (rc (HRC_APP_SYNTAX),
              GetProgramName ());

  if (bWithParams)
  {
    out_append (_T ("\n"));
    out_append (rc (HRC_APP_SYNTAX_OPTIONS));

    // get list of all registered cmdline switches
    CmdlineParamList const* pCmdlineFlags = CmdlineParam_GetList ();

    // create a copy of the list and sort it by name
    CmdlineParamList aCopy;
    aCopy.assign (pCmdlineFlags->begin (), pCmdlineFlags->end ());
    std::sort (aCopy.begin (), aCopy.end (), __CmdlineSorter);

    // foreach cmdline parameter
    CmdlineParamList::const_iterator cit = aCopy.begin ();
    for (; !(cit == aCopy.end ()); ++cit)
    {
      // 11 is the length of the longest switch
      out_format (_T ("  /%-11s - "), (*cit)->m_sName);
      out_append (rc ((*cit)->m_nHelpResID));
      out_append (_T ("\n"));
    }
  }
  else
  {
    // don't list the commandline switches
    // Just tell the user how to list them
    out_append (_T ("\n"));
    out_format (rc (HRC_APP_HOW_TO_SHOW_PARAMS),
                GetProgramName ());
  }
}

/*! Start handling all non-option commandline parameters.
    The configuration file is not considered because it makes
      no sense to have common files to scan all the time.
 */
//--------------------------------------------------------------------
static int __EvalAllFileParameters
                                        (int     nCmdCountStart,
                                         int     argc,
                                         TCHAR** argv)
//--------------------------------------------------------------------
{
  int i;
  int nTotalNonFlagsParameters = 0;

  // loop over all commandline parameters
  for (i = nCmdCountStart; i < argc; i++)
    if (!__IsSwitch (argv[i]))
    {
      ++nTotalNonFlagsParameters;
      __EvaluateCmdlineParameter (argv[i],
                                  HandleFilename,
                                  CmdlineParam_GetBool (GT_FLAG_SUBDIRS));
    }

  return nTotalNonFlagsParameters;
}

/*! Do all the cleanup stuff in the correct order.
 */
//--------------------------------------------------------------------
static void __Cleanup
                                        (const int nTotalNonFlagsParameters)
//--------------------------------------------------------------------
{
  __DisableListmode ();

  // external
  HandleCleanup (nTotalNonFlagsParameters);

  // flush the buffer - print anything left
  out_flush ();

  // and finally call the method to close the output...
  // (may handle any WaitForKey switch!!!)
  out_done ();

#ifdef _MSC_VER
  // cleanup crash tracker
  Fct_CleanupDumpfileForEXE ();
#endif
}

}  // namespace

using namespace GT;

/*
static void __CheckVMWare ()
{
  bool bVMWare = false;
  __asm {
    push ecx
    push eax
    push edx
    mov ecx, 0Ah
    mov eax, 'VMXh'
    mov dx,  'VX'
    in  eax, dx       // leads to 0xc0000096
    cmp ebx, 'VMXh'
    jne done
    mov bVMWare, 1
  done:
    pop edx
    pop eax
    pop ecx
  }
  if (bVMWare)
    fprintf (stderr, "Running in VMWare\n");
}
*/

//--------------------------------------------------------------------
int __cdecl main ()
//--------------------------------------------------------------------
{
  // dummies, in case we'll ever have an WinMain
  HINSTANCE hInstance = 0;
  int nCmdShow = 0;

//  __CheckVMWare ();

  int argc = 0;
  TCHAR **argv = NULL;
  GetCommandLineArgs (argc, argv);

  // temp
  int nCmdCountStart = 1;
  int nTotalNonFlagsParameters = -1;
  int nReturn = 0;

  // give the user a possibility to jump into the debugger!
  if (argc > 1 &&
      _tcscmp (argv[1], _T ("-debug+")) == 0)
  {
    nCmdCountStart++;
    DebugBreak ();
  }

#ifdef _MSC_VER
  // init crash tracker
  Fct_InitDumpfileForEXE ();
#endif

  // First init the output with stdout, because stdout is always present
  __SetStdoutOutput ();

  // determine which commandline parameter are present!
  __RegisterCommandlineParams ();

  // apply the commandline switches passed to the program
  // and (if necessary) apply the flags from the configuration file
  if (!__ApplyCommandlineFlags (nCmdCountStart, argc, argv))
    out_error_append (rc (HRC_APP_ERROR_READ_CFGFILE));

  // handle dependencies between cmdline switches
  __HandleCommandlineDependencies ();

  // now we can really init the output device
  __InitOutput (hInstance, nCmdShow);

  // write headline (defaults to stdout)
  __ShowHeadline ();

  // start timer!
  g_aAlloverTimer.Start ();

  // check if /listopts was used
  if (CmdlineParam_GetBool (GT_FLAG_LISTOPTS))
  {
    out_append (_T ("\n"));
    out_append (rc (HRC_APP_USED_OPTIONS));
    pstring sCmdline = CmdlineParam_GetAsString ();
    __DisableListmode ();
    out_append (sCmdline.c_str ());
  }
  else
  // check if syntax should be displayed
  if (CmdlineParam_GetBool (GT_FLAG_HELP))
  {
    // show with params
    __ShowSyntax (true);
    nReturn = 1;
  }
  else
  {
    if (CmdlineParam_GetBool (GT_FLAG_ALLDRIVES))
      DriveInfo::Instance ().Init (GT_DRIVEINFO_ALL);
    else
    if (CmdlineParam_GetBool (GT_FLAG_ALLFIXED))
      DriveInfo::Instance ().Init (GT_DRIVEINFO_FIXED);
    else
    if (CmdlineParam_GetBool (GT_FLAG_ALLNETWORK))
      DriveInfo::Instance ().Init (GT_DRIVEINFO_NETWORK);

    // external - can/should we start searching?
    if (CanStartSearching ())
    {
      // main stuff
      nTotalNonFlagsParameters = __EvalAllFileParameters (nCmdCountStart, argc, argv);

      // was any valid commandline parameter found?
      if (nTotalNonFlagsParameters > 0)
      {
        // external
        Run ();
      }
      else
      {
        __ShowSyntax (false);
        nReturn = 2;
      }
    }
    else
    {
      nReturn = 3;
    }
  }

  __Cleanup (nTotalNonFlagsParameters);
  return nReturn;
}
