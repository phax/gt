all: dirs objs

include ../global.mak

_OBJS = \
	$(PATH_OBJ)/gt_stdafx$(OBJ_EXT) \

objs: \
	$(_OBJS) \
	$(_OBJS:.obj=.dlo)

# no extension flags!
$(PATH_OBJ)/gt_stdafx$(DLO_EXT): gt_stdafx.cxx gt_stdafx.hxx ../global.mak
	$(CC_DYNAMIC) gt_stdafx.cxx $(CC_BUILD_PCH_DYNAMIC)

# no extension flags!
$(PATH_OBJ)/gt_stdafx$(OBJ_EXT): gt_stdafx.cxx gt_stdafx.hxx ../global.mak
	$(CC_STATIC) gt_stdafx.cxx $(CC_BUILD_PCH_STATIC)

include $(DEPENDFILE)
