#include "gt_output_html.hxx"

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
static OutputMemberList g_aOutList;
static EColor           g_eColor = GT_COLOR_INVALID;
static size_t           g_nIndent = 0;

START_EXTERN_C
void GT_CALL  out_html_flush ();
END_EXTERN_C

//--------------------------------------------------------------------
static void __display (LPCTSTR p, size_t nLen = size_t (-1))
//--------------------------------------------------------------------
{
  g_pWinConsole->DisplayText (p, nLen == size_t (-1) ? _tcslen (p) : nLen);
}

//--------------------------------------------------------------------
static void __printDefaultColor ()
//--------------------------------------------------------------------
{
  ASSERT (!g_bListMode);
  if (g_bColors)
  {
    __display (_T ("</font>"));
  }
}

//--------------------------------------------------------------------
static LPCTSTR __getColorString (const EColor e)
//--------------------------------------------------------------------
{
  ASSERT (int (e) >= 0 && int (e) < 16);

  static const LPCTSTR COLS[] =
  {
    _T ("000000"),
    _T ("0000c0"),
    _T ("00c000"),
    _T ("00c0c0"),
    _T ("c00000"),
    _T ("c000c0"),
    _T ("c0c000"),
    _T ("c0c0c0"),
    // like 0-7 but with intensity
    _T ("808080"),
    _T ("0000ff"),
    _T ("00ff00"),
    _T ("00ffff"),
    _T ("ff0000"),
    _T ("ff00ff"),
    _T ("ffff00"),
    _T ("ffffff"),
    // invalid color
    _T ("[INVALID]"),
  };

  return COLS[e];
}

//--------------------------------------------------------------------
static void __printColor (const EColor e)
//--------------------------------------------------------------------
{
  ASSERT (!g_bListMode);
  if (g_bColors)
  {
    __display (_T ("<font color=\"#"));
    __display (__getColorString (e));
    __display (_T ("\">"));
  }
}

//--------------------------------------------------------------------
static void __prepareListMode (pstring &s)
//--------------------------------------------------------------------
{
  ASSERT (!s.empty ());

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

        // manually append exactly one DIV!
        s += _T ("</div>\n");
      }
    }
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
      __prepareListMode (*pStr);
      __display (pStr->c_str (), pStr->length ());
      delete pStr;
      break;
    }
    case OUT_COLOR:
      __printColor (aMember.AsColor ());
      break;
    case OUT_DEFCOLOR:
      __printDefaultColor ();
      break;
    default:
      ShowErrorMessage (_T ("Invalid html output type %d"),
                        aMember.GetType ());
  }
}

//--------------------------------------------------------------------
static void __replaceHTMLSpecialChars (pstring &s)
//--------------------------------------------------------------------
{
  // replace all chars
  struct Replace
  {
    TCHAR   cOld;   // old char
    LPCTSTR sNew;   // new entity
    UINT    nACP;   // codepage
  };
  const UINT NO_ACP = 0;  // the codepage to replace anyway

  const Replace REPLACE [] = {
  { _T ('&'),  _T ("&amp;"),   NO_ACP },
  { _T ('<'),  _T ("&lt;"),    NO_ACP },
  { _T ('>'),  _T ("&gt;"),    NO_ACP },
  { _T ('"'),  _T ("&quot;"),  NO_ACP },
  { _T ('ß'),  _T ("&szlig;"), NO_ACP },
  { _T ('ä'),  _T ("&auml;"),  1252 },
  { _T ('Ä'),  _T ("&Auml;"),  1252 },
  { _T ('ö'),  _T ("&ouml;"),  1252 },
  { _T ('Ö'),  _T ("&Ouml;"),  1252 },
  { _T ('ü'),  _T ("&uuml;"),  1252 },
  { _T ('Ü'),  _T ("&Uuml;"),  1252 },
  };

  // get current ANSI CodePage
  const UINT nACP = ::GetACP ();

  // replace only if codepage matches!
  for (size_t i = 0; i < GT_ARRAY_SIZE (REPLACE); ++i)
    if (REPLACE[i].nACP == NO_ACP || REPLACE[i].nACP == nACP)
      str_replace_all (s, REPLACE[i].cOld, REPLACE[i].sNew);

  str_replace_all (s, _T ("  "), _T ("&nbsp;&nbsp;"));

  // don't do it in listmode, because our "fetch all following \n wouldn't work"
  if (!g_bListMode)
    str_replace_all (s, _T ('\n'), _T ("<br>\n"));
}

