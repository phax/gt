#ifndef _GT_CONFIGFILE_HXX_
#define _GT_CONFIGFILE_HXX_

#ifdef _MSC_VER
#pragma once
#endif

namespace GT {

class LineReader;

//--------------------------------------------------------------------
class GT_EXPORT_BASIC ConfigFile
//--------------------------------------------------------------------
{
private:
  int         m_nErrorCount;
  TCHAR       m_sConfigFile[512];
  LineReader* m_pFile;

  void _ShowConfigFileError (const resid_t nErrorID,
                             const size_t  nLineNumber,
                                   void*   pAdditional = NULL);

public:
  ConfigFile (      LPCTSTR sConfigFilename,
              const bool    bReadLocal,
              const bool    bReadGlobal);

  virtual ~ConfigFile ();

  bool Read ();
};

}  // namespace

#endif
