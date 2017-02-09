#include "gt_arc_analyzer.hxx"

START_EXTERN_C

using namespace GT;

//--------------------------------------------------------------------
GT_EXPORT_ARC GT_ListResult GT_CALL GT_RunDefaultPlugin_ARC
                                        (FileBuffer *pBuffer)
//--------------------------------------------------------------------
{
  // is it a known packer?
  ArchiveAnalyzer aAnalyzer (pBuffer);
  if (!aAnalyzer.AnalyzerMatch ())
    return GT_LIST_NOT_IDENTIFIED;

  aAnalyzer.AnalyzerShow ();
  return GT_LIST_IDENTIFIED_BREAK;
}

END_EXTERN_C