//--------------------------------------------------------------------
static void __appendText
                                        (      LPCTSTR  s,
                                         const size_t   len,
                                         const bool     bReplaceEntities)
//--------------------------------------------------------------------
{
  ASSERT (s);
  if (s && *s)
  {
    ASSERT (len > 0);
    ASSERT (!IsBadStringPtr (s, len));

    pstring sText (s, len);
    if (bReplaceEntities)
    {
      __replaceHTMLSpecialChars (sText);
    }

    // push color?
    if (g_eColor != GT_COLOR_INVALID)
      g_aOutList.push_back (OutputMember (g_eColor));

    // push text!
    pstring *pStr = NULL;
    if (!g_aOutList.empty () &&
        g_aOutList.back ().GetType () == OUT_STRING)
    {
      // get existing string and append
      pStr = g_aOutList.back ().AsString ();
      pStr->append (sText);
      _CheckForEndlessLoop (pStr);
    }
    else
    {
      // add a new member
      g_aOutList.push_back (OutputMember (sText, OUT_STRING));
      pStr = g_aOutList.back ().AsString ();
    }

    // restore color?
    if (g_eColor != GT_COLOR_INVALID)
    {
      g_aOutList.push_back (OutputMember (OUT_DEFCOLOR));
      g_eColor = GT_COLOR_INVALID;
    }

    // always flush (cmdline switch /flushoutput)?
    if (g_bFlush)
      out_html_flush ();
  }
}

//--------------------------------------------------------------------
static void __appendHTML (LPCTSTR s, size_t len = size_t (-1))
//--------------------------------------------------------------------
{
  __appendText (s, len != size_t (-1) ? len : _tcslen (s), false);
}

//--------------------------------------------------------------------
static void __appendHTML (const pstring& s)
//--------------------------------------------------------------------
{
  __appendText (s.c_str (), s.length (), false);
}

//--------------------------------------------------------------------
static void __appendHTMLFilename (LPCTSTR p)
//--------------------------------------------------------------------
{
  const TCHAR HEXTAB[17] = _T ("0123456789abcdef");
  pstring s;
  while (*p)
  {
    switch (*p)
    {
      case _T ('\\'):
        s += _T ('/');
        break;
      case _T (' '):
        s += _T ("%20");
        break;
      default:
        if (*p < 128)
          s += *p;
        else
        if (*p < 256)
        {
          s += _T ('%');
          s += HEXTAB[(*p & 0xff) >> 4];
          s += HEXTAB[(*p & 0x0f)];
        }
        else
        {
          // #f# problem - Unicodechar!
        }
    }
    ++p;
  }
  __appendHTML (s);
}

/*********************************************/
/**** here the exported functions start!! ****/
/*********************************************/

START_EXTERN_C

