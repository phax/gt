#include "gt_exe_pe.hxx"

#include "gt_exception.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_table.hxx"
#include "gt_utils.hxx"

namespace GT {

// maximum offsets for string tables
const gtuint32 EXE_PE_MAX_DATADIRECTORY_ENTRIES = 16;

/*! Get the name of the PE data directory name from resource.
 */
//--------------------------------------------------------------------
static void __getDataDirName
                                        (gtuint32 nIndex,
                                         pstring& sName)
//--------------------------------------------------------------------
{
  static const resid_t DATA_DIR_NAMES[EXE_PE_MAX_DATADIRECTORY_ENTRIES] =
  {
    HRC_GTLIB_PEDIRTYPE_0,
    HRC_GTLIB_PEDIRTYPE_1,
    HRC_GTLIB_PEDIRTYPE_2,
    HRC_GTLIB_PEDIRTYPE_3,
    HRC_GTLIB_PEDIRTYPE_4,
    HRC_GTLIB_PEDIRTYPE_5,
    HRC_GTLIB_PEDIRTYPE_6,
    HRC_GTLIB_PEDIRTYPE_7,
    HRC_GTLIB_PEDIRTYPE_8,
    HRC_GTLIB_PEDIRTYPE_9,
    HRC_GTLIB_PEDIRTYPE_A,
    HRC_GTLIB_PEDIRTYPE_B,
    HRC_GTLIB_PEDIRTYPE_C,
    HRC_GTLIB_PEDIRTYPE_D,
    HRC_GTLIB_PEDIRTYPE_E,
    HRC_GTLIB_PEDIRTYPE_F
  };

  // valid index?
  if (nIndex < EXE_PE_MAX_DATADIRECTORY_ENTRIES)
    str_assign_from_res (sName, DATA_DIR_NAMES [nIndex]);
  else
    sName.clear ();
}

//--------------------------------------------------------------------
EXE_PE_DataDirectoryHandler::EXE_PE_DataDirectoryHandler
                                        (      FileBuffer*                  pBuffer,
                                               EXE_PE_SectionTableAnalyzer* pSectionTable,
                                         const size_t                       nDirectoryCount,
                                         const file_t                       nOffset)
//--------------------------------------------------------------------
  : BasicBuffer       (pBuffer),
    m_pSectionTable   (pSectionTable),
    m_nDirectoryCount (nDirectoryCount),
    m_nOffset         (nOffset)
{
  ASSERT (pSectionTable);

  if (m_nDirectoryCount > EXE_PE_MAX_DATADIRECTORY_ENTRIES)
  {
    out_error_format (rc (HRC_EXELIB_PE_DD_TOO_MANY),
                      EXE_PE_MAX_DATADIRECTORY_ENTRIES,
                      m_nDirectoryCount);
    m_nDirectoryCount = EXE_PE_MAX_DATADIRECTORY_ENTRIES;
  }

  // get memory and read from file!
  m_pDataDirectory = new EXE_PE_ImageDataDirectory [m_nDirectoryCount];
  if (!m_pBuffer->GetBuffer (m_nOffset,
                             m_pDataDirectory,
                             m_nDirectoryCount * EXE_PE_DATADIRECTORY_SIZE))
  {
    out_error_append (rc (HRC_EXELIB_PE_DD_FAILED_TO_INIT));
  }
}

//--------------------------------------------------------------------
void GT_CALL EXE_PE_DataDirectoryHandler::AddEntryToTable
                                        (const gtuint32 nIndex,
                                               Table*   pTable)
//--------------------------------------------------------------------
{
  ASSERT (nIndex < EXE_PE_MAX_DATADIRECTORY_ENTRIES);
  if (nIndex >= m_nDirectoryCount)
  {
    // invalid index!
    InternalError (m_pBuffer->GetpFileName (), _T (__FILE__), __LINE__);
    return;
  }

  // get current directory entry
  EXE_PE_ImageDataDirectory* pEntry = &m_pDataDirectory[nIndex];
  if (pEntry->nRVA == rva_t (0))
  {
    // RVA 0 means that the directory is not present!
    return;
  }

  // get name of data directory
  pstring sDataDirectoryName;
  __getDataDirName (nIndex, sDataDirectoryName);

  // get the name of the PE section in which the current entry
  // resides. If it is a section without name, give it an index!
  LPTSTR sAccordingSectionName = const_cast <LPTSTR> (m_pSectionTable->GetSectionNameOfRVA (pEntry->nRVA));
  if (_tcscmp (sAccordingSectionName, _T ("[no name]")) == 0)
  {
    // empty name? -> add index in section table
    _stprintf (sAccordingSectionName, _T ("[no name #%d]"),
               m_pSectionTable->GetSectionPosOfRVA (pEntry->nRVA));
  }

  // should match to the table header outside!
  pTable->AddStr (0, sDataDirectoryName);
  pTable->AddInt (1, m_pSectionTable->r2o (pEntry->nRVA));
  pTable->AddInt (2, RVA_VAL (pEntry->nRVA));
  pTable->AddInt (3, pEntry->nSize);
  pTable->AddStr (4, sAccordingSectionName);
}

//--------------------------------------------------------------------
EXE_PE_ImageDataDirectory* GT_CALL EXE_PE_DataDirectoryHandler::GetEntry
                                        (const gtuint32 nIndex)
//--------------------------------------------------------------------
{
  ASSERT (nIndex >= 0 && nIndex < EXE_PE_MAX_DATADIRECTORY_ENTRIES);

  return (nIndex >= m_nDirectoryCount)
           ? NULL
           : &m_pDataDirectory[nIndex];
}

//--------------------------------------------------------------------
size_t GT_CALL EXE_PE_DataDirectoryHandler::GetNumberOfUsedEntries () const
//--------------------------------------------------------------------
{
  size_t nCount = 0;
  for (size_t i = 0; i < m_nDirectoryCount; i++)
    if (m_pDataDirectory[i].nRVA != rva_t (0))
      nCount++;
  return nCount;
}

}  // namespace
