{
Global variables for GetTyp

Copyright (c) 19997, 98 by Philip Helger (philip@dke.univie.ac.at)
}

{$i aflags.inc}

unit GTGlobal;

interface

uses
{$ifdef GETTYP}
     XTimer,
{$endif}
     XString;

const
      nMaximumFiles:longint = 0;
      nMaximumCompare:byte = 80;

type
     String4   = string [4];
     String8   = string [8];
     String12  = string [12];
     String25  = string [25];
     String80  = string [80];
     TMacroStr = string [20];
     LongStr   = string[10];
     ShortStr  = string[80];
     FNameStr  = string[12];

     TPos = word;
{$ifdef WIN32}
     integer = SmallInt;
{$endif}

{$ifdef WIN32}
     readtype = longint;
{$else}
     readtype = word;
{$endif}

     TArray24  = array[1..24] of byte;
     TArray80  = array[1..80] of byte;
     PArray256 = ^TArray256;
     TArray256 = array[0..255] of byte;

     CharSet = set of char;

     TripleLong = object
       nData:array[1..3] of byte;

       function AsLong:longint;
     end;

     { - needed for recursive calls - }
     TEXEProc = function (n:longint; b:boolean):boolean;

const
      TextChars:CharSet = ['a'..'z', 'A'..'Z', '0'..'9',
                           '+', '-', '*', '/',
                           ' ', '_',
                           '.', ',', ';',
                           '!', '?',
                           '(', ')', '[', ']', '<', '>'];


var
    CompareBuf:TArray256;

    nPauseLines:word;

{$ifdef MSDOS}
    bInputRedirected,
    bOutputRedirected,
{$endif}

{$ifdef GETTYP}
    bDoOnlyFound,       { /F   }
    bDoDirectories,     { /ND  }
    bDoStatistics,      { /NS  }
{$endif}
    bVerbose,
    bDoInternal,        { /I   }
    bListMode,          { /L   }
    bMaximumFiles,      { /M   }
    bHeuristics,        { /NH  }
    bLongNames,         { /NL  }
    bOutputToFile,      { /OUT }
    bOverwriteFile,     { /OVR }
    bDoPause,           { /P   }
    bDoSubs,            { /S   }
    bOutputBreak,       { /YB  }
    bSearchARI,         { /ZA  }
    bDoHeader,          { /ZH  }
    bDoOnlyNotFound,    { /ZN  }
    bDoOnlyStatistics,  { /ZS  }
    bDoEXEHeaderAnyway, { /ZE  }
    bDoHelp:boolean;    { /?, /H  }

{$ifdef GETTYP}
    aFileTimer:Timer;
    aTotalTimer:Timer;
{$endif}

procedure SwapLong (src:longint; var dst:longint);
procedure SwapLongSelf (var l:longint);

function  hoch (x,y:byte):longint;
function  Exp2 (y:byte):longint;

function  IsJump (b:byte):boolean;

implementation

uses
{$ifdef WINDOWS}
     WinDOS,
{$endif}
{$ifdef MSDOS}
     DOS,
{$endif}
     GTMem,
     XDos;

{********** global **********}

procedure SwapLong (src:longint; var dst:longint);
const MAX_LONG = 2147483647;
var aSRC:array[1..4] of byte absolute src;
    aDST:array[1..4] of byte absolute dst;
begin
  aDST [1] := aSRC [4];
  aDST [2] := aSRC [3];
  aDST [3] := aSRC [2];
  aDST [4] := aSRC [1];
end;

procedure SwapLongSelf (var l:longint);
var nTemp:longint;
begin
  SwapLong (l, nTemp);
  l := nTemp;
end;

function hoch (x, y:byte):longint;
var r:longint;
    i:byte;
begin
  r := x;
  for i := 1 to y do
    r := r * x;
  hoch := r;
end;

function Exp2 (y:byte):longint;
begin
  Exp2 := longint (1) shl y;
end;

