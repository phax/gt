#ifndef _GT_STRINGLOADER_HXX_
#define _GT_STRINGLOADER_HXX_

#ifdef _MSC_VER
#pragma once
#endif

// include always :)
#include "gt.hrc"
#include "gt_version.hrc"

namespace GT {

GT_DECL_BASIC (int) GetStringResBuf (resid_t nID, LPTSTR *pMsg);
GT_DECL_BASIC (int) GetStringRes    (resid_t nID, LPTSTR sBuf, size_t nBufLen);

}  // namespace

#endif
