#include "gee_output.hxx"
#include "gt_output.hxx"

namespace GEE {

/*! Write the equal bytes as a C struct to file.
 */
//--------------------------------------------------------------------
void WriteCStruct (const gtuint8* pResBuf, const int nEqualCount)
//--------------------------------------------------------------------
{
  // write result as C-array
  out_format (_T ("const gtuint8 __GEEW__ [%d] = {"), nEqualCount);
  for (int i = 0; i < nEqualCount; ++i)
  {
    // break after 12 entries
    if (!(i % 12))
      out_append (_T ("\n "));

    // do not write a ',' after the last entry
    if (i < (nEqualCount - 1))
      out_format (_T (" 0x%02X,"), pResBuf[i]);
    else
      out_format (_T (" 0x%02X"), pResBuf[i]);
  }

  out_append (_T (" };\n\n"));
}

}  // namespace
