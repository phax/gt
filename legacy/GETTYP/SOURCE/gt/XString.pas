{$i aflags.inc}
{&Delphi+}

{$ifdef BORLAND}
{$R-}  { - needed for StringPosInBuffer - }
{$endif}

unit XString;

interface

const
      STR_HEXARRAY:array[0..15] of char = ('0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F');

type
     Boolstr     = string[4];
     AttrStr     = string[4];
     VersionStr  = string[5];
     IntStr      = string[12];
     RealStr     = string[25];
     DateStr     = string[10];
     TimeStr     = string[12];

     HexByteStr  = string[2];
     HexWordStr  = string[4];
     HexDWordStr = string[8];

{ should be defined only in Turbo Pascal 6.0 }
{$ifdef VER60}
     PChar = ^char;
{$endif}

function downstring (s:string):string;
function upstring (s:string):string;
function downcase (c:char):char;
function uppercase (c:char):char;
function ReplaceChar (s:string; o, n:char):string;
function ReplaceString (s, s1, s2:string):string;

function hexbyte(b:byte):HexByteStr;
function hexword(w:word):HexWordStr;
function hexlong(l:longint):HexDWordStr;

function leadingzero (zahl:longint; nLen:word):string;
function lastchar (s:string):char;

function i2s   (i:longint):IntStr;
function i2sf  (i:longint; f:word):IntStr;
function r2s   (r:real):RealStr;
function r2sf  (r:real; f:word):RealStr;
function r2sfd (r:real; f, d:word):RealStr;
function s2r   (s:RealStr):real;
function s2i   (s:IntStr):longint;

function IsSpaceChar (c:char):boolean;
function IsSpaceByte (b:byte):boolean;
function IsNumberChar (c:char):boolean;
function IsHexChar (c:char):boolean;

function StripSpaces (s:string):string;
function Enbraced (s:string):string;
function CopyUntil (s:string; c:char):string;
function CopyFrom (s:string; c:char):string;
function CopyFromLast (s:string; c:char):string;
function NthPos (c:char; s:string; n:byte):byte;
function LastPos (c:char; s:string):byte;
function CPosAt (c:char; s:string; n:byte):byte;
function SPosAt (s1, s2:string; n:byte):byte;
function SLastPos (SubStr, Str:string):byte;
function StrOct2Dec (s:string):longint;
function GetPath (s:string):string;
function GetFilename (s:string):string;
function GetVersionStr (vhi, vlo:word):VersionStr;
function MakeExtension (FName, Ext:string):string;
function RemoveUniCodeChars (s:string):string;

procedure OverwriteString (var dst:string; n:byte; src:string);

function GetDateStr (DT:longint):DateStr;
function GetTimeStr (DT:longint):TimeStr;
function GetCurrentDateStr:DateStr;
function GetCurrentTimeStr:TimeStr;
function GetAttrStr (Attr:word):AttrStr;

function GetCharPos (c:char;
                     s:string):byte;
function GetCharCount (c:char;
                       s:string):byte;
{$ifndef WIN32}
procedure SetLength (var s:string;
                         nLen:byte);
{$endif}

{ taken from Strings.pas; RTL 7.0 }
{$ifdef MSDOS}
function StrPas (Str:PChar):string;
{$endif}
function StringPosInBuffer (var Buffer;
                                Size:word;
                                S:string):word;
function MatchesRegEx (sReg, sStr:string):boolean;

{$ifdef P32}
procedure Str (n:longint; var s:string);
function  Concat (s1, s2:string):string;
function  Concat (c:char; s:string):string;
{$endif}

implementation

{$ifdef WIN32}
uses SysUtils,
     Windows,
     XDos;  { UnpackTime }
{$else}
  {$ifdef WINDOWS}
uses WinDOS;
  {$else}
uses
     DOS;
  {$endif}
{$endif}

function lastchar (s:string):char;
{$ifndef MSDOS}
begin
  lastchar := s[length (s)];
