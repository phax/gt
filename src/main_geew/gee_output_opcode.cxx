#include "gee_output.hxx"
#include "gee_settings.hxx"

#include "gt_cmdline_params.hxx"
#include "gt_output.hxx"

namespace GEE {

//--------------------------------------------------------------------
void _TryToResolveAll (const int     nIndex,
                       const gtuint8 bEqualByte,
                             LPCTSTR pMacro)
//--------------------------------------------------------------------
{
  // build macro evaluation string
  TCHAR sCompleteMacro[128];
  _sntprintf (sCompleteMacro, 128, _T ("%u%s"), nIndex, pMacro);

  // get macro from commandline and add ours
  pstring sMacro = CmdlineParam_GetString (GEEFLAG_MACRO);
  sMacro += sCompleteMacro;

  const bool bResolvedAll = FileDataSeq::Instance ().canResolveAllMacros (sMacro.c_str ());

  out_setcolor (bResolvedAll ? GT_COLOR_LIGHTGREEN : GT_COLOR_LIGHTRED);

  // print info
  out_format (_T ("%4d  %02Xh /macro%s\n"),
              nIndex,
              bEqualByte,
              sMacro.c_str ());
}

/*! Scan the equal bytes for special OpCodes that could be jumps.
 */
//--------------------------------------------------------------------
void WriteByteLookup (const gtuint8* pResBuf,
                      const int nEqualCount,
                      const GEEFormat eFormat)
//--------------------------------------------------------------------
{
  struct SpecialOp
  {
    gtuint8  m_nOpCode;
    gtuint16 m_eFormat;
    LPCTSTR  m_pMacro;
  };

  const SpecialOp OPS[] = {
    { 0x74, ANYFILE | EXE_DOS, _T ("j") },
    { 0x75, ANYFILE | EXE_DOS, _T ("j") },
    { 0xB8, ANYFILE | EXE_DOS, _T ("m") },
    { 0xB9, ANYFILE | EXE_DOS, _T ("m") },
    { 0xBA, ANYFILE | EXE_DOS, _T ("m") },
    { 0xBB, ANYFILE | EXE_DOS, _T ("m") },
    { 0xBC, ANYFILE | EXE_DOS, _T ("m") },
    { 0xBD, ANYFILE | EXE_DOS, _T ("m") },
    { 0xBE, ANYFILE | EXE_DOS, _T ("m") },
    { 0xBF, ANYFILE | EXE_DOS, _T ("m") },
    { 0xE3, ANYFILE | EXE_DOS, _T ("j") },
    { 0xE8, ANYFILE | EXE_DOS, _T ("j") },
    { 0xE8, EXE_PE,            _T ("k") },
    { 0xE9, ANYFILE | EXE_DOS, _T ("j") },
    { 0xE9, EXE_PE,            _T ("r") },
    { 0xEB, ANYFILE | EXE_DOS, _T ("j") },
  };

  out_format (_T ("Scanning %d bytes for special ops\n"), nEqualCount);
  int nFound = 0;
  for (int i = 0; i < nEqualCount; ++i)
  {
    for (size_t j = 0; j < GT_ARRAY_SIZE (OPS); ++j)
    {
      if (OPS[j].m_nOpCode == pResBuf[i] &&
          OPS[j].m_eFormat & eFormat)
      {
        ++nFound;

        // try to resolve macro
        _TryToResolveAll (i,
                          pResBuf[i],
                          OPS[j].m_pMacro);
        break;
      }
    }
  }

  // info if nothing found
  if (nFound == 0)
    out_error_append (_T ("Found no common special operations\n"));

  out_append (_T ("\n"));
}

}  // namespace
