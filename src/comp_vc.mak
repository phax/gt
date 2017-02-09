ifeq ($(GT_VC), 6)
CCDIR = $(MSVCDIR)
endif

ifeq ($(GT_VC), 7)
CCDIR = $(MSVCDir)
endif

ifeq ($(GT_VC), 8)
CCDIR = $(VCINSTALLDIR)
endif

ifeq ($(GT_VC), 9)
CCDIR = $(VCINSTALLDIR)
endif

CC   = cl.exe
LINK = link.exe
LIB  = lib.exe -nologo
#CC   = "$(CCDIR)\bin\cl.exe"
#LINK = "$(CCDIR)\bin\link.exe"
#LIB  = "$(CCDIR)\bin\lib.exe" -nologo
DLL := $(LINK) -DLL
################ -IMPLIB:$*.lib

DLO_EXT = .dlo
OBJ_EXT = .obj
RES_EXT = .res
DLL_EXT = .dll
IMPLIB_EXT = .lib


######################################################################
## Allow a static build?
##

BUILD_STATIC = 1

ifeq ($(GT_VC), 8)
BUILD_STATIC = 0
endif

ifeq ($(GT_VC), 9)
BUILD_STATIC = 0
endif


######################################################################
##
## directory settings
##

PATH_BASE = ../../target/$(GT_VC)
PATH_BIN  = $(PATH_BASE)/bin$(PS)
PATH_OBJ  = $(PATH_BASE)/obj$(PS)

# ensure that the dirs exist
.PHONY: dirs
dirs:
	@test -d $(PATH_BASE) || mkdir $(PATH_BASE)
	@test -d $(PATH_BIN)  || mkdir $(PATH_BIN)
	@test -d $(PATH_OBJ)  || mkdir $(PATH_OBJ)

######################################################################
##
## generic compiler settings
##

## set some global variables which are independed of the compiler!
ifneq ($(GT_DEBUG), 0)

ifeq ($(GT_VC), 6)
CCDEBUG = -Zi  -GZ -Ge
endif

ifeq ($(GT_VC), 7)
CCDEBUG = -Zi -GZ -Ge -RTCcsu -GS
endif

ifeq ($(GT_VC), 8)
CCDEBUG = -Zi -RTCcsu -GS -RTC1
endif

ifeq ($(GT_VC), 9)
CCDEBUG = -Zi -RTCcsu -GS -RTC1
endif

LINKDEBUG = -debug -profile

else  # GT_DEBUG != 0

CCDEBUG = -O2 -Oi -Ot -Gs -Ob2
LINKDEBUG = -release -incremental:no

ifeq ($(GT_VC), 6)
CCDEBUG += -Og
endif

ifeq ($(GT_VC), 7)
CCDEBUG += -Og
endif

ifneq ($(GT_RELEASE), 1)
CCDEBUG += -Zi
LINKDEBUG = -profile -debug
endif # GT_RELEASE == 1

endif # GT_DEBUG != 0

## it is essential for wxWindows to have the "WIN32" symbol defined
C_PREPROC = \
	-D_WIN32 \
	-DWIN32 \
	-DOCLEMU_USE_STLPORT \
	-D_STL=std \
	-DWIN32_LEAN_AND_MEAN \
	-DNOCLIPBOARD \
	-DNOMCX \
	-DNOCOMM \
	-DNOPROFILER \
	-DNORASTEROPS \
	-DNOGDICAPMASKS \
	-DNOCRYPT \
	-DNOSERVICE \
	-DNOKANJI \
	-DNOSOUND \
	-DSTRICT

ifeq ($(GT_UNICODE), 1)
C_PREPROC += \
	-DUNICODE \
	-D_UNICODE \
	-D_USE_INLINING \
	-DwxUSE_UNICODE=1
endif

ifeq ($(GT_RELEASE), 1)
C_PREPROC += \
	-DGT_RELEASE
endif

