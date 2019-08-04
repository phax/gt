{$i aflags.inc}

{$ifdef BORLAND}
{$R-}  { - disable range check for virtual array - }
{$endif}

unit GTMem;

interface

type
     TMemArray = array[0..0] of byte;
     TMemStruct = record
{$ifdef XMS}
       handle:integer;
       bXMS:boolean;
{$endif}
       bInUse:boolean;
       p:^TMemArray;
       size:word;
     end;

{$ifdef XMS}
var
    GTMem_UseXMS:boolean;
{$endif}

procedure CheckMemoryAllocation (p:pointer;
                                 nSize:word;
                                 msg:string);
procedure GetMemory (var MS:TMemStruct;
                         size:word);
procedure CopyToMemoryAt (var src;
                          var MS:TMemStruct;
                              size:word;
                              nOffset:longint);
procedure CopyToMemory (var src;
                        var MS:TMemStruct;
                            size:word);
procedure CopyFromMemoryAt (var MS:TMemStruct;
                            var dst;
                                size:word;
                                nOffset:longint);
procedure CopyFromMemory (var MS:TMemStruct;
                          var dst;
                              size:word);
procedure FreeMemory (var MS:TMEMStruct);

function inc_ptr (p:pointer; n:longint):pointer;

implementation

uses
     IOError,
{$ifdef XMS}
     XMS,
{$endif}
{$ifdef LOG}
     GTDebug, XGlobal,
{$endif}
     XString, XWrite;

{
XString is needed for FastPrint and i2s
FastPrint is needed because otherwise
it may comes to troubles because of 2 different
output methods.
}

const
      ErrMsg_Size = 'GT internal: memory size error';
      ErrMsg_Use  = 'GT internal: memory usage error';

{$ifdef XMS}
const
      MAX_XMS_HANDLE = 256;
var
    nHandleIndex:integer;
    aHandleList:array[1..MAX_XMS_HANDLE] of integer;
{$endif}

