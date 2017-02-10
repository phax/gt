#include "gt_exe_macro.hxx"
#include "gt_exe_pe.hxx"

#include <list>

#include "gt_exception.hxx"
#include "gt_exe_types.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_utils.hxx"

namespace GT {

#if (defined _MSC_VER) || (defined __GNUC__)
#pragma pack (push, 1)
#endif

//--------------------------------------------------------------------
typedef struct _CallRec
//--------------------------------------------------------------------
{
  gtuint8  nOpCode;
  gtuint16 nOffset;
  gtuint16 nSegment;
} CallRec;

//--------------------------------------------------------------------
typedef struct _JumpRec3
//--------------------------------------------------------------------
{
  gtuint8  nOpCode;
  gtuint16 nOffset;
} JumpRec3;

//--------------------------------------------------------------------
typedef struct _JumpRec5
//--------------------------------------------------------------------
{
  gtuint8 nOpCode;
  gtint32 nOffset;
} JumpRec5;

DECLARE_SIZEOF (CALLREC_SIZE,  _CallRec,  5);
DECLARE_SIZEOF (JUMPREC3_SIZE, _JumpRec3, 3);
DECLARE_SIZEOF (JUMPREC5_SIZE, _JumpRec5, 5);

#if (defined _MSC_VER) || (defined __GNUC__)
#pragma pack (pop)
#endif

//--------------------------------------------------------------------
bool GT_CALL EXE_MacroManager::GetMacroPos
                                        (      file_t& nPos,
                                               LPCTSTR sMacro,
                                         const bool    bIsCOM)
//--------------------------------------------------------------------
{
  ASSERT (nPos >= 0);
  ASSERT (sMacro);     // pointer shopuld be valid!
  ASSERT (*sMacro);    // empty macros make no sense ;-)

  // if the 'r' or 'A' macro is contained, m_pSectionTable must be set!
  ASSERT (_tcspbrk (sMacro, _T ("rA")) == NULL || m_pSectionTable);

  // if the 'A' macro is contained, m_pOFH must be set!
  ASSERT (_tcschr (sMacro, _T ('A')) == NULL || m_pOFH);

  bool bNextIsAsm32 = false;
  bool bTraceAsm32 = false;

  // get pointer to first char in macro!
  LPCTSTR p = sMacro;
  while (*p)
  {
    // check for numeric values first (which can contain a minus '-' !!)
    bool bMinus = false;
    if (*p == _T ('-'))
    {
      bMinus = true;
      p = _tcsinc (p);
    }

    // get all numeric chars... remember start position (without the possible minus char)
    LPTSTR pNumberEnd;
    const long nNumber = _tcstol (p, &pNumberEnd, 10);

    // found at least one number??
    if (pNumberEnd > p)
    {
      p = pNumberEnd;

      if (bNextIsAsm32)
      {
        if (bMinus || nNumber == 0)
          return false;

        // number of ASM commands to execute
        if (!GetASM32Pos (nPos, nNumber, bTraceAsm32))
          return false;
      }
      else
      {
        if (bMinus)
          nPos -= nNumber;
        else
          nPos += nNumber;
      }
    }
    else  // no number
    {
      // minus is invalid when no number is following!!
      if (bMinus)
        p = _tcsdec (sMacro, p);

      bNextIsAsm32 = false;
      switch (*p)
      {
        case _T ('#'):
          // just ignore this char
          // can be used as separator between digits
          break;
        case _T ('!'):
          bTraceAsm32 = true;
          break;
        case _T ('A'):
          bNextIsAsm32 = true;
          break;
        case _T ('j'):
        {
          // Opcodes: 0x74, 0x75, 0xE3, 0xE8, 0xE9, 0xEB
          if (!GetJumpPos (nPos, 1))
            return false;
          break;
        }
        case _T ('k'):
        {
          // Opcode: 0xE8
          if (!Get32BitJumpPos (nPos, 1))
            return false;
          break;
        }
        case _T ('m'):
        {
          // Opcode: between 0xB8 and 0xBF
          if (!GetMOVJumpPos (nPos, bIsCOM))
            return false;
          break;
        }
        case _T ('r'):
        {
          if (!Get32BitRVAJumpPos (nPos, 1))
            return false;
          break;
        }
        case _T ('S'):
        {
          // goto char after 'S' -> search for next '%' or EOS
          LPCTSTR pStart = ++p;
          char sSectionName[8];
          memset (sSectionName, 0, sizeof (sSectionName));
          while (*p && (p - pStart) < 8 && *p != _T ('%'))
          {
            sSectionName[p - pStart] = gtuint8 (*p);
            ++p;
          }
          if (!GetSectionPos (nPos, sSectionName))
            return false;
          // go one char back, because it is increased again some lines later
          if (!*p)
            --p;
          break;
        }
        default:
        {
          // found unsupported macro character...
          out_format (rc (HRC_EXELIB_MACRO_INVALID_CHAR),
                      *p);
          break;
        }
      }

      // only increase p here otherwise it was already increased in number loop
      p = _tcsinc (p);
    }
  }

//  PRINTF ("Successfully evaluated %s\n", sMacro);
  return (nPos >= 0 && nPos < m_pBuffer->GetFileSize ());
}

//--------------------------------------------------------------------
bool GT_CALL EXE_MacroManager::GetLongCallPos
                                        (      file_t& nPos,
                                         const file_t  nStartOffset)
//--------------------------------------------------------------------
// Opcodes: 0x9A
// only in 16 bit executables
//--------------------------------------------------------------------
{
  ASSERT (nPos >= 0);

  CallRec aCall;
  if (!m_pBuffer->GetBuffer (nPos, &aCall, CALLREC_SIZE))
    return false;

  // the opcode must be 0xE9!
  if (aCall.nOpCode != 0x9A)
    return false;

  // calculate the new position (offset + c.seg * 16 + c.ofs)
  nPos = nStartOffset + (aCall.nSegment << 4) + aCall.nOffset;
  return true;
}

//--------------------------------------------------------------------
bool GT_CALL EXE_MacroManager::GetJumpPos
                                        (      file_t&  nPos,
                                         const gtuint32 nJumpCount)
//--------------------------------------------------------------------
// Opcodes: 0x74, 0x75, 0xE3, 0xE8, 0xE9, 0xEB
//--------------------------------------------------------------------
{
  ASSERT (nJumpCount > 0);

  // if a negative position is entered -> error!
  if (nPos < 0)
    return false;

  JumpRec3 aJump;

  for (gtuint32 i = 0; i < nJumpCount; i++)
  {
    if (!m_pBuffer->GetBuffer (nPos, &aJump, JUMPREC3_SIZE))
      return false;

    switch (aJump.nOpCode)
    {
      case 0xE8:
      case 0xE9:
      {
        if (aJump.nOffset >= 0x8000 && (nPos + 3 + aJump.nOffset) > m_pBuffer->GetFileSize ())
          nPos += 3 + (gtint16) aJump.nOffset;  // signed -> '-'
        else
          nPos += 3 + (gtuint16) aJump.nOffset;     // unsigned -> '+'
        break;
      }
      case 0x74:
      case 0x75:
      case 0xE3:
      case 0xEB:
      {
        nPos += 2 + ((gtuint8*) &aJump.nOffset)[0];  // make the word a byte array and take the first one ;-)
        break;
      }
      default:
      {
        return false;
      }
    }
  }

  // also allow negative Jumps!!

  return true;
}

//--------------------------------------------------------------------
bool GT_CALL EXE_MacroManager::Get32BitJumpPos
                                        (      file_t&  nPos,
                                         const gtuint32 nJumpCount)
//--------------------------------------------------------------------
// Opcodes: 0xE8
//--------------------------------------------------------------------
{
  if (nPos < 0)
    return false;

  ASSERT (nPos >= 0);
  ASSERT (nJumpCount > 0);

  JumpRec5 aJump;

  for (gtuint32 i = 0; i < nJumpCount; i++)
  {
    if (!m_pBuffer->GetBuffer (nPos, &aJump, JUMPREC5_SIZE))
      return false;

    // offset must be 0xE8
    if (aJump.nOpCode != 0xE8)
      return false;

    nPos += JUMPREC5_SIZE + aJump.nOffset;

    if (nPos > m_pBuffer->GetFileSize () || nPos < 0)
      return false;
  }

  return true;
}

//--------------------------------------------------------------------
bool GT_CALL EXE_MacroManager::Get32BitRVAJumpPos
                                        (      file_t&  nPos,
                                         const gtuint32 nJumpCount)
//--------------------------------------------------------------------
// Opcodes: 0xE9
//--------------------------------------------------------------------
{
  ASSERT (nPos >= 0);
  ASSERT (nJumpCount > 0);
  ASSERT (m_pSectionTable);  // here we need the section table ...

  JumpRec5 aJump;

  // offset 2 RVA
  rva_t nRVA = m_pSectionTable->o2r (gtuint32 (nPos));

  for (gtuint32 i = 0; i < nJumpCount; i++)
  {
    if (!m_pBuffer->GetBuffer (nPos, &aJump, JUMPREC5_SIZE))
      return false;

    // offset must be 0xE9
    if (aJump.nOpCode != 0xE9)
      return false;

    // change RVA
    nRVA += JUMPREC5_SIZE + aJump.nOffset;

    // and back: RVA 2 offset
    nPos = m_pSectionTable->r2o (nRVA);

    if (nPos < 0 || nPos > m_pBuffer->GetFileSize ())
      return false;
  }

  return true;
}

//--------------------------------------------------------------------
bool GT_CALL EXE_MacroManager::GetMOVJumpPos
                                        (      file_t& nPos,
                                         const bool    bIsCOM /* = false */)
//--------------------------------------------------------------------
// Opcodes: 0xB8 .. 0xBF
//--------------------------------------------------------------------
{
  ASSERT (nPos >= 0);

  JumpRec3 aJmp;
  if (!m_pBuffer->GetBuffer (nPos, &aJmp, JUMPREC3_SIZE))
    return false;

  // offset must be between 0xB8 and 0xBF
  if (aJmp.nOpCode < 0xB8 || aJmp.nOpCode > 0xBF)
    return false;

  // is an unsigned value - is it okay?
  nPos += aJmp.nOffset;

  if (bIsCOM)
    nPos -= 0x0100;   // because of COM file's IP...

  return true;
}

//--------------------------------------------------------------------
bool GT_CALL EXE_MacroManager::GetSectionPos
                                        (      file_t& nPos,
                                         const char*   pSectionName)
//--------------------------------------------------------------------
{
  ASSERT (nPos >= 0);
  ASSERT (m_pSectionTable);  // here we need the section table ...

  // find section by name
  EXE_PE_Section* pSection = m_pSectionTable->GetSectionOfName (pSectionName);
  if (!pSection)
    return false;

  // and get physical start offset
  nPos = pSection->nPhysicalOffset;
  return true;
}

//--------------------------------------------------------------------
bool GT_CALL EXE_MacroManager::GetASM32Pos
                                        (      file_t&  nPos,
                                         const gtuint32 nCmdCount,
                                         const bool     bTrace)
//--------------------------------------------------------------------
{
  ASSERT (nPos >= 0);
  ASSERT (m_pSectionTable);  // here we need the section table ...

  size_t n;
  gtuint8 nOpCode;
  gtuint32 nTmpVal;

  union reg_t
  {
    gtuint32 e;
    struct
    {
      gtuint16 l;
      gtuint16 h;
    };
  };

  enum RegIndex
  {
    EAX = 0,
    ECX,
    EDX,
    EBX,
    ESP,
    EBP,
    ESI,
    EDI,
    _REG_LAST
  };

  typedef std::list <gtuint32> stack_t;
#define _PUSH(x)  sp.push_back (x)
#define _POP(x)   if (sp.empty ()) return false; x = sp.back (); sp.pop_back ()
#define _TRACE(x) if (bTrace) out_info_append (_T ("[asm] ") x _T ("\n"))

  // vars
  stack_t sp;
  reg_t regs[_REG_LAST];

  try
  {
    // go there
    m_pBuffer->SetActFilePos (nPos);

    // init all regs
    for (n = 0; n < _REG_LAST; ++n)
      regs[n].e = 0;

    // do it
    for (gtuint32 i = 0; i < nCmdCount; ++i)
    {
      nOpCode = m_pBuffer->GetUint8 ();

      if (nOpCode >= 0x40 && nOpCode <= 0x47)
      {
        // INC r32 [40-47]
        _TRACE (_T ("INC r32"));
        ++regs[nOpCode - 0x40].e;
      }
      else if (nOpCode >= 0x48 && nOpCode <= 0x4F)
      {
        // DEC r32 [48-4F]
        _TRACE (_T ("DEC r32"));
        --regs[nOpCode - 0x48].e;
      }
      else if (nOpCode >= 0x50 && nOpCode <= 0x57)
      {
        // PUSH r32 [50-57]
        _TRACE (_T ("PUSH r32"));
        _PUSH (regs[nOpCode - 0x50].e);
      }
      else if (nOpCode >= 0x58 && nOpCode <= 0x5F)
      {
        // POP r32 [58-5F]
        _TRACE (_T ("POP r32"));
        _POP (regs[nOpCode - 0x58].e);
      }
      else if (nOpCode == 0x60)
      {
        // PUSHAD [60]
        _TRACE (_T ("PUSHAD"));
        //order!
        for (n = 0; n < _REG_LAST; ++n)
        {
          _PUSH (regs[n].e);
        }
      }
      else if (nOpCode == 0x68)
      {
        // PUSH i32 [68]
        _TRACE (_T ("PUSH i32"));
        _PUSH (m_pBuffer->GetInt32 ());
      }
      else if (nOpCode >= 0xB8 && nOpCode <= 0xBF)
      {
        // MOV r32,i32
        _TRACE (_T ("MOV r32,i32"));
        regs[nOpCode - 0xB8].e = m_pBuffer->GetInt32 ();
      }
      else if (nOpCode == 0xC3)
      {
        // RETN [C3]
        _TRACE (_T ("RETN"));
        _POP (nTmpVal);
        nTmpVal -= m_pOFH->nImageBase;
        m_pBuffer->SetActFilePos (m_pSectionTable->r2o (rva_t (nTmpVal)));
      }
      else if (nOpCode == 0xE8)
      {
        // CALL rel32 [E8 cd]
        _TRACE (_T ("CALL rel32"));
        nTmpVal = m_pBuffer->GetInt32 ();
        if (!m_pBuffer->IncActFilePosNoThrow (nTmpVal))
          return false;
      }
      else if (nOpCode == 0xEB)
      {
        // JMP rel8 [EB cb]
        _TRACE (_T ("JMP rel8"));
        nTmpVal = (gtint8) m_pBuffer->GetUint8 ();
        if (!m_pBuffer->IncActFilePosNoThrow (nTmpVal))
          return false;
      }
      else
      {
        if (debug && bTrace)
          out_error_format (_T ("[asm] Invalid opcode 0x%02X\n"), nOpCode);
        return false;
      }
    }

    // all done - save out position
    nPos = m_pBuffer->GetActFilePos ();
    return true;
  }
  catch (const Excpt&)
  {
    // some invalid position inside
    if (debug && bTrace)
      out_error_append (_T ("[asm] exception caught\n"));
    return false;
  }
}

}  // namespace
