{$i aflags.inc}

unit XCmdLine;

interface

const
      cmdlinelength = 127;
      Delimiter = '"';

type
     PCmdString = ^TCmdString;
     TCmdString = string [CmdLineLength];

     TParamStr = string[12];

     PCmdLine=^TCmdLine;
     TCmdLine=object
       function IsOptionString (index:byte):boolean;
       function IsNonOptionString (index:byte):boolean;
       function IsFlag (s:TParamStr):boolean;
       function IsDisabled (s:TParamStr):boolean;
       function IsEnabled (s:TParamStr):boolean;

       { check only NOT option strings }
       function GetOptionStringCount:byte;
       function GetOptionString (index:byte):TCmdString;

       { check only option strings ("/" and "-") }
       function GetNonOptionStringCount:byte;
       function GetNonOptionString (index:byte):TCmdString;

       { don't differ between option and not }
       function GetParam (index:byte):TCmdString;

       function GetIndex (s:string):byte;

       function GetInteger (s:TParamStr):longint;
       function GetReal (s:TParamStr):real;
       function GetString (s:TParamStr):TCmdString;

       function GetCount:byte;
       function IsHelp:boolean;
     end;

var cmdline:TCmdLine;

implementation

uses
     XString;

const
      COUNT_TOTAL:byte=0;
      COUNT_OPT:byte=0;
      COUNT_NOOPT:byte=0;

var
    aOptionPosCache,
    aNonOptionPosCache:array[1..255] of byte;

function XParamStr (index:byte):TCmdString;
var i, nCount:byte;
    bLongParam:boolean;
    s, sOut:string;
label exitloop;
begin
  sOut := '';
  nCount := 0;
  bLongParam := false;

  if (index = 0) then
    goto ExitLoop;

  for i := 1 to ParamCount do
  begin
    s := ParamStr (i);

    if (not bLongParam) then
    begin
      inc (nCount);
      if (nCount = index + 1) then
        goto ExitLoop;
      sOut := s;
    end
    else
      sOut := concat (sOut, ' ', s);

    if (s[1] = Delimiter) then
      bLongParam := true;

    if (LastChar (s) = Delimiter) and
       (length (s) > 1) then
      bLongParam := false;
  end;

exitloop:
  if (sOut [1] = Delimiter) then
    { - delete leading and trailing '"' - }
    sOut := copy (sOut, 2, length (sOut) - 2);

  XParamStr := sOut;
end;

function TCmdLine.IsOptionString (index:byte):boolean;
var s:TCmdString;
begin
  s := XParamStr (index);
  IsOptionString := (s [1] = '/') or (s [1] = '-');
end;

function TCmdLine.IsNonOptionString (index:byte):boolean;
begin
  IsNonOptionString := not IsOptionString (index);
end;

function TCmdLine.IsFlag (s:TParamStr):boolean;
begin
  if length (s) = 0 then
    IsFlag := false
  else
    IsFlag := GetIndex (s) > 0
end;

function TCmdLine.IsDisabled (s:TParamStr):boolean;
begin
  if (length (s) = 0) then
    IsDisabled := false
  else
    IsDisabled := (LastChar (XParamStr (GetIndex (s))) = '-');
end;

function TCmdLine.IsEnabled (s:TParamStr):boolean;
begin
  if (length (s) = 0) then
    IsEnabled := false
  else
    IsEnabled := (LastChar (XParamStr (GetIndex (s))) = '+');
end;

{************************************************************}

function TCmdLine.GetOptionStringCount:byte;
begin
  GetOptionStringCount := COUNT_OPT;
end;

function TCmdLine.GetOptionString (index:byte):TCmdString;
begin
  if (index > COUNT_OPT) then
    GetOptionString := ''
  else
    GetOptionString := XParamStr (aOptionPosCache [index]);
end;

function TCmdLine.GetParam (index:byte):TCmdString;
begin
  GetParam := XParamStr (index);
end;

function TCmdLine.GetIndex (s:string):byte;
var i:byte;
    tmp:TCMDString;
begin
  GetIndex := 0;
  if length (s) = 0 then
    exit;

  s := upstring (s);
  for i:=1 to GetCount do
  begin
    tmp := upstring (XParamStr (i));
    if (pos (concat ('/', s), tmp) +
        pos (concat ('-', s), tmp)) = 1 then
    begin
      GetIndex := i;
      exit;
    end;
  end;
end;

{************************************************************}

function TCmdLine.GetInteger (s:TParamStr):longint;
var index:byte;
{$ifdef WIN32}
    errorcode:longint;
{$else}
    errorcode:integer;
{$endif}
    l:longint;
begin
  GetInteger := 0;
  index := GetIndex (s);
  if index > 0 then
  begin
    val (copy (XParamStr (index), 2 + length (s), 255), l, errorcode);
    GetInteger := l;
  end;
end;

function TCmdLine.GetReal (s:TParamStr):real;
var index:byte;
{$ifdef WIN32}
    errorcode:longint;
{$else}
    errorcode:integer;
{$endif}
    r:real;
begin
  GetReal := 0.0;
  index := GetIndex (s);
  if index > 0 then
  begin
    val (copy (XParamStr (index), 2 + length (s), 255), r, errorcode);
    GetReal := r;
  end;
end;

function TCmdLine.GetString (s:TParamStr):TCmdString;
var index:byte;
begin
  index := GetIndex (s);
  if (index > 0) then
    GetString := copy (XParamStr (index), 2 + length (s), 255)
  else
    GetString := '';
end;

{************************************************************}

function TCmdLine.GetNonOptionStringCount:byte;
begin
  GetNonOptionStringCount := COUNT_NOOPT;
end;

function TCmdLine.GetNonOptionString (index:byte):TCmdString;
begin
  if (index > COUNT_NOOPT) then
    GetNonOptionString := ''
  else
    GetNonOptionString := XParamStr (aNonOptionPosCache [index]);
end;

{************************************************************}

function TCmdLine.GetCount:byte;
begin
  GetCount := COUNT_TOTAL;
end;

function TCmdLine.IsHelp:boolean;
begin
  IsHelp := IsFlag ('?') or IsFlag ('h');
end;

{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{}

procedure SetTotal;
var i, nCount:byte;
    bLongParam:boolean;
    s:string;
begin
  bLongParam := false;
  nCount := 0;
  for i:=1 to Paramcount do
  begin
    s := ParamStr (i);

    if (not bLongParam) then
      inc (nCount);

    if (s[1] = Delimiter) then
      bLongParam := true;

    if (LastChar (s) = Delimiter) and
       (length (s) > 1) then
      bLongParam := false;
  end;

  COUNT_TOTAL := nCount;
end;

procedure SetOther;
var i:byte;
    s:TCmdString;
begin
  for i := 1 to COUNT_TOTAL do
  begin
    s := XParamStr (i);
    if (s[1] = '/') or (s[1] = '-') then
    begin
      inc (COUNT_OPT);
      aOptionPosCache [COUNT_OPT] := i;
    end
    else
    begin
      inc (COUNT_NOOPT);
      aNonOptionPosCache [COUNT_NOOPT] := i;
    end;
  end;
end;

begin
  { fill the buffers with 0 -> else there are problems :( }
  fillchar (aOptionPosCache, SizeOf (aOptionPosCache), 0);
  fillchar (aNonOptionPosCache, SizeOf (aNonOptionPosCache), 0);

  SetTotal;
  SetOther;
end.
