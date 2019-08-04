{
Copyright (c) 1997 - 99 by Philip Helger (philip@dke.univie.ac.at)
}

{$i aflags.inc}

unit GTOutPut;

interface

uses GTGlobal;

const
      DIFF = 2;
      MAXLEN = 80 - 1 - 3;  { without left and right border }
      MAXLEVEL = 20;
      STR_SEPERATOR = ' -- ';

var
    InsLevel:byte;
    nLineCounter:byte;

procedure Note (s:string);
procedure Noteln (s:string);
procedure Appendln (s:string);
procedure Append (s:string);
procedure EmptyLine;
procedure FinishLine;
function  WriteHeader (msg:string):boolean;

procedure IncInsLevel;
procedure DecInsLevel;

procedure ResetOutputParam;

procedure NoteCompiler (s:string);
procedure NoteLinker (s:string);
procedure NoteOverlay (s:string);
procedure NoteEXEPacker (s:string);
procedure NoteEXEEncrypter (s:string);
procedure NoteVariant (s:string);

procedure InternalError (s:string);
procedure SendFileToMe;

var
    TotalSize:longint;
    HeaderSize:longint;

implementation

uses
{$ifdef Win32}
     SysUtils,
{$else}
  {$ifdef Windows}
     WinDOS,
  {$else}
     DOS,
  {$endif}
{$endif}
     GTBuf,
{$ifndef WIN32}
     XCrt,
{$endif}
     XString, XTimer, XWrite;

const STR_OUT_FIRST_LEVEL = '-';
      STR_OUT_OTHER_LEVEL = ' ';
      STR_OUT_NEXT_LINE   = '+';
      MAX_WIDTH = 80 - 2 * 3;

function WriteHeader (msg:string):boolean;
var sLine:string;
    p:byte;
