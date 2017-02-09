#include "gt_gen_analyzer.hxx"

#include "gt_cmdline_params.hxx"

START_EXTERN_C

using namespace GT;

//--------------------------------------------------------------------
GT_EXPORT_GEN GT_ListResult GT_CALL GT_RunDefaultPlugin_GEN
                                        (FileBuffer* pBuffer)
//--------------------------------------------------------------------
{
  // any known thingy?
  GenericAnalyzer aAnalyzer (GenericInput (pBuffer, CmdlineParam_GetBool (GT_FLAG_LISTMODE)));

  // unknown?
  if (!aAnalyzer.AnalyzerMatch ())
    return GT_LIST_NOT_IDENTIFIED;

  // show it
  aAnalyzer.AnalyzerShow ();
  return GT_LIST_IDENTIFIED_BREAK;
}

END_EXTERN_C
