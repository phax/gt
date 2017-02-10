all: dlls

CCDYN_FLAGS  = -D_DLL -DDLL_EXE
#CCSTAT_FLAGS = -D_DLL -DDLL_EXE
ifeq ($(GT_COMPILER), vc)
CCDYN_FLAGS += -LD$(DS)
endif

include ../global.mak

OBJS = \
	$(PATH_OBJ)/gt_exe_analyzer$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_le$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_macro$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_main$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_ne$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_pe$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_pe_cert$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_pe_dir$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_pe_export$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_pe_fixups$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_pe_import$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_pe_res$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_pe_sect$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_pe_tls$(OBJ_EXT) \
	
ifeq ($(GT_VC), 11)
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
	$(PATH_BIN)/gtexe$(PS).dll

DLL_OBJS = \
	$(DYN_OBJS) \
	$(PATH_BIN)/gtdata$(PS)$(IMPLIB_EXT) \
	$(PATH_BIN)/gtbasic$(PS)$(IMPLIB_EXT)

$(PATH_BIN)/gtexe$(PS).dll: $(DLL_OBJS)
	@echo Linking $@
	$(DLL) $(LINKFLAGS) $(LINK_WIN) $^ $(DLL_FLAGS)
#	cp $@* $(GTPATH)

include $(DEPENDFILE)
