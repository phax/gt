{$i aflags.inc}

unit XGlobal;

interface

const
      FILEMODE_READ      = 0;
      FILEMODE_WRITE     = 1;
      FILEMODE_READWRITE = 2;

      BITS:array[0..15] of word = (1, 2, 4, 8, 16, 32, 64,
                                   128, 256, 512, 1024, 2048,
                                   4096, 8192, 16384, 32768);

function max_b (a, b:byte):byte;
function max_i (a, b:integer):integer;
function max_w (a, b:word):word;
function max_l (a, b:longint):longint;
function max_r (a, b:real):real;
function min_b (a, b:byte):byte;
function min_i (a, b:integer):integer;
function min_w (a, b:word):word;
function min_l (a, b:longint):longint;
function min_r (a, b:real):real;

{$ifdef MSDOS}
{$ifndef DPMI}
function GetUsedHeapSize:longint;
function Ptr2Long (p:pointer):longint;
{$endif}
{$endif}

implementation

function max_b (a, b:byte):byte;
begin
  if (a > b) then
    max_b := a
  else
    max_b := b;
end;

function max_i (a, b:integer):integer;
begin
  if (a > b) then
    max_i := a
  else
    max_i := b;
end;

function max_w (a, b:word):word;
begin
  if (a > b) then
    max_w := a
  else
    max_w := b;
end;

function max_l (a, b:longint):longint;
begin
  if (a > b) then
    max_l := a
  else
    max_l := b;
end;

function max_r (a, b:real):real;
begin
  if (a > b) then
    max_r := a
  else
    max_r := b;
end;

function min_b (a, b:byte):byte;
begin
  if (a < b) then
    min_b := a
  else
    min_b := b;
end;

function min_i (a, b:integer):integer;
begin
  if (a < b) then
    min_i := a
  else
    min_i := b;
end;

function min_w (a, b:word):word;
begin
  if (a < b) then
    min_w := a
  else
    min_w := b;
end;

function min_l (a, b:longint):longint;
begin
  if (a < b) then
    min_l := a
  else
    min_l := b;
end;

function min_r (a, b:real):real;
begin
  if (a < b) then
    min_r := a
  else
    min_r := b;
end;

{$ifdef MSDOS}
{$ifndef DPMI}
function GetUsedHeapSize:longint;
begin
  GetUsedHeapSize := Ptr2Long (HeapPtr) - Ptr2Long (HeapOrg);
end;

function Ptr2Long (p:pointer):longint;
begin
  Ptr2Long := longint (seg (p^)) shl 4 + longint (ofs (p^));
end;
{$endif}
{$endif}

end.
