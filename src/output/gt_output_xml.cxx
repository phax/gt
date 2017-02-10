#include "gt_output_xml.hxx"

#include "gt_common_output.hxx"
#include "gt_errormsg.hxx"
#include "gt_table.hxx"
#include "gt_win32_console.hxx"
#include "gt_utils.hxx"

namespace GT {

typedef std::list <pstring> stack_t;

// init global variables
static WindowsConsole*  g_pWinConsole = NULL;
static bool             g_bListMode = false;
static bool             g_bFlush = false;
static OutputMemberList g_aOutList;
static size_t           g_nFileLevel = 0;
static bool             g_bNextIsError = false;

START_EXTERN_C
void GT_CALL  out_xml_flush ();
END_EXTERN_C


//--------------------------------------------------------------------
static void __display (LPCTSTR p, size_t nLen = size_t (-1))
//--------------------------------------------------------------------
{
  g_pWinConsole->DisplayText (p, nLen == size_t (-1) ? _tcslen (p) : nLen);
}

//--------------------------------------------------------------------
static void __prepare (pstring &s)
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

        // replace any directly following '\n' with ' '!
        while (++n < s.length () && s.at (n) == _T ('\n'))
          s.at (n) = _T (' ');

        // search the next '\n'
        n = s.find_first_of (_T ('\n'));
        if (n != pstring::npos)
        {
          // alright, we found one! -> kill until end of string
          s.erase (s.begin () + n, s.end ());
        }
      }

      // manually append exactly one space!
      s += _T ('\n');
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
      __prepare (*pStr);
      __display (pStr->c_str (), pStr->length ());
      delete pStr;
      break;
    }
    case OUT_PLAIN:
    {
      pstring *pStr = aMember.AsString ();
      // no preparing here!
      __display (pStr->c_str (), pStr->length ());
      delete pStr;
      break;
    }
    default:
    {
      ShowErrorMessage (_T ("Invalid XML output type %d"),
                        aMember.GetType ());
    }
  }
}

//--------------------------------------------------------------------
static void __appendOutputMember
                                        (      LPCTSTR  s,
                                         const size_t   len,
                                         const EOutType eType)
//--------------------------------------------------------------------
{
  ASSERT (s);
  ASSERT (eType == OUT_STRING || eType == OUT_PLAIN);
  if (!s || !*s)
    return;
  ASSERT (len > 0);
  ASSERT (!IsBadStringPtr (s, len));

  // push text!
  pstring *pStr = NULL;
  if (!g_aOutList.empty () &&
      g_aOutList.back ().GetType () == eType)
  {
    // get existing string and append
    pStr = g_aOutList.back ().AsString ();
    pStr->append (s, len);
    _CheckForEndlessLoop (pStr);
  }
  else
  {
    // add a new member
    g_aOutList.push_back (OutputMember (s, eType));
    pStr = g_aOutList.back ().AsString ();
  }

  // always flush (cmdline switch /flushoutput)?
  if (g_bFlush)
    out_xml_flush ();
}

//--------------------------------------------------------------------
static void __appendPlainText (LPCTSTR s)
//--------------------------------------------------------------------
{
  __appendOutputMember (s, _tcslen (s), OUT_PLAIN);
}

//--------------------------------------------------------------------
static void __appendPlainText (const pstring& s)
//--------------------------------------------------------------------
{
  __appendOutputMember (s.c_str (), s.length (), OUT_PLAIN);
}

//--------------------------------------------------------------------
static void __appendFormattedText
                                        (const EOutType eType,
                                               LPCTSTR  pFmt,
                                               va_list  args)
//--------------------------------------------------------------------
{
  const int BUFLEN = 8192;
  static TCHAR sSTATIC[BUFLEN];

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
    FatalAppExit (0xdead, _T ("Internal buffer overflow in out_xml_format"));
*/
  }
  else
  {
    __appendOutputMember (pTemp, nLen, eType);
  }

  // set to NULL to avoid further usage
  va_end (args);

  // free mem if necessary
  if (nMemLen > BUFLEN)
    delete [] pTemp;
}

//--------------------------------------------------------------------
void GT_CDECL __appendFormattedPlainText
                                        (LPCTSTR pFmt, ...)
//--------------------------------------------------------------------
{
  ASSERT (pFmt);

  // start with the old args!
  va_list args;
  va_start (args, pFmt);

  __appendFormattedText (OUT_PLAIN, pFmt, args);
}

/*********************************************/
/**** here the exported functions start!! ****/
/*********************************************/

START_EXTERN_C

//--------------------------------------------------------------------
void GT_CALL out_xml_init (Output_Settings *pSettings)
//--------------------------------------------------------------------
{
  // to avoid multiple allocations if out_xml_init is called more than once!
  if (g_pWinConsole)
  {
    // reinit - maybe called to change the settings
    out_xml_flush ();
  }
  else
  {
    // first time init
    g_pWinConsole = new WindowsConsole;

    // open default tags
    __display (_T ("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n")
               _T ("<gt version=\"0.37\">\n"));
  }

  if (pSettings->m_nFlagUsage & GT_OUTPUT_SETTINGS_USE_LISTMODE)
    g_bListMode = pSettings->m_nFlagValues & GT_OUTPUT_SETTINGS_USE_LISTMODE;

  if (pSettings->m_nFlagUsage & GT_OUTPUT_SETTINGS_USE_FLUSH)
    g_bFlush = pSettings->m_nFlagValues & GT_OUTPUT_SETTINGS_USE_FLUSH;
}

