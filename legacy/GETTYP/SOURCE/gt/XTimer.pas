{$define EXTENDED}
{$i aflags.inc}

unit Xtimer;

interface

{$ifdef WIN32}
uses
     SysUtils;
{$endif}

type
{$ifdef WIN32}
     TimeFormat = TTimeStamp;
{$else}
     TimeFormat = longint;
{$endif}

const
      TICKS_PER_SECOND = 18.206;
      TIMER_RESOLUTION = 1193181.667;

type
     Timer = object
     { private ... }
       bStart:boolean;
       bBreak:boolean;
       nOverflows:byte;

       aSaved,
       aTimerStart,
       aTimerStop:TimeFormat;
       function ElapsedTime (aStart, aEnd:TimeFormat):real;

     { public ... }
       function GetTime (var l:TimeFormat):boolean;

       procedure Reset;
       procedure Start;
       procedure Stop;

       procedure DoBreak;
       procedure DoContinue;

       { works only after Start and Stop !! }
       function GetMilliSeconds:longint;
       function GetSeconds:real;

       { works always !! }
       function GetActMilliSeconds:longint;
       function GetActSeconds:real;
     end;

function ReadTimer:TimeFormat;

implementation

const
      MAX_LONG  = 4294967296.0;
      HALF_LONG = 2147483647;

{$ifndef WIN32}
var
    SaveExitProc:pointer;
    nDelta:LongInt;

{ Return the unsigned equivalent of L as a real }
function Cardinal (n:longint):real;
begin
  if (n < 0) then
    Cardinal := MAX_LONG + n
  else
    Cardinal := n;
end;

{ Calculate time elapsed (in milliseconds) between Start and Stop }
function Timer.ElapsedTime (aStart, aEnd:TimeFormat):real;
var dTicks:real;
begin
  dTicks := Cardinal (aEnd - (aStart + nDelta)) + nOverflows * HALF_LONG;
  ElapsedTime := 1000.0 * (dTicks / TIMER_RESOLUTION);
end;

{ read the timer with 1 microsecond resolution }
function ReadTimer:TimeFormat;
begin
  {$ifndef BORLAND}
    {$ifdef OS2}
      ReadTimer := 0;
    {$else}
      ReadTimer := meml [$0040:$6C00];
    {$endif}
  {$else}
  inline(
    $FA/                   { cli             ;Disable interrupts}
    $BA/$20/$00/           { mov  dx,$20     ;Address PIC ocw3}
    $B0/$0A/               { mov  al,$0A     ;Ask to read irr}
    $EE/                   { out  dx,al}
    $B0/$00/               { mov  al,$00     ;Latch timer 0}
    $E6/$43/               { out  $43,al}
    $EC/                   { in   al,dx      ;Read irr}
    $89/$C7/               { mov  di,ax      ;Save it in DI}
    $E4/$40/               { in   al,$40     ;Counter --> bx}
    $88/$C3/               { mov  bl,al      ;LSB in BL}
    $E4/$40/               { in   al,$40}
    $88/$C7/               { mov  bh,al      ;MSB in BH}
    $F7/$D3/               { not  bx         ;Need ascending counter}
    $E4/$21/               { in   al,$21     ;Read PIC imr}
    $89/$C6/               { mov  si,ax      ;Save it in SI}
    $B0/$FF/               { mov  al,$0FF    ;Mask all interrupts}
    $E6/$21/               { out  $21,al}
    $B8/$40/$00/           { mov  ax,$40     ;read low Word of time}
    $8E/$C0/               { mov  es,ax      ;from BIOS data area}
    $26/$8B/$16/$6C/$00/   { mov  dx,es:[$6C]}
    $89/$F0/               { mov  ax,si      ;Restore imr from SI}
    $E6/$21/               { out  $21,al}
    $FB/                   { sti             ;Enable interrupts}
    $89/$F8/               { mov  ax,di      ;Retrieve old irr}
    $A8/$01/               { test al,$01     ;Counter hit 0?}
    $74/$07/               { jz   done       ;Jump if not}
    $81/$FB/$FF/$00/       { cmp  bx,$FF     ;Counter > $FF?}
    $77/$01/               { ja   done       ;Done if so}
    $42/                   { inc  dx         ;else count int req.}
  { done: }
    $89/$5E/$FC/           { mov [bp-4],bx   ;set Function result}
    $89/$56/$FE);          { mov [bp-2],dx}
  {$endif}
end;

{$ifdef MSDOS}
procedure InitializeTimer;  { ReProgram the timer chip to allow 1 microsecond resolution }
begin
  port[$43] := $34;        { 00110100b }
  inline ($EB/$00);        { jmp short $+2 ;Delay }
  port[$40] := $00;        { LSB = 0 }
  inline ($EB/$00);        { jmp short $+2 ;Delay }
  port[$40] := $00;        { MSB = 0 }
