{$i aflags.inc}

{$ifdef BORLAND}
{$R-}  { - disable range check! - }
{$endif}

unit GTBuf;

interface

uses
     GTGlobal;

const
      GTBuf_MaxBuffer = 8 * 1024;  { x KB buffer; must be 2 byte aligned! }

      CaseSensitive = true;
      CaseInsensitive = false;

type
     TBuf256 = record
       case byte of
         1: (B:array[1..256] of byte);
         2: (W:array[1..128] of word);
         3: (I:array[1..128] of integer);
         4: (L:array[1.. 64] of longint);
         5: (C:array[1..256] of char);
     end;

     pByteArray = ^tByteArray;
     tByteArray = array[0..0] of byte;  { disable range check! }

     TBuffer = record
       FName:String[80];     { file name with path }
       DisplayName:string;   { longfilename in Win95/98 }
       FExt:String[3];       { extension - needed for textfile }
       FSize:longint;        { file size }

       _AllocLen:word;       { must be 2 bytes aligned allocated! }
       BufLen:word;          { length of the buffer }
       p:pByteArray;         { array of bytes - length unknown }
       First:TBuf256;        { first 256 bytes in different appearences }

       IsText:boolean;
       IsUnixText:boolean;
       Is7Bit:boolean;
     end;

procedure BUF_Init (sDisplayName:string;
                    sOpenName:string;
                    nSize:longint);
procedure BUF_Done;
procedure BUF_GetFileMode;
function  BUF_GetByte (n:TPos):byte;
function  BUF_GetChar (n:TPos):char;
function  BUF_GetWord (n:TPos):word;
function  BUF_GetLong (n:TPos):longint;
function  BUF_GetString (n:TPos;
                         nLen:byte):string;
function  BUF_GetStringUntil (nStartPos:TPos;
                              nEndByte:byte):string;
function  BUF_GetPascalString (n:TPos):string;
function  BUF_GetPascalStringLen (n:TPos):byte;
function  BUF_GetLongString (n:TPos;
                             nLen:word):string;
procedure BUF_GetBuf (var buf;
                          nStartPos:TPos;
                          nLength:TPos);
function  BUF_GetPosOfByte (nStartPos:TPos;
                            nEndPos:TPos;
                            x:byte):TPos;
function  BUF_GetPosOfChar (nStartPos:TPos;
                            nEndPos:TPos;
                            c:char;
                            sensitive:boolean):TPos;
function  BUF_GetPosOfStringCS (nStartPos:TPos;
                                nEndPos:TPos;
                                s:string):TPos;
function  BUF_GetPosOfStringICS (nStartPos:TPos;
                                 nEndPos:TPos;
                                 s:string):TPos;

const
      POSERROR = 0;

var
    Handle:file;
    b:TBuffer;  { the most important global variable }

implementation

uses
     GTMem,
     IOError,
     XGlobal, XString, XWrite;

{********** TBuffer **********}

procedure BUF_AllocMemory;
begin
  if (bVerbose) then writeln ('BUF_AllocMemory ', b._AllocLen);

  { allocate base memory }
  GetMem (b.p, b._AllocLen);
  CheckMemoryAllocation (b.p, b._AllocLen, 'BUF_AllocMemory');

  if (bVerbose) then writeln ('  allocated');

  { read first buffer }
  seek (Handle, 0);  { faster then reset }
  blockread (Handle, b.p^, b.BufLen);
  CheckIOError (ioresult, b.DisplayName);

  if (bVerbose) then writeln ('  moving');

  move (b.p^, b.First, min_w (b.BufLen, 256));
end;

procedure BUF_Init (sDisplayName:string;
                    sOpenName:string;
                    nSize:longint);
var
    nPos:byte;
    io:integer;
