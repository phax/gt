all: objs

include ../global.mak

OBJS = \
	$(PATH_OBJ)/gt_cmdline_args$(OBJ_EXT) \
	$(PATH_OBJ)/gt_driveinfo$(OBJ_EXT) \
	$(PATH_OBJ)/gt_paramhandler$(OBJ_EXT) \
	$(PATH_OBJ)/gt_app$(OBJ_EXT)

ifneq ($(GT_COMPILER), gcc)
OBJS += $(PATH_OBJ)/gt_crashtracker$(OBJ_EXT)
endif

objs: \
	$(OBJS) \
	$(OBJS:.obj=.dlo)
	
include $(DEPENDFILE)
