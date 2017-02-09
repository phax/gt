#ifndef _GTLIB_ANALYZER_HXX_
#define _GTLIB_ANALYZER_HXX_

#ifdef _MSC_VER
#pragma once
#endif

namespace GT {

class FileBuffer;

//--------------------------------------------------------------------
class GT_EXPORT_BASIC BasicBuffer
//--------------------------------------------------------------------
{
protected:
  FileBuffer* m_pBuffer;

public:
  explicit BasicBuffer (FileBuffer* pBuffer)
    : m_pBuffer (pBuffer)
  {}
};

//--------------------------------------------------------------------
class GT_EXPORT_BASIC BasicAnalyzer : public BasicBuffer
//--------------------------------------------------------------------
{
protected:
  file_t m_nOverlayOffset;

  //! cannot be pure virtual because of gt_da!
  virtual void GT_CALL _ShowResults () {}

public:
  explicit BasicAnalyzer (FileBuffer* pBuffer)
    : BasicBuffer      (pBuffer),
      m_nOverlayOffset (0)
  {}

  //! does the plugin match the file?
  virtual bool GT_CALL AnalyzerMatch () = 0;

  //! performs the main call to _ShowResults
  void GT_CALL AnalyzerShow ();
};

} // namespace

#endif
