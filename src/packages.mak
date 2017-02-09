OBJS_STATIC_RES = \
	$(PATH_OBJ)/gtres$(RES_EXT) \

OBJS_STATIC_DATA = \
	$(PATH_OBJ)/gtc_data$(OBJ_EXT) \

OBJS_STATIC_BASIC = \
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
	$(PATH_OBJ)/gt_output$(OBJ_EXT) \
	$(PATH_OBJ)/gt_registry$(OBJ_EXT) \
	$(PATH_OBJ)/gt_stringhelper$(OBJ_EXT) \
	$(PATH_OBJ)/gt_stringloader$(OBJ_EXT) \
	$(PATH_OBJ)/gt_time$(OBJ_EXT) \
	$(PATH_OBJ)/gt_timer$(OBJ_EXT) \
	$(PATH_OBJ)/gt_utils$(OBJ_EXT) \

OBJS_STATIC_ARC = \
	$(PATH_OBJ)/gt_arc_lister$(OBJ_EXT) \
	$(PATH_OBJ)/gt_arc_analyzer$(OBJ_EXT) \
	$(PATH_OBJ)/gt_arc_main$(OBJ_EXT) \

OBJS_STATIC_EXE = \
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

OBJS_STATIC_GEN = \
	$(PATH_OBJ)/gt_fixedname$(OBJ_EXT) \
	$(PATH_OBJ)/gt_gen_analyzer$(OBJ_EXT) \
	$(PATH_OBJ)/gt_gen_main$(OBJ_EXT) \

OBJS_STATIC_OUTPUT = \
	$(PATH_OBJ)/gt_win32_console$(OBJ_EXT) \
	$(PATH_OBJ)/gt_common_output$(OBJ_EXT) \
	$(PATH_OBJ)/gt_output_stdout$(OBJ_EXT) \
	$(PATH_OBJ)/gt_output_html$(OBJ_EXT) \
	$(PATH_OBJ)/gt_output_xml$(OBJ_EXT) \

OBJS_STATIC_APP = \
	$(PATH_OBJ)/gt_cmdline_args$(OBJ_EXT) \
	$(PATH_OBJ)/gt_driveinfo$(OBJ_EXT) \
	$(PATH_OBJ)/gt_paramhandler$(OBJ_EXT) \
	$(PATH_OBJ)/gt_app$(OBJ_EXT) \

ifneq ($(GT_COMPILER), gcc)
OBJS_STATIC_APP += $(PATH_OBJ)/gt_crashtracker$(OBJ_EXT)
endif


## -------------------------------------------------------------------

OBJS_DYNAMIC_ARC = \
	$(PATH_BIN)/gtarc$(PS)$(IMPLIB_EXT)

OBJS_DYNAMIC_EXE = \
	$(PATH_BIN)/gtexe$(PS)$(IMPLIB_EXT)

OBJS_DYNAMIC_GEN = \
	$(PATH_BIN)/gtgen$(PS)$(IMPLIB_EXT)

OBJS_DYNAMIC_OUTPUT = \
	$(PATH_OBJ)/gt_win32_console$(DLO_EXT) \
	$(PATH_OBJ)/gt_common_output$(DLO_EXT) \
	$(PATH_OBJ)/gt_output_stdout$(DLO_EXT) \
	$(PATH_OBJ)/gt_output_html$(DLO_EXT) \
	$(PATH_OBJ)/gt_output_xml$(DLO_EXT) \

OBJS_DYNAMIC_APP = \
	$(PATH_OBJ)/gt_cmdline_args$(DLO_EXT) \
	$(PATH_OBJ)/gt_driveinfo$(DLO_EXT) \
	$(PATH_OBJ)/gt_paramhandler$(DLO_EXT) \
	$(PATH_OBJ)/gt_app$(DLO_EXT) \

ifneq ($(GT_COMPILER), gcc)
OBJS_DYNAMIC_APP += $(PATH_OBJ)/gt_crashtracker$(DLO_EXT)
endif
