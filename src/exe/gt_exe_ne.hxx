#ifndef _GT_EXE_NE_HXX_
#define _GT_EXE_NE_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gt_analyzer.hxx"
#include "gt_exe_types.hxx"

namespace GT {

//--------------------------------------------------------------------
class EXE_NE_HeaderHdl : public BasicBuffer
//--------------------------------------------------------------------
{
protected:
  gtuint32      m_nNEOffset;
  EXE_NE_Header m_aHeader;

public:
  EXE_NE_HeaderHdl (const gtuint32    nNEOffset,
                          FileBuffer* pBuffer);

  gtuint32 GT_CALL GetAbsoluteSegmentTableOffset () const { return m_nNEOffset + m_aHeader.nSegmentTableOfs; }
  gtuint32 GT_CALL GetAbsoluteResourceTableOffset () const { return m_nNEOffset + m_aHeader.nResourceTableOfs; }
  gtuint32 GT_CALL GetAbsoluteResidentNamesTableOffset () const { return m_nNEOffset + m_aHeader.nResidentNamesTableOfs; }
  gtuint32 GT_CALL GetAbsoluteModuleReferenceTableOffset () const { return m_nNEOffset + m_aHeader.nModuleReferenceTableOfs; }
  gtuint32 GT_CALL GetAbsoluteImportedNamesTableOffset () const { return m_nNEOffset + m_aHeader.nImportedNamesTableOfs; }
  gtuint32 GT_CALL GetAbsoluteEntryPointTableOffset () const { return m_nNEOffset + m_aHeader.nEntryTableOfs; }
  gtuint32 GT_CALL GetSectorSize () const { return (1 << m_aHeader.nFileAlignmentSizeShiftCount); }
  bool     GT_CALL HasSelfLoader () const { return (m_aHeader.nApplicationFlags & BITS[3]) && (m_aHeader.nTargetOS == GT_EXE_NE_OS_WINDOWS); }
  const EXE_NE_Header* GT_CALL GetpHeader () const { return &m_aHeader; }
};

//--------------------------------------------------------------------
class EXE_NE_SegmentHandler : public BasicBuffer
//--------------------------------------------------------------------
{
private:
  EXE_NE_HeaderHdl* m_pHeader;
  EXE_NE_Header*    m_pEXEHeader;

  gtuint32          m_nSectorSize;
  size_t            m_nTableLen;
  EXE_NE_Segment*   m_pSegments;

public:
  EXE_NE_SegmentHandler (FileBuffer*       pBuffer,
                         EXE_NE_HeaderHdl* pHeader);

  virtual ~EXE_NE_SegmentHandler ();

  gtuint32 GT_CALL GetEndPos (const size_t n) const;
  gtuint32 GT_CALL GetLength (const size_t n) const;
  gtuint32 GT_CALL GetStartPos (const size_t n) const;
  gtuint32 GT_CALL GetAllocSize (const size_t n) const;

  gtuint32 GT_CALL GetEntryPoint () const;
  gtuint32 GT_CALL GetCalculatedFileSize () const;

  LPCTSTR GT_CALL GetType (const size_t n) const;
};

//--------------------------------------------------------------------
class EXE_NE_Lister : private EXE_NE_HeaderHdl
//--------------------------------------------------------------------
{
private:
  file_t m_nOverlayOffset;

  bool GT_CALL Check_NE_0J (const gtuint32 nStartPos);

public:
  EXE_NE_Lister (      FileBuffer* pBuffer,
                 const gtuint32    nNEOffset)
    : EXE_NE_HeaderHdl (nNEOffset, pBuffer),
      m_nOverlayOffset (0)
  {}

  void GT_CALL Execute ();

  file_t GT_CALL GetOverlayOffset () const { return m_nOverlayOffset; }
};

}  // namespace

#endif
