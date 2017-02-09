#ifndef _GT_BASIC_HXX_
#define _GT_BASIC_HXX_

#ifdef _MSC_VER
#pragma once
#endif

// needed for $(OBJ_EXT) files that don't have PCHs
#ifndef _DLL
#include "gt_stdafx.hxx"
#include "gt_dll.hxx"
#endif

/*! The rc macro converts a number to an LPCTSTR - so it is read from the RC file!
 */
#define rc(x) reinterpret_cast <LPCTSTR> (x)

#ifdef _MSC_VER
#define heapchk ASSERT (_CrtCheckMemory ())
#else
#define heapchk /* empty */
#endif

#define PERCENTAGE(p,t)        ((t) != 0 ? (double (p) / double (t) * 100.0) : 0)

// is x aligned to n bytes?
#define IS_ALIGNED(x,n)  (!(x & (n - 1)))

#define ALIGN_2(x)   ((x & 0x01) ? x + 0x01 : x)
#define ALIGN_4(x)   ((x & 0x03) ? x + (0x04 - (x & 0x03)) : x)
#define ALIGN_8(x)   ((x & 0x07) ? x + (0x08 - (x & 0x07)) : x)
#define ALIGN_16(x)  ((x & 0x0f) ? x + (0x10 - (x & 0x0f)) : x)


namespace GT {

inline gtuint16 WORD_SWAP_BYTE (gtuint16 n)
{
  return ((n & 0x00FF) << 8) +
         ((n & 0xFF00) >> 8);
}

inline gtuint32 LONG_SWAP_WORD (gtuint32 n)
{
  return ((n & 0x0000FFFF) << 16) +
         ((n & 0xFFFF0000) >> 16);
}

inline gtuint32 LONG_SWAP_BYTE (gtuint32 n)
{
  return ((n & 0x000000FF) << 24) +
         ((n & 0x0000FF00) <<  8) +
         ((n & 0x00FF0000) >>  8) +
         ((n & 0xFF000000) >> 24);
}

inline gtuint8 HI_BYTE (gtuint16 n)
{
  return gtuint8 ((n & 0xFF00) >> 8);
}

inline gtuint8 LO_BYTE (gtuint16 n)
{
  return gtuint8 (n & 0x00FF);
}

inline gtuint16 HI_WORD (gtuint32 n)
{
  return gtuint16 (n >> 16);
}

inline gtuint16 LO_WORD (gtuint32 n)
{
  return gtuint16 (n & 0x0000FFFF);
}

inline gtuint32 LO_DWORD (gtuint64 n)
{
  return gtuint32 (n & 0x000000FFFFFFFF);
}

inline gtuint32 HI_DWORD (gtuint64 n)
{
  return gtuint32 (n >> 32);
}

//--------------------------------------------------------------------
struct DoublePair
//--------------------------------------------------------------------
{
  gtint32  h;   // part before the dot
  gtuint32 l;   // part after the dot
};

// global constants
const size_t GT_MAX_SIZE = 1024;

// global functions, declared extern
GT_DECL_BASIC (gtint32)  _otol (char *s, int nLength);   /* octal to decimal */
GT_DECL_BASIC (void)     _ltoc4 (gtuint32 n, LPTSTR s5);
GT_DECL_BASIC (void)     _splitdouble (double d, DoublePair &a, size_t nDecimals);

// global inline functions

//! maximum function
template <class T> inline T _max (const T a, const T b) { return (a > b ? a : b); }

//! minimum function
template <class T> inline T _min (const T a, const T b) { return (a < b ? a : b); }

//! absolute function
template <class T> inline T _abs (const T a) { return (a < 0 ? -a : a); }

//! generic align function
template <class T> inline void _align (T& a, int n) { if ((a % n) != 0) a = T (((a / n) + 1) * n); }

}  // namespace

// all other include files
#include "gt_stringloader.hxx"
#include "gt_stringhelper.hxx"

#endif
