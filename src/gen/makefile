all: dlls

CCDYN_FLAGS  = -D_DLL -DDLL_GEN
#CCSTAT_FLAGS = -D_DLL -DDLL_GEN
ifeq ($(GT_COMPILER), vc)
CCDYN_FLAGS += -LD$(DS)
endif

include ../global.mak

OBJS = \
	$(PATH_OBJ)/gt_fixedname$(OBJ_EXT) \
	$(PATH_OBJ)/gt_gen_analyzer$(OBJ_EXT) \
	$(PATH_OBJ)/gt_gen_main$(OBJ_EXT)
	
ifeq ($(CC_STDAFX_OBJ_NEEDED), 1)
OBJS += $(PATH_OBJ)/gt_stdafx$(OBJ_EXT)
endif

# "External dependency" :)
.PHONY: $(PATH_OBJ)/gt_stdafx$(OBJ_EXT)

DYN_OBJS = $(OBJS:.obj=.dlo)

objs: \
	$(OBJS) \
	$(DYN_OBJS)

dlls: \
	objs \
	$(PATH_BIN)/gtgen$(PS).dll

DLL_OBJS = \
	$(DYN_OBJS) \
	$(PATH_BIN)/gtdata$(PS)$(IMPLIB_EXT) \
	$(PATH_BIN)/gtbasic$(PS)$(IMPLIB_EXT) \
	$(PATH_BIN)/gtexe$(PS)$(IMPLIB_EXT)

$(PATH_BIN)/gtgen$(PS).dll: $(DLL_OBJS)
	@echo Linking $@
	$(DLL) $(LINKFLAGS) $(LINK_WIN) $^ $(DLL_FLAGS)
#	cp $@* $(GTPATH)

include $(DEPENDFILE)
