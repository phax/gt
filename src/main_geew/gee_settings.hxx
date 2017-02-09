#ifndef _GEE_SETTINGS_HXX_
#define _GEE_SETTINGS_HXX_

// loopkup some jumps automatically
#define GEEFLAG_RESLOOKUP _T ("reslookup")

// set number of bytes to compare
#define GEEFLAG_CMPLEN   _T ("cmplen")
const int DEFAULT_CMPLEN = 80;

#define GEEFLAG_MAXOPENFILES  _T ("maxopen")
const int DEFAULT_MAXOPENFILES = 1024;

// set the macro to be evaluated
#define GEEFLAG_MACRO    _T ("macro")

#endif