end;

procedure RestoreTimer;  { Restore the timer chip to its normal state }
begin
  { select timer mode 3, read/Write channel 0 }
  port[$43] := $36;        { 00110110b }
  inline ($EB/$00);        { jmp short $+2 ;Delay }
  port[$40] := $00;        { LSB = 0 }
  inline ($EB/$00);        { jmp short $+2 ;Delay }
  port[$40] := $00;        { MSB = 0 }
end;

procedure CalibrateTimer;
const REPS = 1000;
var i:Word;
    l1, l2, nDiff:longInt;
begin
  nDelta := 32767;  { maxint }
  For i := 1 to REPS do
  begin
    l1 := ReadTimer;
    l2 := ReadTimer;
    {use the minimum difference}
    nDiff := l2 - l1;
    if nDiff < nDelta then
      nDelta := nDiff;
  end;
end;

{$F+}
procedure NewExitProc;  { Restore timer chip to its original state }
begin
  ExitProc := SaveExitProc;
  RestoreTimer;
end;
{$F-}
{$endif}  { - MSDOS - }

{$else}   { -- WIN32 -- }

{ Calculate time elapsed (in milliseconds) between Start and Stop }
function Timer.ElapsedTime (aStart, aEnd:TimeFormat):real;
begin
  ElapsedTime := (aEnd.Time - aStart.Time);  { - TTimeStamp is in ms - }
end;

{ read the timer with 1 microsecond resolution }
function ReadTimer:TimeFormat;
begin
  ReadTimer := DateTimeToTimeStamp (Time);  { Time is a function!! }
end;

{$endif}  { -- WIN32 -- }


procedure Timer.reset;
begin
{$ifdef WIN32}
  aTimerStart.Time := 0;
  aTimerStop.Time  := 0;
  aSaved.Time      := 0;
{$else}
  aTimerStart      := 0;
  aTimerStop       := 0;
  aSaved           := 0;
{$endif}
  nOverflows       := 0;
  bStart           := false;
  bBreak           := false;
end;

function Timer.GetTime (var l:TimeFormat):boolean;
begin
  if (not bStart) then
    GetTime := false
  else
  begin
    if (bBreak) then
    begin
{$ifdef WIN32}
      l.Time := aTimerStart.Time + aSaved.Time;
{$else}
      l := aTimerStart + aSaved;
{$endif}
    end
    else
    begin
{$ifdef WIN32}
      l.Time := ReadTimer.Time;
{$else}
      l := ReadTimer;
{$endif}
    end;
    GetTime := true;
  end;
end;

procedure Timer.Start;
begin
  Reset;
  bStart := true;
  GetTime (aTimerStart);
end;

procedure Timer.Stop;
begin
  if (not bStart) then
    exit;

  GetTime (aTimerStop);
  bStart := false;
  bBreak := false;
end;

procedure Timer.DoBreak;
var aAct:TimeFormat;
begin
  if (bBreak) or (not bStart) then
    exit;

  aAct := ReadTimer;
{$ifdef WIN32}
  if (aAct.Time > 0) and (aAct.Time - HALF_LONG > aTimerStart.Time) then
  begin
    inc (nOverflows);
    { dec (nAct, HALF_LONG); }
  end
  else
    aSaved.Time := aAct.Time - aTimerStart.Time;
{$else}
  if (aAct > 0) and (aAct - HALF_LONG > aTimerStart) then
  begin
    inc (nOverflows);
    { dec (nAct, HALF_LONG); }
  end
  else
    aSaved := aAct - aTimerStart;
{$endif}

  bBreak := true;
end;

procedure Timer.DoContinue;
begin
  if (not bBreak) or (not bStart) then
    exit;

{$ifdef WIN32}
  aTimerStart.Time := ReadTimer.Time - aSaved.Time;
{$else}
  aTimerStart := ReadTimer - aSaved;
{$endif}
  bBreak := false;
end;

function Timer.GetSeconds:real;
begin
  GetSeconds := GetMilliSeconds / 1000;
end;

function Timer.GetMilliSeconds:longint;
begin
{$ifdef WIN32}
  if (aTimerStop.Time = 0) then
{$else}
  if (aTimerStop = 0) then
{$endif}
    GetMilliSeconds := 0
  else
    GetMilliSeconds := Round (ElapsedTime (aTimerStart, aTimerStop));
end;

function Timer.GetActSeconds:real;
begin
  GetActSeconds := GetActMilliSeconds / 1000;
end;

function Timer.GetActMilliSeconds:longint;
begin
  GetActMilliSeconds := Round (ElapsedTime (aTimerStart, ReadTimer));
end;

begin
{$ifdef MSDOS}
  { set up our Exit handler }
  SaveExitProc := ExitProc;
  ExitProc := @NewExitProc;

  InitializeTimer;
  CalibrateTimer;
{$endif}
end.
