#ifndef _GT_ARC_ANALYZER_HXX_
#define _GT_ARC_ANALYZER_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gt_analyzer.hxx"

namespace GT {

//--------------------------------------------------------------------
enum EArchiveType
//--------------------------------------------------------------------
{
  GT_PACKERENUM_NONE = 0,  // 0 is important for the index in GT_PackerNames!!
  _777,
  ACE,
  AIN,
  AKT7,
  AKT8,
  AKT9,
  AMG,
  ARC,
  ARG,
  ARI,
  ARJ,
  ARK,
  ARQ,
  ARS,
  ARX,
  AR7,
  ASD,
  BIX,
  BLI,
  BOA,
  BS2,
  BSA,
  BSN,
  BZ,
  CAB,
  CHZ,
  COD,
  COMP16,
  CPAC,
  DPA,
  DWC,
  ELI,
  ESP,
  FINISH,
  GZ,
  HA,
  HAP,
  HPA,
  HQX,
  HYP25,
  HYP26,
  INNO,
  IMP,
  INS,
  ISC,
  ISO,
  JAR,
  JRC,
  LG,
  LIM,
  LS,
  LZA,
  LZH,
  LZO,
  MAR,
  MSC,
  NERO,
  NERO5,
  NSIS10,    // 1.0-   deadbeef nsisinstall\0
  NSIS11E,   // 1.1e-  deadbeef NullSoftInst
  NSIS13,    // 1.3-   flags deadbeef NullSoftInst
  NSIS16,    // 1.6-   flags deadbeef NullsoftInst
  PACK,
  PAQ,
  PAQ8,
  PAR,
  PHP,
  PPZ,
  Q,
  QFC,
  RAR0,
  RAR,
  RAX,
  RKV1,
  RKV2,
  RNC,
  SAR,
  SBC,
  SBX,
  SKY,
  SOF,
  SONY,
  SQWEZ,
  SQZ,
  STI,
  SWG,
  SYN,
  SZIP,
  TAR,
  TIP,         // use ZIP lister
  UC2,
  UFA,
  UHA,
  XIE,
  XL,
  XPA,
  YC,
  YZ,
  ZET,
  ZIP,
  ZOO,
  ZPK,
  GT_PACKERENUM_LAST
};

//--------------------------------------------------------------------
class ArchiveAnalyzer : public BasicAnalyzer
//--------------------------------------------------------------------
{
private:
  EArchiveType m_ePacker;
  file_t       m_nLocalOffset;

  void _GetPacker (const int nIndex);

  bool _IsHAArchive (const int nIndex) const;

  void _ListArchiveName () const;
  bool _ListArchiveContent ();

  //! implementation of BasicAnalyzer
  virtual void GT_CALL _ShowResults ();

public:
  explicit ArchiveAnalyzer (FileBuffer *pBuffer);

  //! implementation of BasicAnalyzer
  virtual bool GT_CALL AnalyzerMatch () { return (m_ePacker != GT_PACKERENUM_NONE); }

  static gtuint32 GT_CALL GetTotalArchiveCount ();
  static gtuint32 GT_CALL GetDamagedArchiveCount ();
};

}  // namespace

#endif
