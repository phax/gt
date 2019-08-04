{$i aflags.inc}

unit INIFile;

interface

const
      INI_SEPERATOR = ',';
      INI_COMMENT = ';';

type
     TINIEntry = object
       s:String;

       function Get (n:byte):String;
     end;

     TINIFile = object
       t:text;
       LineCount:word;

       function init (fname:String):boolean;
       procedure done;

       procedure RemoveComment (var line:string);
       function  GetNextEntry (var Name:String;
                               var Entry:TINIEntry):boolean;
       function  GetSection (var line:string):boolean;
       function  SearchSection (sName:string):boolean;
       procedure PrintError (s:String);
     end;

implementation

uses
     XString;

function TINIEntry.Get (n:byte):String;
var StartPos:byte;
begin
  StartPos := NthPos (INI_SEPERATOR, s, n);
  if (StartPos > 0) then
    Get := CopyUntil (StripSpaces (copy (s, StartPos, length (s))), INI_SEPERATOR)
  else
    Get := '';
end;

{ **************************************************************+ }

function TINIFile.init (FName:String):boolean;
begin
  LineCount := 0;

{$ifdef DELPHI}
  AssignFile (t, FName);
{$else}
  assign (t, fname);
{$endif}
  reset (t);

  { kein CheckIOError !!! }
  Init := (ioresult = 0);
end;

procedure TINIFile.done;
begin
{$ifdef DELPHI}
  CloseFile (t);
{$else}
  close (t);
{$endif}
end;

procedure TINIFile.RemoveComment (var line:string);
var p:byte;
begin
  p := pos (INI_COMMENT, line);
  if p > 0 then
    line := copy (line, 1, p - 1);
  line := StripSpaces (line);
end;

function TINIFile.GetNextEntry (var Name:String; var Entry:TINIEntry):boolean;
var line:string;
    bSection:boolean;
begin
  repeat
    inc (LineCount);
    readln (t, line);
    RemoveComment (line);
    bSection := GetSection (line);
  until (line > '') or (eof (t));

  if (line > '') and (not bSection) then
  begin
    GetNextEntry := true;
    Name    := StripSpaces (UpString (CopyUntil (line, '=')));
    Entry.s := StripSpaces (UpString (CopyFrom (line, '=')));
  end
  else
    GetNextEntry := false;
end;

function TINIFile.GetSection (var line:string):boolean;
var p1, p2:byte;
begin
  p1 := pos ('[', line);
  p2 := pos (']', line);
  if (p1 > 0) and (p2 > 0) then
  begin
    GetSection := true;
    line := copy (line, p1 + 1, p2 - p1 - 1);
  end
  else
    GetSection := false;
end;

function TINIFile.SearchSection (sName:string):boolean;
var line:string;
begin
  reset (t);

  repeat
    readln (t, line);
    RemoveComment (line);

    if (GetSection (line)) then
      if (line = sName) then
      begin
        SearchSection := true;
        exit;
      end;
  until (eof (t));

  SearchSection := false;
end;

procedure TINIFile.PrintError (s:String);
begin
  writeln;
  writeln (concat ('Error in configuration file line ', i2s (LineCount), ':'));
  writeln (s);
  writeln;
  halt (1);
end;

end.
