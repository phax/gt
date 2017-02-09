all: exes

include ../global.mak
include ../packages.mak

## EXE targets....

exes: \
	$(PATH_BIN)/gt_ffdet$(PS).exe \

_DYN_APP = \
	$(PATH_BIN)/gtdata$(PS)$(IMPLIB_EXT) \
	$(PATH_BIN)/gtbasic$(PS)$(IMPLIB_EXT) \
	$(OBJS_DYNAMIC_OUTPUT) \
	$(OBJS_DYNAMIC_APP)

GT_FFDET_OBJS = \
	$(_DYN_APP) \
	$(PATH_OBJ)/gt_ffdet$(DLO_EXT) \

$(PATH_BIN)/gt_ffdet$(PS).exe: $(GT_FFDET_OBJS)
	@echo Linking $@
	$(LINK_DYNAMIC) $(LINKFLAGS) $(LINK_CON) $^
#	cp $@* $(GTPATH)

include $(DEPENDFILE)
