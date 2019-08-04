{$i aflags.inc}

unit XEXE;

interface

type
     TEXEHeader = object
       ID:word;           { $00 }
       LastPage:word;     { $02 }
       SumPages:word;     { $04 }
       RelocEntries:word; { $06 }
       HeaderSize:word;   { $08 | paragraphs!! }
       MinMem:word;       { $0A }
       MaxMem:word;       { $0C }
       SS:word;           { $0E }
       SP:word;           { $10 }
       Checksum:word;     { $12 }
       IP:word;           { $14 }
       CS:word;           { $16 | paragraphs!! }
       RelocOffset:word;  { $18 }
       Res1:word;         { $1A }
       Res2:word;         { $1C }
       Res3:word;         { $1E }
     end;

const
      EXE_HEADER_SIZE = SizeOf (TEXEHeader);
      EXE_Magic:word  = $5A4D;
      EXE_Magic2:word = $4D5A;
      EXE_Magic3:word = $4757;

type
     TEXE = object (TEXEHeader)
       function  IsEXE:boolean;
       function  IsExtendedEXE:boolean;
       function  IsEuropeanEXE:boolean;

       function  GetHeaderSize:longint;
       function  GetCS:longint;
       function  GetIP:longint;
       function  GetSizeInHeader:longint;
       function  GetAbsoluteIP:longint;
       procedure CalculateHeadersize;

       function  HasOverlay (nFilesize:longint):boolean;
       function  GetOverlaySize (nFilesize:longint):longint;
{$ifdef difficult}
       function  NEOffsetIsValid:boolean;
{$endif}
     end;

implementation

function TEXE.IsEXE:boolean;
begin
  IsEXE := (ID = EXE_Magic) or
           (ID = EXE_Magic2);
end;

function TEXE.IsExtendedEXE:boolean;
begin
  IsExtendedEXE := (IsEXE) or
                   (ID = EXE_Magic3);
end;

function TEXE.IsEuropeanEXE:boolean;
begin
  IsEuropeanEXE := (ID = EXE_Magic2);
end;

function TEXE.GetHeaderSize:longint;
begin
  GetHeaderSize := longint (HeaderSize) shl 4;
end;

function TEXE.GetCS:longint;
begin
  GetCS := longint (CS) shl 4;
end;

function TEXE.GetIP:longint;
begin
  GetIP := IP;
end;

function TEXE.GetSizeInHeader:longint;
begin
  if LastPage = 0 then  { if exe file is 512 bytes -> overlay = 512 }
    GetSizeInHeader := (longint (512) * SumPages)
  else
    GetSizeInHeader := (longint (512) * (SumPages - 1)) + LastPage;
end;

function TEXE.GetAbsoluteIP:longint;
const MAX = 65536 shl 4;
var _IP:longint;
begin
  _IP := GetHeaderSize + GetCS + longint (IP);
  if _IP >= MAX then
    dec (_IP, MAX);
  GetAbsoluteIP := _IP;
end;

procedure TEXE.CalculateHeadersize;
begin
  HeaderSize := (16 + RelocOffset + RelocEntries shl 2) shr 4;
  if HeaderSize < 2 then
    HeaderSize := 2;
end;

function TEXE.HasOverlay (nFilesize:longint):boolean;
begin
  HasOverlay := (nFilesize > GetSizeInHeader);
end;

function TEXE.GetOverlaySize (nFilesize:longint):longint;
var nSize:longint;
begin
  nSize := GetSizeInHeader;
  if (nFilesize > nSize) then
    GetOverlaySize := nFilesize - nSize
  else
    GetOverlaySize := 0;
end;

{$ifdef difficult}
function TEXE.NEOffsetIsValid:boolean;
begin
  NEOffsetIsValid := (RelocOffset >= $0040);
end;
{$endif}

end.
