#include "gt_exe_analyzer.hxx"

START_EXTERN_C

using namespace GT;

//--------------------------------------------------------------------
GT_EXPORT_EXE GT_ListResult GT_CALL GT_RunDefaultPlugin_EXE
                                        (FileBuffer *pBuffer)
//--------------------------------------------------------------------
{
  EXEAnalyzer aAnalyzer (pBuffer);
  if (!aAnalyzer.AnalyzerMatch ())
    return GT_LIST_NOT_IDENTIFIED;

  // list it!
  aAnalyzer.AnalyzerShow ();
  return GT_LIST_IDENTIFIED_BREAK;
}

END_EXTERN_C
