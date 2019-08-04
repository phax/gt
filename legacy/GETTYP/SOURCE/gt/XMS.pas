{$i aflags.inc}

unit XMS;

interface

var xmm_avail:boolean;
    xmm_error:integer;
    xmm_seg,
    xmm_ofs:word;
    xmm_ptr:pointer;


function  align (n:longint; t:word):longint;
function  Get20BitAddr (s, o:word):longint;
function  GetXMS (size:word):integer;  { returns a handle }
procedure FreeXMS (handle:integer);  { use the handle }
procedure ChangeXMSSize (handle:integer; newsize:word);
procedure CopyMem (sourcehandle:integer;
                   sourceofs:longint;
                   desthandle:integer;
                   destofs:longint;
                   count:longint);

implementation

function Get20BitAddr (s, o:word):longint;
begin
  Get20BitAddr := longint (s) shl 16 or o;
end;

function align (n:longint; t:word):longint;
begin
  if (n mod t = 0) then
    align := n
  else
    align := ((n div t) + 1) * t;
end;

procedure xmm_init;
assembler;
asm
{ check for HIMEM.SYS }
  mov  ax, 4300h
  int  2fh
  cmp  al, 80h
  je   @found
  mov  xmm_avail, 0
  jmp  @quit
@found:
  mov  ax,4310h
  int  2fh
  mov  xmm_seg, es
  mov  xmm_ofs, bx
  mov  xmm_avail, 1
@quit:
end;

function GetXMS (size:word):integer;
assembler;
asm
  mov  ah, $09
  mov  dx, size
  call ds:[xmm_ptr]
  cmp  ax, 1
  je   @okay
  mov  bh, 0
  mov  xmm_error, bx
  jmp  @quit
@okay:
  mov  xmm_error,0
  mov  ax, dx
@quit:
end;

procedure FreeXMS (handle:integer);
assembler;
asm
  mov  ah, $0A
  mov  dx, handle
  call ds:[xmm_ptr]
  mov  xmm_error, 0
  cmp  ax, 1
  je   @quit
  mov  bh, 0
  mov  xmm_error, bx
@quit:
end;

procedure ChangeXMSSize (handle:integer; newsize:word);
assembler;
asm
  mov ah, $0f
  mov bx,newsize
  mov dx,handle
  call ds:[xmm_ptr]
  cmp ax,1
  je @okay
  mov bh, 0
  mov xmm_error, bx
  jmp @quit
@okay:
  mov xmm_error,0
  mov ax,dx
@quit:
end;

procedure CopyMEM (sourcehandle:integer;
                   sourceofs:longint;
                   desthandle:integer;
                   destofs:longint;
                   count:longint);
var xseg, xofs:word;
    xmms:record
      len:longint;
      src_hdl:integer;
      src_ofs:longint;
      dst_hdl:integer;
      dst_ofs:longint;
    end;
    {
    p, pDst:pointer;
    }
begin
  { memory must be 2 byte aligned! }
  if (align (count, 2) <> count) then
  begin
    writeln ('The XMS memory to be copied must be 2 byte aligned (', count, ')!');
    halt (17);
  end;

  xmms.len     := count;
  xmms.src_hdl := sourcehandle;
  xmms.src_ofs := sourceofs;
  xmms.dst_hdl := desthandle;
  xmms.dst_ofs := destofs;

  xseg := seg (xmms);
  xofs := ofs (xmms);
  xmm_error := 0;
  asm
    push ds
    mov  ax, ds
    mov  es, ax        { DS->ES }
    mov  ax, xseg
    mov  ds, ax        { xseg->DS }
    mov  si, xofs      { xofs->SI }
    mov  ah, $0B       { action: copy memory }
    call es:[xmm_ptr]  { call the XMM handler }
    cmp  ax, $0000     { if AX = 1 an error occured; ec in BX }
    je   @found_an_error
    mov  bx, 0         { ensure that the error codes contains 0! }
    jmp  @done
  @found_an_error:
    mov  bh, 0         { error code only in BL }
  @done:
    pop  ds
    mov  ds:[xmm_error], bx
  end;
end;

begin
  xmm_error := 0;
  xmm_init;
  xmm_ptr := ptr (xmm_seg, xmm_ofs);
end.
