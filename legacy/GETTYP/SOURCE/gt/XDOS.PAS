{$define extended}
{$i aflags.inc}

{
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
important note if you use this unit:
You have to use the DOS unit before
you use this unit.
This is because XSearchRec refers to
the DOS.SearchRec
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

unit XDOS;

interface

uses
{$ifdef WIN32}
     SysUtils, Windows;
{$else}
  {$ifdef WINDOWS}
     WinDOS;
  {$else}
     DOS;
  {$endif}
{$endif}

type
{$ifdef MSDOS}
     XSearchRec = SearchRec;
{$else}
     XSearchRec = TSearchRec;
{$endif}

{$ifndef WIN32}
     LONG_SearchRec = record
       attrib:longint;                  {   4 - 004 }
  {$ifdef __TMT__}
       dt_s:array[1..24] of byte;       {  24 - 028 }
       highfilesize:longint;            {   4 - 032 }
       lowfilesize:longint;             {   4 - 036 }
       reserved:array[1..8] of byte;    {   8 - 044 }
  {$else}
       dt_creation:comp;                {   8 - 012 }
       dt_lastaccess:comp;              {   8 - 020 }
       dt_lastmodification:comp;        {   8 - 028 }
       highfilesize:longint;            {   4 - 032 }
       lowfilesize:longint;             {   4 - 036 }
       reserved:comp;                   {   8 - 044 }
  {$endif}
       name:array[0..259] of char;      { 260 - 304 }
       shortname:array[0..13] of char;  {  14 - 318 }
       handle:word;                     {   2 - 320 }
     end;
{$else}
{ -- Win32 specific -- }
     DateTime = record
       Year:Word;
       Month:Word;
       Day:Word;
       Hour:Word;
       Min:Word;
       Sec:Word;
     end;
{$endif}

const
{$ifndef MSDOS}
      XAnyFile   = faAnyFile;
      XArchive   = faArchive;
      XDirectory = faDirectory;
      XHidden    = faHidden;
      XReadOnly  = faReadOnly;
      XSysFile   = faSysFile;
      XVolumeID  = faVolumeID;
{$else}
      XAnyFile   = AnyFile;
      XArchive   = Archive;
      XDirectory = Directory;
      XHidden    = Hidden;
      XReadOnly  = ReadOnly;
      XSysFile   = SysFile;
      XVolumeID  = VolumeID;
{$endif}

var
    bAllowLongFilenames:boolean;

{ - XFindFirst and XFindNext return 0 as success!! - }
function  XFindFirst (    sFilespec:string;
                          nAttr:byte;
                      var sr:XSearchRec):integer;
function  XFindNext  (var sr:XSearchRec):integer;
procedure XFindClose (var sr:XSearchRec);
function  XIsFile (sr:XSearchRec):boolean;

{$ifndef WIN32}
function LONG_FindFirst (    sFilespec:string;
                             nAttr:word;
                         var sr:LONG_SearchRec):integer;
function LONG_FindNext  (var sr:LONG_SearchRec):integer;
function LONG_FindClose (var sr:LONG_SearchRec):integer;
{$endif}

{$ifdef WIN32}
procedure UnpackTime (nDT:longint; var T:DateTime);
{$endif}

function UnixDT2DOSDT (nUnixDT:longint):longint;

{ - use instead of FExpand - }
function ExpandPath (sPath:string):string;

implementation

{$ifdef WINDOWS}
uses
     Strings;
{$endif}

{$ifndef MSDOS}
var
    p1, p2:pChar;
    pOldExit:pointer;
{$endif}

const
      XDOS_STR_SIZE = 1024;

{
Win32: returns 0 on success
}
function XFindFirst (    sFileSpec:string;
                         nAttr:byte;
                     var sr:XSearchRec):integer;
begin
{$ifndef MSDOS}

  { just for not using $X+ }
  if (StrPCopy (p1, sFilespec) = nil) then;

  {$ifdef WIN32}
  XFindFirst := FindFirst (p1, nAttr, sr);
  {$else}
  FindFirst (p1, nAttr, sr);
  XFindFirst := DosError;
  {$endif}

{$else}

  FindFirst (sFilespec, nAttr, sr);
  XFindFirst := DosError;

{$endif}
end;

function XFindNext (var sr:XSearchRec):integer;
begin
{$ifdef WIN32}
  XFindNext := FindNext (sr);
{$else}
  FindNext (sr);
  XFindNext := DosError;
{$endif}
end;

procedure XFindClose (var sr:XSearchRec);
begin
{$ifdef WIN32}
  SysUtils.FindClose (sr);  { else type error when using unit Windows }
{$endif}
end;

function  XIsFile (sr:XSearchRec):boolean;
begin
  XIsFile := (sr.attr and XVolumeID = 0) and
             (sr.attr and XDirectory = 0) and
             (sr.name [1] <> '.');
end;

{$ifdef WIN32}
{ -- now Win32 specific -- }
procedure UnpackTime (nDT: Longint; var T:DateTime);
type
     LongRec = record
       Lo:word;
       Hi:word;
     end;

begin
  T.Year  := LongRec (nDT).Hi shr 9 + 1980;
  T.Month := LongRec (nDT).Hi shr 5 and 15;
  T.Day   := LongRec (nDT).Hi and 31;
  T.Hour  := LongRec (nDT).Lo shr 11;
  T.Min   := LongRec (nDT).Lo shr 5 and 63;
  T.Sec   := LongRec (nDT).Lo and 31 shl 1;
end;

function PackTime (aST:TSystemTime):longint;
var
    wh, wl:word;
begin
  wh := (aST.wYear - 1980) shl 9 +
        (aST.wMonth shl 5) +
        (aST.wDay);
  wl := (aST.wHour shl 11) +
        (aST.wMinute shl 5) +
        (aST.wSecond shr 1);
  PackTime := (wh shl 16) + wl;
end;
{$endif}

{$ifndef WIN32}
{ -- DOS specific -- }
function LONG_FindFirst (    sFilespec:string;
                             nAttr:word;
                         var sr:LONG_SearchRec):integer;
begin
  sFilespec := concat (sFilespec, #0);
  sr.attrib := nAttr;
  asm
    push ds
    push ss
    pop ds
    lea dx, sFilespec + 1  { to skip the [0] length char }
    les di, sr

    mov ax, $714E
    mov cx, nAttr
    mov si, 0
    int $21

    les di, sr
    {
    mov word ptr es:[di + LONG_SearchRec.handle], ax
    }
    mov word ptr es:[di + 318], ax
    jc  @1
    xor ax, ax
  @1:
    mov @result, ax
    pop ds
  end;
end;

function LONG_FindNext (var sr:LONG_SearchRec):integer;
assembler;
asm
  mov ax, $714F
  mov si, 0
  les di, sr
  {
  mov bx, word ptr es:[di + LONG_SearchRec.Handle]
  }
  mov bx, word ptr es:[di + 318]
  int $21
  jc  @quit
  xor ax, ax
@quit:
end;

function LONG_FindClose (var sr:LONG_SearchRec):integer;
assembler;
asm
  mov ax, $71A1
  les di, sr
  {
  mov bx, word ptr es:[di + LONG_SearchRec.Handle]
  }
  mov bx, word ptr es:[di + 318]
  int $21
  jc  @quit
  xor ax, ax
@quit:
end;

{$endif}

function UnixDT2DOSDT (nUnixDT:longint):longint;
const
      DaysPerMonth:array[1..12] of byte =
         (31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31);
      DaysPerLeapMonth:array[1..12] of byte =
         (31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31);

      DaysPerYear:array[1..12] of word =
        (031, 059, 090, 120, 151, 181, 212, 243, 273, 304, 334, 365);
      DaysPerLeapYear:array[1..12] of word =
        (031, 060, 091, 121, 152, 182, 213, 244, 274, 305, 335, 366);

      SecsPerYear    :longint = 31536000;
      SecsPerLeapYear:longint = 31622400;
      SecsPerDay     :longint = 86400;
      SecsPerHour    :longint = 3600;
      SecsPerMinute  :longint = 60;
var
    bDone:Boolean;
    nTotDays:Integer;
    y, mo, d, h, mi, s:word;
{$ifdef MSDOS}
    DT:DateTime;
{$else}
  {$ifdef DELPHI}
    ST:TSystemTime;
    DT:TDateTime;
  {$else}
    ST:TSystemTime;
  {$endif}
{$endif}
    nDT:longint;

    function Leap_Year(year: word) : boolean;
    begin
       if year and 3 <> 0 then
         leap_year := False
       else
         if year mod 100 <> 0 then
           leap_year := True
         else
           if year mod 400 <> 0 then
             leap_year := False
           else
             leap_year := True;
    end;

Begin
  y := 1970;
  mo := 1;
  d := 1;
  bDone := False;

  { get the years }
  While Not bDone Do
  Begin
    If (nUnixDT >= SecsPerLeapYear) and
       (Leap_Year (y + 1)) then
    Begin
      Inc (y);
      Dec (nUnixDT, SecsPerLeapYear);
    End
    else
      If (nUnixDT >= SecsPerYear) Then
      Begin
        Inc (y);
        Dec (nUnixDT, SecsPerYear);
      End
      Else
        bDone := True;
  End;

  bDone := False;
  nTotDays := nUnixDT Div SecsPerDay;
  If Leap_Year (y) Then
  Begin
    Repeat
      If (nTotDays <= DaysPerLeapYear [mo]) Then
      Begin
        bDone := True;
        Dec (nUnixDT, nTotDays * SecsPerDay);
        d := DaysPerLeapMonth [mo] - (DaysPerLeapYear [mo] - nTotDays) + 1;
      End
      else
        Inc (mo);
    Until (bDone);
  End
  Else
  Begin
    Repeat
      If (nTotDays <= DaysPerYear [mo]) Then
      Begin
        bDone := True;
        Dec (nUnixDT, nTotDays * SecsPerDay);
        d := DaysPerMonth [mo] - (DaysPerYear [mo] - nTotDays) + 1;
      End
      else
        Inc (mo);
    Until (bDone);
  End;

  h := nUnixDT Div SecsPerHour;
  Dec (nUnixDT, h * SecsPerHour);
  mi := nUnixDT Div SecsPerMinute;
  Dec (nUnixDT, mi * SecsPerMinute);
  s := nUnixDT;

{$ifdef MSDOS}
  DT.year := y;
  DT.month := mo;
  DT.day := d;
  DT.hour := h;
  DT.min := mi;
  DT.sec := s;
  PackTime (DT, nDT);
{$else}
  ST.wYear := y;
  ST.wMonth := mo;
  { ST.wDayOfWeek := 0; }
  ST.wDay := d;
  ST.wHour := h;
  ST.wMinute := mi;
  ST.wSecond := s;
  ST.wMilliSeconds := 0;
  {$ifdef DELPHI}
  DT := SystemTimeToDateTime (ST);
  nDT := DateTimeToFileDate (DT);
  {$else}
  nDT := PackTime (ST);
  {$endif}
{$endif}
  UnixDT2DOSDT := nDT;
End;


function ExpandPath (sPath:string):string;
{$ifdef WIN32}
var sRes:string;
{$endif}
begin
{$ifndef MSDOS}

  {$ifdef WIN32}

  if (sPath[2] = ':') and (sPath[3] = '\') then  { e.g. "g:\dosutils\*.com" }
  begin
    if (sPath [length (sPath)] <> '\') then
      sPath := concat (sPath, '\');
    ExpandPath := sPath;
    exit;
  end;

  if (sPath[2] = ':') then { something like "g:delphi3\*.pas" }
  begin
    GetDir (ord (upcase (sPath [1])) - ord ('A') + 1, sRes);
    delete (sPath, 1, 2);
  end
  else
  begin
    GetDir (0, sRes);  { e.g. "out_gt\*.*" }

    if (sPath[1] = '\') then   { \foo\*.txt }
    begin
      ExpandPath := concat (sRes[1], sRes[2], sPath);
      exit;
    end;
  end;

  if (sRes [length (sRes)] <> '\') then
    sRes := concat (sRes, '\');

  sRes := concat (sRes, sPath);
  if (sRes [length (sRes)] <> '\') then
    sRes := concat (sRes, '\');

  ExpandPath := sRes;

  {$else}

  StrPCopy (p1, sPath);
  FileExpand (p2, p1);
  ExpandPath := StrPas (p2);

  {$endif}

{$else}
  ExpandPath := FExpand (sPath);
{$endif}
end;

{$ifndef MSDOS}
{$f+}
procedure NewExit;
begin
  freemem (p1, XDOS_STR_SIZE);
  freemem (p2, XDOS_STR_SIZE);
  ExitProc := pOldExit;
end;
{$f-}
{$endif}

begin
{$ifndef MSDOS}
  getmem (p1, XDOS_STR_SIZE);
  getmem (p2, XDOS_STR_SIZE);
  pOldExit := ExitProc;
  ExitProc := @NewExit;
{$endif}

{$ifndef WIN32}
  bAllowLongFilenames := false;
  asm
    mov ax, 71A0h
    int 21h
    cmp al, 00h
    je  @NoLFN
    mov bAllowLongFilenames, true
  @NoLFN:
  end;

  if (not bAllowLongFilenames) and (lo (dosversion) >= 7) then
  begin
    { check if windows is running in enhanced mode ... }
    asm
      mov    ax, 1600h
      int    2Fh
      cmp    al, 1
      jbe    @Real
      cmp    al, 80h
      jae    @Real
      { yep, in enhanced mode... }
      mov    bAllowLongFilenames, true
      jmp    @Quit
    @Real:
      { in real or std mode :( }
      mov    bAllowLongFilenames, false
    @Quit:
    end;
  end;
{$else}
  bAllowLongFilenames := false;
{$endif}
end.