begin
  if (bVerbose) then writeln ('BUF_Init (', sDisplayName, ', ', sOpenName, ', ', nSize, ')');

  { check for ".\" }
  if (sOpenName[1] = '.') then
    delete (sOpenName, 1, 2);

  { - convert Size to word - }
  if (nSize > GTBuf_MaxBuffer) then
    b.BufLen := GTBuf_MaxBuffer
  else { - size surely in the range of word - }
    b.BufLen := nSize and $0000FFFF;

  { - set the length to be allocated - must be 2 byte aligned - }
  if (odd (b.BufLen)) then
    b._AllocLen := b.BufLen + 1
  else
    b._AllocLen := b.BufLen;

  if (b.BufLen > 0) then
  begin
    { - the name has to be assigned earlier because of error - }
    nPos := LastPos ('\', sOpenName);
    if (nPos <> 0) then
      b.FName := UpString (copy (sOpenName, nPos + 1, 255))
    else
      b.FName := UpString (sOpenName);

    {
      FileMode is already set to readonly
      Handle is the only (global) file handle ...
    }
    b.p := nil;
    b.DisplayName := sDisplayName;
    b.FExt        := CopyFromLast (b.FName, '.');
    b.FSize       := nSize;

{$ifdef DELPHI}
    AssignFile (Handle, sOpenName);
{$else}
    assign (Handle, sOpenName);
{$endif}
    reset (Handle, 1);
    io := ioresult;
    if (io > 0) then
    begin
      if (bShowIOErrors) then
      begin
        NoteIOError (io);
        FastPrint (concat (' (', sOpenName, ')'#13#10));
      end;
      b.BufLen := 0;
      exit;
    end;

    { first init the variables, then read }
    BUF_AllocMemory;
    BUF_GetFileMode;  { determine wether ascii or ... not }
  end;
end;

procedure BUF_done;
begin
  if (b.p <> nil) then
  begin
    freemem (b.p, b._AllocLen);
    b._AllocLen := 0;
    b.BufLen := 0;
    b.p := nil;
{$ifdef DELPHI}
    CloseFile (Handle);
{$else}
    close (Handle);
{$endif}
  end;
end;

procedure BUF_GetFileMode;
var ASCII, UU, CHR10, CHR13:TPos;
    i:TPos;
    n:byte;
begin
  if (bVerbose) then writeln ('BUF_GetFileMode');
  ASCII := 0;
  UU    := 0;
  CHR10 := 0;
  CHR13 := 0;

  for i:=1 to b.BufLen do
  begin
    n := b.p^ [i - 1];

    if (n = 10) then
    begin
      inc (ASCII);
      inc (UU);
      inc (CHR10);
    end
    else
      if (n = 13) then
      begin
        inc (ASCII);
        inc (UU);
        inc (CHR13);
      end
      else
        if (n = 9) or (n = 12) or (n = 26) then
        begin
          inc (ASCII);
          inc (UU);
        end
        else
        begin
          if (n >= 32) then
            inc (ASCII);
          if (n < 128) then
            inc (UU);
        end;
  end;

  if (b.BufLen > 0) then
  begin
    b.IsText     := (ASCII >= (b.BufLen * 0.99));
    b.IsUnixText := ((CHR13 = 0) and (CHR10 > 0));
    b.Is7Bit     := (UU = b.BufLen);
  end
  else
  begin
    b.IsText     := true;
    b.IsUnixText := false;
    b.Is7Bit     := false;
  end;
end;

function BUF_GetByte (n:TPos):byte;
begin
  if (n <= b.BufLen) then
    BUF_GetByte := b.p^ [n - 1]
  else
    BUF_GetByte := POSERROR;
end;

function BUF_GetChar (n:TPos):char;
begin
  if (n <= b.BufLen) then
    BUF_GetChar := chr (b.p^ [n - 1])
  else
    BUF_GetChar := chr (POSERROR);
end;

function BUF_GetWord (n:TPos):word;
var w:word;
begin
  if (n + 1 <= b.BufLen) then
  begin
    move (b.p^ [n - 1], w, 2);
    BUF_GetWord := w;
  end
  else
    BUF_GetWord := word (POSERROR);
end;

function BUF_GetLong (n:TPos):longint;
var l:longint;
begin
  if (n + 3 <= b.BufLen) then
  begin
    move (b.p^ [n - 1], l, 4);
    BUF_GetLong := l;
  end
  else
    BUF_GetLong := longint (POSERROR);
end;

procedure BUF_GetBuf (var buf;
                          nStartPos:TPos;
                          nLength:TPos);
{$ifdef FPC}
var mem_ptr:pointer;
{$endif}
begin
  if (nStartPos + nLength <= b.BufLen) then
  begin
{$ifdef FPC}
    mem_ptr := ptr (b.pseg, b.pofs + nStartPos - 1);
    move (mem_ptr^, buf, nLength);
{$else}
    move (b.p^ [nStartPos - 1], buf, nLength);
{$endif}
  end
  else
    FillChar (buf, nLength, 0);
end;

function BUF_GetPosOfByte (nStartPos:TPos;
                           nEndPos:TPos;
                           x:byte):TPos;
var n:word;
begin
  n := nStartPos;
  if (nEndPos > b.BufLen) then
    nEndPos := b.BufLen;

  while (n <= nEndPos) and (BUF_GetByte (n) <> x) do
    inc (n);

  if n <= nEndPos then
    BUF_GetPosOfByte := n
  else
    BUF_GetPosOfByte := POSERROR;
end;

function BUF_GetString (n:TPos;
                        nLen:byte):string;
var s:string;
{$ifdef FPC}
    mem_ptr:pointer;
{$endif}
begin
  if (n + nLen <= b.BufLen) then
  begin
    SetLength (s, nLen);
{$ifdef FPC}
    mem_ptr := ptr (b.pseg, b.pofs + n -1);
    move (mem_ptr^, s[1], nLen);
{$else}
    move (b.p^ [n - 1], s[1], nLen);
{$endif}
    BUF_GetString := s;
  end
  else
    BUF_GetString := 'buffer too small';
end;

function  BUF_GetStringUntil (nStartPos:TPos;
                              nEndByte:byte):string;
var nEndPos:TPos;
    s:string;
    nLen:byte;
{$ifdef FPC}
    mem_ptr:pointer;
{$endif}
begin
  nEndPos := nStartPos;
  while (nEndPos < b.BufLen) and (b.p^ [nEndPos - 1] <> nEndByte) do
    inc (nEndPos);
  nLen := nEndPos - nStartPos;
{$ifdef FPC}
  mem_ptr := ptr (b.pseg, b.pofs + nStartPos - 1);
  move (mem_ptr^, s[1], nLen);
{$else}
  move (b.p^ [nStartPos - 1], s[1], nLen);
{$endif}
  SetLength (s, nLen);
  BUF_GetStringUntil := s;
end;

function BUF_GetPascalString (n:TPos):string;
var nLen:byte;
begin
  nLen := BUF_GetByte (n);
  BUF_GetPascalString := BUF_GetString (n + 1, nLen);
end;

function BUF_GetpascalStringLen (n:TPos):byte;
begin
  BUF_GetPascalStringLen := BUF_GetByte (n) + 1;
end;

function  BUF_GetLongString (n:TPos;
                             nLen:word):string;
var nStrLen:byte;
begin
  if (nLen > 255) then
    nStrLen := 255
  else
    nStrLen := nLen;
  BUF_GetLongString := BUF_GetString (n, nStrLen);
end;

function BUF_GetPosOfChar (nStartPos:TPos;
                           nEndPos:TPos;
                           c:char;
                           sensitive:boolean):TPos;
var n:TPos;
    aByte:byte;
begin
  BUF_GetPosOfChar := POSERROR;
  n := nStartPos;
  if (nEndPos > b.BufLen) then
    nEndPos := b.BufLen;

  aByte := ord (c);
  if sensitive then
    while (n <= nEndPos) and (b.p^ [n - 1] <> aByte) do
      inc (n)
  else
  begin
    c := upcase (c);
    while (n <= nEndPos) and (upcase (BUF_GetChar (n)) <> c) do
      inc (n);
  end;

  if n <= nEndPos then
    BUF_GetPosOfChar := n;
end;

function BUF_GetPosOfStringCS (nStartPos:TPos;
                               nEndPos:TPos;
                               s:string):TPos;
begin
{
  important: add the start position, because StringPosInBuffer returns
  only the relative position
  used in TypPAS.pas
}
  if (nEndPos > b.BufLen) then
    nEndPos := b.BufLen;
  dec (nEndPos, length (s) - 1);
  BUF_GetPosOfStringCS := nStartPos - 1 +
                          StringPosInBuffer (b.p^ [nStartPos - 1],
                                             nEndPos - nStartPos + 1,
                                             s);
end;

function BUF_GetPosOfStringICS (nStartPos:TPos;
                                nEndPos:TPos;
                                s:string):TPos;
var temp:string;
    posi:TPos;
    len:byte;
begin
  if (nEndPos < nStartPos) then
    BUF_GetPosOfStringICS := POSERROR
  else
  begin

    s := UpString (s);
    len := length (s);
    posi := nStartPos - 1;

    repeat
      inc (posi);
      posi := BUF_GetPosOfChar (posi, nEndPos, s[1], CASEINSENSITIVE);
      if posi > POSERROR then
      begin
        BUF_GetBuf (temp [1], posi, len);
        SetLength (temp, length (s));
        temp := UpString (temp);
      end;
    until (posi = POSERROR) or (temp = s);
    BUF_GetPosOfStringICS := posi;
  end;
end;

end.
