all: objs exes

ifeq ($(GT_COMPILER), vc)
LINK_FLAGS = -heap:0x20000 -stack:0x20000
endif

include ../global.mak
include ../packages.mak

## OBJ targets....

OBJS_GT2_STATIC = \
	$(PATH_OBJ)/gt_mainlister$(OBJ_EXT) \
	$(PATH_OBJ)/gt_main$(OBJ_EXT) 

ifeq ($(GT_VC), 11)
OBJS_GT2_STATIC += $(PATH_OBJ)/gt_stdafx$(OBJ_EXT)
endif

OBJS_GT2_DYNAMIC = $(OBJS_GT2_STATIC:.obj=.dlo) 

OBJS_TEST = \
	$(PATH_OBJ)/gt_test$(DLO_EXT) \

ifeq ($(GT_VC), 11)
OBJS_TEST += $(PATH_OBJ)/gt_stdafx$(DLO_EXT)
endif

OBJS_DATA = \
	$(OBJS_DYNAMIC_ARC) \
	$(OBJS_DYNAMIC_EXE) \
	$(OBJS_DYNAMIC_GEN) \
	$(OBJS_GT2_DYNAMIC) \

OBJS_STATIC = \
	$(OBJS_STATIC_RES) \
	$(OBJS_STATIC_DATA) \
	$(OBJS_STATIC_BASIC) \
	$(OBJS_STATIC_ARC) \
	$(OBJS_STATIC_EXE) \
	$(OBJS_STATIC_GEN) \
	$(OBJS_STATIC_OUTPUT) \
	$(OBJS_STATIC_APP) \

OBJS_DYN_APP = \
	$(OBJS_DYNAMIC_OUTPUT) \
	$(OBJS_DYNAMIC_APP) \
	$(PATH_BIN)/gtdata$(PS)$(IMPLIB_EXT) \
	$(PATH_BIN)/gtbasic$(PS)$(IMPLIB_EXT) \

OBJS_PATHMOD = \
	$(PATH_OBJ)/gt_cmdline_args$(DLO_EXT) \
	$(PATH_OBJ)/gt_pathmod$(DLO_EXT)

ifeq ($(GT_VC), 11)
OBJS_PATHMOD += $(PATH_OBJ)/gt_stdafx$(DLO_EXT)
endif

objs: \
	$(OBJS_GT2_STATIC) \
	$(OBJS_GT2_DYNAMIC) \
	$(OBJS_TEST)

# "External dependency" :)
.PHONY: $(PATH_OBJ)/gt_stdafx$(OBJ_EXT)

EXES = \
	$(PATH_BIN)/gt2$(PS).exe \
	$(PATH_BIN)/gt_test$(PS).exe \
	$(PATH_BIN)/gt_pathmod$(PS).exe

ifeq ($(GT_VC), 7)
EXES += $(PATH_BIN)/gt2s$(PS).exe
endif

# VC 8 and higher don't support static linking!

exes: $(EXES)

$(PATH_BIN)/gt2$(PS).exe: $(OBJS_GT2_DYNAMIC) $(OBJS_DYN_APP) $(OBJS_DATA)
	@echo Linking $@
	$(LINK_DYNAMIC) $(LINKFLAGS) $(LINK_CON) $(LINK_FLAGS) $^

$(PATH_BIN)/gt2s$(PS).exe: $(OBJS_GT2_STATIC) $(OBJS_STATIC)
	@echo Linking $@
	$(LINK_STATIC) $(LINKFLAGS) $(LINK_CON) $(LINK_FLAGS) $^

$(PATH_BIN)/gt_test$(PS).exe: $(OBJS_TEST) $(OBJS_DYN_APP)
	@echo Linking $@
	$(LINK_DYNAMIC) $(LINKFLAGS) $(LINK_CON) $(LINK_FLAGS) $^

$(PATH_BIN)/gt_pathmod$(PS).exe: $(OBJS_PATHMOD)
	@echo Linking $@
	$(LINK_DYNAMIC) $(LINKFLAGS) $(LINK_WIN) $^

include $(DEPENDFILE)
