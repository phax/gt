/*
 * Compile with:
 *   cl filext_reader.cxx -nologo -EHsc -Zi -MDd
 */


#define WIN32

#include "w3c.cpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

typedef vector <string> string_list;

static string _trim (string s)
{
  while (!s.empty () && isspace ((int) (unsigned char) *(s.begin ())))
    s.erase (0, 1);
  while (!s.empty () && isspace ((int) (unsigned char) *(s.rbegin ())))
    s.erase (s.length () - 1);
  return s;
}

static string _masked (const string& s)
{
  string r;
  const char *p = s.c_str ();
  while (*p)
  {
    if (*p == '\\' || *p == '"')
      r += '\\';
    r += *p++;
  }
  return r;
}

static void str_split (const string& sContent,
                       string_list &aList,
                       const char cSplitChar)
{
  size_t nPos = 0, nFind;
  while ((nFind = sContent.find (cSplitChar, nPos)) != string::npos)
  {
    aList.push_back (sContent.substr (nPos, nFind - nPos));
    nPos = nFind + 1;
  }
  aList.push_back (sContent.substr (nPos));
}

static void read_from_url (string &result)
{
  result.erase ();

  W3Client w3;
  if (w3.Connect ("filext.com", 80))
  {
    if(w3.Request("/alphalist.php?extstart=."))
    {
      char buf[1025]="\0";
      unsigned long nread, ntotal = 0;

      while((nread=w3.Response(reinterpret_cast<unsigned char *>(buf), 1024)))
      {
        buf[nread]='\0';
        result.append (buf);

#ifdef _DEBUG
        ntotal += nread;
        cout << ntotal << ' ';
#endif
      }
    }
    w3.Close();
  }
}

class ext_map_t : public map <const string, string_list*>
{
private:
  unsigned long m_nLength;
public:
  ext_map_t ()
    : m_nLength (0)
  {}

  ~ext_map_t ()
  {
    const_iterator cit = begin (), cite = end ();
    for (; cit != cite; ++cit)
      delete (*cit).second;
  }

  void add (const string& sExt, const string& sDesc)
  {
    ++m_nLength;
    iterator it = find (sExt);
    string_list *pList;
    if (it == end ())
    {
      pList = new string_list;
      insert (make_pair (sExt, pList));
    }
    else
      pList = (*it).second;
    pList->push_back (sDesc);
  }

  unsigned long getLength ()
  {
    return m_nLength;
  }
};


int main (int argc, char** argv)
{
  if (argc > 1 && !strcmp (argv[1], "-debug+"))
    DebugBreak ();

  const char* pSourceHTMLFile = "filext.htm";
  const char* pSourceCPPFile = "gtc_fixedext_filext.cxx";

  // get content
  string sContent;
  {
    FILE *f = fopen (pSourceHTMLFile, "rb");
    if (!f)
    {
      cout << "Fetching new data from filext.com\n";
      read_from_url (sContent);

      // save to file
      f = fopen (pSourceHTMLFile, "wb");
      if (!f)
        cerr << "Failed to write to file\n";
      else
      {
        fwrite (sContent.c_str (), sContent.length (), 1, f);
        fclose (f);
      }
    }
    else
    {
      cout << "Reusing data from " << pSourceHTMLFile << "\n";
      char buf[1025];
      size_t nRead;
      while ((nRead = fread (buf, 1, 1024, f)) > 0)
      {
        buf[nRead] = '\0';
        sContent += buf;
      }
      fclose (f);
    }
  }

  // parse content
  ext_map_t aExtMap;
  {
    string_list aList;
    str_split (sContent, aList, '\n');

    string_list::const_iterator cit = aList.begin (), cite = aList.end ();
    // FILE *f = fopen ("_t", "wb"); for (; cit != cite; ++cit) fwrite ((*cit).c_str (), (*cit).length (), 1, f); fclose (f);

    const char* sSearch1 = "filext.com/file-extension.php?extension=";
    const char* sSearch2 = "filext.com/file-extension/";
    const char* pFound;
    string sExt, sDesc;
    int nSkipLength;
    for (; cit != cite; ++cit)
    {
      const string& sLine = *cit;

      // Search one of the two possible strings
      pFound = strstr (sLine.c_str (), sSearch1);
      if (pFound != NULL)
        nSkipLength = strlen (sSearch1);
      else
      {
        pFound = strstr (sLine.c_str (), sSearch2);
        if (pFound != NULL)
          nSkipLength = strlen (sSearch2);
      }

      if (pFound != NULL)
      {
        sExt.erase ();
        sDesc.erase ();

        // skip search tag
        pFound += nSkipLength;

        // goto begin of extension text
        while (*pFound != '>')
          ++pFound;
        ++pFound;

        // copy all extension chars
        while (*pFound != '<')
          sExt += *pFound++;
        sExt = _trim (sExt);

        // next line
        ++cit;

        // goto description
        pFound = (*cit).c_str ();
        while (*pFound != '>')
          ++pFound;
        ++pFound;

        // get desc
        while (*pFound != '<')
          sDesc += *pFound++;
        sDesc = _trim (sDesc);

        // save
        if (sExt.length () > 0)
          aExtMap.add (sExt, sDesc);
      }
    }
  }

  // write result
  cout << "Creating result file\n";
  {

    FILE *f = fopen (pSourceCPPFile, "wt");
    fputs ("//--------------------------------------------------------------------\n", f);
    fprintf (f, "const FixedExtensionEntry aFilextDotCom [%lu] = {\n", aExtMap.getLength ());
    fputs ("//--------------------------------------------------------------------\n", f);

    string sDesc;
    ext_map_t::const_iterator cit = aExtMap.begin (), cite = aExtMap.end ();
    for (; cit != cite; ++cit)
    {
      const string& sExt = _masked ((*cit).first);
      string_list* pList = (*cit).second;
      string_list::const_iterator cit2 = pList->begin (), cit2e = pList->end ();
      for (; cit2 != cit2e; ++cit2)
      {
        sDesc = _masked (*cit2);
        fprintf (f, "{ _T (\"%s\"), _T (\"%s\"), GT_FIXED_EXT_FLAG_FILEXT_COM },\n",
                    sExt.c_str (),
                    sDesc.c_str ());
      }
    }

    fputs ("};\n", f);
    fclose (f);

    cout << pSourceCPPFile << " was created\n";
  }

  return 0;
}
