{$i aflags.inc}
{$R-}

unit GTRegist;

interface

const
      bInitialized:boolean = false;
{$ifndef GETTYP}
      KEYFile2 = 't';
      KEYFile3 = '.';
      KEYFile4 = 'k';
      KEYFile5 = 'e';
{$endif}

procedure PrintRegisteredString;
function  EncodeString (nNumber:longint; sName:string):string;
procedure DecodeString (DATA:string);
function  GetIDOfName (s:string):longint;

var
    USER_NAME:string[35];
    USER_ID:longint;

implementation

uses
     GTOutput,
     XString;

const
      keylen=6;
      MINLEN = 4;

procedure PrintRegisteredString;
var s, sUN:string;
    i, p:byte;
begin
  s := '-';
  { check registration info somewhere in the code ... }
  if (bInitialized) and (USER_NAME = '') then
    halt;
  { end check }
  for i:=1 to 37 do s := concat (s, '--');
  if (bInitialized) then
  begin
    sUN := concat (' # Registered to "', USER_NAME, '" # ');
    p := 4;
  end
  else
  begin
    sUN := concat (' # free edition # ');
    p := 76 - length (sUN) - 3;
  end;
  move (sUN [1], s [p], length (sUN));
  Noteln (s);
end;

function EncodeString (nNumber:longint; sName:string):string;
const
      key:array[1..keylen] of byte = (234, 233, 53, 34, 143, 47);
var i, len:byte;
    nBytes:array[1..4] of byte absolute nNumber;
    k, k1:array[1..keylen] of byte;
    s:string;
    OUT:string;

    procedure encodestr (n:byte);
    var i, q:byte;
    begin
      SetLength (s, len);
      for i:=1 to len do
      begin
        q := ord (sName [i]) and n;
        s [i] := chr (q xor k [(i mod keylen) + 1]);
      end;
    end;

const KEY1 = 27;
      KEY2 = 255 - KEY1;
begin
  OUT := '';
  len := length (sName);

  if (len < MINLEN) then
  begin
    Noteln ('String too short');
    exit;
  end;

  k [1] := (key [1] * key [5]);
  k [1] := k [1] mod 248;
  k [2] := (key [3] * ((nNumber mod 4711) + 2)) mod 128;
  k [4] := (k [2] * (nNumber mod key [3]) - key [6]) mod 128;
  k [5] := k [1] + k [2];
  k [6] := k [1] * k [2];
  k [3] := (k [4] div 2 * k [5] + k [6]) mod 256;

  for i:=1 to keylen do
    OUT := concat (OUT, chr (k [i]));
  OUT := concat (OUT, chr (len));
  OUT := concat (OUT, chr (nBytes [1]));
  encodestr (KEY1);
  OUT := concat (OUT, s);
  OUT := concat (OUT, chr (nBytes [2]));
  move (k, k1, keylen);  { save k }
  for i:=1 to keylen do
    k [i] := k [i] and (nBytes [1] or nBytes [2]);
  for i:=1 to keylen do
    OUT := concat (OUT, chr (k [i]));
  move (k1, k, keylen);  { restore k }
  encodestr (KEY2);
  OUT := concat (OUT, s);
  OUT := concat (OUT, chr (nBytes [4]));
  OUT := concat (OUT, chr (nBytes [3]));
  OUT := concat (OUT, chr (length (OUT) + 1));

  EncodeString := OUT;
end;

procedure DecodeString (DATA:string);
var nBytes:array[1..4] of byte;
    nNumber:longint absolute nBytes;
    b, b1:array[1..keylen] of byte;
    nNameLen:byte;
    sName1:string;
    i, x, nBufPos:byte;
begin
  USER_NAME := '';
  USER_ID := 0;
  nBufPos := 0;

  { read key set }
  move (DATA [nBufPos + 1], b, keylen);
  inc (nBufPos, keylen);
  { check key set }
  if (b [2] >= 128) then exit;
  x := (b [4] div 2 * b [5] + b [6]) mod 256; if (b[3] <> x) then exit;
  if (b [4] >= 128) then exit;
  x := b [1] + b [2]; if (b [5] <> x) then exit;
  x := b [1] * b [2]; if (b [6] <> x) then exit;
  bInitialized := (x > 0);
  { get length of name }
  inc (nBufPos);
  nNameLen := ord (DATA [nBufPos]);
  { check length of name }
  if (nNameLen < MINLEN) then exit;
  SetLength (USER_NAME, nNameLen);
  FillChar (USER_NAME [1], nNameLen, 0);
  { read a ID part }
  inc (nBufPos);
  nBytes [1] := ord (DATA [nBufPos]);
  { read a name part }
  SetLength (sName1, nNameLen);
  move (DATA [nBufPos + 1], sName1 [1], nNameLen);
  inc (nBufPos, nNameLen);
  { decode a part }
  for i:=1 to nNameLen do
    USER_NAME [i] := chr (ord (USER_NAME [i]) or (ord (sName1 [i]) xor b [(i mod keylen) + 1]));
  { read a ID part }
  inc (nBufPos);
  nBytes [2] := ord (DATA [nBufPos]);
  { save b }
  move (b, b1, keylen);
  { read key set }
  move (DATA [nBufPos + 1], b, keylen);
  inc (nBufPos, keylen);
  { check key set }
  for i:=1 to keylen do
    if (b [i] <> b1 [i] and (nBytes [1] or nBytes [2])) then
      exit;
  bInitialized := true;
  { restore b }
  move (b1, b, keylen);
  bInitialized := false;
  { read a name part }
  SetLength (sName1, nNameLen);
  move (DATA [nBufPos + 1], sName1 [1], nNameLen);
  inc (nBufPos, nNameLen);
  { decode a part }
  for i:=1 to nNameLen do
    USER_NAME [i] := chr (ord (USER_NAME [i]) or (ord (sName1 [i]) xor b [(i mod keylen) + 1]));
  { read a ID part }
  inc (nBufPos);
  nBytes [4] := ord (DATA [nBufPos]);
  { read a ID part }
  inc (nBufPos);
  nBytes [3] := ord (DATA [nBufPos]);
  bInitialized := (USER_NAME > '') and (nNumber > 0);
  { finished }
  USER_ID := nNumber;
end;

function  GetIDOfName (s:string):longint;
var n:longint;
    i, len:byte;
    crypted, saved:string;
    OFFSET:longint;
begin
  saved := USER_NAME;
  OFFSET := 289;
  repeat
    inc (OFFSET);
    n := 1;
    len := length (s);
    for i:=1 to len do
      n := (n + ord (s [len]) * ord (s [i]) * OFFSET) xor $47110815;
      {
      inc (n, (ord (s [len]) * ord (s [i]) * OFFSET) xor $47110815);
      }

    crypted := EncodeString (n, s);
    DecodeString (crypted);
    if (USER_NAME <> s) then
      Noteln (concat ('Invalid offset: ', i2s (OFFSET), '; ID = $', hexlong (n)));
  until (USER_NAME = s);
  USER_NAME := saved;

  GetIDOfName := n;
end;

end.
