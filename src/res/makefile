all: objs

include ../global.mak

objs: $(PATH_OBJ)/gtres$(RES_EXT)

gtres.rc gt.hrc: gtprf.pl gt.prf gt_extensions.prf gt_filenames.prf
	@perl gtprf.pl

## resource file depends on .rc file
$(PATH_OBJ)/gtres$(RES_EXT): gtres.rc
