all: objs

include ../global.mak

OBJS = \
	$(PATH_OBJ)/gt_win32_console$(OBJ_EXT) \
	$(PATH_OBJ)/gt_common_output$(OBJ_EXT) \
	$(PATH_OBJ)/gt_output_stdout$(OBJ_EXT) \
	$(PATH_OBJ)/gt_output_html$(OBJ_EXT) \
	$(PATH_OBJ)/gt_output_xml$(OBJ_EXT) \

objs: \
	$(OBJS) \
	$(OBJS:.obj=.dlo)
	
include $(DEPENDFILE)
