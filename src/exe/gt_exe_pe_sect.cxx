#include "gt_exe_pe.hxx"

#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_utils.hxx"

namespace GT {

//--------------------------------------------------------------------
void GT_CALL EXE_PE_SectionTableAnalyzer::_Init
                                        (const file_t nOffset)
//--------------------------------------------------------------------
{
  if (m_nTableLen > 0)
  {
    m_pSections = new EXE_PE_Section[m_nTableLen];

    // read from file
    if (!m_pBuffer->GetBuffer (nOffset, m_pSections, _GetTableSize ()))
      out_error_append (rc (HRC_EXELIB_PE_ST_FAILED_TO_READ));

    // according to any PE EXE dox, there should not be more than 96 sections
    // but in COFF OBJ files there can be more sections
    if (m_bEXEMode && m_nTableLen > 96)
      out_error_append (rc (HRC_EXELIB_PE_ST_TOO_MANY_SECTIONS));
  }
  else
  {
    out_error_append (rc (HRC_EXELIB_PE_ST_NOT_FOUND));
  }
}

//--------------------------------------------------------------------
EXE_PE_SectionTableAnalyzer::EXE_PE_SectionTableAnalyzer
                                        (      FileBuffer*             pBuffer,
                                               EXE_PE_ImageFileHeader* pIFH,
                                         const file_t                  nPEOffset,
                                         const bool                    bEXEMode)
//--------------------------------------------------------------------
  : m_pBuffer    (pBuffer),
    m_nPEOffset  (nPEOffset),
    m_pSections  (NULL),
    m_nTableLen  (pIFH->nNumberOfSections),
    m_bEXEMode   (bEXEMode)
{
  ASSERT (pBuffer);
  ASSERT (pIFH);
  ASSERT (nPEOffset > 0);

  // read sections
  _Init (m_nPEOffset + EXE_PE_IMAGEHEADER_SIZE + pIFH->nOptionalHeaderSize);
}

//--------------------------------------------------------------------
EXE_PE_SectionTableAnalyzer::EXE_PE_SectionTableAnalyzer
                                        (      FileBuffer* pBuffer,
                                         const file_t      nOffset,
                                         const size_t      nNumberOfSections,
                                         const bool        bEXEMode)
//--------------------------------------------------------------------
  : m_pBuffer    (pBuffer),
    m_nPEOffset  (nOffset),
    m_pSections  (NULL),
    m_nTableLen  (nNumberOfSections),
    m_bEXEMode   (bEXEMode)
{
  ASSERT (pBuffer);
  ASSERT (nOffset > 0);

  // read sections
  _Init (nOffset);
}

//--------------------------------------------------------------------
EXE_PE_SectionTableAnalyzer::~EXE_PE_SectionTableAnalyzer ()
//--------------------------------------------------------------------
{
  // free memory
  delete [] m_pSections;
  m_pSections = NULL;
}

/*  The EXE size is determined by the last section with a valid physical
      offset and size
    loop over all sections from last to first and check which section has
     valid values.
 */
//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_PE_SectionTableAnalyzer::GetEXESize () const
//--------------------------------------------------------------------
{
// old algorithm: find last section with a physical size > 0
// new algorithm: find section with largester phsyical offset

  EXE_PE_Section *pLargest = NULL;
  for (size_t i = 0; i < m_nTableLen; ++i)
  {
    if (!pLargest || m_pSections[i].nPhysicalOffset > pLargest->nPhysicalOffset)
      pLargest = &m_pSections[i];
  }

  // for ACE archives: do not align to 16 bit
  // Note: normally it is automatically aligned to 16 bit (in header)
  return pLargest
           ? pLargest->nPhysicalOffset + pLargest->nPhysicalSize
           : 0;

/* old
  gtuint32 nResult;

  for (size_t i = m_nTableLen; i >  0; i--)
  {
    nResult = m_pSections[i - 1].nPhysicalOffset +
              m_pSections[i - 1].nPhysicalSize;
    if (nResult > 0)
      return nResult;
  }

  return 0;
*/
}

/*! Find the section with the given name.
    Simply compares the 8 chars!
 */
//--------------------------------------------------------------------
EXE_PE_Section* GT_CALL EXE_PE_SectionTableAnalyzer::GetSectionOfName
                                        (const char *pSectionName) const
//--------------------------------------------------------------------
{
  ASSERT (pSectionName);
  ASSERT (strlen (pSectionName) <= 8);

  for (size_t i = 0; i < m_nTableLen; ++i)
    if (memcmp (m_pSections[i].sName, pSectionName, 8) == 0)
      return &m_pSections[i];
  return NULL;
}

/*! Get the name of the section.
 */
//--------------------------------------------------------------------
LPCTSTR GT_CALL EXE_PE_SectionTableAnalyzer::GetSectionName
                                        (const EXE_PE_Section* pSection,
                                         const bool            bFillWithSpaces /* = false */) const
//--------------------------------------------------------------------
{
  const size_t MAX_NAME_LEN = 8;
  static TCHAR sTerminatedName[MAX_NAME_LEN + 1];
  size_t nNameLen = 0;

  ASSERT (pSection);

  // convert from char to TCHAR
  for (const char *p = pSection->sName;
       *p && nNameLen < MAX_NAME_LEN;
       p++, nNameLen++)
  {
    *_tcsninc (sTerminatedName, nNameLen) = TCHAR (*p);
  }

  if (nNameLen == 0)
  {
    // if the section has no name make a default name with <= 8 characters
    nNameLen = _tcslen (_tcscpy (sTerminatedName, _T ("[noname]")));
  }

  if (bFillWithSpaces)
  {
    // fill the rest with spaces
    for (; nNameLen < MAX_NAME_LEN; nNameLen++)
      *_tcsninc (sTerminatedName, nNameLen) = _T (' ');
  }

  // ensure there is a trailing '\0'!!
  *_tcsninc (sTerminatedName, nNameLen) = _T ('\0');
  return sTerminatedName;
}

//--------------------------------------------------------------------
int GT_CALL EXE_PE_SectionTableAnalyzer::GetSectionPosOfRVA
                                        (const rva_t nRVA) const
//--------------------------------------------------------------------
{
  // must be signed!
  for (size_t i = m_nTableLen; i > 0; i--)
    if (m_pSections[i - 1].nRVA <= nRVA)
      return int (i - 1);
  return -1;
}

//--------------------------------------------------------------------
LPCTSTR GT_CALL EXE_PE_SectionTableAnalyzer::GetSectionNameOfRVA
                                        (const rva_t nRVA) const
//--------------------------------------------------------------------
{
  EXE_PE_Section *pSection = GetSectionOfRVA (nRVA);
  return pSection
           ? GetSectionName (pSection, false)
           : _T ("[section not found]");
}

//--------------------------------------------------------------------
EXE_PE_Section* GT_CALL EXE_PE_SectionTableAnalyzer::GetSectionOfRVA
                                        (const rva_t nRVA) const
//--------------------------------------------------------------------
{
  // must be signed!
  for (size_t i = m_nTableLen; i > 0; i--)
    if (nRVA >= m_pSections[i - 1].nRVA)
      return &(m_pSections[i - 1]);
  return NULL;
}

//--------------------------------------------------------------------
EXE_PE_Section* GT_CALL EXE_PE_SectionTableAnalyzer::GetSectionOfPhysicalOffset
                                        (const gtuint32 nOffset) const
//--------------------------------------------------------------------
{
  // must do it from back to front!!
  for (size_t i = m_nTableLen; i > 0; i--)
    if (nOffset >= m_pSections[i - 1].nPhysicalOffset &&
        m_pSections[i - 1].nPhysicalSize > 0)
    {
      return &(m_pSections[i - 1]);
    }
  return NULL;
}

/*! Convert RVA to physical offset.
 */
//--------------------------------------------------------------------
gtuint32 GT_CALL EXE_PE_SectionTableAnalyzer::r2o
                                        (const rva_t nRVA) const
//--------------------------------------------------------------------
{
  EXE_PE_Section *pSection = GetSectionOfRVA (nRVA);
  return pSection
           ? pSection->nPhysicalOffset + RVA_VAL (nRVA - pSection->nRVA)
           : RVA_VAL (nRVA);
}

/*! Convert physical offset to RVA.
 */
//--------------------------------------------------------------------
rva_t GT_CALL EXE_PE_SectionTableAnalyzer::o2r
                                        (const gtuint32 nPhysicalOffset) const
//--------------------------------------------------------------------
{
  EXE_PE_Section *pSection = GetSectionOfPhysicalOffset (nPhysicalOffset);
  return pSection
           ? pSection->nRVA + (nPhysicalOffset - pSection->nPhysicalOffset)
           : rva_t (nPhysicalOffset);
}

}  // namespace
