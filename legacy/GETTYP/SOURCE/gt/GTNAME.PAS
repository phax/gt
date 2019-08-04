{
GTName - comments to files with special names ...

Copyright (c) 1997, 98 by Philip Helger (philip@dke.univie.ac.at)
}

{$i aflags.inc}

unit GTName;

interface

function Check_Name:boolean;

implementation

uses
     GTBuf, GTOutput;

function Check_Name:boolean;

    procedure Print (msg:string);
    begin
      if WriteHeader (msg) then
        FinishLine;
    end;

begin
  Check_Name := true;
       if (b.FName = 'AUTOEXEC.BAT') then Print ('DOS startup file')
  else if (b.FName = 'COMMAND.COM') then  Print ('DOS command interpreter')
  else if (b.FName = 'CONFIG.SYS') then   Print ('DOS startup file')
  else if (b.FName = 'DESCRIPT.ION') then Print ('4DOS directory information file')
  else if (b.FName = 'DIRINFO') then      Print ('Norton Commander directory information file')
  else if (b.FName = 'FILE_ID.DIZ') then  Print ('BBS directory information file')
  else if (b.FName = 'IO.SYS') then       Print ('DOS boot file')
  else if (b.FName = 'MSDOS.SYS') then    Print ('DOS boot file')
  else if (b.FName = 'MIRORSAV.FIL') then Print ('PC Tools Mirror configuration file')
  else if (b.FName = 'MIRROR.FIL') then   Print ('PC Tools Mirror file')
  else if (b.FName = 'STARTUP.CMD') then  Print ('OS/2 startup file')
  else if (b.FName = 'GE.EXE') then       Print ('Oh, it seems to be my small "brother"')
  else if (b.FName = 'GT.EXE') then       Print ('Why are you scanning this file???')
  else if (b.FName = 'GE.CFG') then       Print ('It''s the configuration file of my small "brother"')
  else if (b.FName = 'GT.CFG') then       Print ('It''s my configuration file')
  else if (b.FName = 'WIN.COM') then      Print ('Windows startup file')
  else if (b.FName = 'TREEINFO.NCD') then Print ('Norton Commander directory tree file')
  else if (b.FName = '!!LWSCAN.DIR') then Print ('LWScan directory information file')
  else if (b.FName = '!!LWSCAN.FIL') then Print ('LWScan file information file')
  else if (b.FName = 'MAKEFILE') then     Print ('Makefile for make or nmake')
  else if (b.FName = 'WIN386.SWP') then   Print ('Windows 95/98 swap file')
  else if (b.FName = 'TURBOC.CFG') then   Print ('Turbo C(++) configuration file')
  else Check_Name := false;
end;

end.
