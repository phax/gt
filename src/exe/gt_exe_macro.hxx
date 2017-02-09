#ifndef _GT_EXE_MACRO_HXX_
#define _GT_EXE_MACRO_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gt_analyzer.hxx"

namespace GT {

class EXE_PE_SectionTableAnalyzer;
struct EXE_PE_OptionalFileHeader;

//--------------------------------------------------------------------
class GT_EXPORT_EXE EXE_MacroManager : public BasicBuffer
//--------------------------------------------------------------------
{
private:
  EXE_PE_SectionTableAnalyzer* m_pSectionTable;
  EXE_PE_OptionalFileHeader*   m_pOFH;

public:
  // pSectionTable is needed for the 'r' and 'A' macro - can be NULL!
  // pOFH is needed for the 'A' macro - can be NULL!
  EXE_MacroManager (FileBuffer*                  pBuffer,
                    EXE_PE_SectionTableAnalyzer* pSectionTable,
                    EXE_PE_OptionalFileHeader*   pOFH)
    : BasicBuffer     (pBuffer),
      m_pSectionTable (pSectionTable),
      m_pOFH          (pOFH)
  {}

  //! resolve macro sMacro at current position nPos
  bool GT_CALL GetMacroPos (      file_t& nPos,
                                  LPCTSTR sMacro,
                            const bool    bIsCOM);

  // needs to be public
  bool GT_CALL GetJumpPos (      file_t&  nPos,
                           const gtuint32 nJumpCount);

  bool GT_CALL GetLongCallPos (      file_t& nPos,
                               const file_t  nStartOffset);

  bool GT_CALL Get32BitJumpPos (      file_t&  nPos,
                                const gtuint32 nJumpCount);

  bool GT_CALL Get32BitRVAJumpPos (      file_t&  nPos,
                                   const gtuint32 nJumpCount);

  bool GT_CALL GetMOVJumpPos (      file_t& nPos,
                              const bool    bIsCOM);

  bool GT_CALL GetSectionPos (      file_t& nPos,
                              const char*   pSectionName);

  bool GT_CALL GetASM32Pos (      file_t&  nPos,
                            const gtuint32 nCmdCount,
                            const bool     bTrace);
};

}  // namespace

#endif
