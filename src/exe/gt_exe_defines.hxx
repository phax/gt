#ifndef _GT_EXE_DEFINES_HXX_
#define _GT_EXE_DEFINES_HXX_

#ifdef _MSC_VER
#pragma once
#endif

namespace GT {

#define OUTPUT_HEX_8    _T ("%02Xh")
#define OUTPUT_HEX_16   _T ("%04Xh")
#define OUTPUT_HEX_32   _T ("%08Xh")
#define OUTPUT_HEX_64   _T ("%08I64Xh")

#define OUTPUT_HEX_DEC_32   OUTPUT_HEX_32 _T (" / %lu")
#define OUTPUT_HEX_DEC_64   OUTPUT_HEX_64 _T (" / %I64u")

}

#endif
