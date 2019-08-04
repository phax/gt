{$i aflags.inc}

unit xcrt;

interface

{$ifdef WIN32}
{$error Sorry, this unit does not support Win32 :(}
{$endif}

function readkey:char;
function keypressed:boolean;
function wherex:byte;
function wherey:byte;
procedure mydelay (ms:word);

implementation

uses
{$ifdef WINDOWS}
     WinCRT,
{$endif}
     XTimer;

{$ifndef WINDOWS}
var
    VideoSeg:word;
    ScanCode:byte;
    _readkey:byte;
{$endif}

function readkey:char;
{$ifdef WINDOWS}
begin
  readkey := WinCRT.readkey;
end;
{$else}
assembler;
asm
  mov   al, scancode
  mov   scancode, 0
  or    al, al
  jne   @done
  xor   ah, ah
  int   $16
  or    al, al
  jne   @done
  mov   scancode, ah
@done:
end;
{$endif}

function keypressed:boolean;
{$ifdef WINDOWS}
begin
  keypressed := WinCRT.keypressed;
end;
{$else}
assembler;
asm
  mov  ah, $01
  int  $16
  jz   @not
  mov  al, 1
  jmp  @done
@not:
  xor  al, al
@done:
end;
{$endif}

function wherex:byte;
{$ifdef WINDOWS}
begin
  wherex := WinCRT.wherex;
end;
{$else}
assembler;
asm
  mov ah, 3
  mov bh, 0
  int $10
  mov al, dl
  inc al
end;
{$endif}

function wherey:byte;
{$ifdef WINDOWS}
begin
  wherey := WinCRT.wherey;
end;
{$else}
assembler;
asm
  mov ah, 3
  mov bh, 0
  int $10
  mov al, dh
  inc al
end;
{$endif}

procedure mydelay (ms:word);
var l:longint;
begin
  l := ReadTimer;
  while (ReadTimer - l < ms) do;
end;

begin
{$ifndef WINDOWS}
  asm
    int $11
    and ax, $30
    cmp ax, $30
    je  @mono
    mov VideoSeg, $b800
    jmp @quit
  @mono:
    mov VideoSeg, $b000
  @quit:
    mov Scancode, 0
    mov _ReadKey, 0
  end;
{$endif}
end.
