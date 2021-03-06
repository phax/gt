# VC compiler version (7, 8, 9, 10, 11)
# Is set in the _make_*.cmd files
ifndef GT_VC
GT_VC = 11
endif

ifndef GT_RELEASE
GT_RELEASE=0
endif

ifeq ($(GT_RELEASE),1)
GT_DEBUG=0
endif

ifndef GT_DEBUG
GT_DEBUG=1
endif

ifndef GT_UNICODE
GT_UNICODE=1
endif

## DEBUG?
ifneq ($(GT_DEBUG),0)
DS = d
else
DS =
endif

## UNICODE?
ifeq ($(GT_UNICODE),0)
PS =
else
PS = u
endif

## RELEASE?
ifeq ($(GT_RELEASE),1)
PS := $(PS)r
else
PS := $(PS)$(DS)
endif

## set default compiler
ifndef GT_COMPILER
GT_COMPILER=vc
endif