//--------------------------------------------------------------------
void GT_CALL out_xml_done ()
//--------------------------------------------------------------------
{
  // close default tags
  __display (_T ("</gt>\n"));

  // delete console
  delete g_pWinConsole;
  g_pWinConsole = NULL;
}

//--------------------------------------------------------------------
void GT_CALL out_xml_flush ()
//--------------------------------------------------------------------
{
  // execute all output members
  OutputMemberList::const_iterator cit = g_aOutList.begin ();
  for (; !(cit == g_aOutList.end ()); ++cit)
    __executeOutputMember (*cit);

  g_aOutList.clear ();
}

//--------------------------------------------------------------------
void GT_CALL out_xml_setcolor (const EColor eColor)
//--------------------------------------------------------------------
{
  g_bNextIsError = (eColor == eColorERROR);
}

//--------------------------------------------------------------------
void GT_CALL out_xml_append (LPCTSTR s)
//--------------------------------------------------------------------
{
  size_t nRealLen;

  if (g_bNextIsError)
    __appendPlainText (_T ("<error>"));

  if (str_isresptr (s))
  {
    // it's an ID!
    LPTSTR pReal;
    nRealLen = GetStringResBuf ((resid_t) s, &pReal);
    __appendOutputMember (pReal, nRealLen, OUT_STRING);
  }
  else
  {
    // a string...
    __appendOutputMember (s, _tcslen (s), OUT_STRING);
  }

  if (g_bNextIsError)
  {
    __appendPlainText (_T ("</error>"));
    g_bNextIsError = false;
  }
}

//--------------------------------------------------------------------
void GT_CDECL out_xml_format (LPCTSTR pFmt, ...)
//--------------------------------------------------------------------
{
  ASSERT (pFmt);

  if (g_bNextIsError)
    __appendPlainText (_T ("<error>"));

  // start with the old args!
  va_list args;
  va_start (args, pFmt);

  __appendFormattedText (OUT_STRING, pFmt, args);


  if (g_bNextIsError)
  {
    __appendPlainText (_T ("</error>"));
    g_bNextIsError = false;
  }
}

//--------------------------------------------------------------------
void GT_CALL out_xml_filestart (Output_FilenameStruct *pData)
//--------------------------------------------------------------------
{
  ASSERT (pData);

  const bool bOpenedFile = (pData->m_nTotalFileSize >= 0);

  if (pData->m_nScanOffset == 0)
  {
    ASSERT (g_nFileLevel == 0);
    g_nFileLevel = 1;

    // new file
    __appendFormattedPlainText (_T ("<file name=\"%s\" open=\"%d\">\n"),
                                pData->m_sFilename,
                                bOpenedFile ? 1 : 0);

    // not possible e.g. on c:\pagefile.sys
    if (bOpenedFile)
    {
      // print size and type
      __appendFormattedPlainText (_T ("<meta size=\"%I64d\" type=\"%s\"/>\n"),
                                  pData->m_nTotalFileSize,
                                  pData->m_nFlags & GT_OUTPUT_FILENAME_DOSTEXT
                                    ? _T ("TEXT_DOS")
                                    : pData->m_nFlags & GT_OUTPUT_FILENAME_UNIXTEXT
                                      ? _T ("TEXT_UNIX")
                                      : pData->m_nFlags & GT_OUTPUT_FILENAME_MACTEXT
                                        ? _T ("TEXT_MAC")
                                        : _T ("BIN"));

      // check for special attributes
      DWORD nAttributes = ::GetFileAttributes (pData->m_sFilename);
      if (nAttributes != (DWORD) -1)
      {
        __appendFormattedPlainText (_T ("<attr compr=\"%d\" offline=\"%d\"/>\n"),
                                    nAttributes & FILE_ATTRIBUTE_COMPRESSED ? 1 : 0,
                                    nAttributes & FILE_ATTRIBUTE_OFFLINE ? 1 : 0);
      }
    }
  }
  else
  {
    // listmode should disable overlay stuff automatically
    ASSERT (!g_bListMode);
    ASSERT (g_nFileLevel > 0);
    ++g_nFileLevel;

    __appendFormattedPlainText (_T ("<level offset=\"%I64d\">\n"), pData->m_nScanOffset);
  }
}

//--------------------------------------------------------------------
void GT_CALL out_xml_filedone ()
//--------------------------------------------------------------------
{
  // close any previously opened level
  if (g_nFileLevel > 1)
    __appendFormattedPlainText (_T ("</level>\n"));

  out_xml_flush ();
}

