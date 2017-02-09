ifndef DEBUG
DEBUG = 0
endif

ifdef OS
WIN      = 1
OBJ      = .obj
EXE      = .exe
TOOLSDIR = ../../tools/
else
WIN      = 0
OBJ      = .o
EXE      =
TOOLSDIR = ../../tools
endif

ifeq ($(DEBUG), 1)
ifdef OS
CD = -Zi -GZ
LD = -debug
else  # OS
CD = -g
LD = -lstdc++
endif  # OS
DS = d
else  # DEBUG
ifdef OS
CD = -Ox -Ob2
LD = -release -profile
else # OS
CD =
LD = -lstdc++
endif # OS
DS =
endif # DEBUG

.SUFFIXES: .cxx .c $(OBJ)

# all object files:
_OBJ_FILES = checkdep$(OBJ)

# main target
all: checkdep$(EXE)

# executable:
checkdep$(EXE): $(_OBJ_FILES)
	@echo linking $@
ifdef OS
	@link $(_OBJ_FILES) $(LD) -out:$@ -nodefaultlib -nologo kernel32.lib msvcrt$(DS).lib msvcprt$(DS).lib
else
	@gcc $(_OBJ_FILES) $(LD) -o $@
endif
	@echo Copying to tools dir
ifneq ($(DEBUG), 1)
	@cp $@ $(TOOLSDIR)
ifdef OS
	@cp $@* $(TOOLSDIR)
endif
endif

# dependencies
checkdep$(OBJ): checkdep.cxx checkdep.h portability.h

# build rules
.cxx.obj:
	@cl -c -MD$(DS) $(CD) $< -Fo$@ -nologo -TP -EHsc

.cxx.o:
	@echo $<
	@gcc -c $< -o$@ -Wall $(CD)

## clean
clean:
	@rm -f $(_OBJ_FILES) checkdep$(EXE)