end;
{$else}
assembler;
asm
  push ds
  cld
  lds  si, s[0]
  lodsb
  xor  ah, ah    { empty ah }
  xchg ax, cx    { al -> cx }
  jcxz @SetZero  { if length is zero -> quit }
  rep  lodsb     { skip through until the last char }
  jmp  @Quit     { the last char is in AX }
@SetZero:
  xor  ax, ax    { return null byte }
@Quit:
  pop  ds
end;
{$endif}

function leadingzero (zahl:longint; nLen:word):string;
var tmp:string;
    index:byte;
begin
  str (zahl, tmp);
  for index := length (tmp) + 1 to nLen do
    tmp := concat ('0', tmp);
  leadingzero := tmp;
end;

function uppercase (c:char):char;
{$ifdef WIN32}
begin
  uppercase := upcase (c);
end;
{$else}
assembler;
asm
  mov al, c
  cmp al, 'a'
  jb  @isupper
  cmp al, 'z'
  ja  @isupper
  sub al, $20
@isupper:
end;
{$endif}

function downcase (c:char):char;
{$ifdef WIN32}
begin
  if (c >= 'A') and (c <= 'Z') then
    downcase := chr (ord (c) + 32)
  else
    downcase := c;
end;
{$else}
assembler;
asm
  mov al, c
  cmp al, 'A'
  jb  @islower
  cmp al, 'Z'
  ja  @islower
  add al, $20
@islower:
end;
{$endif}

function upstring (s:string):string;
{$ifndef BORLAND}
var i:byte;
begin
  for i:=1 to length (s) do
    s[i] := upcase (s[i]);
  upstring := s;
end;
{$else}
  {$ifdef WIN32}
begin
  upstring := SysUtils.UpperCase (s);
end;
  {$else}
assembler;
asm
  push ds
  cld
  lds  si, s[0]
  les  di, @Result
  lodsb          { get the current length of the string -> in "al" }
  stosb          { write the length to the result string }
  xor  ah, ah    { empty ah }
  xchg ax, cx    { al -> cx }
  jcxz @quit     { if length is zero -> quit }
@loop:
  lodsb          { load next char }
  cmp  al, 'a'   { check for 'a' }
  jb   @isupper  { if smaller -> next char }
  cmp  al, 'z'   { check for 'z' }
  ja   @isupper  { if greater -> next char }
  sub  al, $20   { add $20 -> now uppercase }
@isupper:
  stosb          { write byte }
  loop @loop     { goto next char until cx is 0 }
@quit:
  pop  ds
end;
  {$endif}  { - WIN32 - }
{$endif}

function downstring (s:string):string;
{$ifndef BORLAND}
var i:byte;
begin
  for i:=1 to length (s) do
    s[i] := downcase (s[i]);
  downstring := s;
end;
{$else}
  {$ifdef WIN32}
begin
  downstring := SysUtils.LowerCase (s);
end;
  {$else}
assembler;
asm
  push ds
  cld
  lds  si, s[0]
  les  di, @result
  lodsb
  stosb
  xor  ah, ah
  xchg ax, cx
  jcxz @quit
@read:
  lodsb
  cmp  al, 'A'
  jb   @write
  cmp  al, 'Z'
  ja   @write
  add  al, $20
@write:
  stosb
  loop @read
@quit:
  pop  ds
end;
  {$endif}
{$endif}

function ReplaceChar (s:string; o, n:char):string;
{$ifndef BORLAND_DOS}
var i:byte;
begin
  for i:=1 to length (s) do
    if (char (s[i]) = o) then
      s[i] := n;
  ReplaceChar := s;
end;
{$else}
assembler;
asm
  push ds
  cld
  lds  si, s[0]
  les  di, @result
  lodsb
  stosb
  xor  ah, ah
  xchg ax, cx
  jcxz @quit
@loop:
  lodsb
  cmp  al, o
  jne  @write
  mov  al, n
