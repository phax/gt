{$i aflags.inc}

{$ifdef BORLAND}
{$R-}  { - disable range check for virtual array - }
{$endif}

unit GTCodes;

interface

uses GTCompar, GTMem, GTGlobal;

const MAX_ARRAY_COUNT = 100;

type
     pSCRecordArray = ^tSCRecordArray;
     tSCRecordArray = array[0..0] of tSCRecord;

     pSCRecords = ^tSCRecords;
     tSCRecords = record
       nCount:byte;
       next:pSCRecords;
       aMS:tMemStruct;
     end;

procedure InitStartupCodes;

var aStartupCodes:tSCRecords;

implementation

uses XString;

const DAT_FILENAME = 'gt.dat';

procedure InitStartupCodes;
var t:text;
    rec:tSCRecord;
    s, sLine:string;
    p:byte;
    nLineNumber:longint;
    bDone:boolean;
    i, nIndex:integer;

    procedure SyntaxError;
    begin
      writeln ('Syntax error in line ', i2s (nLineNumber), ' in the DAT file');
      halt (77);
    end;

    procedure AddRecord;
    var pCurrent, pNew:pSCRecords;
        nOffset:longint;
    begin
      { search the end of the list }
      pCurrent := @aStartupCodes;
      while (pCurrent^.next <> nil) do
      begin
        {
        writeln ('$', hexword (seg (pCurrent)), ':', hexword (ofs (pCurrent)));
        }
        pCurrent := pCurrent^.next;
      end;

      { do we need a new array? }
      if (pCurrent^.nCount = MAX_ARRAY_COUNT) then
      begin
        if (pCurrent = @aStartupCodes) then
          aStartupCodes.nCount := 0;

        getmem (pNew, sizeof (tSCRecords));
        pNew^.nCount := 0;
        pNew^.aMS.bInUse := false;
        GetMemory (pNew^.aMS, MAX_ARRAY_COUNT * sizeof (tSCRecord));
        pNew^.next := nil;
        pCurrent^.next := pNew;
        pCurrent := pNew;
      end;


      { add the curent record }
      nOffset := pCurrent^.nCount * sizeof (tSCRecord);
      CopyToMemoryAt (rec,
                      pCurrent^.aMS,
                      sizeof (tSCRecord),
                      nOffset);
      inc (pCurrent^.nCount);
    end;

begin
  if (bVerbose) then writeln ('Reading datfile');

{$ifdef DELPHI}
  AssignFile (t, DAT_FILENAME);
{$else}
  assign (t, DAT_FILENAME);
{$endif}
  reset (t);
  if (ioresult <> 0) then
  begin
{$ifdef DELPHI}
    AssignFile (t, concat (GetPath (ParamStr (0)), DAT_FILENAME));
{$else}
    assign (t, concat (GetPath (ParamStr (0)), DAT_FILENAME));
{$endif}
    reset (t);
    if (ioresult <> 0) then
    begin
      writeln ('Failed to open the data file ', DAT_FILENAME, ' for reading.');
      halt (815);
    end;
  end;

  readln (t, sLine);
  nLineNumber := 1;

  repeat
    { determine whether we have a new section or not }
    if (sLine[1] <> '[') or (sLine [length (sLine)] <> ']') then SyntaxError;
    s := copy (sLine, 2, length (sLine) - 2);
         if (s = 'EXE0') then rec.EXEType := EXE_DOS_00
    else if (s = 'EXE1') then rec.EXEType := EXE_DOS_01
    else if (s = 'LE0')  then rec.EXEType := EXE_LE_00
    else if (s = 'LE1')  then rec.EXEType := EXE_LE_01
    else if (s = 'NE0')  then rec.EXEType := EXE_NE_00
    else if (s = 'PE0')  then rec.EXEType := EXE_PE_00
    else if (s = 'PE1')  then rec.EXEType := EXE_PE_01
    else SyntaxError;

    bDone := false;
    while (not eof (t)) and (not bDone) do
    begin
      readln (t, sLine);
      inc (nLineNumber);

      if (sLine[1] = '[') then
      begin
        bDone := true;
      end
      else
      begin
        { get the name and kill it }
        if (sLine[1] <> '<') then SyntaxError;
        p := pos ('><', sLine);
        if (p <= 0) then SyntaxError;
        rec.Name := copy (sLine, 2, p - 2);
        delete (sLine, 1, p);

        { get the type and kill it }
        if (sLine[1] <> '<') then SyntaxError;
        p := pos ('><', sLine);
        if (p <= 0) then SyntaxError;
        s := copy (sLine, 2, p - 2);
        delete (sLine, 1, p);
             if (s = 'NORMAL')    then rec.NameType := NORMAL
        else if (s = 'PACKER')    then rec.NameType := PACKER
        else if (s = 'ENCRYPTER') then rec.NameType := ENCRYPTER
        else if (s = 'COMPILER')  then rec.NameType := COMPILER
        else if (s = 'LINKER')    then rec.NameType := LINKER
        else if (s = 'CONVERTER') then rec.NameType := CONVERTER
        else if (s = 'PROTECTOR') then rec.NameType := PROTECTOR
        else if (s = 'STICKER')   then rec.NameType := STICKER
        else if (s = 'EXTENDER')  then rec.NameType := EXTENDER
        else if (s = 'PASSWORD')  then rec.NameType := PASSWORD
        else SyntaxError;

        { get the data and kill it }
        if (sLine[1] <> '<') then SyntaxError;
        p := pos ('>', sLine);
        if (p <= 0) then SyntaxError;
        s := copy (sLine, 2, p - 2);
        rec.DataLen := length (s) div 2;
        if (rec.DataLen = 0) or (rec.DataLen > 80) then
          SyntaxError;

        nIndex := 1;
        for i:=1 to rec.DataLen do
        begin
          rec.data[i] := s2i (concat ('$', s[nIndex], s[nIndex + 1]));
          inc (nIndex, 2);
        end;

        AddRecord;
      end;
    end;
  until (eof (t));

{$ifdef Delphi}
  CloseFile (t);
{$else}
  close (t);
{$endif}
  if (bVerbose) then writeln ('Finished reading datfile');
end;

begin
  aStartupCodes.nCount := MAX_ARRAY_COUNT;
  aStartupCodes.next := nil;
end.
