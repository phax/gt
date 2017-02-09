#ifndef _GT_EXE_PE_HXX_
#define _GT_EXE_PE_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gt_analyzer.hxx"
#include "gt_exe_types.hxx"

namespace GT {

class Table;

//--------------------------------------------------------------------
class GT_EXPORT_EXE EXE_PE_SectionTableAnalyzer
//--------------------------------------------------------------------
{
private:
        FileBuffer*     m_pBuffer;
        file_t          m_nPEOffset;

        EXE_PE_Section* m_pSections;
  const size_t          m_nTableLen;
  const bool            m_bEXEMode;

  //! get the size of the table in the file
  size_t GT_CALL _GetTableSize () const { return m_nTableLen * EXE_PE_SECTION_SIZE; }

  // create the section buffer and read from memory
  void GT_CALL _Init (const file_t nOffset);

public:
  // init section table analyzer with an existing PE ImageFileHeader struct
  EXE_PE_SectionTableAnalyzer (      FileBuffer*             pBuffer,
                                     EXE_PE_ImageFileHeader* pIFH,
                               const file_t                  nPEOffset,
                               const bool                    bEXEMode);

  // init with the information only
  EXE_PE_SectionTableAnalyzer (      FileBuffer* pBuffer,
                               const file_t      nOffset,
                               const size_t      nNumberOfSections,
                               const bool        bEXEMode);

  virtual ~EXE_PE_SectionTableAnalyzer ();

  gtuint32 GT_CALL GetEXESize () const;

  //------------------------------------------------------------------
  size_t GT_CALL GetNumberOfSections () const
  //------------------------------------------------------------------
  {
    return m_nTableLen;
  }

  //------------------------------------------------------------------
  EXE_PE_Section* GT_CALL GetSectionPtr () const
  //------------------------------------------------------------------
  {
    return m_pSections;
  }

  //------------------------------------------------------------------
  LPCTSTR GT_CALL GetSectionName (const size_t nIndex,
                                  const bool   bFillWithSpaces = false) const
  //------------------------------------------------------------------
  {
    ASSERT (nIndex < m_nTableLen);
    return GetSectionName (&m_pSections[nIndex], bFillWithSpaces);
  }

  //------------------------------------------------------------------
  EXE_PE_Section* GT_CALL GetSectionOfPos (const size_t nIndex) const
  //------------------------------------------------------------------
  {
    ASSERT (nIndex < m_nTableLen);
    return &m_pSections[nIndex];
  }

  EXE_PE_Section* GT_CALL GetSectionOfName (const char *pSectionName) const;

  LPCTSTR GT_CALL GetSectionName (const EXE_PE_Section* pSection,
                                  const bool            bFillWithSpaces = false) const;

  int     GT_CALL GetSectionPosOfRVA  (const rva_t nRVA) const;
  LPCTSTR GT_CALL GetSectionNameOfRVA (const rva_t nRVA) const;

  EXE_PE_Section* GT_CALL GetSectionOfRVA (const rva_t nRVA) const;
  EXE_PE_Section* GT_CALL GetSectionOfPhysicalOffset (const gtuint32 nOffset) const;

  gtuint32 GT_CALL r2o (const rva_t nRVA) const;
  rva_t    GT_CALL o2r (const gtuint32 nPhysicalOffset) const;
};

//--------------------------------------------------------------------
class EXE_PE_BasicLister : public BasicBuffer
//--------------------------------------------------------------------
{
protected:
  EXE_PE_SectionTableAnalyzer* m_pSectionTable;
  EXE_PE_ImageDataDirectory*   m_pDataDir;
  file_t                       m_nOffset;

public:
  EXE_PE_BasicLister (FileBuffer*                  pBuffer,
                      EXE_PE_SectionTableAnalyzer* pSectionTable,
                      EXE_PE_ImageDataDirectory*   pDataDir);

  virtual void GT_CALL List () = 0;

  file_t GT_CALL GetOffset () const { return m_nOffset; }
};

//--------------------------------------------------------------------
class EXE_PE_ExportAnalyzer : public EXE_PE_BasicLister
//--------------------------------------------------------------------
{
public:
  EXE_PE_ExportAnalyzer (FileBuffer*                  pBuffer,
                         EXE_PE_SectionTableAnalyzer* pSectionTable,
                         EXE_PE_ImageDataDirectory*   pDataDir)
    : EXE_PE_BasicLister (pBuffer, pSectionTable, pDataDir)
  {}

  virtual void GT_CALL List ();
};

//--------------------------------------------------------------------
class EXE_PE_ImportAnalyzer : public EXE_PE_BasicLister
//--------------------------------------------------------------------
{
private:
  void GT_CALL _ListImportedNames (const gtuint32 nLookupTableOffset);

public:
  EXE_PE_ImportAnalyzer (FileBuffer*                  pBuffer,
                         EXE_PE_SectionTableAnalyzer* pSectionTable,
                         EXE_PE_ImageDataDirectory*   pDataDir)
    : EXE_PE_BasicLister (pBuffer, pSectionTable, pDataDir)
  {}

  virtual void GT_CALL List ();
};

//--------------------------------------------------------------------
class EXE_PE_ResourceAnalyzer : public EXE_PE_BasicLister
//--------------------------------------------------------------------
{
private:
  int      m_nRecursionDepth;
  gtuint32 m_nCurResTypeID;      //!< at level 1
  gtuint32 m_nCurResNameID;      //!< at level 2
  gtuint32 m_nCurResLanguageID;  //!< at level 3
  int      m_nDataSize;
  int      m_nTOCSize;
  int      m_nListLevel;

  enum
  {
    LIST_ENTRY   = 0x01,
    LIST_STRING  = 0x02,
    LIST_VERSION = 0x04
  };

  void GT_CALL _ListVersionInfo (gtuint8* pData,
                                 gtuint32 nDataSize);
  bool GT_CALL _ListTable (const rva_t nTableRVA);
  bool GT_CALL _ListEntry (const rva_t nEntryRVA);

public:
  EXE_PE_ResourceAnalyzer (FileBuffer*                  pBuffer,
                           EXE_PE_SectionTableAnalyzer* pSectionTable,
                           EXE_PE_ImageDataDirectory*   pDataDir);

  virtual void GT_CALL List ();
};

//--------------------------------------------------------------------
class EXE_PE_CertificateAnalyzer : public EXE_PE_BasicLister
//--------------------------------------------------------------------
{
public:
  EXE_PE_CertificateAnalyzer (FileBuffer*                  pBuffer,
                              EXE_PE_SectionTableAnalyzer* pSectionTable,
                              EXE_PE_ImageDataDirectory*   pDataDir)
    : EXE_PE_BasicLister (pBuffer, pSectionTable, pDataDir)
  {}

  virtual void GT_CALL List ();
};

//--------------------------------------------------------------------
class EXE_PE_FixupsAnalyzer : public EXE_PE_BasicLister
//--------------------------------------------------------------------
{
protected:
  LPCTSTR GT_CALL _GetFixupEntryText (gtuint16 nEntryType);

public:
  EXE_PE_FixupsAnalyzer (FileBuffer*                  pBuffer,
                         EXE_PE_SectionTableAnalyzer* pSectionTable,
                         EXE_PE_ImageDataDirectory*   pDataDir)
    : EXE_PE_BasicLister (pBuffer, pSectionTable, pDataDir)
  {}

  virtual void GT_CALL List ();
};

//--------------------------------------------------------------------
class EXE_PE_TLSAnalyzer : public EXE_PE_BasicLister
//--------------------------------------------------------------------
{
public:
  EXE_PE_TLSAnalyzer (FileBuffer*                  pBuffer,
                      EXE_PE_SectionTableAnalyzer* pSectionTable,
                      EXE_PE_ImageDataDirectory*   pDataDir)
    : EXE_PE_BasicLister (pBuffer, pSectionTable, pDataDir)
  {}

  virtual void GT_CALL List ();
};

//--------------------------------------------------------------------
class EXE_PE_DataDirectoryHandler : public BasicBuffer
//--------------------------------------------------------------------
{
private:
  EXE_PE_SectionTableAnalyzer* m_pSectionTable;
  size_t                       m_nDirectoryCount;
  file_t                       m_nOffset;
  EXE_PE_ImageDataDirectory*   m_pDataDirectory;

public:
  EXE_PE_DataDirectoryHandler (      FileBuffer*                  pBuffer,
                                     EXE_PE_SectionTableAnalyzer* pSectionTable,
                               const size_t                       nDirectoryCount,
                               const file_t                       nOffset);

  virtual ~EXE_PE_DataDirectoryHandler ()
  {
    delete m_pDataDirectory;
  }

  EXE_PE_ImageDataDirectory* GT_CALL GetEntry (const gtuint32 nIndex);
  void                       GT_CALL AddEntryToTable (const gtuint32 nIndex, Table* pTable);

  size_t                     GT_CALL GetNumberOfUsedEntries () const;

  size_t                     GT_CALL GetRealDirectoryCount () { return m_nDirectoryCount; }
};

//--------------------------------------------------------------------
class EXE_PE_Lister : public BasicBuffer
//--------------------------------------------------------------------
{
private:
  gtuint32                     m_nNEOffset;
  EXE_PE_ImageFileHeader       m_aIFH;
  EXE_PE_OptionalFileHeader    m_aOFH;
  EXE_PE_SectionTableAnalyzer* m_pST;
  file_t                       m_nOverlayOffset;

  bool GT_CALL _Check_PE_0J (const gtuint32 nStartPos);
  bool GT_CALL _Check_EXT_PE (const gtuint32 nStartPos);

  void GT_CALL _PrintSections  ();
  void GT_CALL _CheckDataDirectory ();
  void GT_CALL _CheckForCode (const gtuint32 nEntryPoint,
                              const bool     bExeMod);
  void GT_CALL _CheckForSpecialSections ();

  bool GT_CALL _AnalyzeAsDOTNET ();
  bool GT_CALL _AnalyzeAsPE ();

public:
  EXE_PE_Lister (      FileBuffer* pBuffer,
                 const gtuint32    nNEOffset);
  virtual ~EXE_PE_Lister ();

  void GT_CALL Execute ();
  file_t GT_CALL GetOverlayOffset () { return m_nOverlayOffset; }
};

}  // namespace

#endif