@write:
  stosb
  loop @loop
@quit:
  pop ds
end;
{$endif}

function ReplaceString (s, s1, s2:string):string;
var p:byte;
begin
  p := pos (s1, s);
  if (p > 0) then
  begin
    delete (s, p, length (s1));
    insert (s2, s, p);
  end;
  ReplaceString := s;
end;

function hexbyte(b:byte):HexByteStr;
begin
  hexbyte := concat (STR_HEXARRAY[b shr 4], STR_HEXARRAY[b and $f]);
end;

function hexword(w:word):HexWordStr;
begin
  hexword := concat (hexbyte (hi (w)), hexbyte (lo (w)));
end;

function hexlong(l:longint):HexDWordStr;
var t:array[1..2] of word absolute l;
begin
  hexlong := concat (hexword (t[2]), hexword (t[1]));
end;

function i2s (i:longint):IntStr;
var sdummy:IntStr;
begin
  str (i, sdummy);
  i2s := sdummy;
end;

function i2sf (i:longint; f:word):IntStr;
var sdummy:IntStr;
begin
  str (i:f, sdummy);
  i2sf := sdummy;
end;

function r2s (r:real):RealStr;
var sdummy:RealStr;
begin
  str (r, sdummy);
  r2s := sdummy;
end;

function r2sf (r:real; f:word):RealStr;
var sdummy:RealStr;
begin
  str (r:f, sdummy);
  r2sf := sdummy;
end;

function r2sfd (r:real; f, d:word):RealStr;
var sdummy:RealStr;
begin
  str (r:f:d, sdummy);
  r2sfd := sdummy;
end;

function s2r (s:RealStr):real;
var code:integer;
    a:real;
begin
  val (s, a, code);
  s2r := a;
end;

function s2i (s:IntStr):longint;
var code:integer;
    a:longint;
begin
  val (s, a, code);
  s2i := a;
end;

