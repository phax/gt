{$i aflags.inc}

unit GTDebug;

interface

procedure StartLog (s:string);
procedure StopLog;
procedure Log (s:string);
function hexaddr (p:pointer):string;
function heapstr:string;

implementation

uses XString;

var t:text;
    bOpen:boolean;

procedure StartLog (s:string);
begin
  assign (t, s);
  {
  append (t);
  if (ioresult <> 0) then
  }
  rewrite (t);
  bOpen := true;
  Log ('Starting log');
end;

procedure StopLog;
begin
  if (bOpen) then
  begin
    Log ('Closing log');
    close (t);
    bOpen := false;
  end;
end;

procedure Log (s:string);
begin
  if (bOpen) then
  begin
    writeln (t, s);
    flush (t);
  end;
end;

function hexaddr (p:pointer):string;
begin
  hexaddr := hexword (seg (p^)) + ':' + hexword (ofs (p^));
end;

function heapstr:string;
begin
  heapstr := concat (
              's: ', hexword (seg (heaporg^)), ':', hexword (ofs (heaporg^)), ' ',
              'e: ', hexword (seg (heapend^)), ':', hexword (ofs (heapend^)), ' ',
              'a: ', hexword (seg (heapptr^)), ':', hexword (ofs (heapptr^)), ' ');
end;

begin
  bOpen := false;
end.
