MAKE_ANALYZER (Source);

namespace Source {

//--------------------------------------------------------------------
enum ESourceType
//--------------------------------------------------------------------
{
  SOURCE_NONE,
  SOURCE_PROGRAM,
  SOURCE_LIBRARY,
  SOURCE_UNIT,
  SOURCE_PROCEDURE,
  SOURCE_FUNCTION,
  SOURCE_PASCAL,
  SOURCE_MODULE,
  SOURCE_MODULA_DEF,
  SOURCE_MODULA_IMP,
  SOURCE_PROLOG,
  SOURCE_C,
  SOURCE_CPP,
  SOURCE_JAVA,
  SOURCE_JSCRIPT,
  SOURCE_ADL,
  SOURCE_PERL_PACKAGE,
};

static pstring     g_sName;
static ESourceType g_eType = SOURCE_NONE;

//--------------------------------------------------------------------
static file_t _GetPascalStartPos (FileBuffer* pBuffer)
//--------------------------------------------------------------------
{
  file_t nPos = 0;
  bool bScanNext = true;
  gtuint8 b, b2;
  const file_t nBufSize = pBuffer->GetFileSize () <= 4096
                            ? pBuffer->GetFileSize ()
                            : 4096;

  do
  {
    b = pBuffer->GetUint8 (nPos);
    if (++nPos >= nBufSize)
      return GT_NOT_FOUND;

    switch (b)
    {
      case '{':
      {
        // search for "{ ... }"
        nPos = pBuffer->Find ('}', nPos);
        if (nPos != GT_NOT_FOUND)
        {
          nPos += 1 + 1;     // 1 for trailing } and one for next GetUint8
        }
        break;
      }
      case '(':
      {
        // search for "(* ... *)"
        b2 = pBuffer->GetUint8 (nPos);
        ++nPos;

        if (b2 == '*')
        {
          // yep, we found a "(*" -> search for the next '*'
          nPos = pBuffer->Find ('*', nPos);
          if (nPos == GT_NOT_FOUND)
            bScanNext = false;
          while (nPos != GT_NOT_FOUND)
          {
            // yes we found a '*' -> check if the next char ís a ')'
            if (pBuffer->GetUint8 (nPos + 1) == ')')
            {
              nPos += 1 + 1;  // +1 for ')' and +1 for the next char
              bScanNext = true;
              break;
            }

            // nop, the next char is not a ')' -> search for the next '*'
            nPos = pBuffer->Find ('*', nPos + 1);
          }
        }
        else
          bScanNext = false;

        break;
      }
      default:
      {
        bScanNext = isspace (b);
        break;
      }
    }

    if (nPos >= nBufSize)
      return GT_NOT_FOUND;
  } while (bScanNext && nPos != GT_NOT_FOUND);  // GT_NOT_FOUND == -1

  return (nPos == GT_NOT_FOUND)
           ? nPos
           : nPos - 1;
}

//--------------------------------------------------------------------
static file_t _GetCStartPos (FileBuffer* pBuffer)
//--------------------------------------------------------------------
{
  file_t nPos = 0;
  bool bScanNext = true;
  gtuint8 b, b2;

  const file_t nBufSize = pBuffer->GetFileSize () <= 4096
                            ? pBuffer->GetFileSize ()
                            : 4096;

  do
  {
    b = pBuffer->GetUint8 (nPos);

    if (++nPos >= nBufSize)
      return GT_NOT_FOUND;

    if (b == '/')
    {
      b2 = pBuffer->GetUint8 (nPos);
      if (++nPos >= nBufSize)
        return GT_NOT_FOUND;

      switch (b2)
      {
        case '/':
        {
          // skip "//" until end of line
          nPos = pBuffer->Find ('\n', nPos);

          if (nPos == GT_NOT_FOUND)
            return GT_NOT_FOUND;

          break;
        }
        case '*':
        {
          // skip until next "*/"
          nPos = pBuffer->Find ('*', nPos + 1);
          while (nPos != GT_NOT_FOUND)
          {
            if (pBuffer->GetUint8 (nPos + 1) == '/')
            {
              nPos += 2;  // +1 for '/' and +1 for the next char
              bScanNext = true;
              break;
            }
            nPos = pBuffer->Find ('*', nPos + 1);
          }

          if (nPos == GT_NOT_FOUND)
            return GT_NOT_FOUND;

          if (nPos >= nBufSize)
            return nPos - 1;

          break;
        }
        default:
        {
          bScanNext = false;
          break;
        }
      }

    }
    else
    {
      bScanNext = isspace (b);
    }
  } while (bScanNext && nPos != GT_NOT_FOUND);

  return (nPos == GT_NOT_FOUND)
            ? nPos
            : nPos - 1;
}

// experimental - not in use
//--------------------------------------------------------------------
static file_t _GetShellStartPos (FileBuffer* pBuffer)
//--------------------------------------------------------------------
{
  file_t nPos = 0;
  int bScanNext = true;
  gtuint8 b;

  const file_t nBufSize = pBuffer->GetFileSize () <= 4096
                            ? pBuffer->GetFileSize ()
                            : 4096;

  do
  {
    b = pBuffer->GetUint8 (nPos);
    if (++nPos >= nBufSize)
      return GT_NOT_FOUND;

    if (b == '#')
    {
      // skip "#" until end of line
      nPos = pBuffer->Find ('\n', nPos);
      if (nPos == GT_NOT_FOUND)
        return GT_NOT_FOUND;
    }
    else
    {
      bScanNext = isspace (b);
    }
  } while (bScanNext && nPos != GT_NOT_FOUND);

  return (nPos == GT_NOT_FOUND)
            ? nPos
            : nPos - 1;
}

//--------------------------------------------------------------------
static bool _ReadAString
                                        (      FileBuffer* pBuffer,
                                         const file_t      nStartPos,
                                         const char        cEnd)
//--------------------------------------------------------------------
{
  const file_t MAX_SIZE = pBuffer->GetFileSize () <= 4096
                            ? pBuffer->GetFileSize ()
                            : 4096;
  file_t nPos = nStartPos;
  gtuint8 b;

  // skip space chars....
  while (nPos < MAX_SIZE)
  {
    b = pBuffer->GetUint8 (nPos++);
    if (isspace (b))
      break;
  }

  const file_t nNameStartPos = nPos;

  // get name string (alpha numeric, underscore and dot are allowed)
  while (nPos < MAX_SIZE)
  {
    b = pBuffer->GetUint8 (nPos++);
    if (b == 0 || b == cEnd)
      break;
    if (!isalnum (b) && b != '_' && b != '.')
      break;
  }

  const size_t nLen = size_t (nPos - nNameStartPos - 1);
  return pBuffer->GetFixedSizeAString (nNameStartPos, g_sName, nLen);
}

//--------------------------------------------------------------------
struct SourceType
//--------------------------------------------------------------------
{
  file_t  m_nStartPos;
  pstring m_sKeyword;

  void Init (file_t (*pCallback) (FileBuffer*), FileBuffer* pBuffer)
  {
    m_nStartPos = (*pCallback) (pBuffer);
    if (m_nStartPos != GT_NOT_FOUND)
    {
      /* the following call can fail for:
         Pascal: E:\__All Backups\_backup_ibmOld\Progs\_copy\MySourcecode\C++\adig_adoscript_dll\05 rtf2html\tuorfa\rtf2htm\test\fontsize.rtf
         C:      C:\cygwin\usr\src\gcc-3.3.1-3\gcc\config\ia64\ia64-modes.def

         The length of 14 is used because the longest keyword
           "implementation"
         has exactly 14 chars
       */
      if (!pBuffer->GetFixedSizeAString (m_nStartPos, m_sKeyword, 14))
      {
        m_sKeyword.clear ();
      }
    }
  }
};

//--------------------------------------------------------------------
static bool _Init
                                        (FileBuffer* pBuffer)
//--------------------------------------------------------------------
{
  SourceType aPascal, aC, aShell;

  // find startup positions
  aPascal.Init (_GetPascalStartPos, pBuffer);
  aC.Init (_GetCStartPos, pBuffer);
  aShell.Init (_GetShellStartPos, pBuffer);

  // reset global vars
  g_eType = SOURCE_NONE;
  g_sName.clear ();

  // bitfield
  enum
  {
    KW_PAS      = 0x01,  //!< use pascal style comments
    KW_C        = 0x02,  //!< use C/C++ style comments
    KW_SHELL    = 0x04,  //!< use shell style comments
    KW_NAME     = 0x08,  //!< read an identifier after the keyword
    KW_SECONDKW = 0x10,  //!< second keyword required
    KW_FILEEXT  = 0x20,  //!< only if file extension matches
  };

  struct SourceGetter
  {
    LPCTSTR     pKeyword;        //!< main keyword
    LPCTSTR     pSecondKeyword;  //!< next keyword that needs to follow
    gtuint32    nFlags;          //!< any combination of the above bitfield for detection
    ESourceType eType;           //!< what kind of source is it?
  };

  const SourceGetter SRCGET [] = {
  { _T ("program"),        NULL,               KW_PAS | KW_NAME,      SOURCE_PROGRAM },
  { _T ("library"),        NULL,               KW_PAS | KW_NAME,      SOURCE_LIBRARY },
  { _T ("unit"),           NULL,               KW_PAS | KW_NAME,      SOURCE_UNIT },
  { _T ("procedure"),      NULL,               KW_PAS | KW_NAME,      SOURCE_PROCEDURE },
  { _T ("function"),       NULL,               KW_PAS | KW_NAME,      SOURCE_FUNCTION },
// const can also be C++
//  { _T ("const"),          NULL,               KW_PAS | KW_NAME,      SOURCE_PASCAL },
  { _T ("type"),           NULL,               KW_PAS | KW_NAME,      SOURCE_PASCAL },
  { _T ("uses"),           NULL,               KW_PAS | KW_NAME,      SOURCE_PASCAL },
  { _T ("var"),            NULL,               KW_PAS | KW_NAME,      SOURCE_PASCAL },
  { _T ("module"),         NULL,               KW_PAS,                SOURCE_MODULE },
  { _T ("definition"),     _T ("module"),      KW_PAS | KW_SECONDKW,  SOURCE_MODULA_DEF },
  { _T ("implementation"), _T ("module"),      KW_PAS | KW_SECONDKW,  SOURCE_MODULA_IMP },
  { _T ("predicates"),     NULL,               KW_C,                  SOURCE_PROLOG },
  { _T ("domains"),        NULL,               KW_C,                  SOURCE_PROLOG },
  { _T ("#define"),        NULL,               KW_C,                  SOURCE_C },
  { _T ("# define"),       NULL,               KW_C,                  SOURCE_C },
  { _T ("#ifdef"),         NULL,               KW_C,                  SOURCE_C },
  { _T ("# ifdef"),        NULL,               KW_C,                  SOURCE_C },
  { _T ("#ifndef"),        NULL,               KW_C,                  SOURCE_C },
  { _T ("# ifndef"),       NULL,               KW_C,                  SOURCE_C },
  { _T ("#if"),            NULL,               KW_C,                  SOURCE_C },
  { _T ("# if"),           NULL,               KW_C,                  SOURCE_C },
  { _T ("#include"),       NULL,               KW_C,                  SOURCE_C },
  { _T ("# include"),      NULL,               KW_C,                  SOURCE_C },
  { _T ("#line"),          NULL,               KW_C,                  SOURCE_C },
  { _T ("#pragma"),        NULL,               KW_C,                  SOURCE_C },
  { _T ("__declspec"),     NULL,               KW_C,                  SOURCE_C },
  { _T ("class"),          NULL,               KW_C | KW_NAME,        SOURCE_C },
  { _T ("enum"),           NULL,               KW_C | KW_NAME,        SOURCE_C },
  { _T ("extern"),         NULL,               KW_C | KW_NAME,        SOURCE_C },
  { _T ("inline"),         NULL,               KW_C | KW_NAME,        SOURCE_C },
  { _T ("static"),         NULL,               KW_C | KW_NAME,        SOURCE_C },
  { _T ("struct"),         NULL,               KW_C | KW_NAME,        SOURCE_C },
  { _T ("typedef"),        NULL,               KW_C | KW_NAME,        SOURCE_C },
  { _T ("namespace"),      NULL,               KW_C | KW_NAME,        SOURCE_CPP },
  { _T ("import"),         NULL,               KW_C,                  SOURCE_JAVA },
  { _T ("package"),        _T ("java"),        KW_C | KW_FILEEXT,     SOURCE_JAVA },
  { _T ("package"),        _T ("pm"),          KW_SHELL | KW_FILEEXT, SOURCE_PERL_PACKAGE },
  { _T ("package"),        _T ("al"),          KW_SHELL | KW_FILEEXT, SOURCE_PERL_PACKAGE },
  { _T ("private"),        NULL,               KW_C,                  SOURCE_JAVA },
  { _T ("protected"),      NULL,               KW_C,                  SOURCE_JAVA },
  { _T ("public"),         NULL,               KW_C,                  SOURCE_JAVA },
  { _T ("var"),            NULL,               KW_C,                  SOURCE_JSCRIPT },
  { _T ("function"),       NULL,               KW_C,                  SOURCE_JSCRIPT },
  { _T ("working"),        _T ("environment"), KW_C | KW_SECONDKW,    SOURCE_ADL },
  { _T ("business"),       _T ("process"),     KW_C | KW_SECONDKW,    SOURCE_ADL },
  };

  size_t nKeywordLen;
  const SourceGetter* pSG = NULL;
  SourceType* pST = NULL;
  DirEntry aDE (pBuffer->GetFileName ());
  for (size_t i = 0; i < GT_ARRAY_SIZE (SRCGET); ++i)
  {
    pSG = &SRCGET[i];
    pST = pSG->nFlags & KW_PAS   ? &aPascal :
          pSG->nFlags & KW_C     ? &aC :
          pSG->nFlags & KW_SHELL ? &aShell : NULL;
    ASSERT (pSG && pST);

    // flags may only be used either or
    ASSERT (!((pSG->nFlags & KW_SECONDKW) && (pSG->nFlags & KW_FILEEXT)));

    // if the second keyword is set, one of the falgs needs to be set
    ASSERT (!pSG->pSecondKeyword || (pSG->nFlags & KW_SECONDKW) || (pSG->nFlags & KW_FILEEXT));

    // read keyword from file.
    // must match the first keyword
    nKeywordLen = _tcslen (pSG->pKeyword);
    if (nKeywordLen <= pST->m_sKeyword.length () &&
        _tcsnicmp (pST->m_sKeyword.c_str (), pSG->pKeyword, nKeywordLen) == 0 &&
        _ReadAString (pBuffer,
                      pST->m_nStartPos + nKeywordLen,
                      pSG->pSecondKeyword   // if 2nd keyword present -> must be a ' '
                        ? ' '
                        : ';'))
    {
      // the read keyword is stored in g_sName
      // check for second keyword!
      if ((!(pSG->nFlags & KW_SECONDKW) || _tcsicmp (g_sName.c_str (), pSG->pSecondKeyword) == 0) &&
          (!(pSG->nFlags & KW_FILEEXT)  || aDE.HasFileNameExtension (pSG->pSecondKeyword)))
      {
        // if KW_NAME is set, g_sName may not be empty!
        if (!(pSG->nFlags & KW_NAME) || !g_sName.empty ())
        {
          // if we don't want the name, makr sure there is no name!
          if (!(pSG->nFlags & KW_NAME))
            g_sName.clear ();

          g_eType = pSG->eType;
          break;
        }
      }
    }
  }

  // for the entries with a second keyword, we have to find the name!
  if (g_eType == SOURCE_MODULA_DEF ||
      g_eType == SOURCE_MODULA_IMP)
  {
    ASSERT (pSG && pSG->pSecondKeyword);

    // read the next string after the second keyword
    const file_t nSecondKeywordPos = pBuffer->Find ((char) pSG->pSecondKeyword[0], pST->m_nStartPos + _tcslen (pSG->pKeyword));
    _ReadAString (pBuffer, nSecondKeywordPos + _tcslen (pSG->pSecondKeyword), ';');
  }
  else if (g_eType == SOURCE_ADL)
  {
    // read string between '<' and '>'
    const file_t p1 = pBuffer->Find ('<', pST->m_nStartPos);
    if (p1 != GT_NOT_FOUND)
    {
      const file_t p2 = pBuffer->Find ('>', p1);
      if (p2 != GT_NOT_FOUND)
      {
        const size_t len = size_t (p2 - p1 - 1);
        pBuffer->GetFixedSizeAString (p1 + 1, g_sName, len);
      }
    }
  }

  return g_eType != SOURCE_NONE;
}

}  // namespace Source

