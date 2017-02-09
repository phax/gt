#ifndef _GT_EXE_ANALYZER_HXX_
#define _GT_EXE_ANALYZER_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gt_exe_types.hxx"

namespace GT {

class Table;

//--------------------------------------------------------------------
class EXEAnalyzer : public EXEHeader
//--------------------------------------------------------------------
{
private:
  enum EXEType
  {
    EXE_DOS,
    EXE_PE,
    EXE_NE,
    EXE_LE
  };

  EXEType m_eEXEType;
  gtint32 m_nNewExeHeaderOffset;

  void GT_CALL _CheckForNewEXE ();
  bool GT_CALL _RecalculateHeader (const gtuint32 nHeaderSize,
                                   const bool     bTooSmall,
                                         Table*   pTable);

  bool GT_CALL _Check_C2E ();
  bool GT_CALL _Check_EXE_0J (const file_t nStartPos);
  bool GT_CALL _Check_EXE_1J (const file_t nStartPos);
  bool GT_CALL _Check_EXE_2J (const file_t nStartPos);
  bool GT_CALL _Check_EXE_1LJ (const file_t nStartPos);
  bool GT_CALL _Check_EXT_EXE (const file_t nStartPos, const bool bIsCOM);
  void GT_CALL _Check_EXE_Always (const file_t nRealIP, const bool bIsCOM);
  bool GT_CALL _Check_EXE_Generic (const file_t nRealIP);
  bool GT_CALL _CheckStartupCode (const bool bIsEXE);

  void GT_CALL _ListNEEXEs ();
  void GT_CALL _ListCOM ();
  void GT_CALL _ListEXE ();

  //! implementation of BasicAnalyzer
  virtual void GT_CALL _ShowResults ();

public:
  explicit EXEAnalyzer (FileBuffer* pBuffer)
    : EXEHeader             (pBuffer),
      m_eEXEType            (EXE_DOS),
      m_nNewExeHeaderOffset (0)
  {}
};

}  // namespace

#endif