procedure Error (i:byte);
begin
  case i of
    1:FastPrint (ErrMsg_Size);
    2:FastPrint (concat (ErrMsg_Use, ' 1'));
    3:FastPrint (concat (ErrMsg_Use, ' 2'));
  end;
  FastPrint (' - program halted'#13#10);
  halt (255);
end;

{$ifdef XMS}
procedure AddHandle (i:integer);
begin
  inc (nHandleIndex);
  if (nHandleIndex > MAX_XMS_HANDLE) then
    writeln ('Error: Too many XMS handles are in use!')
  else
    aHandleList [nHandleIndex] := i;
end;

procedure FreeHandle (i:integer);
begin
  if (aHandleList [nHandleIndex] <> i) then
    writeln ('Internal error: cannot free wrong XMS handle');

  dec (nHandleIndex);
  if (nHandleIndex < 0) then
    writeln ('Internal error: invalid handle index');
end;
{$endif}

procedure GetMemory (var MS:TMemStruct;
                         size:word);
begin
  if (MS.bInUse) then
    Error (2);

{$ifdef LOG}
  Log (concat ('GtMem: allocating ', i2s (size), ' bytes'));
  Log (concat ('       used heap size 1: ', i2s (getusedheapsize)));
{$endif}

{$ifdef XMS}
  if XMM_Avail and GTMem_UseXMS then
  begin
    { if size is not multiple of 1024 then increase the size }
    size := ((size div 1024) + 1) * 1024;
{$ifdef LOG}
    Log (concat ('       real size: ', i2s (size), ' bytes'));
{$endif}

    MS.bXMS := true;
    MS.size := size;

    { allocate XMS only per KB }
    MS.handle := GetXMS (size div 1024);
    CheckXMSError (XMM_Error, 'GetXMS');
    MS.p := nil;
    MS.bInUse := true;

    AddHandle (MS.handle);
  end
  else
{$endif}
  begin
    MS.size := size;
    getmem (MS.p, MS.size);
    MS.bInUse := true;
    CheckMemoryAllocation (MS.p, MS.size, 'GetMemory');
  end;

{$ifdef LOG}
  Log (concat ('       used heap size 2: ', i2s (getusedheapsize)));
{$endif}
end;

procedure CopyToMemoryAt (var src;
                          var MS:TMemStruct;
                              size:word;
                              nOffset:longint);
begin
   if (not MS.bInUse) then
    Error (3);

  if (size > MS.size) then
    Error (1);

{$ifdef XMS}
  if (MS.bXMS) and (GTMem_UseXMS) then
  begin
    { NOTE: it's safe to align size to 2 because
      the each memory block is allocated in 1024 byte
      aligned blocks! think at it'll be clear! }
    CopyMEM (0,
             Get20BitAddr (seg (src), ofs (src)),
             MS.handle,
             nOffset,
             align (size, 2));
    CheckXMSError (XMM_Error, 'CopyMem (to)');
  end
  else
{$endif}
  begin
    move (src, MS.p^[nOffset], size)
  end;
end;

procedure CopyToMemory (var src;
                        var MS:TMemStruct;
                            size:word);
begin
  CopyToMemoryAt (src, MS, size, 0);
end;

procedure CopyFromMemoryAt (var MS:TMemStruct;
                            var dst;
                                size:word;
                                nOffset:longint);
begin
  if (not MS.bInUse) then
    Error (3);

  if (size > MS.size) then
    Error (1);

{$ifdef XMS}
  if (MS.bXMS) and (GTMem_UseXMS) then
  begin
    CopyMEM (MS.handle,
             nOffset,
             0,
             Get20BitAddr (seg (dst), ofs (dst)),
             size);
    CheckXMSError (XMM_Error, 'CopyMem (from)');
  end
  else
{$endif}
  begin
    move (MS.p^[nOffset], dst, size)
  end;
end;

procedure CopyFromMemory (var MS:TMemStruct;
                          var dst;
                              size:word);
begin
  CopyFromMemoryAt (MS, dst, size, 0);
end;

procedure FreeMemory (var MS:TMEMStruct);
begin
  if (not MS.bInUse) then
    Error (3);

{$ifdef LOG}
  Log (concat ('GtMem: freeing ', i2s (ms.size), ' bytes'));
  Log (concat ('       used heap size 1: ', i2s (getusedheapsize)));
{$endif}

{$ifdef XMS}
  if (MS.bXMS) and (GTMem_UseXMS) then
  begin
{$ifdef LOG}
    Log (concat ('       handle ', i2s (MS.handle)));
{$endif}
    FreeXMS (MS.handle);
    CheckXMSError (XMM_Error, 'FreeXMS');
    FreeHandle (MS.handle);
    MS.bXMS   := false;
    MS.handle := 0;
  end
  else
{$endif}
  begin
{$ifdef LOG}
    Log (concat ('       ', i2s (MS.size), ' bytes'));
{$endif}
    freemem (MS.p, MS.size);
  end;

{$ifdef LOG}
  Log (concat ('       used heap size 2: ', i2s (getusedheapsize)));
{$endif}

  MS.bInUse := false;
  MS.p      := nil;
  MS.size   := 0;
end;

procedure CheckMemoryAllocation (p:pointer;
                                 nSize:word;
                                 msg:string);
begin
  if (p = nil) and (nSize > 0) then
  begin
    FastPrint (concat (msg, #13#10));
    FastPrint (concat ('Cannot allocate ', i2s (nSize), ' bytes of base memory.'#13#10));
    FastPrint ('Aborting program.'#13#10);
    halt (255);
  end;
end;

function inc_ptr (p:pointer; n:longint):pointer;
begin
  inc_ptr := pointer (longint (p) + n);
end;

{$ifdef XMS}
var pOldExit:pointer;

{$f+}
procedure FreeOpenXMSHandles;
var handle:integer;
begin
{$ifdef LOG}
  Log ('FreeOpenXMSHandles');
{$endif}
  while (nHandleIndex > 0) do
  begin
    handle := aHandleList [nHandleIndex];
{$ifdef LOG}
    Log (concat ('autofreeing handle ', i2s (handle)));
{$endif}
    FreeXMS (handle);
    CheckXMSError (XMM_Error, 'FreeXMS');
    FreeHandle (handle);
  end;

  ExitProc := pOldExit;
end;
{$f-}
{$endif}

begin
{$ifdef XMS}
  nHandleIndex := 0;
  pOldExit := ExitProc;
  ExitProc := @FreeOpenXMSHandles;
{$endif}
end.
