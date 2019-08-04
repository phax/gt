{
GTFile - file handling, exefiles and archive handling and output

Copyright (c) 1997, 98 by Philip Helger (philip@dke.univie.ac.at)
}

{$i aflags.inc}

unit GTFile;

interface

uses
{$ifdef WIN32}
     SysUtils,
{$else}
  {$ifdef Windows}
     WinDOS,
  {$else}
     DOS,
  {$endif}
{$endif}
{$ifdef LOG}
     GTDebug,
{$endif}
     GTGlobal,
     XEXE_LE, XEXE_PE;

const
      TFILE_ERR_NONE  = 0;
      { std IO errors between 1 and 163 - }
      TFILE_ERR_USER  = 200;
      TFILE_ERR_EOF   = 201;
      TFILE_ERR_ID    = 202;
      TFILE_ERR_VALUE = 203;

type
{$ifdef WIN32}
      myint = smallint;
{$else}
      myint = integer;
{$endif}

var
    __FPos, __RVA:longint;
    TFile_Error:myint;
    OldAttr:word;

{ needed in typexepe and in gtfile :( }
    aLE_OTH:tLE_ObjectTableHandler;
    aPE_OTH:tPE_ObjectTableHandler;

procedure TFILE_Init (StartPos:longint);

function  TFile_ReadActByte:byte;
function  TFile_ReadActChar:char;
function  TFile_ReadActWord:word;
function  TFile_ReadActLong:longint;
function  TFile_ReadActBuf (var buf; nSize:TPos):word;

function  TFile_ReadByteAt (n:longint):byte;
function  TFile_ReadCharAt (n:longint):char;
function  TFile_ReadWordAt (n:longint):word;
function  TFile_ReadLongAt (n:longint):longint;
function  TFile_ReadBufAt (    n:longint;
                           var buf;
                               nSize:TPos):TPos;

function  TFile_ReadPascalString (nPos:longint):string;
function  TFile_ReadActPascalString:string;
function  TFile_ReadString (nStartPos:longint;
                            nLen:byte):string;
function  TFile_ReadActString (nLen:byte):string;

function  TFile_Read_To (c:char):string;
{$ifdef GETTYP}
function  TFile_Print_To (c:char):word;
procedure TFile_Print (len:word);
{$endif}
function  TFile_ReadActLongString (nLen:word):string;
function  TFile_GetFilePos:longint;
procedure TFile_GotoFilePos (FilePos:longint);
procedure TFile_IncFilePos (value:longint);
function  TFile_IsOkay:boolean;
procedure TFile_Finish;
procedure TFile_SetError (e:word);
function  TFile_IsError:boolean;

function TEXEFile_Get32BitJumpPos (var nPos:longint;
                                       nJumps:byte):boolean;
function TEXEFile_GetJumpPos (var nPos:longint;
                                  nJumps:byte):boolean;
function TEXEFile_GetMOVJumpPos (var nStartPos:longint;
                                     typ:byte):boolean;
function TEXEFile_GetFarJumpPos (var nStartPos:longint;
                                     nOffset:longint;
                                     typ:byte):boolean;
function TEXEFile_GetLongCallPos (var nPos:longint;
                                  nOffset:longint):boolean;
function TEXEFile_GetMacroPos (    s:string;
                               var nPos:longint;
                                   typ:byte):boolean;

implementation

uses
     GTBuf, GTCompar, GTMem,
{$ifdef GETTYP}
     GTOutput,
{$endif}
     IOError,
     XString, XWrite;

var
    ExitSave:pointer;

{ ********************   ******************** }

procedure _seek (nPos:longint);
begin
  seek (Handle, nPos);
  {
  seems like the ioresult after "seek" is random :(
  }
  TFile_SetError (ioresult);
end;

procedure _blockread (var buf;
                          nLen:word;
                      var nBytesRead:word);
{$ifdef WIN32}
var nRead:longint;
{$endif}
begin
  { fill the buffer with 0's }
  fillchar (buf, nLen, 0);
{$ifdef WIN32}
  blockread (Handle,
             buf,
             nLen,
             nRead);
  TFile_SetError (ioresult);
  nBytesRead := nRead;
{$else}
  blockread (Handle,
             buf,
             nLen,
             nBytesRead);
  TFile_SetError (ioresult);
{$endif}
end;

procedure _blockread_simple (var buf;
                                 nLen:word);
begin
  fillchar (buf, nLen, 0);
  blockread (Handle, buf, nLen);
  TFile_SetError (ioresult);
end;

procedure TFile_Init (StartPos:longint);
begin
  if (bVerbose) then writeln ('TFile_Init');
  __FPos  := StartPos;
  TFile_Error := TFILE_ERR_NONE;
  _seek (__FPos);
end;

function TFile_Read_To (c:char):string;
const BufSize = 1000;
label LoopEnd;
var bFound:boolean;
    buf:array[1..BufSize] of char;
    i, FoundPos, BytesRead:word;
    s:string;
begin
  if (bVerbose) then writeln ('TFile_Read_To (', c, ')');

  FoundPos := 0;
  s := '';
  bFound := false;

  repeat
    _blockread (buf, BufSize, BytesRead);

    for i:= 1 to BytesRead do
      if (buf [i] = c) then
      begin
        inc (FoundPos, i);
        bFound := true;
        goto LoopEnd;
      end
      else
        s := concat (s, buf [i]);
LoopEnd:

    if (not bFound) then
      inc (FoundPos, BytesRead);
  until (bFound) or (BytesRead < BufSize);

  Inc (__FPos, FoundPos);
  _seek (__FPos);

  TFile_Read_To := s;
end;

{$ifdef GETTYP}
function TFile_Print_To (c:char):word;
const BufSize = 100;
label LoopEnd;
var bFound:boolean;
    FoundPos:TPos;
    buf:array[1..BufSize] of char;
    i:byte;
    BytesRead:word;
begin
  FoundPos := 0;
  repeat
    bFound := false;
    _blockread (buf, BufSize, BytesRead);
    for i:= 1 to BytesRead do
      if (buf[i] = c) then
      begin
        inc (FoundPos, i);
        bFound := true;
        goto LoopEnd;
      end
      else
        if (buf[i] = #10) then
          FinishLine
        else
          if (buf[i] <> #13) then
            FastPrint (buf[i]);
LoopEnd:

    if (not bFound) then
      inc (FoundPos, BufSize);
  until (bFound) or (eof (Handle));

  inc (__FPos, FoundPos);
  _seek (__FPos);

  TFile_Print_To := FoundPos - 1;
end;

procedure TFile_Print (len:word);
begin
  Append (TFile_ReadActLongString (Len));
end;
{$endif}

function TFile_ReadActLongString (nLen:word):string;
const MAXLEN = 255;
var s:string;
    pTemp:pointer;  { temporary buffer }
    RealLen:byte;
begin
  if (nLen = 0) then
    s := ''
  else
  begin
    if (nLen > MAXLEN) then
      RealLen := MAXLEN - 3  { - 3 for the "..." }
    else
      RealLen := nLen;

    getmem (pTemp, nLen);
    CheckMemoryAllocation (pTemp, nLen, 'TFile_ReadActLongString');

    if (TFile_ReadActBuf (pTemp^, nLen) <> nLen) then;

    SetLength (s, RealLen);
    move (pTemp^, s[1], RealLen);

    freemem (pTemp, nLen);

    if (nLen > MAXLEN) then
      s := concat (s, '...');
  end;

  TFile_ReadActLongString := s;
end;

function TFile_ReadActByte:byte;
var b:byte;
begin
  _blockread_simple (b, 1);
  inc (__FPos, 1);
  TFile_ReadActByte := b;
end;

function TFile_ReadActChar:char;
var c:char;
begin
  _blockread_simple (c, 1);
  inc (__FPos, 1);
  TFile_ReadActChar := c;
end;

function TFile_ReadActWord:word;
var w:word;
begin
  _blockread_simple (w, 2);
  inc (__FPos, 2);
  TFile_ReadActWord := w;
end;

function TFile_ReadActLong:longint;
var l:longint;
begin
  _blockread_simple (l, 4);
  inc (__FPos, 4);
  TFile_ReadActLong := l;
end;

function TFile_ReadByteAt (n:longint):byte;
var b:byte;
begin
  _seek (n);
  _blockread_simple (b, 1);
  TFile_ReadByteAt := b;
  __FPos := n + 1;
end;

function TFile_ReadCharAt (n:longint):char;
var b:char;
begin
  _seek (n);
  _blockread_simple (b, 1);
  TFile_ReadCharAt := b;
  __FPos := n + 1;
end;

function TFile_ReadWordAt (n:longint):word;
var w:word;
begin
  _seek (n);
  _blockread_simple (w, 2);
  TFile_ReadWordAt := w;
  __FPos := n + 2;
end;

function TFile_ReadLongAt (n:longint):longint;
var l:longint;
begin
  _seek (n);
  _blockread_simple (l, 4);
  TFile_ReadLongAt := l;
  __FPos := n + 4;
end;

function TFile_ReadActBuf (var buf; nSize:TPos):word;
begin
  TFile_ReadActBuf := TFile_ReadBufAt (__FPos, buf, nSize);
end;

function TFile_ReadBufAt (    n:longint;
                          var buf;
                              nSize:TPos):TPos;
var nBytesRead:word;
begin
  TFile_ReadBufAt := 0;

  if (not TFile_IsOkay) then
    exit;

  _seek (n);
  _blockread (buf, nSize, nBytesRead);

  if (nBytesRead < nSize) then
    if (nBytesRead = 0) then
      TFile_Finish
    else
      if (n + nBytesRead = b.FSize) then
        TFile_SetError (TFILE_ERR_EOF);

  { - don't have to seek there - already there - }
  __FPos := n + nBytesRead;

  TFile_ReadBufAt := nBytesRead;
end;

function TFile_ReadString (nStartPos:longint;
                           nLen:byte):string;
var s:string;
begin
  _seek (nStartPos);

  SetLength (s, nLen);
  _blockread_simple (s[1], nLen);
  inc (__FPos, nLen);

  { mark the current position as new fpos }
  _seek (__FPos);

  TFile_ReadString := s;
end;

function TFile_ReadActString (nLen:byte):string;
var s:string;
begin
  SetLength (s, nLen);
  if (nLen > 0) then
  begin
    _blockread_simple (s[1], nLen);
    inc (__FPos, nLen);
  end;
  TFile_ReadActString := s;
end;

function TFile_ReadPascalString (nPos:longint):string;
var s:string;
    nLen:byte;
begin
  TFile_ReadPascalString := '';
  _seek (nPos);
  __FPos := nPos;

  _blockread_simple (nLen, 1);
  if (TFile_Error <> TFILE_ERR_NONE) then
    exit;

  SetLength (s, nLen);

  _blockread_simple (s[1], nLen);
  if (TFile_Error <> TFILE_ERR_NONE) then
    exit;

  { mark the current position as new fpos }
  inc (__FPos, 1 + nLen);

  TFile_ReadPascalString := s;
end;

function TFile_ReadActPascalString:string;
begin
  TFile_ReadActPascalString := TFile_ReadPascalString (__FPos);
end;

function TFile_GetFilePos:longint;
begin
  TFile_GetFilePos := __FPos;
end;

procedure TFile_GotoFilePos (FilePos:longint);
begin
  if (FilePos <> __FPos) then
  begin
    __FPos := FilePos;
    _seek (__FPos);
  end;
end;

procedure TFile_IncFilePos (value:longint);
begin
  TFile_GotoFilePos (__FPos + value);
end;

function TFile_IsOkay:boolean;
begin
  TFile_IsOkay := (TFile_Error = TFILE_ERR_NONE);
end;

procedure TFile_Finish;
begin
  TFile_Error := TFILE_ERR_USER;
end;

procedure TFile_SetError (e:word);
begin
  if (TFile_Error = TFILE_ERR_NONE) then
    if (e <> 0) then
      TFile_Error := e;
end;

function TFile_IsError:boolean;
begin
  TFile_IsError := (TFile_Error <> 0);
end;

{ ******************** Archive ******************** }

function TEXEFile_Get32BitJumpPos (var nPos:longint;
                                       nJumps:byte):boolean;
type Jmp32Rec = record
                  ID:byte;
                  Ofs:longint;
                end;
var j:byte;
    Jmp:Jmp32Rec;
begin
  TEXEFile_Get32BitJumpPos := false;

  for j:=1 to nJumps do
  begin
    if (TFile_ReadBufAt (nPos, Jmp, 5) <> 5) then;
    case Jmp.ID of
      $E8:inc (nPos, 5 + Jmp.Ofs);
    else
      exit;
    end;

    if (nPos > b.FSize) or (nPos < 0) then
      exit;
  end;

  TEXEFile_Get32BitJumpPos := true;
end;

function TEXEFile_GetJumpPos (var nPos:longint; nJumps:byte):boolean;
type JmpRec = record
                ID:byte;
                Ofs:word;
              end;
var j:byte;
    Jmp:JmpRec;
begin
  TEXEFile_GetJumpPos := false;

  for j:=1 to nJumps do
  begin
    if (TFile_ReadBufAt (nPos, Jmp, 3) <> 3) then;

    case Jmp.ID of
      $E8,
      $E9:begin
            if (Jmp.Ofs >= $8000) and (nPos + 3 + Jmp.Ofs > b.FSize) then
              inc (nPos, 3 + myint (Jmp.Ofs))
            else
              inc (nPos, 3 + word (Jmp.Ofs));
          end;
      $74,
      $75,
      $E3,
      $EB:inc (nPos, 2 + lo (Jmp.Ofs))
    else
      exit;
    end;

    if (nPos > b.FSize) or (nPos < 0) then
      exit;
  end;

  TEXEFile_GetJumpPos := true;
end;

function TEXEFile_GetMOVJumpPos (var nStartPos:longint; typ:byte):boolean;
type JmpRec = record
                ID:byte;
                Ofs:word;
              end;
const
      JMP_SIZE = SizeOf (JmpRec);
var Jmp:JmpRec;
begin
  TEXEFile_GetMOVJumpPos := false;

  if (TFile_ReadBufAt (nStartPos, Jmp, JMP_SIZE) <> JMP_SIZE) then;
  if (Jmp.ID >= $B8) and (Jmp.ID <= $BF) then
  begin
    inc (nStartPos, Jmp.Ofs);
    if (typ = COM) then
      dec (nStartPos, $0100);  { - $0100 because of a COM file's IP }
    TEXEFile_GetMOVJumpPos := true;
  end;
end;

function TEXEFile_GetFarJumpPos (var nStartPos:longint; nOffset:longint; typ:byte):boolean;
type CallRecMem = record
                    ID:word;
                    ofs:word;
                  end;
     CallRec = record
                 o, s:word;
               end;
const
      CRM_SIZE = SizeOf (CallRecMem);
      CR_SIZE  = SizeOf (CallRec);
var CallMem:CallRecMem;
    Call:CallRec;
    nPos:longint;
begin
  TEXEFile_GetFarJumpPos := false;
  if (TFile_ReadBufAt (nStartPos + nOffset, CallMem, CRM_SIZE) <> CRM_SIZE) then;

  if (CallMem.ID = $2EFF) then
  begin
    nPos := nStartPos + CallMem.ofs - $0100;  { $0100 because of COM file }
    if (TFile_ReadBufAt (nPos, Call, CR_SIZE) <> CR_SIZE) then;

    if (typ = COM) then
      dec (Call.s, $0010);  { only $10 because it will be mutiplied
                              by $10 in the next command }

    inc (nStartPos, longint (Call.s) shl 4 + Call.o);
    TEXEFile_GetFarJumpPos := true;
  end
end;

function TEXEFile_GetLongCallPos (var nPos:longint; nOffset:longint):boolean;
type CallRec = record
                 ID:byte;
                 ofs, seg:word;
               end;
const
      CALL_SIZE = SizeOf (CallRec);
var Call:CallRec;
begin
  TEXEFile_GetLongCallPos := false;
  if (TFile_ReadBufAt (nPos, Call, CALL_SIZE) <> CALL_SIZE) then;

  if (Call.ID = $9A) then
  begin
    nPos := nOffset + longint (Call.seg) shl 4 + longint (Call.ofs);
    TEXEFile_GetLongCallPos := true;
  end;
end;

function TEXEFile_Get32BitRVAJumpPos (var nPos:longint;
                                          nJumps:byte):boolean;
type
     Jump32 = record
       ID:byte;
       Offset:longint;
     end;
const
      JUMP32_SIZE = SizeOf (Jump32);
var j:byte;
    nRVA:longint;
    Data:Jump32;
begin
  TEXEFile_Get32BitRVAJumpPos := false;

  for j:=1 to nJumps do
  begin
    aPE_OTH.GetRVAByPhysOfs (nPos, nRVA);
    if (TFile_ReadBufAt (nPos, Data, JUMP32_SIZE) <> JUMP32_SIZE) then;

    if (Data.ID = $E9) then
    begin
      inc (nRVA, 5 + Data.Offset);
      aPE_OTH.GetPhysOfsByRVA (nRVA, nPos);
    end
    else
      exit;
  end;

  TEXEFile_Get32BitRVAJumpPos := true;
end;

function TEXEFile_GetMacroPos (    s:string;
                               var nPos:longint;
                                   typ:byte):boolean;

      function GetNumber:longint;
      var j:byte;
      begin
        j := 1;
        while (j < length (s)) and (IsNumberChar (s [j + 1])) do
          inc (j);
        GetNumber := s2i (copy (s, 1, j));
        delete (s, 1, j);
      end;

var nDiff, nStart:longint;
begin
  TEXEFile_GetMacroPos := false;
  nStart := nPos;

  while (length (s) > 0) do
  begin
    if IsNumberChar (s[1]) then
      inc (nPos, GetNumber)  { skip bytes }
    else
    begin
      case s [1] of
        'f':begin
              nDiff := nPos - nStart;
              nPos := nStart;
              if not TEXEFile_GetFarJumpPos (nPos, nDiff, typ) then
                exit;
            end;
        'j':if not TEXEFile_GetJumpPos (nPos, 1) then
              exit;
        'k':if not TEXEFile_Get32BitJumpPos (nPos, 1) then
              exit;
        'm':if not TEXEFile_GetMOVJumpPos (nPos, typ) then
              exit;
        'r':if not TEXEFile_Get32BitRVAJumpPos (nPos, 1) then
              exit;
        '_':;  { just skip "_"'s }
      else
        exit;
      end;
      delete (s, 1, 1);
    end;
  end;

  TEXEFile_GetMacroPos := true;
end;

procedure NewExit; far;
begin
{$ifdef LOG}
  StopLog;
{$endif}
  ExitProc := ExitSave;
end;

begin
  ExitSave := ExitProc;
  ExitProc := @NewExit;
  { make a default init ... }

{$ifdef LOG}
  StartLog ('_debug.txt');
{$endif}
end.