(*

procedure SwitchLED (bOn:boolean; value:byte);
assembler;
asm
  cmp bOn, 0
  je @BeDarkness
@BeLight:
  mov si, 40h
  mov es, si
  mov al, es:[0017h]
  or  al, value
  mov es, si
  mov es:[0017h], al
  jmp @finish
@BeDarkness:
  mov si, 40h
  mov es, si
  mov al, es:[0017h]
  mov bl, $FF
  sub bl, value
  and al, bl
  mov es, si
  mov es:[0017h], al
@finish:
end;

procedure SetLED (b:byte);
begin
  SwitchLED (b and $01 > 0, $20);  { num }
  SwitchLED (b and $02 > 0, $40);  { caps }
  SwitchLED (b and $04 > 0, $10);  { scroll }
end;

procedure UpdateKB;
const k:byte = 0;
begin
  k := (k + 1) mod 3;
  case k of
    0:SetLED ($01);
    1:SetLED ($02);
    2:SetLED ($04);
  else
    SetLED ($07);  { turn all on }
  end;
end;

var bNum, bCaps, bScroll:boolean;

procedure SaveKB;
begin
  bNum    := (mem [$0040:$0017] and $20 > 0);
  bCaps   := (mem [$0040:$0017] and $40 > 0);
  bScroll := (mem [$0040:$0017] and $10 > 0);
  SetLED (1);
end;

procedure RestoreKB;

    procedure x (b:boolean; i:byte);
    begin
      if b then
        mem [$0000:$0417] := mem [$0000:$0417] or i
      else
        mem [$0000:$0417] := mem [$0000:$0417] and (255 - i);
    end;

begin
  x (bNum,    $20);
  x (bCaps,   $40);
  x (bScroll, $10);
end;

*)

{$ifdef MSDOS}
function IsInputRedirected:boolean;
var handle:word absolute input;
    nDX:word;
    r:registers;
begin
  r.ax := $4400;
  r.bx := handle;
  msdos (r);
  IsInputRedirected := (r.dx and $81 <> $81);
end;

function IsOutputRedirected:boolean;
var handle:word absolute output;
    nDX:word;
    r:registers;
begin
  r.ax := $4400;
  r.bx := handle;
  msdos (r);
  IsOutputRedirected := (r.dx and $82 <> $82);
end;
{$endif}

function TripleLong.AsLong:longint;
var l:longint;
begin
  move (nData, l, 3);
  l := l and $00FFFFFF;
  AsLong := l;
end;

function IsJump (b:byte):boolean;
var j:byte;
begin
  j := CompareBuf [b];
  IsJump := (j = $74) or
            (j = $75) or
            (j = $E3) or
            (j = $E8) or
            (j = $E9) or
            (j = $EB);
end;

begin
  bVerbose           := false;
{$ifdef GETTYP}
  bDoOnlyFound       := false;      { f   }
{$endif}
  bDoHelp            := false;      { h   }
  bDoInternal        := false;      { i   }
  bListMode          := false;      { l   }
  bMaximumFiles      := false;      { m   }
{$ifdef GETTYP}
  bDoDirectories     := true;       { nd  }
{$endif}
  bHeuristics        := true;       { nh  }
{$ifdef WIN32}
  bLongNames         := false;      { nl  default in Win32 - }
{$else}
  bLongNames         := bAllowLongFilenames;
{$endif}
{$ifdef GETTYP}
  bDoStatistics      := true;       { ns  }
{$endif}
{$ifdef XMS}
  GTMem_UseXMS       := true;       { nx  }
{$endif}
  bOutputToFile      := false;      { out }
  bOverwriteFile     := false;      { ovr }
  bDoPause           := false;      { p   }
  nPauseLines        := 24;         { px  }
  bDoSubs            := false;      { s   }
  bOutputBreak       := false;      { yb  }
  bSearchARI         := false;      { za  }
  bDoEXEHeaderAnyway := false;      { ze  }
  bDoHeader          := true;       { zh  }
  bDoOnlyNotFound    := false;      { zn  }
  bDoOnlyStatistics  := false;      { zs  }

{$ifdef MSDOS}
  bInputRedirected  := IsInputRedirected;
  bOutputRedirected := IsOutputRedirected;
{$endif}
end.