//--------------------------------------------------------------------
void GT_CALL out_html_init (Output_Settings *pSettings)
//--------------------------------------------------------------------
{
  // to avoid multiple allocations if out_html_init is called more than once!
  if (g_pWinConsole)
  {
    // reinit - maybe called to change the settings
    out_html_flush ();
  }
  else
  {
    // first time init
    g_pWinConsole = new WindowsConsole;

    // open default tags only once
    __display (_T ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n")
               _T ("<html>\n")
               _T ("<head>\n")
               _T ("<title>GT2 results</title>\n")
               _T ("<style type=\"text/css\">\n")
               _T ("<!--\n")
               _T ("body { color:#303030; font-family:monospace; }\n")
               _T ("table { border-collapse:collapse; }\n")
               _T ("table.border { border:1px solid; }\n")
               _T ("th { border:1px solid; }\n")
               _T ("th.left   { text-align:left; }\n")
               _T ("th.center { text-align:center; }\n")
               _T ("th.right  { text-align:right; }\n")
               _T ("td { border:1px solid; }\n")
               _T ("td.left   { text-align:left; }\n")
               _T ("td.center { text-align:center; }\n")
               _T ("td.right  { text-align:right; }\n")
               _T (".boxed { background:#e0e0e0; }\n")
               _T (".filehdr { padding:4px; margin-bottom:2px; }\n")
               _T (".file_bg0 { background:#f0c0c0; }\n")
               _T (".file_bg1 { background:#c0c0f0; }\n")
               _T (".filename { border:0px solid; /*background:#e0e0e0;*/ padding:2px; margin-right:2px; }\n")
               _T (".file_fg0 { color:#f03030; }\n")
               _T (".file_fg1 { color:#3030f0; }\n")
               _T ("-->\n")
               _T ("</style>\n")
               _T ("</head>\n")
               _T ("<body>\n"));

    // print current codepage to comment
    {
      TCHAR sBuf[128];
      _stprintf (sBuf, _T ("<!-- CP: %u -->\n"), ::GetACP ());
      __display (sBuf);
    }
  }

  if (pSettings->m_nFlagUsage & GT_OUTPUT_SETTINGS_USE_LISTMODE)
    g_bListMode = pSettings->m_nFlagValues & GT_OUTPUT_SETTINGS_USE_LISTMODE;

  if (pSettings->m_nFlagUsage & GT_OUTPUT_SETTINGS_USE_NOCOLOR)
    g_bColors = !(pSettings->m_nFlagValues & GT_OUTPUT_SETTINGS_USE_NOCOLOR);

  if (pSettings->m_nFlagUsage & GT_OUTPUT_SETTINGS_USE_FLUSH)
    g_bFlush = pSettings->m_nFlagValues & GT_OUTPUT_SETTINGS_USE_FLUSH;
}

//--------------------------------------------------------------------
void GT_CALL out_html_done ()
//--------------------------------------------------------------------
{
  // close default tags
  __display (_T ("</body></html>\n"));

  delete g_pWinConsole;
  g_pWinConsole = NULL;
}

//--------------------------------------------------------------------
void GT_CALL out_html_flush ()
//--------------------------------------------------------------------
{
  // execute all
  OutputMemberList::const_iterator cit = g_aOutList.begin ();
  for (; !(cit == g_aOutList.end ()); ++cit)
    __executeOutputMember (*cit);

  g_aOutList.clear ();
}

//--------------------------------------------------------------------
void GT_CALL out_html_setcolor (const EColor eColor)
//--------------------------------------------------------------------
{
  if (!g_bListMode)
    g_eColor = eColor;
}

//--------------------------------------------------------------------
void GT_CALL out_html_append (LPCTSTR s)
//--------------------------------------------------------------------
{
  size_t nRealLen;

  if (str_isresptr (s))
  {
    // it's an ID!
    LPTSTR pReal;
    nRealLen = GetStringResBuf ((resid_t) s, &pReal);
    __appendText (pReal, nRealLen, true);
  }
  else
  {
    // a string...
    __appendText (s, _tcslen (s), true);
  }
}

//--------------------------------------------------------------------
void GT_CDECL out_html_format (LPCTSTR pFmt, ...)
//--------------------------------------------------------------------
{
  const int BUFLEN = 8192;
  static TCHAR sSTATIC[BUFLEN];

  ASSERT (pFmt);

  // start with the old args!
  va_list args;
  va_start (args, pFmt);

  // resolve potential Resource ID
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
    FatalAppExit (0xdead, _T ("Internal buffer overflow in out_html_format"));
*/
  }
  else
  {
    __appendText (pTemp, nLen, true);
  }

  // set to NULL to avoid further usage
  va_end (args);

  // free mem if necessary
  if (nMemLen > BUFLEN)
    delete [] pTemp;
}

