#ifndef _GT_LOCALE_HXX_
#define _GT_LOCALE_HXX_

namespace GT {

GT_DECL_BASIC (void) GetEnglishCountryName (const LCID nLCID, pstring &ret);
GT_DECL_BASIC (void) GetEnglishLanguageName (const LCID nLCID, pstring &ret);

}  // namespace

#endif
