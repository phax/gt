#include "gee_filedata.hxx"
#include "gt_exe_macro.hxx"

namespace GEE {

const int MAX_BUFFER = 4096;

//--------------------------------------------------------------------
void GEEFileData::_Read ()
//--------------------------------------------------------------------
{
  size_t nSize = MAX_READ;
  if (!m_pBuffer->GetBuffer (&m_aBuf[0], nSize))
  {
    // to little bytes left
    nSize = size_t (m_pBuffer->GetFileSize () - m_pBuffer->GetActFilePos ());
    ASSERT (nSize < MAX_READ);

    if (nSize > 0)
      m_pBuffer->GetBufferX (&m_aBuf[0], nSize);
  }

  m_pCur = &m_aBuf[0];
  m_pEnd = &m_aBuf[0] + nSize;
}

//--------------------------------------------------------------------
GEEFileData::GEEFileData (LPCTSTR sFilename)
//--------------------------------------------------------------------
  : m_pBuffer       (NULL),
    m_eFormat       (UNKNOWN),
    m_nNewExeOffset (-1),
    m_nEntryPoint   (-1),
    m_pCur          (NULL),
    m_pEnd          (NULL),
    m_pSTA          (NULL)
{
  m_pBuffer = new FileBuffer (sFilename, 0, NULL, NULL);
  if (m_pBuffer->Init () != OPEN_SUCCESS)
  {
    // can't use it :(
    delete m_pBuffer;
    m_pBuffer  = NULL;
  }
}

//--------------------------------------------------------------------
GEEFileData::~GEEFileData ()
//--------------------------------------------------------------------
{
  delete m_pSTA;
  delete m_pBuffer;
}

//--------------------------------------------------------------------
void GEEFileData::resetToStart ()
//--------------------------------------------------------------------
{
  m_pCur = &m_aBuf[0];
}

//--------------------------------------------------------------------
bool GEEFileData::isOkay () const
//--------------------------------------------------------------------
{
  return m_pBuffer != NULL;
}

//--------------------------------------------------------------------
bool GEEFileData::determineFileFormat ()
//--------------------------------------------------------------------
{
  // init it after buffer.init!
  EXEHeader aEXE (m_pBuffer);
  if (aEXE.IsValidEXE ())
  {
    m_nNewExeOffset = m_pBuffer->GetInt32 (0x3C);

    // check whether the offset is valid
    if (m_nNewExeOffset < 0 ||
        m_nNewExeOffset >= m_pBuffer->GetFileSize ())
    {
      // plain old DOS
      m_eFormat = EXE_DOS;

      // get entry point
      m_nEntryPoint = aEXE.GetAbsoluteIP ();
    }
    else
    {
      const gtuint16 nID = m_pBuffer->GetUint16 (m_nNewExeOffset);

      switch (nID)
      {
        case 0x584C:  // LX
        case 0x454C:  // LE
          m_eFormat = EXE_LE;
          break;
        case 0x454E:  // NE
          m_eFormat = EXE_NE;
          break;
        case 0x4550:  // PE
          m_eFormat = EXE_PE;

          // read from file
          VERIFY (m_pBuffer->GetBuffer (m_nNewExeOffset, &m_aIFH, EXE_PE_IMAGEHEADER_SIZE));
          VERIFY (m_pBuffer->GetBuffer (&m_aOFH, EXE_PE_OPTIONALHEADER_SIZE));

          // Seems to be incorrect for 64Bit apps?!
          // That's why the magic check has been added!
          ASSERT (m_aIFH.nOptionalHeaderSize == EXE_PE_OPTIONALHEADER_SIZE + (m_aOFH.nNumberOfRVAsAndSizes * EXE_PE_DATADIRECTORY_SIZE) ||
                  m_aOFH.nMagic == 0x020B);

          // needed for entrypoint (alters buffer position!)
          m_pSTA = new EXE_PE_SectionTableAnalyzer (m_pBuffer, &m_aIFH, m_nNewExeOffset, true);

          // find it (assertion fails on certain tELock 0.98 compacted EXEs (debug check, mutex)!)
          // ASSERT (RVA_VAL (m_aOFH.nAddressOfEntryPoint) <= m_pBuffer->GetFileSize ());
          m_nEntryPoint = m_pSTA->r2o (m_aOFH.nAddressOfEntryPoint);
          ASSERT (m_nEntryPoint <= m_pBuffer->GetFileSize ());
          break;
      }
    }
  }
  else
  {
    m_eFormat = ANYFILE;
    m_nEntryPoint = 0;
  }

  // EP found?
  if (m_nEntryPoint == -1)
    return false;

  // goto EP
  m_pBuffer->SetActFilePos (m_nEntryPoint);

  // currently only supporting PEs
  return m_eFormat == ANYFILE ||
         m_eFormat == EXE_DOS ||
         m_eFormat == EXE_PE;
}

//--------------------------------------------------------------------
bool GEEFileData::evaluateMacro (LPCTSTR pMacro)
//--------------------------------------------------------------------
{
  EXE_MacroManager aMM (m_pBuffer, m_pSTA, &m_aOFH);

  // get current position
  file_t nPos = m_pBuffer->GetActFilePos ();

  // do evaluate macro
  bool bOk = aMM.GetMacroPos (nPos, pMacro, m_eFormat == ANYFILE);
  if (bOk)
  {
    // goto target position
    m_pBuffer->SetActFilePos (nPos);
  }

  return bOk;
}

//--------------------------------------------------------------------
int GEEFileData::getNextChar ()
//--------------------------------------------------------------------
{
  if (m_pCur >= m_pEnd)
  {
    _Read ();

    // EOF?
    if (m_pCur == m_pEnd)
      return GEE_EOF;
  }

  int n = *m_pCur;
  m_pCur++;
  return n;
}

//--------------------------------------------------------------------
bool GEEFileData::getTargetPosition
                                        (file_t  nStartPos,
                                         LPCTSTR pMacro,
                                         file_t& nTargetPos)
//--------------------------------------------------------------------
{
  EXE_MacroManager aMM (m_pBuffer, m_pSTA, &m_aOFH);

  // init target pos
  nTargetPos = nStartPos;

  // do evaluate macro (nTargetPos is a out variable)
  return aMM.GetMacroPos (nTargetPos, pMacro, m_eFormat == ANYFILE);
}

//--------------------------------------------------------------------
FileDataSeq& FileDataSeq::Instance ()
//--------------------------------------------------------------------
{
  static FileDataSeq aInstance;
  return aInstance;
}

}  // namespace
