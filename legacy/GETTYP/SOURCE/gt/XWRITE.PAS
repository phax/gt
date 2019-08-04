{$i aflags.inc}

unit XWrite;

interface

function HandleCanBeOpened (sFilename:string;
                            bAllowOverwrite:boolean):boolean;
procedure OpenHandle (sFilename:string;
                      bAllowOverwrite:boolean);
procedure CloseHandle;
procedure ChangeHandleToScreen;
procedure FastPrint (s:string);
procedure FastNewLine;
function  ScreenOutput:boolean;

implementation

var
    pOldExitProc:pointer;
    pTest:pointer; { - test pointer whether handle is already assigned - }
    output_handle:text;
    bDoNotSetExitProc,
    bScreen:boolean;

procedure CloseHandle;
begin
  ExitProc := pOldExitProc;

  { - ExitProc should only be changed if pTest is assigned - }
  if (pTest = nil) then
    exit;
  close (output_handle);
  pTest := nil;
end;

{$f+}
procedure ChangeHandleToScreen;
begin
  bDoNotSetExitProc := true;
  CloseHandle;
  OpenHandle ('', true);
end;
{$f-}

function HandleCanBeOpened (sFilename:string;
                            bAllowOverwrite:boolean):boolean;
var f:text;
begin
  assign (f, sFilename);
  reset (f);

  if (ioresult = 0) then
  begin
    close (f);
    HandleCanBeOpened := bAllowOverwrite;
  end
  else
    HandleCanBeOpened := true;
end;

procedure OpenHandle (sFilename:string;
                      bAllowOverwrite:boolean);
begin
  { - if handle is already opened - quit - }
  if (pTest <> nil) then
  begin
    writeln ('internal error: OpenHandle');
    exit;
  end;

  assign (output_handle, sFilename);
  reset (output_handle);

  if (ioresult = 0) and (not bAllowOverwrite) then
  begin
    close (output_handle);
    { - as default connect to con - }
    assign (output_handle, '');
    bScreen := true;
  end
  else
    bScreen := (sFilename = '');

  rewrite (output_handle);

  { - mark handle as assigned - }
  pTest := @output_handle;

  if (not bDoNotSetExitProc) then
  begin
    { - save original exitproc - }
    pOldExitProc := ExitProc;
    ExitProc := @ChangeHandleToScreen;
  end;
end;

procedure FastPrint (s:string);
begin
  if (pTest = nil) then
    exit;
  write (output_handle, s);
end;

procedure FastNewLine;
begin
  if (pTest = nil) then
    exit;
  writeln (output_handle);
end;

function ScreenOutput:boolean;
begin
  ScreenOutput := bScreen;
end;

begin
  pTest := nil;
  bDoNotSetExitProc := false;
end.
