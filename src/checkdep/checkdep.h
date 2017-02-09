#ifndef _CHECKDEP_H_
#define _CHECKDEP_H_

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portability.h"

#ifdef _MSC_VER
#pragma warning (disable:4711)
#endif

#define false                    FALSE
#define true                     TRUE
#define MAX_CHAR                 (8 * 1024)
#define PATH_BUFFER              (8 * 1024)
#define CHECKDEP_VERSION         "1.00"

#ifdef _WIN32
#define PATH_SEP_CHAR            '/'
#define ENV_SEP_CHAR             ';'
#define ENV_SEP_STR              ";"
#else // _WIN32
#define PATH_SEP_CHAR            '/'
#define ENV_SEP_CHAR             ':'
#define ENV_SEP_STR              ":"
#endif // _WIN32

#define MAKEFILE_WIN_PREFIX      "$(OBJPATH)\\"
#define MAKEFILE_WIN_SUFFIX      ".obj"

#define MAKEFILE_LIN_PREFIX      "$(OBJPATH)/"
#define MAKEFILE_LIN_SUFFIX      ".o"

#define GEN_FILE_SUFFIX          "_gen.hxx"

#define CHECKDEP_SIZEOF(x)   (sizeof (x) / sizeof (x[0]))
#define CHECKDEP_ASSERT(b)   __CheckdepAssert(b, #b, __FILE__, __LINE__)

#endif
