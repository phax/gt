include ../settings.mak

DEPENDFILE = __depend

##!message Building for compiler $(GT_COMPILER)

ifeq ($(GT_COMPILER), gcc)
include ../comp_gcc.mak
else
include ../comp_vc.mak
endif

#
# -a      extract all debug info (with -x)
# -b POS  specify initial base address
# -f      strip relocations
# -x DIR  extract debug info to seperate dir
#

REBASE     = rebase
REBASE_EXE = @$(REBASE) -f -a -x $(PATH_BIN)\ -b
REBASE_DLL = @$(REBASE) -f -a -x $(PATH_BIN)\ -b

CHECKDEP = ../../tools/checkdep.exe $(CCINCLUDE:-I=--include) --no_console --makefile "--prefix$$" "--prefix(PATH_OBJ)/"

rc_files  := $(wildcard *.rc)
cxx_files := $(wildcard *.cxx)

.PHONY: dep
dep:
	@rm $(DEPENDFILE)
	@echo ### GENERATED - DO NOT EDIT > $(DEPENDFILE)
	$(if $(rc_files),$(CHECKDEP) --suffix.res $(rc_files) >> $(DEPENDFILE),)
	$(if $(rc_files),$(CHECKDEP) --suffix.o   $(rc_files) >> $(DEPENDFILE),)
	$(if $(cxx_files),$(CHECKDEP) --suffix$(DLO_EXT) $(cxx_files) >> $(DEPENDFILE),)
	$(if $(cxx_files),$(CHECKDEP) --suffix$(OBJ_EXT) $(cxx_files) >> $(DEPENDFILE),)
