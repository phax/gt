#ifndef _GEE_OUTPUT_HXX_
#define _GEE_OUTPUT_HXX_

#include "gee_filedata.hxx"

namespace GEE {

void WriteCStruct (const gtuint8* pResBuf,
                   const int nEqualCount);

void WriteByteLookup (const gtuint8* pResBuf,
                      const int nEqualCount,
                      const GEEFormat eFormat);

}  // namespace

#endif