//--------------------------------------------------------------------
void GT_CALL out_html_filestart (Output_FilenameStruct *pData)
//--------------------------------------------------------------------
{
  ASSERT (pData);
  ASSERT (g_nIndent == 0);

  DoublePair aPos;
  const bool bOpenedFile = (pData->m_nTotalFileSize >= 0);
  g_nIndent = 0;

  static size_t nCounter = 0;
  if (pData->m_nScanOffset == 0)
    ++nCounter;

  __appendHTML (_T ("<div class=\"boxed"));
  if (g_bListMode)
    if (nCounter & 1)
      __appendHTML (_T (" filehdr file_bg0"));
    else
      __appendHTML (_T (" filehdr file_bg1"));
  __appendHTML (_T ("\">"));

  if (!g_bListMode)
  {
    __appendHTML (_T ("<div"));
    if (nCounter & 1)
      __appendHTML (_T (" class=\"filehdr file_bg0\">"));
    else
      __appendHTML (_T (" class=\"filehdr file_bg1\">"));
  }

  if (pData->m_nScanOffset == 0)
  {
    // for files which could not be opened but the name was identified
    if (nCounter & 1)
      __appendHTML (_T ("<span class=\"filename file_fg0\">"));
    else
      __appendHTML (_T ("<span class=\"filename file_fg1\">"));
    __appendHTML (_T ("<a href=\""));
    __appendHTMLFilename (pData->m_sFilename);
    __appendHTML (_T ("\">"));
    out_html_append (pData->m_sFilename);
    __appendHTML (_T ("</a></span>"));

    if (bOpenedFile && !g_bListMode)
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

      out_html_format (rc (HRC_APP_OUTPUT_HTML_FILESTART_LIST),
                       pData->m_nTotalFileSize,
                       sMode.c_str ());

      // get file attributes
      DWORD nAttributes = ::GetFileAttributes (pData->m_sFilename);
      if (nAttributes == (DWORD) -1)
      {
        out_html_setcolor (eColorERROR);
        out_html_format (rc (HRC_APP_OUTPUT_FILESTART_NO_ATTRS),
                         pData->m_sFilename);
      }
      else
      {
        if (nAttributes & FILE_ATTRIBUTE_COMPRESSED)
        {
          out_html_setcolor (eColorINTERESTING);
          out_html_append (rc (HRC_APP_OUTPUT_FILESTART_ATTRS_COMPRESSED));
        }

        if (nAttributes & FILE_ATTRIBUTE_OFFLINE)
        {
          out_html_setcolor (eColorINTERESTING);
          out_html_append (rc (HRC_APP_OUTPUT_FILESTART_ATTRS_OFFLINE));
        }
      }
    }
  }
  else
  {
    // listmode should disable overlay stuff automatically
    ASSERT (!g_bListMode);

    _splitdouble (double (pData->m_nScanOffset) / pData->m_nTotalFileSize * 100, aPos, 2);

    out_html_format (rc (HRC_APP_OUTPUT_HTML_FILESTART_NEXT_LEVEL),
                     pData->m_nScanOffset,
                     pData->m_nScanOffset,
                     pData->m_nTotalFileSize - pData->m_nScanOffset,
                     pData->m_nTotalFileSize - pData->m_nScanOffset,
                     aPos.h, aPos.l);
  }


  if (!g_bListMode)
    __appendHTML (_T ("</div>"));
}

//--------------------------------------------------------------------
void GT_CALL out_html_filedone ()
//--------------------------------------------------------------------
{
  ASSERT (g_nIndent == 0);

  // close the "boxed" div
  __appendHTML (_T ("</div>"));

  if (!g_bListMode)
  {
    // automatically append one newline
    __appendHTML (_T ("<br>"));
  }

  // don't do this
//  __appendHTML (_T ("\n"));

  out_html_flush ();
}

