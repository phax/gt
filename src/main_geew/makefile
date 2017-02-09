all: exes

include ../global.mak
include ../packages.mak

OBJS = \
	$(PATH_OBJ)/gee_filedata$(OBJ_EXT) \
	$(PATH_OBJ)/gee_output_c$(OBJ_EXT) \
	$(PATH_OBJ)/gee_output_opcode$(OBJ_EXT) \
	$(PATH_OBJ)/geew$(OBJ_EXT) \

## EXE targets....

exes: \
	$(PATH_BIN)/geew$(PS).exe

ifneq ($(GT_VC), 8)
exes += $(PATH_BIN)/geews$(PS).exe
endif

IMPORT_LIBS = \
	$(PATH_BIN)/gtexe$(PS)$(IMPLIB_EXT) \
	$(PATH_BIN)/gtbasic$(PS)$(IMPLIB_EXT) \
	$(PATH_BIN)/gtdata$(PS)$(IMPLIB_EXT) \

GEEW_DLO_OBJS = \
	$(OBJS_DYNAMIC_OUTPUT) \
	$(OBJS_DYNAMIC_APP) \
	$(PATH_BIN)/gtexe$(PS)$(IMPLIB_EXT) \
	$(PATH_BIN)/gtbasic$(PS)$(IMPLIB_EXT) \
	$(OBJS:.obj=.dlo) \

GEEW_STATIC_OBJS = \
	$(OBJS_STATIC_RES) \
	$(OBJS_STATIC_BASIC) \
	$(PATH_OBJ)/gt_exe_macro$(OBJ_EXT) \
	$(PATH_OBJ)/gt_exe_pe_sect$(OBJ_EXT) \
	$(OBJS_STATIC_OUTPUT) \
	$(OBJS_STATIC_APP) \
	$(OBJS)

$(PATH_BIN)/geew$(PS).exe: $(GEEW_DLO_OBJS)
	@echo Linking $@
	$(LINK_DYNAMIC) $(LINKFLAGS) $(LINK_CON) $^
#	cp $@* $(GTPATH)

$(PATH_BIN)/geews$(PS).exe: $(GEEW_STATIC_OBJS)
	@echo Linking $@
	$(LINK_STATIC) $(LINKFLAGS) $(LINK_CON) $^
#	cp $@* $(GTPATH)

include $(DEPENDFILE)