function IsSpaceChar (c:char):boolean;
begin
  IsSpaceChar := (c = #9) or
                 (c = #10) or
                 (c = #13) or
                 (c = #32);
end;

function IsSpaceByte (b:byte):boolean;
begin
  IsSpaceByte := (b = 9) or
                 (b = 10) or
                 (b = 13) or
                 (b = 32);
end;

function IsNumberChar (c:char):boolean;
begin
  IsNumberChar := (c >= '0') and (c <= '9');
end;

function IsHexChar (c:char):boolean;
begin
  IsHexChar := ((c >= '0') and (c <= '9')) or
               ((c >= 'a') and (c <= 'f')) or
               ((c >= 'A') and (c <= 'F'));
end;

function StripSpaces (s:string):string;
var len, p:byte;
begin
  StripSpaces := '';
  len := length (s);

  if len = 0 then
    exit;

  p := 0;
  while (IsSpaceChar (s[p + 1])) and (p + 1 <= len) do
    inc (p);

  if (p = len) then
    exit;

  if (p > 0) then
  begin
{$ifdef WIN32}
    s := copy (s, p + 1, 255);
{$else}
    move (s[p + 1], s[1], length (s) - p);
    dec (s[0], p);
{$endif}
  end;

  len := length (s);
  while (IsSpaceChar (s[len])) do
    dec (len);

{$ifdef WIN32}
  s := copy (s, 1, len);
{$else}
  s[0] := chr (len);
{$endif}

  StripSpaces := s;
end;

function Enbraced (s:string):string;
begin
  Enbraced := concat ('[', s, ']');
end;

function CopyUntil (s:string; c:char):string;
var p:byte;
begin
  p := pos (c, s);
  if (p > 0) then
    CopyUntil := copy (s, 1, p - 1)
  else
    CopyUntil := s;
end;

function CopyFrom (s:string; c:char):string;
var p:byte;
begin
  p := pos (c, s);
  if (p > 0) then
    CopyFrom := copy (s, p + 1, length (s))
  else
    CopyFrom := '';
end;

function CopyFromLast (s:string; c:char):string;
var p:byte;
begin
  p := LastPos (c, s);
  if (p > 0) then
    CopyFromLast := copy (s, p + 1, length (s))
  else
    CopyFromLast := '';
end;

function LastPos (c:char; s:string):byte;
var i:byte;
begin
  for i:=length (s) downto 1 do
  begin
    if (s[i] = c) then
    begin
      LastPos := i;
      exit;
    end;
  end;

  LastPos := 0;
end;

function NthPos (c:char;
                 s:string;
                 n:byte):byte;
var i, f:byte;
begin
  if (n = 0) then
    NthPos := 1
  else
  begin
    f := 0;
    for i:= 1 to length (s) do
      if (s[i] = c) then
      begin
        inc (f);
        if (f = n) then
        begin
          NthPos := i;
          exit;
        end;
      end;
    NthPos := 0;
  end;
end;

function CPosAt (c:char; s:string; n:byte):byte;
var p:byte;
begin
  s := copy (s, n, 255);
  p := pos (c, s);
  if (p > 0) then
    CPosAt := p + n - 1
  else
    CPosAt := 0;
end;

function SPosAt (s1, s2:string; n:byte):byte;
var p:byte;
begin
  s2 := copy (s2, n, 255);
  p := pos (s1, s2);
  if (p > 0) then
    SPosAt := p + n - 1
  else
    SPosAt := 0;
end;

function SLastPos (SubStr, Str:string):byte;
var lenss, lens, i, p:byte;
begin
  lenss := length (SubStr);
  lens  := length (Str);

  SLastPos := 0;

  if (lens > 0) and (lenss > 0) and (lens >= lenss) then
  begin
    p := lenss;
    { traverse back through original string }
    for i:=lens downto lens - lenss do
    begin
      if (Str [i] = SubStr [p]) then
      begin
        dec (p);
        if (p = 0) then
        begin
          SLastPos := i;
          exit;
        end;
      end
      else
        p := lenss;
    end;
  end;
end;

function StrOct2Dec (s:string):longint;
const MAXSIZELEN = 11;
      x:array[1..MAXSIZELEN] of longint = (         1,
                                                    8,
                                                   64,
                                                  512,
                                                 4096,
                                                32768,
                                               262144,
                                              2097152,
                                             16777216,
                                            134217728,
                                           1073741827);
var v:longint;
    i:byte;
begin
  v := 0;
  for i:=0 to MAXSIZELEN - 1 do
    inc (v, (ord (s[i + 1]) - 48) * x[MAXSIZELEN - i]);
  StrOct2Dec := v;
end;

function GetPath (s:string):string;
var i:byte;
begin
  for i:=length (s) downto 1 do
    if (s[i] = '\') then
    begin
      GetPath := copy (s, 1, i);
      exit;
    end;
  GetPath := '';
end;

function GetFilename (s:string):string;
var i:byte;
begin
  for i:=length (s) downto 1 do
    if (s[i] = '\') then
    begin
      GetFilename := copy (s, i + 1, 255);
      exit;
    end;
  GetFilename := '';
end;

function GetVersionStr (vhi, vlo:word):VersionStr;
begin
  GetVersionStr := concat (i2s (vhi), '.', i2s (vlo));
end;

function MakeExtension (FName, Ext:string):string;
begin
  if pos ('.', FName) = 0 then
    MakeExtension := concat (FName, '.', Ext)
  else
    MakeExtension := concat (CopyUntil (FName, '.'), '.', Ext);
end;

function RemoveUniCodeChars (s:string):string;
var i:byte;
    sRes:string;
begin
  sRes := '';
  for i:=1 to length (s) do
    if (s[i] <> #0) then
      sRes := concat (sRes, s[i]);
  RemoveUniCodeChars := sRes;
end;

procedure OverwriteString (var dst:string; n:byte; src:string);
var i, len:byte;
begin
  len := length (src);
  if (n + len - 1 > length (dst)) then
    len := length (dst) - n + 1;
  for i:=0 to len - 1 do
    dst [n + i] := src [i + 1];
end;

function GetDateStr (DT:longint):DateStr;
var dtrec:DateTime;
begin
  UnpackTime (DT, dtrec);
  GetDateStr := concat (LeadingZero (dtrec.Day, 2),   '.',
                        LeadingZero (dtrec.Month, 2), '.',
                        LeadingZero (dtrec.Year mod 100, 2));
end;

function GetTimeStr (DT:longint):TimeStr;
var dtrec:DateTime;
begin
  UnpackTime (DT, dtrec);
  GetTimeStr := concat (LeadingZero (dtrec.Hour, 2), ':',
                        LeadingZero (dtrec.Min, 2),  ':',
                        LeadingZero (dtrec.Sec, 2));
end;

function GetCurrentDateStr:DateStr;
const DATESTR = 'DD.MM.YYYY';
var
{$ifndef WIN32}
    w,
{$endif}
    y, m, d:word;
    s:string;
    nPos, nLen:byte;
{$ifdef WIN32}
    aST:TSystemTime;
{$endif}
begin
{$ifdef WIN32}
  GetSystemTime (aST);
  y := aST.wYear;
  m := aST.wMonth;
  d := aST.wDay;
{$else}
  GetDate (y, m, d, w);
{$endif}
  s := DATESTR;

  nPos := pos ('D', DATESTR);
  nLen := GetCharCount ('D', DATESTR);
  OverWriteString (s, nPos, LeadingZero (d, nLen));

  nPos := pos ('M', DATESTR);
  nLen := GetCharCount ('M', DATESTR);
  OverWriteString (s, nPos, LeadingZero (m, nLen));

  nPos := pos ('Y', DATESTR);
  nLen := GetCharCount ('Y', DATESTR);
  OverWriteString (s, nPos, LeadingZero (y, nLen));

  GetCurrentDateStr := s;
end;

function GetCurrentTimeStr:TimeStr;
const TIMESTR = 'HH:MM:SS';
var h, m, sec, ms:word;
    s:string;
    nPos, nLen:byte;
{$ifdef WIN32}
    aST:TSystemTime;
{$endif}
begin
{$ifdef WIN32}
  GetSystemTime (aST);
  h   := aST.wHour;
  m   := aST.wMinute;
  sec := aST.wSecond;
  ms  := 0;
{$else}
  GetTime (h, m, sec, ms);
{$endif}
  s := TIMESTR;

  nPos := pos ('H', TIMESTR);
  nLen := GetCharCount ('H', TIMESTR);
  OverWriteString (s, nPos, LeadingZero (h, nLen));

  nPos := pos ('M', TIMESTR);
  nLen := GetCharCount ('M', TIMESTR);
  OverWriteString (s, nPos, LeadingZero (m, nLen));

  nPos := pos ('S', TIMESTR);
  nLen := GetCharCount ('S', TIMESTR);
  OverWriteString (s, nPos, LeadingZero (sec, nLen));

  GetCurrentTimeStr := s;
end;

function GetAttrStr (Attr:word):AttrStr;
var s:AttrStr;
begin
  s := 'תתתת';
  if (Attr and $0001 > 0) then s[1] := 'R';
  if (Attr and $0002 > 0) then s[2] := 'H';
  if (Attr and $0004 > 0) then s[3] := 'S';
  if (Attr and $0020 > 0) then s[4] := 'A';
  GetAttrStr := s;
end;

function GetCharPos (c:char;
                     s:string):byte;
begin
  if (s = '') then
    GetCharPos := 0
  else
    GetCharPos := pos (c, s);
end;

function GetCharCount (c:char;
                       s:string):byte;
var i, nCount, nLen:byte;
begin
  nCount := 0;
  nLen := length (s);
  for i:=1 to nLen do
    if (s[i] = c) then
      inc (nCount);
  GetCharCount := nCount;
end;

{$ifndef WIN32}
procedure SetLength (var s:string;
                         nLen:byte);
begin
  s[0] := chr (nLen);
end;
{$endif}

{$ifdef MSDOS}
function StrPas (Str:PChar):string;
assembler;
asm
  push  ds
  cld
  LES   DI, Str
  MOV   CX, 0FFFFH
  XOR   AL, AL
  REPNE SCASB
  NOT   CX
  DEC   CX
  LDS   SI, Str
  LES   DI, @Result
  MOV   AL, CL
  STOSB
  REP   MOVSB
  POP   DS
end;
{$endif}

function StringPosInBuffer (var Buffer;
                                Size:word;
                                S:string):word;
{ Search in Buffer of Size bytes for the string S }
{$ifdef SLOW_STRINGPOSINBUFFER}
type TDummyArray = array[0..1] of char;
var nPos:word;
    nSavedPos, nLen:byte;
    aBuf:TDummyArray absolute Buffer;
begin
  nLen := length (s);
  nSavedPos := 1;
  for nPos:=1 to Size do { to Size - nLen + 1 do }
  begin
    if (s [nSavedPos] = aBuf [nPos]) then
    begin
      inc (nSavedPos);
      if (nSavedPos > nLen) then
      begin
        StringPosInBuffer := nPos;
        exit;
      end;
    end
    else
      nSavedPos := 1;
  end;

  StringPosInBuffer := 0;
end;
{$else}
begin
  { Load "buffer" address into ES:DI, "buffer" offset into BX, Length(s) -
    1 into DX, contents of "s[1]" into AL, offset of "s[2]" into SI, and
    "size" - Length(s) + 1 into CX.  If "size" < Length(s), or if
    Length(s) = 0, return zero. }

  Inline($1E/               {        PUSH    DS           }
         $16/               {        PUSH    SS           }
         $1F/               {        POP     DS           }
         $C4/$BE/>buffer/   {        LES     DI,buffer[BP]}
         $89/$FB/           {        MOV     BX,DI        }
         $8B/$8E/>size/     {        MOV     CX,size[bp]  }
         $8D/$B6/>s+2/      {        LEA     SI,s+2[bp]   }
         $8A/$86/>s+1/      {        MOV     AL,s+1[bp]   }
         $8A/$96/>s/        {        MOV     DL,s[bp]     }
         $84/$D2/           {        TEST    DL,DL        }
         $74/$23/           {        JZ      ERROR        }
         $FE/$CA/           {        DEC     DL           }
         $30/$F6/           {        XOR     DH,DH        }
         $29/$D1/           {        SUB     CX,DX        }
         $76/$1B/           {        JBE     ERROR        }

  { Scan the ES:DI buffer, looking for the first occurrence of "s[1]."  If
    not found prior to reaching Length(s) characters before the end of the
    buffer, return zero.  If Length(s) = 1, the entire string has been
    found, so report success. }

       $FC/               {        CLD                  }
       $F2/               {NEXT:   REPNE                }
       $AE/               {        SCASB                }
       $75/$16/           {        JNE     ERROR        }
       $85/$D2/           {        TEST    DX,DX        }
       $74/$0C/           {        JZ      FOUND        }

  { Compare "s" (which is at SS:SI) with the ES:DI buffer, in both cases
    starting with the first byte just past the length byte of the string.
    If "s" does not match what is at the DI position of the buffer, reset
    the registers to the values they had just prior to the comparison, and
    look again for the next occurrence of the length byte. }

         $51/               {        PUSH    CX           }
         $57/               {        PUSH    DI           }
         $56/               {        PUSH    SI           }
         $89/$D1/           {        MOV     CX,DX        }
         $F3/               {        REPE                 }
         $A6/               {        CMPSB                }
         $5E/               {        POP     SI           }
         $5F/               {        POP     DI           }
         $59/               {        POP     CX           }
         $75/$EC/           {        JNE     NEXT         }

  { String found in buffer.  Set AX to the offset, within buffer, of the
    first byte of the string (the length byte), assuming that the first
    byte of the buffer is at offset 1. }

         $89/$F8/           {FOUND:  MOV     AX,DI        }
         $29/$D8/           {        SUB     AX,BX        }
         $EB/$02/           {        JMP     SHORT RETURN }

  { An "error" condition.  Return zero. }

         $31/$C0/           {ERROR:  XOR     AX,AX        }
         $89/$46/$FE/       {RETURN: MOV     [BP-2],AX    }
         $1F)               {        POP     DS           }
end;
{$endif}

function DOS_MatchesRegEx (sRegEx, sStr:string):boolean;
var i, nRegEx, nName:byte;

    function IsOnlyWildcard (s:string):boolean;
    var j, nLen:byte;
    begin
      IsOnlyWildcard := false;
      nLen := length (s);
      for j:=1 to nLen do
        if (s[j] <> '?') and (s[j] <> '*') then
          exit;
      IsOnlyWildcard := true;
    end;

begin
  nRegEx := length (sRegEx);
  nName  := length (sStr);

  if (IsOnlyWildcard (sRegEx)) then
  begin
    DOS_MatchesRegEx := true;
    exit;
  end;

  if (nName = 0) or (nRegEx = 0) then
  begin
    if (nRegEx = 0) then
      { - if no regex ("makefile.") only match if ext also "" - }
      DOS_MatchesRegEx := (nName = 0)
    else
      { - len (regex) > 0 -> len (name) = 0 -> no only wildcars -> false - }
      DOS_MatchesRegEx := false;
    exit;
  end;

  { sRegEx: "gtaction.txt" sName: "gt.txt" }
  if (nRegEx > nName) then
    if (not IsOnlyWildcard (copy (sRegEx, nName + 1, 255))) then
    begin
      DOS_MatchesRegEx := false;
      exit;
    end;

  for i:=1 to nName do
  begin
    { e.g. "gt.?x?" does no match "gt2.exe" }
    if (i > nRegEx) then
    begin
      DOS_MatchesRegEx := false;
      exit;
    end;

    case upcase (sRegEx [i]) of
      '?':;  { - ignore - }
      '*':begin
            DOS_MatchesRegEx := true;
            exit; { matches!! }
          end;
    else
      if (sStr [i] <> sRegEx [i]) then
      begin
        DOS_MatchesRegEx := false;
        exit;
      end;
    end;
  end;

  DOS_MatchesRegEx := true;
end;

function MatchesRegEx (sReg, sStr:string):boolean;
const bCaseSensitive = false;
var nRegLen, nStrLen:byte;
    nRegPos, nMinimumRestLen, nStrPos:byte;
    cReg, cStr:char;
    sTemp:string;

    procedure NextRegChar;
    begin
      inc (nRegPos);
      if (nRegPos > nRegLen) then
        cReg := #0
      else
        cReg := sReg [nRegPos];
    end;

    procedure NextStrChar;
    begin
      inc (nStrPos);
      if (nStrPos > nStrLen) then
        cStr := #0
      else
        cStr := sStr [nStrPos];
    end;

    function EqualChars:boolean;
    begin
      EqualChars := (cReg = cStr);
    end;

    function OnlyWildCardsLeft:boolean;
    begin
      OnlyWildCardsLeft := false;

      while (cReg <> #0) do
      begin
        { if '?' does not mean necessarily a char add "and (cReg <> '?')" }
        if (cReg <> '*') then
          exit;
        NextRegChar;
      end;

      OnlyWildCardsLeft := true;
    end;

    procedure GetNonWildcardString;
    begin
      sTemp := '';
      while (cReg <> #0) and (cReg <> '?') and (cReg <> '*') do
      begin
        sTemp := concat (sTemp, cReg);
        NextRegChar;
      end;
    end;

begin
  if (not bCaseSensitive) then
  begin
    sStr := upstring (sStr);
    sReg := upstring (sReg);
  end;

  nRegLen := length (sReg);
  nStrLen := length (sStr);

  nRegPos := 0;
  nStrPos := 0;

  NextRegChar;
  NextStrChar;

  repeat
    case cReg of
      '?':begin
            NextRegChar;
            NextStrChar;
          end;  { simply do not compare }
      '*':begin
            { get the next non wildcard char }
            { e.g. "bla*??*ha.pas" }
            nMinimumRestLen := 0;
            repeat
              NextRegChar;
              if (cReg = '?') then
                inc (nMinimumRestLen);
            until (cReg <> '*') and (cReg <> '?');
            {
            here a ? does absolutley not matter.
            a ? matters only at the end of a string "*pas*?"
            but here is not the end of a string
            }

            { if it is the last char - skip all and true }
            if (cReg = #0) then
            begin
              { -1 because StrPos is already one fiurther }
              MatchesRegEx := (nStrPos + nMinimumRestLen - 1 <= nStrLen);
              exit;
            end;

            { get all between }
            GetNonWildcardString;
            {
            if (cReg = #0) then
            begin
              MatchesRegEx := true;
              exit;
            end;
            }

            { sTemp can be '' if cReg is '?' }

            { check if end of regex string:
              if end: get last position otherwise
              there would be an error when matching "*L" and "DLL"
              because the first L would be takne. }
            if (cReg = #0) then
              nStrPos := SLastPos (sTemp, sStr)
            else
              nStrPos := SPosAt (sTemp, sStr, nStrPos);

            if (nStrPos = 0) then
            begin
              MatchesRegEx := false;
              exit;
            end
            else
            begin
              {
              if length of sTemp is 1 then back on previous pos
              but with NextStrChar back on original position
              and nStrPos is surely >= 1 otherwise not here ...
              }
              inc (nStrPos, length (sTemp) - 1);
              NextStrChar;
            end;
          end;
    else
      if (not EqualChars) then
      begin
        MatchesRegEx := false;
        exit;
      end;
      NextRegChar;
      NextStrChar;
    end;
  until (cReg = #0) or (cStr = #0);

  { matched only if both are finished equally }
  if (cStr <> #0) then
    MatchesRegEx := false
  else
    MatchesRegEx := (cReg = #0) or (OnlyWildCardsLeft);
end;

{$ifdef P32}
procedure Str (n:longint; var s:string);
var temp:byte;
    bNegative:boolean;
begin
  s := '';
  if (n < 0) then
  begin
    bNegative := true;
    n := abs (n);
  end
  else
    bNegative := false;

  repeat
    temp := n mod 10;
    s := concat (chr (ord ('0') + temp), s);
    n := n div 10;
  until (n = 0);

  if (bNegative) then
    s := concat ('-', s);
end;

function  Concat (s1, s2:string):string;
assembler;
asm
        CLD
        MOV     EBX,ESP
        MOV     EDX,EDS
        LES     EDI,SS:[EBX+8]  { load s1 }
        LDS     ESI,SS:[EBX+4]  { load s2 }
        MOV     CL,ES:[EDI]    { get length byte }
        XOR     CH,CH
        LODSB
        ADD     ES:[EDI],AL    { inc position with source length }
        JNC     @@1           { check overflow }
        MOV     BYTE PTR ES:[EDI],255
        MOV     AL,CL
        NOT     AL
@@1:
        ADD     EDI,ECX         { inc index of first string }
        INC     EDI            { and one for the length }
        MOV     CL,AL
        REP     MOVSB
        MOV     EDS,EDX
        RETF    4
end;

function  Concat (c:char; s:string):string;
begin
  Concat := c + s;
end;
{$endif}

end.