//--------------------------------------------------------------------
void GT_CALL out_html_fileend ()
//--------------------------------------------------------------------
{
  /* empty */
}

//--------------------------------------------------------------------
void GT_CALL out_html_status (LPCTSTR s)
//--------------------------------------------------------------------
{
  ASSERT (s);

  str_auto_resolve_resid (s);
  g_pWinConsole->SetTitle (s);
}

//--------------------------------------------------------------------
void GT_CALL out_html_table (Table* pTable)
//--------------------------------------------------------------------
{
  ASSERT (pTable);

  size_t c, r;
  pstring sCellSettings;
  pstring sCellStart;
  LPCTSTR pCellEnd;

  const size_t nCols          = pTable->GetColumnCount ();
  const size_t nRows          = pTable->GetRowCount ();
  TableColumnVector& aColumns = pTable->GetColumns ();

  // start a new table
  __appendHTML (_T ("<table cellspacing=\"0\">\n"));

  // create the headline
  if (pTable->IsHeadlineEnabled ())
  {
    __appendHTML (_T ("<thead>\n"));
    __appendHTML (_T ("<tr>"));
    for (c = 0; c < nCols; c++)
    {
      TableColumn* pColumn = aColumns[c];

      // switch concerning alignment
      if (pColumn->GetType () == STR)
      {
        if (pColumn->GetFormatWidth () == ALIGN_CENTER)
          __appendHTML (_T ("<th class=\"center\">"));
        else
        if (pColumn->GetFormatWidth () == ALIGN_RIGHT)
          __appendHTML (_T ("<th class=\"right\">"));
        else
          __appendHTML (_T ("<th class=\"left\">"));
      }
      else
      {
        // numbers have no alignment
        __appendHTML (_T ("<th>"));
      }
      out_html_append (pColumn->GetpName ());
      __appendHTML (_T ("</th>"));
    }
    __appendHTML (_T ("</tr>\n"));
    __appendHTML (_T ("</thead>\n"));
  }

  // open body
  __appendHTML (_T ("<tbody>\n"));

  // remember the row index for each column (for colspan)
  size_t *nRowIndex = new size_t[nCols];
  for (c = 0; c < nCols; ++c)
    nRowIndex[c] = 0;

  // loop over all rows
  for (r = 0; r < nRows; r++)
  {
    __appendHTML (_T ("<tr>"));

    // loop over all columns
    for (c = 0; c < nCols; c++)
    {
      TableColumn* pColumn = aColumns[c];
      TableCell* pCell = pColumn->GetCell (nRowIndex[c]);

      // next row index for this column
      nRowIndex[c]++;

      // build settings
      sCellSettings.erase ();

      // check colspan
      const size_t nColSpan = pCell->GetColSpan ();
      if (nColSpan > 1)
      {
        str_append_format (sCellSettings, _T (" colspan=\"%lu\""), nColSpan);
        c += nColSpan - 1;
      }

      // check color
      if (pCell->HasCustomColor ())
      {
        str_assign_format (sCellStart, _T ("<span style=\"color:#%s;\">"), __getColorString (pCell->GetColor ()));
        pCellEnd = _T ("</span>");
      }
      else
      {
        sCellStart.erase ();
        pCellEnd = _T ("");
      }

      // do the output depending on the type
      switch (pColumn->GetType ())
      {
        case NUM_ZERO:
        case NUM_SPACE:
        case HEX:
        {
          __appendHTML (_T ("<td class=\"right\""));
          __appendHTML (sCellSettings);
          __appendHTML (_T (">"));
          __appendHTML (sCellStart);

          // int32 or int64?
          switch (pCell->GetType ())
          {
            case CELL_INT32:
            {
              if (pColumn->IsAligned ())
              {
                if (pColumn->GetType () == NUM_SPACE)
                {
                  // already right aligned
                  out_html_format (_T ("%ld"),
                                   pCell->GetInt32 ());
                }
                else
                {
                  // some prefix required
                  out_html_format (pColumn->GetType () == NUM_ZERO
                                     ? _T ("%0*ld")
                                     : _T ("%0*Xh"),
                                   pColumn->GetFormatWidth (),
                                   pCell->GetInt32 ());
                }
              }
              else
              {
                // no alignment
                out_html_format (pColumn->GetType () == HEX
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
                if (pColumn->GetType () == NUM_SPACE)
                {
                  // already right aligned
                  out_html_format (_T ("%I64ld"),
                                   pCell->GetInt64 ());
                }
                else
                {
                  // some prefix required
                  out_html_format (pColumn->GetType () == NUM_ZERO
                                     ? _T ("%0*I64ld")
                                     : _T ("%0*I64Xh"),
                                   pColumn->GetFormatWidth (),
                                   pCell->GetInt64 ());
                }
              }
              else
              {
                out_html_format (pColumn->GetType () == HEX
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
          __appendHTML (_T ("<td class=\"right\""));
          __appendHTML (sCellSettings);
          __appendHTML (_T (">"));
          __appendHTML (sCellStart);

          const int nFormatWidth = pColumn->IsAligned ()
                                     ? pColumn->GetFormatWidth ()
                                     : DEFAULT_FORMATWIDTH_DOUBLE;

          DoublePair aDbl;
          _splitdouble (pCell->GetDouble (), aDbl, nFormatWidth);
          out_html_format (_T ("%d.%0*d"), aDbl.h, nFormatWidth, aDbl.l);
          break;
        }
        case STR:
        {
          // consider alignment!
          if (pColumn->GetFormatWidth () == ALIGN_CENTER)
            __appendHTML (_T ("<td class=\"center\""));
          else
          if (pColumn->GetFormatWidth () == ALIGN_RIGHT)
            __appendHTML (_T ("<td class=\"right\""));
          else
            __appendHTML (_T ("<td class=\"left\""));
          __appendHTML (sCellSettings);
          __appendHTML (_T (">"));
          __appendHTML (sCellStart);

          out_html_append (pCell->GetStr ()->c_str ());
          break;
        }
        default:
        {
          // nothing
          break;
        }
      }

      // align width of the column from formatwidth to width
      __appendHTML (pCellEnd);
      __appendHTML (_T ("</td>"));
    }

    // end of line
    __appendHTML (_T ("</tr>\n"));
  }


  // free memory
  delete [] nRowIndex;

  // close body and table
  __appendHTML (_T ("</tbody>\n</table>\n"));
}

//--------------------------------------------------------------------
void GT_CALL out_html_incindent (void)
//--------------------------------------------------------------------
{
  // no indentation in listmode
  if (!g_bListMode)
  {
    ++g_nIndent;

    // indentation is handled via tables
    __appendHTML (_T ("<table style=\"border:0; border-collapse:collapse; margin-left:20px;\">")
                  _T ("<tr>")
                  _T ("<td style=\"border:0; padding:0;\">\n"));
  }
}

//--------------------------------------------------------------------
void GT_CALL out_html_decindent (void)
//--------------------------------------------------------------------
{
  // no indentation in listmode
  if (!g_bListMode)
  {
    ASSERT (g_nIndent > 0);
    if (g_nIndent > 0)
    {
      --g_nIndent;
      __appendHTML (_T ("</td></tr></table>\n"));
    }
  }
}

END_EXTERN_C

//--------------------------------------------------------------------
OutputFuncPtrs GT_CALL GetHTMLOutputFuncPtrs ()
//--------------------------------------------------------------------
{
  static const OutputFuncPtrs ofp = {
    out_html_init,
    out_html_done,

    out_html_setcolor,
    out_html_append,
    out_html_format,
    out_html_table,

    out_html_incindent,
    out_html_decindent,

    out_html_filestart,
    out_html_filedone,
    out_html_fileend,

    out_html_flush,
    out_html_status,
  };
  return ofp;
}

}  // namespace