//--------------------------------------------------------------------
void GT_CALL out_xml_fileend ()
//--------------------------------------------------------------------
{
  // the level may be decremented here!
  ASSERT (g_nFileLevel > 0);
  --g_nFileLevel;

  if (g_nFileLevel == 0)
    __appendFormattedPlainText (_T ("</file>\n"));
}

//--------------------------------------------------------------------
void GT_CALL out_xml_status (LPCTSTR s)
//--------------------------------------------------------------------
{
  ASSERT (s);

  str_auto_resolve_resid (s);
  g_pWinConsole->SetTitle (s);
}

//--------------------------------------------------------------------
void GT_CALL out_xml_table (Table* pTable)
//--------------------------------------------------------------------
{
  ASSERT (pTable);

  size_t c, r;
  pstring sCellSettings;

  const size_t nCols          = pTable->GetColumnCount ();
  const size_t nRows          = pTable->GetRowCount ();
  TableColumnVector& aColumns = pTable->GetColumns ();

  // start a new table
  __appendPlainText (_T ("<table>\n"));

  // create the headline
  if (pTable->IsHeadlineEnabled ())
  {
    __appendPlainText (_T ("<header>\n"));
    for (c = 0; c < nCols; c++)
    {
      TableColumn* pColumn = aColumns[c];

      __appendPlainText (_T ("<column>"));
      out_xml_append (pColumn->GetpName ());
      __appendPlainText (_T ("</column>\n"));
    }
    __appendPlainText (_T ("</header>\n"));
  }

  // remember the row index for each column (for colspan)
  size_t *nRowIndex = new size_t[nCols];
  for (c = 0; c < nCols; ++c)
    nRowIndex[c] = 0;

  // loop over all rows
  for (r = 0; r < nRows; r++)
  {
    __appendPlainText (_T ("<row>"));

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

      // ignore color!!

      // do the output depending on the type
      switch (pColumn->GetType ())
      {
        case NUM_ZERO:
        case NUM_SPACE:
        case HEX:
        {
          __appendPlainText (_T ("<cell"));
          __appendPlainText (sCellSettings);
          __appendPlainText (_T (">"));

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
                  out_xml_format (_T ("%ld"),
                                  pCell->GetInt32 ());
                }
                else
                {
                  // some prefix required
                  out_xml_format (pColumn->GetType () == NUM_ZERO
                                    ? _T ("%0*ld")
                                    : _T ("%0*Xh"),
                                  pColumn->GetFormatWidth (),
                                  pCell->GetInt32 ());
                }
              }
              else
              {
                // no alignment
                out_xml_format (pColumn->GetType () == HEX
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
                  out_xml_format (_T ("%I64ld"),
                                  pCell->GetInt64 ());
                }
                else
                {
                  // some prefix required
                  out_xml_format (pColumn->GetType () == NUM_ZERO
                                    ? _T ("%0*I64ld")
                                    : _T ("%0*I64Xh"),
                                  pColumn->GetFormatWidth (),
                                  pCell->GetInt64 ());
                }
              }
              else
              {
                out_xml_format (pColumn->GetType () == HEX
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
          __appendPlainText (_T ("<cell"));
          __appendPlainText (sCellSettings);
          __appendPlainText (_T (">"));

          const int nFormatWidth = pColumn->IsAligned ()
                                     ? pColumn->GetFormatWidth ()
                                     : DEFAULT_FORMATWIDTH_DOUBLE;

          DoublePair aDbl;
          _splitdouble (pCell->GetDouble (), aDbl, nFormatWidth);
          out_xml_format (_T ("%d.%0*d"), aDbl.h, nFormatWidth, aDbl.l);
          break;
        }
        case STR:
        {
          __appendPlainText (_T ("<cell"));
          __appendPlainText (sCellSettings);
          __appendPlainText (_T (">"));

          out_xml_append (pCell->GetStr ()->c_str ());
          break;
        }
        default:
        {
          // nothing
          break;
        }
      }

      // align width of the column from formatwidth to width
      __appendPlainText (_T ("</cell>\n"));
    }

    // end of line
    __appendPlainText (_T ("</row>\n"));
  }

  // free memory
  delete [] nRowIndex;

  // close table
  __appendPlainText (_T ("</table>\n"));
}

//--------------------------------------------------------------------
void GT_CALL out_xml_incindent (void)
//--------------------------------------------------------------------
{
  /* empty */
}

//--------------------------------------------------------------------
void GT_CALL out_xml_decindent (void)
//--------------------------------------------------------------------
{
  /* empty */
}

END_EXTERN_C

//--------------------------------------------------------------------
OutputFuncPtrs GT_CALL GetXMLOutputFuncPtrs ()
//--------------------------------------------------------------------
{
  static const OutputFuncPtrs ofp = {
    out_xml_init,
    out_xml_done,

    out_xml_setcolor,
    out_xml_append,
    out_xml_format,
    out_xml_table,

    out_xml_incindent,
    out_xml_decindent,

    out_xml_filestart,
    out_xml_filedone,
    out_xml_fileend,

    out_xml_flush,
    out_xml_status,
  };
  return ofp;
}

}  // namespace
