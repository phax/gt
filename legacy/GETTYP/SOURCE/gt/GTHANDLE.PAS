{$i aflags.inc}

unit GTHandle;

interface

implementation

uses
     DOS;

var
    ExitSave:pointer;
    ProcOld1B, ProcOld23:procedure;

const
      CtrlBreakPressed:boolean = false;

{$F+}
procedure CheckCtrlBreak1B;
interrupt;
begin
  CtrlBreakPressed := true;
  ProcOld1B;
  halt (255);
end;

procedure CheckCtrlBreak23;
interrupt;
begin
  CtrlBreakPressed := true;
  ProcOld23;
  halt (255);
end;

procedure NewExit;
begin
  SetIntVec ($1B, @ProcOld1B);
  SetIntVec ($23, @ProcOld23);
  ExitProc := ExitSave;

  if (CtrlBreakPressed) then
    writeln ('Aborted by user');
end;
{$F-}

begin
  ExitSave := ExitProc;
  ExitProc := @NewExit;

  GetIntVec ($1B, @ProcOld1B);
  SetIntVec ($1B, @CheckCtrlBreak1B);

  GetIntVec ($23, @ProcOld23);
  SetIntVec ($23, @CheckCtrlBreak23);
end.
