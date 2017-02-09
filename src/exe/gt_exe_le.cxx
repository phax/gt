#include "gt_exe_le.hxx"

#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_table.hxx"
#include "gt_utils.hxx"

namespace GT {

#define EXE_LE_REALINDEX(n) ((n) >= m_nTableLen ? m_nTableLen - 1 : (n))

//--------------------------------------------------------------------
static LPCTSTR _uint16_bitstr (const gtuint16 n)
//--------------------------------------------------------------------
{
  const size_t BITCOUNT = 16;
  static TCHAR s [BITCOUNT + 1];
  for (size_t i = 0; i < BITCOUNT; i++)
    s[i] = (n & BITS[i]) ? _T ('x') : _T ('.');
  s [BITCOUNT] = _T ('\0');
  return s;
}

/*! Don't do anything with the EXE_LE_Header inside since it is
      not inited yet!
    After reading the header from the filebuffer, you have to
      call Init!
 */
//--------------------------------------------------------------------
EXE_LE_ObjectTableHandler::EXE_LE_ObjectTableHandler
                                        (FileBuffer*    pBuffer,
                                         EXE_LE_Header* pHeader)
//--------------------------------------------------------------------
  : BasicBuffer (pBuffer),
    m_pHeader   (pHeader),
    m_nTableLen (size_t (-1)),
    m_pObjects  (NULL),
    m_nLast     (size_t (-1))
{
  ASSERT (m_pHeader);
}

//--------------------------------------------------------------------
EXE_LE_ObjectTableHandler::~EXE_LE_ObjectTableHandler ()
//--------------------------------------------------------------------
{
  delete [] m_pObjects;
}

//--------------------------------------------------------------------
void GT_CALL EXE_LE_ObjectTableHandler::Init
                                        (const gtuint32 nLEOffset)
//--------------------------------------------------------------------
{
  m_nTableLen  = m_pHeader->nObjectTableEntries;

  if (m_nTableLen > 0)
  {
    m_pObjects = new EXE_LE_Object [m_nTableLen];

    // read from file
    m_pBuffer->GetBuffer (nLEOffset + m_pHeader->nObjectTableOfs,
                          m_pObjects,
                          m_nTableLen * EXE_LE_OBJECT_SIZE);
  }
}

//--------------------------------------------------------------------
EXE_LE_Object* GT_CALL EXE_LE_ObjectTableHandler::GetObj
                                        (const size_t nIndex) const
//--------------------------------------------------------------------
{
  ASSERT (nIndex >= 0 && nIndex < m_nTableLen);
  return &m_pObjects [EXE_LE_REALINDEX (nIndex)];
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_LE_ObjectTableHandler::GetRealAddrOfObject
                                        (const size_t nIndex) const
//--------------------------------------------------------------------
{
  ASSERT (nIndex >= 0 && nIndex < m_nTableLen);
  const size_t nRealIndex = EXE_LE_REALINDEX (nIndex);

  return m_pHeader->nDataPagesOfsFromTopOfFile +
         (m_pObjects [nRealIndex].nPageTableIndex - 1) * m_pHeader->nMemoryPageSize;
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_LE_ObjectTableHandler::GetSizeOfObject
                                        (const size_t nIndex) const
//--------------------------------------------------------------------
{
  ASSERT (nIndex >= 0 && nIndex < m_nTableLen);
  const size_t nRealIndex = EXE_LE_REALINDEX (nIndex);

  if (nRealIndex == m_nTableLen - 1)
  {
    // last entry size is different!

    if (1)
    {
      /* old code:
         Seemed to be wrong according to NVidia's nvmini2.vxd file!
      */
      if (m_pObjects [nRealIndex].nPageTableEntries >= 1)
        return m_pObjects [nRealIndex].nPageTableEntries * m_pHeader->nMemoryPageSize;
      else
        return (m_pObjects [nRealIndex].nPageTableEntries - 1) * m_pHeader->nMemoryPageSize + m_pHeader->nLastPageBytes;
    }
    else
    {
      // for ECBOOTIL.VXD nPageTableEntries == 0!!
      ASSERT (m_pObjects [nRealIndex].nPageTableEntries > 0);
      return (m_pObjects [nRealIndex].nPageTableEntries - 1) * m_pHeader->nMemoryPageSize + m_pHeader->nLastPageBytes;
    }
  }

  // and except the last
  return m_pObjects [nRealIndex].nPageTableEntries * m_pHeader->nMemoryPageSize;
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_LE_ObjectTableHandler::GetRealEndAddrOfObject
                                        (const size_t nIndex) const
//--------------------------------------------------------------------
{
  ASSERT (nIndex >= 0 && nIndex < m_nTableLen);
  return GetRealAddrOfObject (nIndex) + GetSizeOfObject (nIndex);
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_LE_ObjectTableHandler::GetEXESize () const
//--------------------------------------------------------------------
{
  gtuint32 nOfs, nSize;
  size_t nIndex = m_nTableLen - 1;

  // iterate table from back to front
  // and find the first object with a size != 0
  do
  {
    nOfs = GetRealAddrOfObject (nIndex);
    nSize = GetSizeOfObject (nIndex);
    nIndex--;
  } while (nIndex >= 0 && nSize == 0);
  return nOfs + nSize;
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_LE_ObjectTableHandler::GetEntryPoint () const
//--------------------------------------------------------------------
{
  if (m_pHeader->nInitCS == 0)
    return 0;

  return GetRealAddrOfObject (m_pHeader->nInitCS - 1) + m_pHeader->nInitEIP;
}

//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_LE_ObjectTableHandler::GetEntryPointBaseAddr () const
//--------------------------------------------------------------------
{
  if (m_pHeader->nInitCS == 0)
    return 0;

  return m_pObjects[m_pHeader->nInitCS - 1].nRelocationBaseAddr + m_pHeader->nInitEIP;
}

//--------------------------------------------------------------------
EXE_LE_Lister::EXE_LE_Lister
                                        (      FileBuffer* pBuffer,
                                         const gtuint32    nLEOffset)
//--------------------------------------------------------------------
  : BasicBuffer (pBuffer),
    m_nLEOffset (nLEOffset),
    m_aOTH      (pBuffer, &m_aHeader)
{
  // read header (afterwards, we can use m_aOTH)
  m_pBuffer->GetBuffer (m_nLEOffset,
                        &m_aHeader,
                        EXE_LE_HEADER_SIZE);

  // init ObjectTale Handler
  m_aOTH.Init (m_nLEOffset);
}

//--------------------------------------------------------------------
void GT_CALL EXE_LE_Lister::Execute ()
//--------------------------------------------------------------------
{
  out_append (_T ("\n"));
  out_info_format (rc (HRC_EXELIB_LE_AT_OFFSET),
                   m_nLEOffset, m_nLEOffset);
  out_incindent ();

  const gtuint32 nCalculatedEXESize = m_aOTH.GetEXESize ();
  const gtuint32 nEntryPoint = m_aOTH.GetEntryPoint ();
  const gtuint32 nEntryPointBaseAddr = m_aOTH.GetEntryPointBaseAddr ();

  if (m_aHeader.nNonResidentNamesTableOfsFromTopOfFile < m_pBuffer->GetFileSize ())
  {
    pstring sNonResidentName;
    m_pBuffer->GetPascalString (m_aHeader.nNonResidentNamesTableOfsFromTopOfFile, sNonResidentName);
    out_format (_T ("'%s'\n"), sNonResidentName.c_str ());
  }

  out_format (rc (HRC_EXELIB_LE_EP),
              nEntryPoint, nEntryPoint,
              nEntryPointBaseAddr, nEntryPointBaseAddr);

  out_incindent ();
  out_format (rc (HRC_EXELIB_LE_CS_EIP),
              m_aHeader.nInitCS, m_aHeader.nInitEIP);

  out_format (rc (HRC_EXELIB_LE_SS_ESP),
              m_aHeader.nInitSS, m_aHeader.nInitESP);
  out_decindent ();

  out_format (rc (HRC_EXELIB_LE_CALCED_EXE_SIZE),
              nCalculatedEXESize, nCalculatedEXESize);

  out_format (rc (HRC_EXELIB_LE_MEM_PAGE_COUNT),
              m_aHeader.nDataPagesOfsFromTopOfFile, m_aHeader.nDataPagesOfsFromTopOfFile);

  out_format (rc (HRC_EXELIB_LE_MEM_PAGE_SIZE),
              m_aHeader.nMemoryPageSize, m_aHeader.nMemoryPageSize);

  out_format (rc (HRC_EXELIB_LE_BYTES_ON_LAST_PAGE),
              m_aHeader.nLastPageBytes, m_aHeader.nLastPageBytes);

  out_format (rc (HRC_EXELIB_LE_FIXUP_SECTION_SIZE),
              m_aHeader.nFixupSize, m_aHeader.nFixupSize);

  out_append (_T ("\n"));
  out_append (rc (HRC_EXELIB_LE_OBJ_TABLE_LISTING));

  {
    out_incindent ();
    out_format (rc (HRC_EXELIB_LE_OBJ_TABLE_INFO),
                m_aOTH.GetTableLen (),
                m_aHeader.nObjectTableOfs,
                m_nLEOffset + m_aHeader.nObjectTableOfs);

    // init table
    Table aTable;
    aTable.AddColumn (rc (HRC_EXELIB_LE_OBJ_TABLE_COLUMN_NR),           NUM_SPACE,  4, 2);
    aTable.AddColumn (rc (HRC_EXELIB_LE_OBJ_TABLE_COLUMN_PHYSOFS),      HEX,       10, 8);
    aTable.AddColumn (rc (HRC_EXELIB_LE_OBJ_TABLE_COLUMN_ENDOFS),       HEX,       10, 8);
    aTable.AddColumn (rc (HRC_EXELIB_LE_OBJ_TABLE_COLUMN_SIZE),         HEX,       10, 8);
    aTable.AddColumn (rc (HRC_EXELIB_LE_OBJ_TABLE_COLUMN_INDEX),        HEX,       6,  4);
    aTable.AddColumn (rc (HRC_EXELIB_LE_OBJ_TABLE_COLUMN_ENTRIES),      HEX,       6,  4);
    aTable.AddColumn (rc (HRC_EXELIB_LE_OBJ_TABLE_COLUMN_VIRTUAL_SIZE), HEX,       10, 8);
    aTable.AddColumn (rc (HRC_EXELIB_LE_OBJ_TABLE_COLUMN_BASE_ADDR),    HEX,       10, 8);
    aTable.AddColumn (rc (HRC_EXELIB_LE_OBJ_TABLE_COLUMN_FLAGS),        STR,       16, ALIGN_LEFT);

    for (gtuint32 i = 0; i < m_aOTH.GetTableLen (); i++)
    {
      EXE_LE_Object *pObj = m_aOTH.GetObj (i);

      aTable.AddInt (0, i);
      aTable.AddInt (1, m_aOTH.GetRealAddrOfObject (i));
      aTable.AddInt (2, m_aOTH.GetRealEndAddrOfObject (i));
      aTable.AddInt (3, m_aOTH.GetSizeOfObject (i));
      aTable.AddInt (4, pObj->nPageTableIndex);
      aTable.AddInt (5, pObj->nPageTableEntries);
      aTable.AddInt (6, pObj->nVirtualSegmentSize);
      aTable.AddInt (7, pObj->nRelocationBaseAddr);
      aTable.AddStr (8, _uint16_bitstr (gtuint16 (pObj->nFlags)));
    }

    out_table (&aTable);
    out_decindent ();
  }

  out_decindent ();
}

//--------------------------------------------------------------------
file_t GT_CALL EXE_LE_Lister::GetOverlayOffset ()
//--------------------------------------------------------------------
{
  return m_aOTH.GetEXESize ();
}

}  // namespace
