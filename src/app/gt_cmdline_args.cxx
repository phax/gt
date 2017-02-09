namespace GT {

#define QUOTE_CHAR  _T ('"')
#define NUL_CHAR    _T ('\0')
#define SLASH_CHAR  _T ('\\')
#define SPACE_CHAR  _T (' ')
#define TAB_CHAR    _T ('\t')

//--------------------------------------------------------------------
static void GT_CALL __ParseCommandLine
                                        (LPCTSTR sCmdline,
                                         TCHAR **argv,
                                         TCHAR *args,
                                         int &numargs,
                                         int &numchars)
//--------------------------------------------------------------------
{
  LPTSTR s = (LPTSTR) sCmdline;
  int inquote, copychar, numslash;
  TCHAR c;

  numchars = 0;
  numargs = 1;

  // set filename as first argumen!
  if (argv)
    *argv++ = args;

  // 1. scan program name
  if (*s == QUOTE_CHAR)
  {
    while (*(++s) != QUOTE_CHAR && *s != NUL_CHAR)
    {
      // copy character
      ++numchars;
      if (args)
        *args++ = *s;
    }

    // trailing zero
    ++numchars;
    if (args)
      *args++ = NUL_CHAR;

    if (*s == QUOTE_CHAR)
      ++s;
  }
  else
  {
    // not a quoted name!
    do
    {
      ++numchars;
      if (args)
        *args++ = *s;

      c = (TCHAR) *s++;
    } while (c != SPACE_CHAR && c != NUL_CHAR && c != TAB_CHAR);

    if (c == NUL_CHAR)
    {
      --s;
    }
    else
    {
      // replace the space or tab with ZERO
      if (args)
        *(args - 1) = NUL_CHAR;
    }
  }

  // 2. scan all parameters
  inquote = 0;
  for (;;)
  {
    // skip all leading spaces and tabs
    if (*s)
    {
      while (*s == SPACE_CHAR || *s == TAB_CHAR)
        ++s;
    }

    // end of args??
    if (*s == NUL_CHAR)
      break;

    // next argument!
    if (argv)
      *argv++ = args;
    ++numargs;

    for (;;)
    {
      copychar = 1;
      numslash = 0;

      // get number of slashes!
      while (*s == SLASH_CHAR)
      {
        ++s;
        ++numslash;
      }

      // quote char??
      if (*s == QUOTE_CHAR)
      {
        if (numslash % 2 == 0)
        {
          if (inquote)
          {
            if (s[1] == QUOTE_CHAR)
            {
              // double quotes inside quoted string -> skip first and copy second
              ++s;
            }
            else
            {
              // end of args
              copychar = 0;
            }
          }
          else
          {
            // don't copy leading quote
            copychar = 0;
          }

          // change bool
          inquote = !inquote;
        }

        // divide number of slashes by 2
        numslash /= 2;
      }

      // copy slashes!
      while (numslash--)
      {
        if (args)
          *args++ = SLASH_CHAR;
        ++numchars;
      }

      // end of argument?
      if (*s == NUL_CHAR || (!inquote && (*s == SPACE_CHAR || *s == TAB_CHAR)))
        break;

      if (copychar)
      {
        if (args)
          *args++ = *s;
        ++numchars;
      }

      ++s;
    }

    // terminate string
    if (args)
      *args++ = _T ('\0');
    ++numchars;
  }

  // add a last argument: NULL ptr
  if (argv)
    *argv++ = _T ('\0');
  ++numargs;
}

//--------------------------------------------------------------------
void GT_CALL GetCommandLineArgs (int &argc, TCHAR **&argv)
//--------------------------------------------------------------------
{
  int numargs, numchars;
  LPTSTR pBuf;

  __ParseCommandLine (GetCommandLine (),
                      NULL,
                      NULL,
                      numargs,
                      numchars);

  // allocate memory for numargs pointer and numchars charcter
  pBuf = (LPTSTR) malloc (numargs * sizeof (TCHAR*) + numchars * sizeof (TCHAR));

  __ParseCommandLine (GetCommandLine (),
                      (TCHAR**) pBuf,
                      (TCHAR*) (((char*) pBuf) + numargs * sizeof (TCHAR*)),
                      numargs,
                      numchars);

  argc = numargs - 1;
  argv = (TCHAR**) pBuf;
}

}  // namespace
