all: dlls

CCDYN_FLAGS  = -D_DLL -DDLL_BASIC
#CCSTAT_FLAGS = -D_DLL -DDLL_BASIC
ifeq ($(GT_COMPILER), vc)
CCDYN_FLAGS += -LD$(DS)
endif

include ../global.mak

OBJS = \
	$(PATH_OBJ)/gtres$(RES_EXT) \
	$(PATH_OBJ)/gt_analyzer$(OBJ_EXT) \
	$(PATH_OBJ)/gt_cmdline_params$(OBJ_EXT) \
	$(PATH_OBJ)/gt_compare$(OBJ_EXT) \
	$(PATH_OBJ)/gt_configfile$(OBJ_EXT) \
	$(PATH_OBJ)/gt_crc32$(OBJ_EXT) \
	$(PATH_OBJ)/gt_direntry$(OBJ_EXT) \
	$(PATH_OBJ)/gt_errormsg$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exception$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_types$(OBJ_EXT) \
	$(PATH_OBJ)/gt_file$(OBJ_EXT) \
	$(PATH_OBJ)/gt_filebuffer$(OBJ_EXT) \
	$(PATH_OBJ)/gt_filesearch$(OBJ_EXT) \
	$(PATH_OBJ)/gt_linereader$(OBJ_EXT) \
	$(PATH_OBJ)/gt_locale$(OBJ_EXT) \
	$(PATH_OBJ)/gt_output$(OBJ_EXT) \
	$(PATH_OBJ)/gt_registry$(OBJ_EXT) \
	$(PATH_OBJ)/gt_stringhelper$(OBJ_EXT) \
	$(PATH_OBJ)/gt_stringloader$(OBJ_EXT) \
	$(PATH_OBJ)/gt_time$(OBJ_EXT) \
	$(PATH_OBJ)/gt_timer$(OBJ_EXT) \
	$(PATH_OBJ)/gt_utils$(OBJ_EXT)

DYN_OBJS = $(OBJS:.obj=.dlo)

objs: \
	$(OBJS) \
	$(DYN_OBJS)

dlls: \
	objs \
	$(PATH_BIN)/gtbasic$(PS).dll  \

# $^ == $(**)
$(PATH_BIN)/gtbasic$(PS).dll: $(DYN_OBJS)
	@echo Linking $@
	$(DLL) $(LINKFLAGS) $(LINK_WIN) $^ $(DLL_FLAGS)
#	cp $@* $(GTPATH)

include $(DEPENDFILE)
