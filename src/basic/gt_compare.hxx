#ifndef _GT_COMPARE_HXX_
#define _GT_COMPARE_HXX_

#ifdef _MSC_VER
#pragma once
#endif

// no namespace!
#include "gtc_datatypes.hxx"

namespace GT {

class FileBuffer;

const gtuint8 GT_COMPARE_MAGIC_C2E = 0xAA;
const gtuint8 GT_COMPARE_MAGIC_EXE = 0x00;
const gtuint8 GT_COMPARE_MAGIC_NE  = 0x00;
const gtuint8 GT_COMPARE_MAGIC_PE  = 0x00;
const gtuint8 GT_COMPARE_MAGIC_SYS = 0x00;

// function ptr prototype
typedef void (*SignatureCmpProc) (const EXEEntry*,
                                  gtuint8*,
                                  FileBuffer*);

//--------------------------------------------------------------------
enum ESignatureCmpErr
//--------------------------------------------------------------------
{
  GTCOMP_MATCH = 1701,
  GTCOMP_NO_MATCH,
  GTCOMP_PARTIAL_MATCH
};

//--------------------------------------------------------------------
class GT_EXPORT_BASIC SignatureCmp
//--------------------------------------------------------------------
{
private:
        gtuint8*         m_pFileBuffer;
  const gtuint8          m_nMagic;
  const size_t           m_nTolerance;
        FileBuffer*      m_pBuffer;
        SignatureCmpProc m_pFunc;

public:
  SignatureCmp (      gtuint8*         pFileBuffer,
                const gtuint8          nMagic,
                const size_t           nTolerance,
                      FileBuffer*      pBuffer,
                      SignatureCmpProc pFunc);

  ESignatureCmpErr GT_CALL ExplicitMatches
                                        (const gtuint8* pCompareBuffer,
                                         const size_t   nCompareLen);

  ESignatureCmpErr GT_CALL Matches (const EXEEntry *pCompareEntry);

  static void GT_CALL DisplayOutput
                                        (const ENameTypes       eNameType,
                                         const ESignatureCmpErr eCmdErr,
                                               LPCTSTR          pDisplayName);
};

}  // namespace

#endif
