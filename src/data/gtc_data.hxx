#ifndef _GTC_DATA_HXX_
#define _GTC_DATA_HXX_

#ifdef _MSC_VER
#pragma once
#endif

#include "gtc_datatypes.hxx"

START_EXTERN_C

GT_DECL_DATA (LPCTSTR) GT_GetVersion ();

GT_DECL_DATA (int)                 GT_C2E_Count ();
GT_DECL_DATA (const C2EEntry*)     GT_C2E_Index (const unsigned int nIndex);

GT_DECL_DATA (int)                 GT_NE_0J_Count ();
GT_DECL_DATA (const EXEEntry*)     GT_NE_0J_Index (const unsigned int nIndex);

GT_DECL_DATA (int)                 GT_PE_0J_Count ();
GT_DECL_DATA (const EXEEntry*)     GT_PE_0J_Index (const unsigned int nIndex);
GT_DECL_DATA (int)                 GT_PE_Ext_Count ();
GT_DECL_DATA (const ExtEXEEntry*)  GT_PE_Ext_Index (const unsigned int nIndex);

GT_DECL_DATA (int)                 GT_EXE_0J_Count ();
GT_DECL_DATA (const EXEEntry*)     GT_EXE_0J_Index (const unsigned int nIndex);
GT_DECL_DATA (int)                 GT_EXE_1J_Count ();
GT_DECL_DATA (const EXEEntry*)     GT_EXE_1J_Index (const unsigned int nIndex);
GT_DECL_DATA (int)                 GT_EXE_2J_Count ();
GT_DECL_DATA (const EXEEntry*)     GT_EXE_2J_Index (const unsigned int nIndex);
GT_DECL_DATA (int)                 GT_EXE_1LJ_Count ();
GT_DECL_DATA (const EXEEntry*)     GT_EXE_1LJ_Index (const unsigned int nIndex);
GT_DECL_DATA (int)                 GT_EXE_Ext_Count ();
GT_DECL_DATA (const ExtEXEEntry*)  GT_EXE_Ext_Index (const unsigned int nIndex);

GT_DECL_DATA (int)                 GT_SYS_Count ();
GT_DECL_DATA (const SYSEntry*)     GT_SYS_Index (const unsigned int nIndex);

GT_DECL_DATA (int)                        GT_FixedName_Count ();
GT_DECL_DATA (const FixedNameEntry*)      GT_FixedName_Index (const unsigned int nIndex);
GT_DECL_DATA (int)                        GT_FixedExtension_Count ();
GT_DECL_DATA (const FixedExtensionEntry*) GT_FixedExtension_Index (const unsigned int nIndex);
GT_DECL_DATA (int)                        GT_FilextDotCom_Count ();
GT_DECL_DATA (const FixedExtensionEntry*) GT_FilextDotCom_Index (const unsigned int nIndex);

END_EXTERN_C

#endif
