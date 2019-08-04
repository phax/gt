{$i aflags.inc}

unit GTCompar;

interface

uses
     GTGlobal,
     XString;

const
      EXE   = $01;
      COM   = $02;
      PEEXE = $04;
      NEEXE = $08;
      LEEXE = $10;
      MAGIC = $00;

      FOUND_NO_MODIFIER = 'Found no known modifier or compiler.';
      USE_ZE = 'Use /ZE to see details.';

type
     TModifierType = (
       NORMAL,
       PACKER,
       ENCRYPTER,
       COMPILER,
       LINKER,
       CONVERTER,
       PROTECTOR,
       STICKER,
       EXTENDER,
       PASSWORD
     );

     TEXEType = (
       EXE_DOS_00,
       EXE_DOS_01,
       EXE_NE_00,
       EXE_LE_00,
       EXE_LE_01,
       EXE_PE_00,
       EXE_PE_01
     );

     pC2ERec = ^tC2ERec;
     tC2ERec = record
       Name:string[16];
       Data:TArray24;
     end;

     pCompPtrRec = ^tCompPtrRec;
     tCompPtrRec = record
       DataLen:byte;
       FileType:byte;
       NameType:TModifierType;
       Name:^String;
       Data:pointer;
     end;

     pCompEXECOMRec = ^tCompEXECOMRec;
     tCompEXECOMRec = record
       data:TCompPtrRec;
       StartPos:word;
     end;

     pSCRecord = ^tSCRecord;
     tSCRecord = record
       EXEType:TEXEType;
       Name:String80;
       DataLen:byte;
       NameType:TModifierType;
       Data:array[1..80] of byte;
       next:pSCRecord;
     end;

function ComparePtrBufFile (pBuf:pSCRecord):boolean;
function ComparePtrBuf (pBuf:PCompPtrRec):boolean;
function CompareHeader (pBuf:PC2ERec):boolean;

implementation

uses
     GTOutput,
     XGlobal;

{ returns the number of different bytes }
function Compare (aBuf:TArray256;
                  nLen:byte;
                  nMagic:byte;
                  nMax:byte):byte;
{$ifdef SLOW_COMPARE}
var i:byte;
    Counter:byte;
begin
  Counter := 0;

  for i:=0 to nLen - 1 do
  begin
    if (aBuf [i] <> CompareBuf [i]) and (aBuf [i] <> nMagic) then
    begin
      inc (Counter);
      Compare := Counter;
      if (Counter > nMax) then
        exit;
    end;
  end;

  Compare := Counter;
end;
{$else}
assembler;
asm
{
  cl   [bp - 3]  i
  ch   [bp - 4]  counter
  dh   [bp - 5]  internal counter end
}
  cld

  { init counter }
  xor ch, ch             { counter }

  { init loop }
  mov dh, [bp + 8]        { nLen }
  dec dh

  mov al, 0
  cmp al, dh              { nLen }
  ja  @exit

  xor cl, cl              { i }
  jmp @first_pass

@loop_next:
  inc cl                  { i++ }

@first_pass:
  mov al, cl       { get offset as word }
  xor ah, ah

  lea bx, CompareBuf
  add bx, ax       { offset + i }
  mov dl, [bx]     { CompareBuf[i] }

  push ds
    lds  bx, aBuf
    add  bx, ax    { offset + i }
    mov  al, [bx]  { aBuf[i] }
  pop ds

  cmp al, dl
  je  @not_wrong_char

  { al still contains aBuf[i] }
  mov bl, [bp + 6]
  cmp al, [bp + 6]        { nMagic }
  je  @not_wrong_char

@wrong_char:
  inc ch                  { counter }

  cmp ch, [bp + 4]        { nMax }
  ja  @exit

@not_wrong_char:
  cmp cl, dh              { nLen }
  jne @loop_next

@exit:
  mov al, ch              { counter }
end;
{$endif}

procedure NoteNameType (n:TModifierType);
begin
  case n of
    NORMAL:   Note ('Code detected');
    PACKER:   Note ('Packer');
    ENCRYPTER:Note ('Encrypter');
    COMPILER: Note ('Compiler');
    LINKER:   Note ('Linker');
    CONVERTER:Note ('Converter');
    PROTECTOR:Note ('Protector');
    STICKER:  Note ('Sticker');
    EXTENDER: Note ('Extender');
    PASSWORD: Note ('Password protector');
  else
    InternalError ('nametype');
  end;
end;

type TWrongStr = string [16];
function MakeWrongStr (nWrong:byte):TWrongStr;
var tmp:TWrongStr;
    i:byte;
begin
  tmp := '';
  if (nWrong > 0) then
  begin
    tmp := ' (';
    for i:=1 to nWrong do
      tmp := concat (tmp, '?');
    tmp := concat (tmp, ')');
  end;
  MakeWrongStr := tmp;
end;

{ for comparing EXE startup codes }
function ComparePtrBufFile (pBuf:pSCRecord):boolean;
const MAX_WRONG:byte = 2;
var b1:TArray256;
    WrongCount:byte;
    nCompareLen:byte;
begin
  ComparePtrBufFile := false;

  { - here the switch /NH worx - }
  if (not bHeuristics) then
    MAX_WRONG := 0;

  move (pBuf^.Data, b1, pBuf^.DataLen);
  nCompareLen := min_b (pBuf^.DataLen, nMaximumCompare);
  WrongCount := Compare (b1,
                         nCompareLen,
                         MAGIC,
                         MAX_WRONG);
  if (WrongCount > MAX_WRONG) then
    exit;

  if (pBuf^.Name > '') then
  begin
    NoteNameType (pBuf^.NameType);
    Appendln (concat (': ', pBuf^.Name, MakeWrongStr (WrongCount)));
  end;

  ComparePtrBufFile := (WrongCount = 0);
end;

function ComparePtrBuf (pBuf:PCompPtrRec):boolean;
const MAX_WRONG:byte = 2;
var b1:TArray256;
    WrongCount:byte;
    nCompareLen:byte;

begin
  ComparePtrBuf := false;

  { - here the switch /NH worx - }
  if (not bHeuristics) then
    MAX_WRONG := 0;

  move (pBuf^.Data^, b1, pBuf^.DataLen);  { sizeof (TArrayMax); }
  nCompareLen := min_b (pBuf^.DataLen, nMaximumCompare);
  WrongCount := Compare (b1,
                         nCompareLen,
                         MAGIC,
                         MAX_WRONG);
  if (WrongCount > MAX_WRONG) then
    exit;

  if (pBuf^.Name^ > '') then
  begin
    NoteNameType (pBuf^.NameType);
    Appendln (concat (': ', pBuf^.Name^, MakeWrongStr (WrongCount)));
  end;

  ComparePtrBuf := (WrongCount = 0);
end;

function CompareHeader (pBuf:PC2ERec):boolean;
const MAGIC = $AA;
      MAX_WRONG = 0;
var b1:TArray256;
    Count:byte;
begin
  CompareHeader := false;
  move (pBuf^.Data, b1, SizeOf (TArray24));
  Count := Compare (b1,
                    SizeOf (TArray24),
                    MAGIC,
                    MAX_WRONG);
  if (Count > MAX_WRONG) then
    exit;

  Noteln (concat ('COM to EXE by ', pBuf^.Name));
  CompareHeader := true;
end;

end.
