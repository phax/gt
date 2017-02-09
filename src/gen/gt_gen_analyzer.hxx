#ifndef _GT_GEN_ANALYZER_HXX_
#define _GT_GEN_ANALYZER_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gt_analyzer.hxx"

namespace GT {

/*! Just a wrapper around the parameters for the GenericAnalyzer.
 */
//--------------------------------------------------------------------
class GenericInput
//--------------------------------------------------------------------
{
public:
  FileBuffer* m_pBuffer;
  const bool  m_bListMode;

  GenericInput (FileBuffer* pBuffer,
                bool        bListMode)
    : m_pBuffer   (pBuffer),
      m_bListMode (bListMode)
  {}
};

class GenericLister;

//--------------------------------------------------------------------
class GenericAnalyzer : public BasicAnalyzer
//--------------------------------------------------------------------
{
private:
  GenericLister* m_pLister;

  //! implementation of BasicAnalyzer
  virtual void GT_CALL _ShowResults ();

public:
  explicit GenericAnalyzer (const GenericInput &aGI);
  virtual ~GenericAnalyzer ();

  //! implementation of BasicAnalyzer
  virtual bool GT_CALL AnalyzerMatch () { return m_pLister != NULL; }
};

}  // namespace

#endif