ifneq ($(GT_DEBUG), 0)
C_PREPROC += \
	-DGT_DEBUG \
	-D__WXDEBUG__ \
	-D_DEBUG
endif

CCINCLUDE = \
	-I. \
	-I..\\00 \
	-I..\\res \
	-I..\\basic \
	-I..\\data \
	-I..\\arc \
	-I..\\exe \
	-I..\\gen \
	-I..\\output \
	-I..\\app \
	"-I$(CCDIR)\\include"

ifeq ($(GT_WX), 1)
CCINCLUDE += "-I$(WXWIN)\\include"
endif

# -c: compile only
# -nologo: no compiler copyright
# -W3: warning level 3
# -G5: optimize for Pentium
# -G6: optimize for PPro, PII and PIII
# -GF: readonly string pooling
# -Gz: _stdcall
# -GX: exception handling
# -Oy-: disable frame pointer omission
# -GR: enable RTTI
# -Wp64: enable 64bit porting warnings
# -WL: enable one-line diagnostics
# -Zc:forScope: enforce Standard C++ for scoping rules
_COMPILER_FLAGS = \
	-c \
	-nologo \
	-W3 \
	-GF \
	-Oy- \
	-GR \
	"-Fd$(PATH_BIN)/_gt.pdb" \
	$(CCINCLUDE) \
	$(C_PREPROC) \
	$(CCDEBUG) \

# -Gz \

ifeq ($(GT_VC), 6)
COMPILER_FLAGS = $(_COMPILER_FLAGS) \
	-G6 \
	-GX
endif

ifeq ($(GT_VC), 7)
COMPILER_FLAGS = $(_COMPILER_FLAGS) \
	-G6 \
	-GX \
	-Wp64 \
	-WL \
	-Zc:forScope
endif

ifeq ($(GT_VC), 8)
COMPILER_FLAGS = $(_COMPILER_FLAGS) \
	-EHsc \
	-Wp64 \
	-WL \
	-Zc:forScope
endif

ifeq ($(GT_VC), 9)
COMPILER_FLAGS = $(_COMPILER_FLAGS) \
	-EHsc \
	-WL \
	-Zc:forScope
endif

######################################################################
##
## wxWindows settings
##

ifeq ($(GT_UNICODE), 1)
WX_SUFFIX := $(WX_SUFFIX)u
endif

ifeq ($(GT_DEBUG), 1)
WX_SUFFIX := $(WX_SUFFIX)d
endif

######################################################################
##
## PCH file stuff
##

# the relative path must be equal while creating and using and including!
_PCHFILE_DYNAMIC = $(PATH_OBJ)/_gt_dynamic.pch
_PCHFILE_STATIC  = $(PATH_OBJ)/_gt_static.pch

ifneq ($(GT_WX), 1)
# the PCH include file must be the first include file!
CC_USE_PCH = \
	-FIgt_stdafx.hxx \
	-Yugt_stdafx.hxx \
	-FIgt_dll.hxx

CC_USE_PCH_DYNAMIC = $(CC_USE_PCH) "-Fp$(_PCHFILE_DYNAMIC)"
CC_USE_PCH_STATIC  = $(CC_USE_PCH) "-Fp$(_PCHFILE_STATIC)"

CC_BUILD_PCH_DYNAMIC = -Ycgt_stdafx.hxx "-Fp$(_PCHFILE_DYNAMIC)"
CC_BUILD_PCH_STATIC  = -Ycgt_stdafx.hxx "-Fp$(_PCHFILE_STATIC)"
endif

######################################################################
##
## linker flags
##

ifeq ($(GT_VC), 6)
LINK_WIN = -subsystem:windows,4.0
LINK_CON = -subsystem:console,4.0
endif

ifeq ($(GT_VC), 7)
LINK_WIN = -subsystem:windows,4.0
LINK_CON = -subsystem:console,4.0
endif

