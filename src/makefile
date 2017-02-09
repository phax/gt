all: dirs

include settings.mak

_MAKE = $(MAKE) dirs all GT_DEBUG=$(GT_DEBUG) GT_UNICODE=$(GT_UNICODE) GT_RELEASE=$(GT_RELEASE) GT_COMPILER=$(GT_COMPILER)


_DIRS = \
	00 \
	res \
	data \
	basic \
	arc \
	exe \
	gen \
	output \
	app \
	main_gt2 \
	main_geew \
	main_tools

# CppUnit only compiled for VC 2008
#ifeq ($(GT_VC), 9)
#_DIRS += main_test
#endif

.PHONY: dirs dep

# DIRSCMD contains the complete command to be executed
DIRSCMD := $(foreach dir, $(_DIRS), echo $(dir) && $(MAKE) -s -C $(dir) &&) echo Done
dirs:
	@$(DIRSCMD)

# DEPCMD contains the complete command to be executed
DEPCMD := $(foreach dir, $(_DIRS), echo $(dir) && $(MAKE) -s -C $(dir) dep &&) echo Done
dep:
	@$(DEPCMD)
