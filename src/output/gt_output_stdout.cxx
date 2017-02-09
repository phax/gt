#include "gt_output_stdout.hxx"

#include "gt_common_output.hxx"
#include "gt_errormsg.hxx"
#include "gt_table.hxx"
#include "gt_win32_console.hxx"
#include "gt_utils.hxx"

namespace GT {

// init global variables
static WindowsConsole*  g_pWinConsole = NULL;
static bool             g_bListMode = false;
static bool             g_bColors = true;
static bool             g_bFlush = false;
static EColor           g_eColor = GT_COLOR_INVALID;
static OutputMemberList g_aOutList;
static size_t           g_nIndent = 0;

START_EXTERN_C
void GT_CALL  out_stdout_flush ();
END_EXTERN_C

/*! Reset the color to the console default.
 */
//--------------------------------------------------------------------
static void _DoDefColor ()
//--------------------------------------------------------------------
{
  ASSERT (!g_bListMode);
  if (g_bColors)
  {
    ASSERT (g_pWinConsole);
    g_pWinConsole->RestoreOriginalColor ();
  }
}

/*! Set the color to the specified color.
 */
//--------------------------------------------------------------------
static void _DoColor (EColor e)
//--------------------------------------------------------------------
{
  ASSERT (!g_bListMode);
  if (g_bColors)
  {
    ASSERT (g_pWinConsole);
    g_pWinConsole->SetTextColor (e);
  }
}

/*! Print the text s for len chars.
 */
//--------------------------------------------------------------------
static void __display (LPCTSTR s, size_t len)
//--------------------------------------------------------------------
{
  // main display command (ANSI to OEM is performed inside DisplayText)
  g_pWinConsole->DisplayText (s, len);
}

//--------------------------------------------------------------------
static void _DoString (pstring &s)
//--------------------------------------------------------------------
{
  // ignore empty string
  if (!s.empty ())
  {
    if (g_bListMode)
    {
      /*! If we're in listmode (commandline parameter /l) we
            allow no newline ('\n') in the output. Therefore we
          But since after the first newline some nice information
            is printed (at least I expect this) the content of
            the second line is appended to the first line by
            replacing the '\n' with a space (' ') character.
       */

      // search for the first '\n' in the string
      size_t n = s.find_first_of (_T ('\n'));
      if (n != pstring::npos)
      {
        // okay, we found one -> make a space
        s.at (n) = _T (' ');
        ++n;

        // erase any directly following '\n'!
        while (n < s.length () && s.at (n) == _T ('\n'))
          s.erase (n, 1);

        // search the next '\n'
        n = s.find_first_of (_T ('\n'));
        if (n != pstring::npos)
        {
          // alright, we found one! -> kill until end of string
          s.erase (n);
        }
      }

      // manually append exactly one space!
      s += _T ('\n');
    }

    __display (s.c_str (), s.length ());
  }
}

/*! Must be a copy of a OutputMember!!
 */
//--------------------------------------------------------------------
static void __executeOutputMember (OutputMember aMember)
//--------------------------------------------------------------------
{
  switch (aMember.GetType ())
  {
    case OUT_STRING:
    {
      pstring *pStr = aMember.AsString ();
      _DoString (*pStr);
      delete pStr;
      break;
    }
    case OUT_COLOR:
      _DoColor (aMember.AsColor ());
      break;
    case OUT_DEFCOLOR:
      _DoDefColor ();
      break;
    default:
    {
      ShowErrorMessage (_T ("Invalid stdout output type %d"),
                        aMember.GetType ());
    }
  }
}

//--------------------------------------------------------------------
static void _DoAppend
                                        (      LPCTSTR s,
                                         const size_t  len)
//--------------------------------------------------------------------
{
  ASSERT (s);
  if (!s || !*s)
    return;
  ASSERT (len > 0);
  ASSERT (!IsBadStringPtr (s, len));

  static bool bAddIndent = true;

  // nodebug version
  if (g_eColor != GT_COLOR_INVALID)
    g_aOutList.push_back (OutputMember (g_eColor));

  // get pointer to string
  pstring* pStr = NULL;
  if (!g_aOutList.empty () &&
      g_aOutList.back ().GetType ( )== OUT_STRING)
  {
    // get last member
    pStr = g_aOutList.back ().AsString ();
  }

  // add indent
  if (bAddIndent && g_nIndent > 0)
  {
    // build indent string
    const size_t nIndentChars = g_nIndent * 2; // 2: each indent means 2 chars
    LPTSTR pIndent = (LPTSTR) _alloca ((nIndentChars + 1) * sizeof (TCHAR));
    pIndent[nIndentChars] = _T ('\0');

    // unfortunately, _tcsset does not work since the allocated
    // buffer from _alloca can contain \0 characters!
    for (size_t i = 0; i < nIndentChars; ++i)
      pIndent[i] = _T (' ');

    // add in list
    if (pStr)
    {
      // use existing string
      pStr->append (pIndent, nIndentChars);
    }
    else
    {
      g_aOutList.push_back (OutputMember (pIndent, nIndentChars, OUT_STRING));
      pStr = g_aOutList.back ().AsString ();
    }
  }

  // add string
  if (pStr)
  {
    // append to existing string
    pStr->append (s, len);
    _CheckForEndlessLoop (pStr);
  }
  else
  {
    // add a new string
    g_aOutList.push_back (OutputMember (s, len, OUT_STRING));
    pStr = g_aOutList.back ().AsString ();
  }

  // check if indent should be added next time this method is called
  ASSERT (pStr);
  ASSERT (!pStr->empty ());
  bAddIndent = ((*pStr)[pStr->length () - 1] == _T ('\n'));

  // reset color (if necessary)
  if (g_eColor != GT_COLOR_INVALID)
  {
    g_aOutList.push_back (OutputMember (OUT_DEFCOLOR));
    g_eColor = GT_COLOR_INVALID;
  }

  // always flush (cmdline switch /flushoutput) ?
  if (g_bFlush)
    out_stdout_flush ();
}

/**** here the exported functions start!! ****/

START_EXTERN_C

//--------------------------------------------------------------------
void GT_CALL out_stdout_init (Output_Settings *pSettings)
//--------------------------------------------------------------------
{
  // to avoid multiple allocations if out_stdout_init is called more than once!
  if (g_pWinConsole)
  {
    // reinit - maybe called to change the settings
    out_stdout_flush ();
  }
  else
  {
    // first time init
    g_pWinConsole = new WindowsConsole;
  }

  if (pSettings->m_nFlagUsage & GT_OUTPUT_SETTINGS_USE_LISTMODE)
    g_bListMode = pSettings->m_nFlagValues & GT_OUTPUT_SETTINGS_USE_LISTMODE;

  if (pSettings->m_nFlagUsage & GT_OUTPUT_SETTINGS_USE_NOCOLOR)
    g_bColors = !(pSettings->m_nFlagValues & GT_OUTPUT_SETTINGS_USE_NOCOLOR);

  if (pSettings->m_nFlagUsage & GT_OUTPUT_SETTINGS_USE_FLUSH)
    g_bFlush = pSettings->m_nFlagValues & GT_OUTPUT_SETTINGS_USE_FLUSH;

  if (pSettings->m_nFlagUsage & GT_OUTPUT_SETTINGS_USE_PAUSE)
    g_pWinConsole->SetPauseAfterXLines (pSettings->m_nPauseAfterLines);

  if (pSettings->m_nFlagUsage & GT_OUTPUT_SETTINGS_USE_LINENUMS)
    g_pWinConsole->EnablePrintLineNumbers (pSettings->m_nFlagValues & GT_OUTPUT_SETTINGS_USE_LINENUMS);
}

//--------------------------------------------------------------------
void GT_CALL out_stdout_done ()
//--------------------------------------------------------------------
{
  // delete console handler
  delete g_pWinConsole;
  g_pWinConsole = NULL;
}

//--------------------------------------------------------------------
void GT_CALL out_stdout_setcolor (const EColor eColor)
//--------------------------------------------------------------------
{
  if (!g_bListMode)
    g_eColor = eColor;
}

//--------------------------------------------------------------------
void GT_CALL out_stdout_append (LPCTSTR s)
//--------------------------------------------------------------------
{
  size_t nRealLen;

  if (str_isresptr (s))
  {
    // it's an ID!
    LPTSTR pReal;
    nRealLen = GetStringResBuf ((resid_t) s, &pReal);
    _DoAppend (pReal, nRealLen);
  }
  else
  {
    // a string...
    _DoAppend (s, _tcslen (s));
  }
}

//--------------------------------------------------------------------
void GT_CDECL out_stdout_format (LPCTSTR pFmt, ...)
//--------------------------------------------------------------------
{
  const int BUFLEN = 8192;
  static TCHAR sSTATIC[BUFLEN];

  ASSERT (pFmt);

  // start with the old args!
  va_list args;
  va_start (args, pFmt);

  // resolve potential resource ID
  str_auto_resolve_resid (pFmt);

  // get length of result string (VC7 special)
  // If length is too long for static buffer, create a dynamic one
  int nMemLen = _vsctprintf (pFmt, args) + 1;
  LPTSTR pTemp = nMemLen <= BUFLEN
                   ? sSTATIC
                   : new TCHAR [nMemLen];

  // resolve format
  int nLen = _vstprintf (pTemp, pFmt, args);

  // check result
  if (nLen < 0)
  {
/* Happens currently :(
    ShowErrorMessage (_T ("Internal buffer overflow at %s: %d\n"), _T (__FILE__), __LINE__);
    FatalAppExit (0xdead, _T ("Internal buffer overflow in out_stdout_format"));
*/
  }
  else
  {
    _DoAppend (pTemp, nLen);
  }

  // set to NULL to avoid further usage
  va_end (args);

  // free mem if necessary
  if (nMemLen > BUFLEN)
    delete [] pTemp;
}

//--------------------------------------------------------------------
void GT_CALL out_stdout_flush ()
//--------------------------------------------------------------------
{
  if (g_bListMode)
  {
    /*
     * if list mode is active collect only the strings
     * and call the execute methode
     * colors are not of interest here!!
     */
    if (!g_aOutList.empty ())
    {
      ASSERT (g_aOutList.size () == 1);
      __executeOutputMember (g_aOutList.front ());
    }
  }
  else
  {
    // if we're in normal mode switch the colors like we need it!
    OutputMemberList::const_iterator cit = g_aOutList.begin ();
    for (; !(cit == g_aOutList.end ()); ++cit)
      __executeOutputMember (*cit);
  }
  g_aOutList.clear ();

  // restore original (still required in 0.31)!
  if (g_bColors)
  {
    ASSERT (g_pWinConsole);
    g_pWinConsole->RestoreOriginalColor ();
  }
}

//--------------------------------------------------------------------
void GT_CALL out_stdout_filestart (Output_FilenameStruct *pData)
//--------------------------------------------------------------------
{
  ASSERT (pData);

  DoublePair aPos;
  const bool bOpenedFile = (pData->m_nTotalFileSize >= 0);

  // set no indentation
  // Can be != 0 if an exception occured in analyzing of last file
  g_nIndent = 0;

  if (pData->m_nScanOffset == 0)
  {
    // first filename header of the file
    out_stdout_setcolor (eColorHEADER);
    if (!bOpenedFile || g_bListMode)
    {
      // for files which could not be opened but the name was identified
      out_stdout_format (rc (HRC_APP_OUTPUT_STDOUT_FILESTART_EXT),
                         pData->m_sFilename);
    }
    else
    {
      // file was normally opened

      // get text mode/binary mode
      pstring sMode;
      if (pData->m_nFlags & GT_OUTPUT_FILENAME_DOSTEXT)
        str_assign_from_res (sMode, HRC_APP_OUTPUT_FILESTART_DOS_TEXT);
      else
      if (pData->m_nFlags & GT_OUTPUT_FILENAME_UNIXTEXT)
        str_assign_from_res (sMode, HRC_APP_OUTPUT_FILESTART_UNIX_TEXT);
      else
      if (pData->m_nFlags & GT_OUTPUT_FILENAME_MACTEXT)
        str_assign_from_res (sMode, HRC_APP_OUTPUT_FILESTART_MAC_TEXT);
      else
        str_assign_from_res (sMode, HRC_APP_OUTPUT_FILESTART_BINARY);

      // write list mode info
      out_stdout_format (rc (HRC_APP_OUTPUT_STDOUT_FILESTART_LIST),
                         pData->m_sFilename,
                         pData->m_nTotalFileSize,
                         sMode.c_str ());
    }
  }
  else
  {
    // listmode should disable overlay stuff automatically
    ASSERT (!g_bListMode);

    _splitdouble (double (pData->m_nScanOffset) / pData->m_nTotalFileSize * 100, aPos, 2);

    out_stdout_format (rc (HRC_APP_OUTPUT_STDOUT_FILESTART_NEXT_LEVEL),
                       pData->m_nScanOffset,
                       pData->m_nScanOffset,
                       pData->m_nTotalFileSize - pData->m_nScanOffset,
                       pData->m_nTotalFileSize - pData->m_nScanOffset,
                       aPos.h, aPos.l);
  }

  // not possible e.g. on c:\pagefile.sys
  if (bOpenedFile && pData->m_nScanOffset == 0)
  {
    DWORD nAttributes = ::GetFileAttributes (pData->m_sFilename);
    if (nAttributes == (DWORD) -1)
    {
      out_append (_T ("\n"));
      out_stdout_setcolor (eColorERROR);
      out_stdout_format (rc (HRC_APP_OUTPUT_FILESTART_NO_ATTRS),
                         pData->m_sFilename);
      return;
    }

    if (nAttributes & FILE_ATTRIBUTE_COMPRESSED)
    {
      out_stdout_setcolor (eColorINTERESTING);
      out_stdout_append (rc (HRC_APP_OUTPUT_FILESTART_ATTRS_COMPRESSED));
    }

    if (nAttributes & FILE_ATTRIBUTE_OFFLINE)
    {
      out_stdout_setcolor (eColorINTERESTING);
      out_stdout_append (rc (HRC_APP_OUTPUT_FILESTART_ATTRS_OFFLINE));
    }
  }

  out_stdout_append (_T ("\n\n"));
}

//--------------------------------------------------------------------
void GT_CALL out_stdout_filedone ()
//--------------------------------------------------------------------
{
  // not necessarily - e.g. not on overlay data
  // Happens when an exception is thrown in analyzing
  ASSERT (g_nIndent == 0);

  // automatically append one newline
  out_stdout_append (_T ("\n"));

  out_stdout_flush ();
}

//--------------------------------------------------------------------
void GT_CALL out_stdout_fileend ()
//--------------------------------------------------------------------
{
  /* empty */
}

//--------------------------------------------------------------------
void GT_CALL out_stdout_status (LPCTSTR s)
//--------------------------------------------------------------------
{
  ASSERT (s);

  str_auto_resolve_resid (s);
  g_pWinConsole->SetTitle (s);
}

//--------------------------------------------------------------------
void GT_CALL out_stdout_table (Table* pTable)
//--------------------------------------------------------------------
{
  // this amount is needed on large export table strings (e.g. "orb_pr.dll" : 2048)
  const size_t GT_TABLE_BUFSIZE = 2048;

  size_t c, r, i;
  TCHAR sOutput[GT_TABLE_BUFSIZE];

  const size_t nCols          = pTable->GetColumnCount ();
  const size_t nRows          = pTable->GetRowCount ();
  TableColumnVector& aColumns = pTable->GetColumns ();

  // create the headline
  if (pTable->IsHeadlineEnabled ())
  {
    for (c = 0; c < nCols; c++)
    {
      TableColumn* pColumn = aColumns[c];

      // do not align the last column
      if (pColumn->GetType () == STR)
      {
        size_t nTextWidth = pColumn->GetWidth () - 2;

        // align the string!
        if (pColumn->GetFormatWidth () == ALIGN_RIGHT)
          out_stdout_format (_T ("%*s"), nTextWidth, pColumn->GetpName ());
        else
        if (pColumn->GetFormatWidth () == ALIGN_CENTER)
          out_stdout_format (_T ("%*s"), nTextWidth / 2, pColumn->GetpName ());
        else
          out_stdout_format (_T ("%-*s"), nTextWidth, pColumn->GetpName ());

        // append 2 spaces separator
        out_stdout_append (_T ("  "));
      }
      else
      {
        // align left
        out_stdout_format (_T ("%-*s"), pColumn->GetWidth (), pColumn->GetpName ());
      }
    }
    out_append (_T ("\n"));
  }

  // remember the row index for each column (for colspan)
  size_t *nRowIndex = new size_t[nCols];
  for (c = 0; c < nCols; ++c)
    nRowIndex[c] = 0;

  // loop over all rows
  for (r = 0; r < nRows; ++r)
  {
    // loop over all columns
    for (c = 0; c < nCols; ++c)
    {
      TableColumn* pColumn = aColumns[c];
      TableCell* pCell = pColumn->GetCell (nRowIndex[c]);

      // next row index for this column
      nRowIndex[c]++;

      // handle colspan
      const size_t nColSpan = pCell->GetColSpan ();
      if (nColSpan > 1)
        c += nColSpan - 1;

      // handle color
      if (pCell->HasCustomColor ())
        out_stdout_setcolor (pCell->GetColor ());

      // do the output depending on the type
      switch (pColumn->GetType ())
      {
        case NUM_ZERO:
        case NUM_SPACE:
        case HEX:
        {
          // int32 or int64?
          switch (pCell->GetType ())
          {
            case CELL_INT32:
            {
              if (pColumn->IsAligned ())
              {
                _stprintf (sOutput, pColumn->GetType () == NUM_ZERO
                                      ? _T ("%0*ld")
                                      : pColumn->GetType () == NUM_SPACE
                                        ? _T ("%*ld")
                                        : _T ("%0*Xh"),
                                    pColumn->GetFormatWidth (),
                                    pCell->GetInt32 ());
              }
              else
              {
                _stprintf (sOutput, pColumn->GetType () == HEX
                                      ? _T ("%Xh")
                                      : _T ("%ld"),
                                    pCell->GetInt32 ());
              }
              break;
            }
            case CELL_INT64:
            {
              if (pColumn->IsAligned ())
              {
                _stprintf (sOutput, pColumn->GetType () == NUM_ZERO
                                      ? _T ("%0*I64ld")
                                      : pColumn->GetType () == NUM_SPACE
                                        ? _T ("%*I64ld")
                                        : _T ("%0*I64Xh"),
                                    pColumn->GetFormatWidth (),
                                    pCell->GetInt64 ());
              }
              else
              {
                _stprintf (sOutput, pColumn->GetType () == HEX
                                      ? _T ("%I64Xh")
                                      : _T ("%I64ld"),
                                    pCell->GetInt64 ());
              }
              break;
            }
            default:
              // invalid type!
              ASSERT (0);
          } // cell-type
          break;
        }
        case NUM_DOUBLE:
        {
          const int nFormatWidth = pColumn->IsAligned ()
                                     ? pColumn->GetFormatWidth ()
                                     : DEFAULT_FORMATWIDTH_DOUBLE;

          DoublePair aDbl;
          _splitdouble (pCell->GetDouble (), aDbl, nFormatWidth);
          _stprintf (sOutput, _T ("%d.%0*d"), aDbl.h, nFormatWidth, aDbl.l);
          break;
        }
        case STR:
        {
          if (pCell->GetStr ()->length () >= GT_TABLE_BUFSIZE)
          {
            ShowErrorMessage (_T ("Internal error: string is too long %d/%d"),
                              pCell->GetStr ()->length (), GT_TABLE_BUFSIZE);
          }

          size_t nTextWidth = pColumn->GetWidth () - 2;
          LPCTSTR pText = pCell->GetStr ()->c_str ();

          // align the string!
          if (pColumn->GetFormatWidth () == ALIGN_RIGHT)
            _stprintf (sOutput, _T ("%*s"), nTextWidth, pText);
          else
          if (pColumn->GetFormatWidth () == ALIGN_CENTER)
            _stprintf (sOutput, _T ("%*s"), nTextWidth / 2, pText);
          else
            _stprintf (sOutput, _T ("%-*s"), nTextWidth, pText);
          break;
        }
      }

      // align width of the column from current length to the specified width
      i = _tcslen (sOutput);
      if (i < pColumn->GetWidth ())
      {
        do
        {
          sOutput[i++] = _T (' ');
        } while (i < pColumn->GetWidth ());
        sOutput[i] = 0;
      }
      out_stdout_append (sOutput);
    }

    // end of line
    out_stdout_append (_T ("\n"));
  }

  // free memory
  delete [] nRowIndex;
}

//--------------------------------------------------------------------
void GT_CALL out_stdout_incindent (void)
//--------------------------------------------------------------------
{
  // no indentation in listmode
  if (!g_bListMode)
    ++g_nIndent;
}

//--------------------------------------------------------------------
void GT_CALL out_stdout_decindent (void)
//--------------------------------------------------------------------
{
  // no indentation in listmode
  if (!g_bListMode)
  {
    ASSERT (g_nIndent > 0);
    if (g_nIndent > 0)
      --g_nIndent;
  }
}

END_EXTERN_C

//--------------------------------------------------------------------
OutputFuncPtrs GT_CALL GetStdoutOutputFuncPtrs ()
//--------------------------------------------------------------------
{
  static const OutputFuncPtrs ofp = {
    out_stdout_init,
    out_stdout_done,

    out_stdout_setcolor,
    out_stdout_append,
    out_stdout_format,
    out_stdout_table,

    out_stdout_incindent,
    out_stdout_decindent,

    out_stdout_filestart,
    out_stdout_filedone,
    out_stdout_fileend,

    out_stdout_flush,
    out_stdout_status,
  };
  return ofp;
}

}  // namespace
