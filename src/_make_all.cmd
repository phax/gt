@echo off
call _make_vs7.cmd GT_DEBUG=0 GT_UNICODE=0
call _make_vs7.cmd GT_DEBUG=0 GT_UNICODE=1
call _make_vs7.cmd GT_DEBUG=1 GT_UNICODE=0
call _make_vs7.cmd GT_DEBUG=1 GT_UNICODE=1

call _make_vs8.cmd GT_DEBUG=0 GT_UNICODE=0
call _make_vs8.cmd GT_DEBUG=0 GT_UNICODE=1
call _make_vs8.cmd GT_DEBUG=1 GT_UNICODE=0
call _make_vs8.cmd GT_DEBUG=1 GT_UNICODE=1

call _make_vs9.cmd GT_DEBUG=0 GT_UNICODE=0
call _make_vs9.cmd GT_DEBUG=0 GT_UNICODE=1
call _make_vs9.cmd GT_DEBUG=1 GT_UNICODE=0
call _make_vs9.cmd GT_DEBUG=1 GT_UNICODE=1

call _make_vs10.cmd GT_DEBUG=0 GT_UNICODE=0
call _make_vs10.cmd GT_DEBUG=0 GT_UNICODE=1
call _make_vs10.cmd GT_DEBUG=1 GT_UNICODE=0
call _make_vs10.cmd GT_DEBUG=1 GT_UNICODE=1

call _make_vs11.cmd GT_DEBUG=0 GT_UNICODE=0
call _make_vs11.cmd GT_DEBUG=0 GT_UNICODE=1
call _make_vs11.cmd GT_DEBUG=1 GT_UNICODE=0
call _make_vs11.cmd GT_DEBUG=1 GT_UNICODE=1
