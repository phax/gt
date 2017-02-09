#ifndef _GT_EXE_LE_HXX_
#define _GT_EXE_LE_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gt_analyzer.hxx"
#include "gt_exe_types.hxx"

namespace GT {

//--------------------------------------------------------------------
class EXE_LE_ObjectTableHandler : public BasicBuffer
//--------------------------------------------------------------------
{
private:
  EXE_LE_Header* m_pHeader;
  size_t         m_nTableLen;
  EXE_LE_Object* m_pObjects;
  size_t         m_nLast;

public:
  EXE_LE_ObjectTableHandler (FileBuffer*    pBuffer,
                             EXE_LE_Header* pHeader);

  virtual ~EXE_LE_ObjectTableHandler ();

  void           GT_CALL Init (const gtuint32 nLEOffset);
  EXE_LE_Object* GT_CALL GetObj (const size_t nIndex) const;
  size_t         GT_CALL GetTableLen () const { return m_nTableLen; }

  gtuint32 GT_CALL GetRealAddrOfObject (const size_t nIndex) const;
  gtuint32 GT_CALL GetSizeOfObject (const size_t nIndex) const;
  gtuint32 GT_CALL GetRealEndAddrOfObject (const size_t nIndex) const;
  gtuint32 GT_CALL GetEXESize () const;
  gtuint32 GT_CALL GetEntryPoint () const;
  gtuint32 GT_CALL GetEntryPointBaseAddr () const;
};

//--------------------------------------------------------------------
class EXE_LE_Lister : private BasicBuffer
//--------------------------------------------------------------------
{
private:
  gtuint32                  m_nLEOffset;
  EXE_LE_Header             m_aHeader;
  EXE_LE_ObjectTableHandler m_aOTH;

public:
  EXE_LE_Lister (      FileBuffer* pBuffer,
                 const gtuint32    nLEOffset);

  void   GT_CALL Execute ();
  file_t GT_CALL GetOverlayOffset ();
};

}  // namespace

#endif