begin
  WriteHeader := false;

  if (not bDoOnlyNotFound) and (not bDoOnlyStatistics) then
  begin
    if bListMode then
    begin
      sLine := b.DisplayName;

      { show only the filename not the path name }
      p := LastPos ('\', sLine);
      if (p > 0) then
        sLine := copy (sLine, p + 1, 255);

      for p := length (sLine) + 1 to 18 do
        sLine := concat (sLine, ' ');
      Noteln (concat (sLine, ' - ', msg));
    end
    else
    begin
      WriteHeader := true;
      { - if a prior WriteHeader inced it ... - }
      DecInsLevel;

      {
      Noteln (concat (i2s (memavail), ' ', i2s (maxavail)));
      }
      if (InsLevel = 1) then
        Noteln (concat (Enbraced (b.DisplayName), ' -----'))
      else
        Noteln ('--- next detection level --------');

      IncInsLevel;
      Note (msg);
    end;
  end;
end;

procedure Noteln (s:string);
begin
  Note (s);
  FinishLine;
end;

procedure Note (s:string);
var p, nPos:byte;
    MAXLENGTH:byte;

    procedure InsertLine (c:char);
    var temp:string [1 + MAXLEVEL];
    const TEMP_SIZE = SizeOf (temp);
    begin
      fillchar (temp, TEMP_SIZE, ' ');
      SetLength (temp, 1 + InsLevel);
      temp [1] := c;

      FastPrint (temp);
    end;

begin
  nPos := length (s);
  if (s[nPos] = #10) and (nPos > 0) then
    dec (nPos);
  if (s[nPos] = #13) and (nPos > 0) then
    dec (nPos);
  SetLength (s, nPos);

  MAXLENGTH := MAXLEN - InsLevel;

  if (InsLevel = 1) then
    InsertLine (STR_OUT_FIRST_LEVEL)
  else
    InsertLine (STR_OUT_OTHER_LEVEL);

  { disable these annoying sounds }
  for p:=1 to length (s) do
    if (s[p] = #7) then
      s[p] := ' ';

  repeat
    { check for newline character }
    p := pos (#10, s);
    if (p > MAXLENGTH) then
      p := 0;

    { if newline character in current line }
    if (p > 0) then
    begin
      FastPrint (copy (s, 1, p));

      delete (s, 1, p);
      if (length (s) > 0) then
        InsertLine (STR_OUT_NEXT_LINE);
    end
    else
      { if newline character NOT in current line }
{$ifndef WIN32}
      if (
          (not bOutputRedirected) or
          (bOutputRedirected and bOutputBreak)
         ) and
         (not bOutputToFile) and
         (ScreenOutput) and
         (length (s) > MAXLENGTH) then  { if string longer than one line }
{$else}
      if (length (s) > MAXLENGTH) and
         (not bOutputToFile) then
{$endif}
      begin
        { break after MAXLENGTH chars }
        FastPrint (concat (copy (s, 1, MAXLENGTH), ' ', STR_OUT_FIRST_LEVEL));
        FinishLine;

        delete (s, 1, MAXLENGTH);
        InsertLine (STR_OUT_NEXT_LINE);
      end
      else  { string fits in one line }
      begin
        FastPrint (s);

        s := '';
      end;
  until (s = '');  { faster then absolute }
end;

procedure Appendln (s:string);
begin
  Append (s);
  FinishLine;
end;

procedure Append (s:string);
var p:byte;
begin
  { disable these annoying sounds }
  for p:=1 to length (s) do
    if (s[p] = #7) then
      s[p] := ' ';

  FastPrint (s);
end;

procedure EmptyLine;
begin
  FastPrint (STR_OUT_OTHER_LEVEL);
  FinishLine;
end;

procedure FinishLine;
{$ifdef WIN32}
const Str = 'Press Enter to continue. "P" + Enter to disable pause, "Q" + Enter to quit ';
{$else}
const Str = 'Press any key to continue, P to disable pause, Q to quit';
{$endif}
var
{$ifndef DELPHI}
    s:String80;
{$endif}
    c:char;
begin
  FastNewLine;

  inc (nLineCounter);

  if (nLineCounter = nPauseLines) then
  begin
    if (bDoPause) then
    begin
{$ifdef GETTYP}
      aFileTimer.DoBreak;
      aTotalTimer.DoBreak;
{$endif}

      FastPrint (Str);

{$ifdef WIN32}
      read (c);
      { already in the next line - no repaint :( }

      if (upcase (c) = 'P') then
        bDoPause := false;
      if (upcase (c) = 'Q') then
        halt;

      read (c);  { to flush the internal buffer - fucking shit :( }
{$else}
      c := upcase (readkey);

      FillChar (s, SizeOf (s), #32);
      SetLength (s, length (Str));
      FastPrint (concat (#13, s, #13));   { #13 is "Wagenruecklauf" }

      case c of
        #0:c := readkey;
        'P': bDoPause := false;
        'Q': halt; { should call ExitProcs }
      end;
{$endif}

{$ifdef GETTYP}
      aFileTimer.DoContinue;
      aTotalTimer.DoContinue;
{$endif}
    end;

    nLineCounter := 0;
  end;
end;

procedure IncInsLevel;
begin
  if (InsLevel + DIFF <= MAXLEVEL) then
    inc (InsLevel, DIFF);
end;

procedure DecInsLevel;
begin
  if (InsLevel > DIFF) then
    dec (InsLevel, DIFF);
end;

procedure ResetOutputParam;
begin
  InsLevel := 1;
end;

procedure NoteCompiler (s:string);
begin
  Noteln (concat ('Compiler: ', s));
end;

procedure NoteLinker (s:string);
begin
  Noteln (concat ('Linker: ', s));
end;

procedure NoteEXEEncrypter (s:string);
begin
  Noteln (concat ('EXE encrypter: ', s));
end;

procedure NoteEXEPacker (s:string);
begin
  Noteln (concat ('EXE packer: ', s));
end;

procedure NoteOverlay (s:string);
begin
  Noteln (concat ('Overlay type: ', s));
end;

procedure NoteVariant (s:string);
begin
  Noteln (concat ('May be a variant of ', s));
end;

procedure InternalError (s:string);
begin
  Noteln (concat ('Internal error "', s, '" !'));
  SendFileToMe;
  halt (3);
end;

procedure SendFileToMe;
begin
  Noteln ('!!Please!! send this file to phax@writeme.com - !!THANX!!');
end;

begin
  ResetOutputParam;
  nLineCounter := 0;
end.