/*! Textfiles only
 */
//--------------------------------------------------------------------
MAKE_IS_A (Source)
//--------------------------------------------------------------------
{
  Source::g_eType = Source::SOURCE_NONE;
  Source::g_sName.clear ();

  // must be a text file
  if (!aGI.m_pBuffer->IsText ())
    return false;

  // determine source type
  return Source::_Init (aGI.m_pBuffer);
}

//--------------------------------------------------------------------
MAKE_DISPLAY (Source)
//--------------------------------------------------------------------
{
  switch (Source::g_eType)
  {
    case Source::SOURCE_PROGRAM:      out_append (rc (HRC_GENLIB_SOURCE_PASCAL_PROGRAM)); break;
    case Source::SOURCE_LIBRARY:      out_append (rc (HRC_GENLIB_SOURCE_PASCAL_LIBRARY)); break;
    case Source::SOURCE_UNIT:         out_append (rc (HRC_GENLIB_SOURCE_PASCAL_UNIT)); break;
    case Source::SOURCE_PROCEDURE:    out_append (rc (HRC_GENLIB_SOURCE_PASCAL_PROC)); break;
    case Source::SOURCE_FUNCTION:     out_append (rc (HRC_GENLIB_SOURCE_PASCAL_FUNC)); break;
    case Source::SOURCE_PASCAL:       out_append (rc (HRC_GENLIB_SOURCE_PASCAL_SOURCE )); break;
    case Source::SOURCE_MODULE:       out_append (rc (HRC_GENLIB_SOURCE_MODULA_PROGRAM)); break;
    case Source::SOURCE_MODULA_DEF:   out_append (rc (HRC_GENLIB_SOURCE_MODULA_DEFFILE)); break;
    case Source::SOURCE_MODULA_IMP:   out_append (rc (HRC_GENLIB_SOURCE_MODULA_IMPFILE)); break;
    case Source::SOURCE_PROLOG:       out_append (rc (HRC_GENLIB_SOURCE_PROLOG)); break;
    case Source::SOURCE_C:            out_append (rc (HRC_GENLIB_SOURCE_C)); break;
    case Source::SOURCE_CPP:          out_append (rc (HRC_GENLIB_SOURCE_CPP)); break;
    case Source::SOURCE_JAVA:         out_append (rc (HRC_GENLIB_SOURCE_JAVA)); break;
    case Source::SOURCE_JSCRIPT:      out_append (rc (HRC_GENLIB_SOURCE_JAVASCRIPT)); break;
    case Source::SOURCE_ADL:          out_append (rc (HRC_GENLIB_SOURCE_ADONIS)); break;
    case Source::SOURCE_PERL_PACKAGE: out_append (rc (HRC_GENLIB_SOURCE_PERL)); break;
    default:
      InternalError (m_pBuffer->GetpFileName (), _T (__FILE__), __LINE__);
      break;
  }

  // append name (if any)
  if (!Source::g_sName.empty ())
  {
    out_format (rc (HRC_GENLIB_SOURCE_IDENTIFIER),
                Source::g_sName.c_str ());
  }

  out_append (_T ("\n"));
}
