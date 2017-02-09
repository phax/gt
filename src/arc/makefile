all: dlls

CCDYN_FLAGS  = -D_DLL -DDLL_ARC
#CCSTAT_FLAGS = -D_DLL -DDLL_ARC
ifeq ($(GT_COMPILER), vc)
CCDYN_FLAGS += -LD$(DS)
endif

include ../global.mak

OBJS = \
	$(PATH_OBJ)/gt_arc_lister$(OBJ_EXT) \
	$(PATH_OBJ)/gt_arc_analyzer$(OBJ_EXT) \
	$(PATH_OBJ)/gt_arc_main$(OBJ_EXT) \

DYN_OBJS = $(OBJS:.obj=.dlo)

objs: \
	$(OBJS) \
	$(DYN_OBJS)

dlls: \
	objs \
	$(PATH_BIN)/gtarc$(PS).dll  \

DLL_OBJS = \
	$(DYN_OBJS) \
	$(PATH_BIN)/gtbasic$(PS)$(IMPLIB_EXT)

$(PATH_BIN)/gtarc$(PS).dll: $(DLL_OBJS)
	@echo Linking $@
	$(DLL) $(LINKFLAGS) $(LINK_WIN) $^ $(DLL_FLAGS)
#	cp $@* $(GTPATH)

include $(DEPENDFILE)
