#ifndef _GT_DLL_HXX_
#define _GT_DLL_HXX_

#ifdef _MSC_VER
#pragma once
#endif

// VC 2003 WinNT.h has no define for DECLSPEC_EXPORT -> copied from import
#if (defined _MSC_VER) && (!defined DECLSPEC_EXPORT)
#if (defined(_M_IX86) || defined(_M_IA64) || defined(_M_AMD64)) && !defined(MIDL_PASS)
#define DECLSPEC_EXPORT __declspec(dllexport)
#else
#define DECLSPEC_EXPORT
#endif
#endif

// _DLL is defined if /MD[d] is used
// There is no switch to find out when /LD[d] is used :(
#ifdef _DLL
#define _GT_EXPORT    DECLSPEC_EXPORT
#define _GT_IMPORT    DECLSPEC_IMPORT
#else
#define _GT_EXPORT    /**/
#define _GT_IMPORT    /**/
#endif

#ifdef DLL_BASIC
#define GT_EXPORT_BASIC  _GT_EXPORT
#else
#define GT_EXPORT_BASIC  _GT_IMPORT
#endif

#define GT_DECL_BASIC(_rettype) GT_EXPORT_BASIC _rettype GT_CALL
#define GT_IMPL_BASIC(_rettype) GT_EXPORT_BASIC _rettype GT_CALL


#ifdef DLL_DATA
#define GT_EXPORT_DATA  _GT_EXPORT
#else
#define GT_EXPORT_DATA  _GT_IMPORT
#endif

#define GT_DECL_DATA(_rettype) GT_EXPORT_DATA _rettype GT_CALL
#define GT_IMPL_DATA(_rettype) GT_EXPORT_DATA _rettype GT_CALL


#ifdef DLL_GEN
#define GT_EXPORT_GEN  _GT_EXPORT
#else
#define GT_EXPORT_GEN  _GT_IMPORT
#endif


#ifdef DLL_ARC
#define GT_EXPORT_ARC  _GT_EXPORT
#else
#define GT_EXPORT_ARC  _GT_IMPORT
#endif


#ifdef DLL_EXE
#define GT_EXPORT_EXE  _GT_EXPORT
#else
#define GT_EXPORT_EXE  _GT_IMPORT
#endif

#endif
