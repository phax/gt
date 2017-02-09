#include "gt_output.hxx"

namespace GT {

OutputFuncPtrs g_aOutputFuncs = { NULL };

/*! Set a new bunch of output function.
    None of them may be NULL.
 */
//--------------------------------------------------------------------
GT_EXPORT_BASIC void GT_CALL SetOutputFuncPtrs
                                        (const OutputFuncPtrs& ofp)
//--------------------------------------------------------------------
{
  const size_t nElements = (sizeof (OutputFuncPtrs) / sizeof (T_out_init));
  for (size_t i = 0; i < nElements; ++i)
  {
    // check if function is NULL -> may not happen
    if (((T_out_init*) &ofp)[i] == NULL)
    {
      ASSERT (0);
      return;
    }
  }

  // save to set
  g_aOutputFuncs = ofp;
}

/*! Get set of output function ptrs.
    Never alter the container function ptrs - READONLY!!
 */
//--------------------------------------------------------------------
GT_EXPORT_BASIC const OutputFuncPtrs& GT_CALL GetOutputFuncPtrs ()
//--------------------------------------------------------------------
{
  ASSERT (g_aOutputFuncs._init != NULL);
  return g_aOutputFuncs;
}

}  // namespace
