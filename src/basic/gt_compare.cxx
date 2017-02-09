#include "gt_compare.hxx"

#include "gt_output.hxx"
#include "gt_utils.hxx"

namespace GT {

//--------------------------------------------------------------------
SignatureCmp::SignatureCmp
                                        (      gtuint8*         pFileBuffer,
                                         const gtuint8          nMagic,
                                         const size_t           nTolerance,
                                               FileBuffer*      pBuffer,
                                               SignatureCmpProc pFunc)
//--------------------------------------------------------------------
  : m_pFileBuffer (pFileBuffer),
    m_nMagic      (nMagic),
    m_nTolerance  (nTolerance),
    m_pBuffer     (pBuffer),
    m_pFunc       (pFunc)
{
  ASSERT (m_pFileBuffer);
  ASSERT (m_pBuffer);
}

//--------------------------------------------------------------------
ESignatureCmpErr GT_CALL SignatureCmp::ExplicitMatches
                                        (const gtuint8* pCompareBuffer,
                                         const size_t   nCompareLen)
//--------------------------------------------------------------------
{
  ASSERT (pCompareBuffer);

  size_t nAllowedFaults = m_nTolerance;
  gtuint8 *pCurrentFileBuffer = m_pFileBuffer;

  for (size_t i = 0; i < nCompareLen; ++i)
  {
    // if the current byte doesn't match the byte in the file
    // and it's not the magic char, add a new fault!
    if (*pCompareBuffer != *pCurrentFileBuffer &&
        *pCompareBuffer != m_nMagic)
    {
      // if there are no faults left, return an error
      if (nAllowedFaults == 0)
        return GTCOMP_NO_MATCH;
      --nAllowedFaults;
    }

    // go to next byte
    ++pCompareBuffer;
    ++pCurrentFileBuffer;
  }

  // If no byte is different -> total match
  // If more bytes are different than we need to compare -> no match
  // Else it's a partial match
  return (nAllowedFaults == m_nTolerance)
            ? GTCOMP_MATCH
            : (m_nTolerance - nAllowedFaults > nCompareLen)
              ? GTCOMP_NO_MATCH
              : GTCOMP_PARTIAL_MATCH;
}

//--------------------------------------------------------------------
ESignatureCmpErr GT_CALL SignatureCmp::Matches
                                        (const EXEEntry *pCompareEntry)
//--------------------------------------------------------------------
{
  ASSERT (pCompareEntry);

  ESignatureCmpErr nError = ExplicitMatches (pCompareEntry->pData,
                                             pCompareEntry->nDataLen);
  if (nError != GTCOMP_NO_MATCH)
  {
    // yep, we found something!
    // check the index!
    ASSERT (pCompareEntry->nNameType < _LAST_TYPE);

    // if a userdefined output function was defined, use it
    // else use the default match function
    if (m_pFunc)
    {
      m_pFunc (pCompareEntry, m_pFileBuffer, m_pBuffer);
    }
    else
    {
      SignatureCmp::DisplayOutput (pCompareEntry->nNameType,
                                   nError,
                                   pCompareEntry->sName);
    }
  }

  return nError;
}

//--------------------------------------------------------------------
void GT_CALL SignatureCmp::DisplayOutput
                                        (const ENameTypes       eNameType,
                                         const ESignatureCmpErr eCmdErr,
                                               LPCTSTR          pDisplayName)
//--------------------------------------------------------------------
{
  ASSERT (pDisplayName);

  //------------------------------------------------------------------
  static const resid_t NameTypeStringID[_LAST_TYPE] =
  //------------------------------------------------------------------
  {
    HRC_GTLIB_COMPARE_CODE,
    HRC_GTLIB_COMPARE_PACKER,
    HRC_GTLIB_COMPARE_ENCRYPTER,
    HRC_GTLIB_COMPARE_COMPILER,
    HRC_GTLIB_COMPARE_LINKER,
    HRC_GTLIB_COMPARE_CONVERTER,
    HRC_GTLIB_COMPARE_PROTECTOR,
    HRC_GTLIB_COMPARE_STICKER,
    HRC_GTLIB_COMPARE_EXTENDER,
    HRC_GTLIB_COMPARE_PASSWORD,
    HRC_GTLIB_COMPARE_MODIFIER,
  };

  // and print the match
  pstring sWhat, sMatch;
  str_assign_from_res (sWhat, NameTypeStringID[eNameType]);

  if (eCmdErr == GTCOMP_PARTIAL_MATCH)
    str_assign_from_res (sMatch, HRC_GTLIB_COMPARE_PARTIAL);
  else
    sMatch.clear ();

  out_info_format (rc (HRC_GTLIB_COMPARE_FOUND),
                   sWhat.c_str (),
                   pDisplayName,
                   sMatch.c_str ());
}

}  // namespace
