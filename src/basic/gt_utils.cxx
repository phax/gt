#include "gt_utils.hxx"

namespace GT {

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) _ltoc4 (gtuint32 n, LPTSTR s5)
//--------------------------------------------------------------------
{
  LPTSTR p = s5;
  for (int i = 0; i < 4; i++)
  {
    *p = (TCHAR) (n & 0xFF);
    n >>= 8;
    p = _tcsinc (p);
  }
  *p = 0;
}

//--------------------------------------------------------------------
GT_IMPL_BASIC (void) _splitdouble
                                        (double      d,
                                         DoublePair& a,
                                         size_t      nDecimals)
//--------------------------------------------------------------------
{
  gtuint32 nMod = 1;
  while (nDecimals-- > 0)
    nMod *= 10;

  a.h = (gtint32) d;
  d -= a.h;  // only the part after the dot should remain

  // convert all decimal places we want to full number :()
  d *= nMod;
  a.l = gtuint32 (d);

  // round up if the remaining decimal places of d are >= 0.5
  if (d >= 0.5)
  {
    ++a.l;

    // overflow?
    if (a.l == nMod)
    {
      a.l = 0;
      ++a.h;
    }
  }
}

}  // namespace
