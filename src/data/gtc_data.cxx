#include "gtc_datatypes.hxx"
#include "gt_utils.hxx"  // rc

#include "gt.hrc"

START_EXTERN_C

/* a macro which helps to init two members with the correct size */
#define ENTRY(s)    GT_ARRAY_SIZE (s), s

#include "gtc_ne_0j.cxx"
#include "gtc_pe_0j.cxx"
#include "gtc_pe_ext.cxx"
#include "gtc_exe_0j.cxx"
#include "gtc_exe_1j.cxx"
#include "gtc_exe_2j.cxx"
#include "gtc_exe_1lj.cxx"
#include "gtc_exe_ext.cxx"
#include "gtc_com2exe.cxx"
#include "gtc_sys.cxx"
#include "gtc_fixedname.cxx"
#include "gtc_fixedext.cxx"
#include "gtc_fixedext_filext.cxx"

//--------------------------------------------------------------------
GT_IMPL_DATA (LPCTSTR) GT_GetVersion ()
//--------------------------------------------------------------------
{
  static TCHAR s[] = _T (__DATE__);
  return s;
}

#define _MAKE_ACCESS_FUN(x,t) \
  GT_IMPL_DATA (int) GT_##x##_Count () { return GT_ARRAY_SIZE (a##x); } \
  GT_IMPL_DATA (const t##Entry*) GT_##x##_Index (const unsigned int nIndex) { \
    return (nIndex >= 0 && nIndex < GT_ARRAY_SIZE (a##x) ? &a##x[nIndex] : NULL); \
  }

_MAKE_ACCESS_FUN (C2E, C2E)
_MAKE_ACCESS_FUN (NE_0J, EXE)
_MAKE_ACCESS_FUN (PE_0J, EXE)
_MAKE_ACCESS_FUN (PE_Ext, ExtEXE)
_MAKE_ACCESS_FUN (EXE_0J, EXE)
_MAKE_ACCESS_FUN (EXE_1J, EXE)
_MAKE_ACCESS_FUN (EXE_2J, EXE)
_MAKE_ACCESS_FUN (EXE_1LJ, EXE)
_MAKE_ACCESS_FUN (EXE_Ext, ExtEXE)
_MAKE_ACCESS_FUN (SYS, SYS)
_MAKE_ACCESS_FUN (FixedName, FixedName)
_MAKE_ACCESS_FUN (FixedExtension, FixedExtension)
_MAKE_ACCESS_FUN (FilextDotCom, FixedExtension)

END_EXTERN_C