ifeq ($(GT_VC), 8)
LINK_WIN = -subsystem:windows,4.0
LINK_CON = -subsystem:console,4.0
endif

ifeq ($(GT_VC), 9)
LINK_WIN = -subsystem:windows,5.0
LINK_CON = -subsystem:console,5.0
endif

## these options disable profiling of double values???
# -merge:.rdata=.text
# -merge:.data=.text
# -section:.text,EWR
# -ignore:4078
LINKFLAGS = \
	-release \
	-ignore:4001 \
	-machine:ix86 \
	-map \
	-mapinfo:exports \
	-nodefaultlib \
	-nologo \
	-out:$@ \
	-version:0.36 \
	advapi32.lib \
	kernel32.lib \
	user32.lib \
	shell32.lib \
	$(LINKDEBUG)

ifeq ($(GT_VC), 6)
LINKFLAGS += "-libpath:$(CCDIR)\lib" oldnames.lib
endif

ifeq ($(GT_VC), 7)
LINKFLAGS += "-libpath:$(CCDIR)\lib" "-libpath:$(CCDIR)\PlatformSDK\lib"
endif

ifeq ($(GT_VC), 8)
LINKFLAGS += "-libpath:$(CCDIR)\lib" "-libpath:$(CCDIR)\PlatformSDK\lib" -SAFESEH
endif

ifeq ($(GT_VC), 9)
LINKFLAGS += "-libpath:$(CCDIR)\lib" "-libpath:$(WINDOWSSDKDIR)\lib" -SAFESEH
endif

ifeq ($(GT_VC), 6)
_LINK_DYNAMIC = msvcrt$(DS).lib msvcirt$(DS).lib
_LINK_STATIC  = libc$(DS).lib   libci$(DS).lib
else
_LINK_DYNAMIC = msvcrt$(DS).lib msvcprt$(DS).lib
_LINK_STATIC =  libc$(DS).lib   libcp$(DS).lib
endif

ifeq ($(GT_VC), 6)
LINKFLAGS += -mapinfo:fixups \
             -mapinfo:lines
endif

ifeq ($(GT_VC), 7)
LINKFLAGS += -mapinfo:fixups \
             -mapinfo:lines
endif

## DLLs are only dynamic
DLL += $(_LINK_DYNAMIC)

CC_DYNAMIC   = $(CC) $(EXTENSION_FLAGS) $(COMPILER_FLAGS) -MD$(DS) -Fo$@
LINK_DYNAMIC = $(LINK) $(_LINK_DYNAMIC)

ifeq ($(BUILD_STATIC), 1)
CC_STATIC   = $(CC) $(EXTENSION_FLAGS) $(COMPILER_FLAGS) -ML$(DS) -Fo$@
LINK_STATIC = $(LINK) $(_LINK_STATIC)
else
CC_STATIC   = @echo NO STATIC
LINK_STATIC = @echo NO STATIC
endif

$(PATH_OBJ)/%$(DLO_EXT) : %.cxx
	$(CC_DYNAMIC) $(CCDYN_FLAGS) $< $(CC_USE_PCH_DYNAMIC)

$(PATH_OBJ)/%$(OBJ_EXT) : %.cxx
	$(CC_STATIC) $(CCSTAT_FLAGS) $< $(CC_USE_PCH_STATIC)

%.i : %.cxx
	$(CC_DYNAMIC) $(CCDYN_FLAGS) $< $(CC_USE_PCH_DYNAMIC) -P

######################################################################
##
## resource compiler
## The /c flag with 1252 specifies ANSI input!
##

RC_FLAGS = \
	-DGT_SUFFIX=$(PS)

ifneq ($(GT_DEBUG), 0)
RC_FLAGS += -D_DEBUG
endif

$(PATH_OBJ)/%$(RES_EXT): %.rc
	rc $(RC_FLAGS) $(CCINCLUDE:-I=-i) /c 1252 /fo $@ $<
