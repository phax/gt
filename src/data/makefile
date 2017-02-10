all: dlls

CCDYN_FLAGS  = -D_DLL -DDLL_DATA
#CCSTAT_FLAGS = -D_DLL -DDLL_DATA
ifeq ($(GT_COMPILER), vc)
CCDYN_FLAGS += -LD$(DS)
DLL_FLAGS = -noentry -heap:4096 -stack:4096
endif

include ../global.mak

OBJS = \
       $(PATH_OBJ)/gtc_datares$(RES_EXT) \
       $(PATH_OBJ)/gtc_data$(OBJ_EXT) \
       
ifeq ($(CC_STDAFX_OBJ_NEEDED), 1)
OBJS += $(PATH_OBJ)/gt_stdafx$(OBJ_EXT)
endif

# "External dependency" :)
.PHONY: $(PATH_OBJ)/gt_stdafx$(OBJ_EXT)

objs: \
	$(OBJS) \
	$(OBJS:.obj=.dlo)

dlls: \
	objs \
	$(PATH_BIN)/gtdata$(PS)$(DLL_EXT)  \

$(PATH_BIN)/gtdata$(PS)$(DLL_EXT): $(OBJS:.obj=.dlo)
	@echo Linking $@
	@echo $(DLL) $(LINKFLAGS) $(LINK_WIN) $^ $(DLL_FLAGS)
	$(DLL) $(LINKFLAGS) $(LINK_WIN) $^ $(DLL_FLAGS)

## dependency
$(PATH_OBJ)/gtc_datares$(RES_EXT): gtc_datares.rc ../res/gt_version.rc

include $(DEPENDFILE)
