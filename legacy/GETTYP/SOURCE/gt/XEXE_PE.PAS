{$i aflags.inc}

{$ifdef BORLAND}
{$R-}  { - disable range check - }
{$endif}

unit XEXE_PE;

interface

type
     PE_ImageFileHeader = record
       ID:longint;
       CPUType:word;
       NumberOfSections:word;
       DateTimeStamp:longint;
       SymbolTablePtr:longint;
       NumberOfSymbols:longint;
       OptionalHeaderSize:word;
       Flags:word;
     end;

const
      PE_NUMBER_OF_DIRECTORY_ENTRIES = 16;

type
     GT_EXE_PE_ImageDataDirectory = record
       VA:longint;
       Size:longint;
     end;

     PE_OptionalFileHeader = record
     {
       standard fields
     }
       Magic:word;
       LinkerMajor:byte;
       LinkerMinor:byte;
       SizeOfCode:longint;
       SizeOfInitializedData:longint;
       SizeOfUninitializedData:longint;
       AddressOfEntryPoint:longint;
       BaseOfCode:longint;
       BaseOfData:longint;

     {
       NT additional fields
     }

       ImageBase:longint;
       SectionAlignment:longint;
       FileAlignment:longint;
       OSMajor:word;
       OSMinor:word;
       UserMajor:word;
       UserMinor:word;
       SubSysMajor:word;
       SubSysMinor:word;
       Res1:longint;
       SizeOfImage:longint;
       SizeOfHeader:longint;
       Checksum:longint;
       SubSystem:word;
       DLLFlags:word;
       SizeOfStackReserve:longint;
       SizeOfStackCommit:longint;
       SizeOfHeapReserve:longint;
       SizeOfHeapCommit:longint;
       LoaderFlags:longint;
       NumberOfRVAAndSizes:longint;
       DataDirectory:array[1..PE_NUMBER_OF_DIRECTORY_ENTRIES] of GT_EXE_PE_ImageDataDirectory;
     end;

     pPE_Object = ^tPE_Object;
     tPE_Object = record
       sName:array[1..8] of char;
       nVirtSize:longint;
       nRVA:longint;
       nPhysSize:longint;
       nPhysOfs:longint;
       res1:longint;
       res2:longint;
       res3:longint;
       nFlags:longint
     end;

     pPE_Table = ^tPE_Table;
     tPE_Table = array[1..1] of tPE_Object;

const
      SIZEOF_PE_IMAGE_FILE_HEADER = SizeOf (PE_ImageFileHeader);  { 24 }
      SIZEOF_PE_OPTIONAL_FILE_HEADER = SizeOf (PE_OptionalFileHeader);  { 224 }
      PE_OBJECT_SIZE = SizeOf (tPE_Object);

type
     ObjNameStr = string[8];

     pPE_ObjectTableHandler = ^tPE_ObjectTableHandler;
     tPE_ObjectTableHandler = object
       procedure Init (nEntryCount:byte);
       procedure Done;
       procedure ReadFromFile (var f:file;
                                   nPos:longint);
       function  GetEXESize:longint;

       procedure GetObject          (nPos:byte;    var pObj:pPE_Object);
       procedure GetObjectByName    (s:ObjNameStr; var pObj:pPE_Object);
       procedure GetObjectByRVA     (nRVA:longint; var pObj:pPE_Object);
       procedure GetObjectByPhysOfs (nOfs:longint; var pObj:pPE_Object);

       procedure GetPhysOfsByRVA (nRVA:longint; var nOfs:longint);
       procedure GetRVAByPhysOfs (nOfs:longint; var nRVA:longint);

       function  GetObjectCount:byte;

     private
       pPEBase:pPE_Table;
       nTableLen:byte;
       nTableSize:word;
     end;

implementation

uses
{$ifdef LOG}
     GTBuf, GTDebug,
{$endif}
     GTMem,
     XGlobal, XString;

{ - next class - }

function tPE_ObjectTableHandler.GetObjectCount:byte;
begin
  GetObjectCount := nTableLen;
end;

procedure tPE_ObjectTableHandler.Init (nEntryCount:byte);
begin
  nTableLen := nEntryCount;
  nTableSize := nTableLen * PE_OBJECT_SIZE;

  if (nEntryCount > 0) then
  begin
{$ifdef LOG}
    LOG (concat ('PE.init: ', b.fname));
    Log (concat ('PE.init; 1: ', i2s (getusedheapsize)));
{$endif}
    GetMem (pPEBase, nTableSize);
    CheckMemoryAllocation (pPEBase, nTableSize, 'PE_ObjectTableHandler.init');
{$ifdef LOG}
    Log (concat ('PE.init; 2: ', i2s (getusedheapsize)));
{$endif}
  end
  else
    pPEBase := nil;
end;

procedure tPE_ObjectTableHandler.Done;
begin
  if (pPEBase <> nil) then
  begin
{$ifdef LOG}
    Log (concat ('PE.done: 1: ', i2s (getusedheapsize)));
{$endif}
    freemem (pPEBase, nTableSize);
{$ifdef LOG}
    Log (concat ('PE.done: 2: ', i2s (getusedheapsize)));
{$endif}
    pPEBase := nil;
  end;
end;

procedure tPE_ObjectTableHandler.ReadFromFile (var f:file;
                                               nPos:longint);
var i, j:byte;
begin
  seek (f, nPos);
  blockread (f, pPEBase^, nTableSize);

  { - replace all #0 with #32 - }
  for i:=1 to nTableLen do
    for j:=1 to 8 do
      if (pPEBase^ [i].sName [j] = #0) then
        pPEBase^ [i].sName [j] := ' ';
end;

function tPE_ObjectTableHandler.GetEXESize:longint;
var aLastObj:tPE_Object;
    index:byte;
    nRes:longint;
begin
  index := nTableLen;
  while (index > 0) do
  begin
    aLastObj := pPEBase^ [index];
    nRes := aLastObj.nPhysOfs + aLastObj.nPhysSize;
    if (nRes > 0) then
    begin
      GetEXESize := nRes;
      exit;
    end;
    dec (index);
  end;
  GetEXESize := 0;
end;

procedure tPE_ObjectTableHandler.GetObject (    nPos:byte;
                                            var pObj:pPE_Object);
begin
  pObj := Addr (pPEBase^ [nPos]);
end;

procedure tPE_ObjectTableHandler.GetObjectByName (    s:ObjNameStr;
                                                  var pObj:pPE_Object);
var i:byte;
begin
  for i:=1 to nTableLen do
  begin
    GetObject (i, pObj);
    if pos (s, pObj^.sName) = 1 then
      exit;
  end;
  pObj := nil;
end;

procedure tPE_ObjectTableHandler.GetObjectByRVA (    nRVA:longint;
                                                 var pObj:pPE_Object);
var i:byte;
begin
  { - get the first element where its RVA is <= the nRVA - }
  for i:=nTableLen downto 1 do
  begin
    GetObject (i, pObj);
    if (pObj^.nRVA <= nRVA) then
      exit;
  end;
  pObj := nil;
end;

procedure tPE_ObjectTableHandler.GetObjectByPhysOfs (    nOfs:longint;
                                                     var pObj:pPE_Object);
var i:byte;
{    pTempObj:pPE_Object; }
begin
  { - get the first element where its RVA is <= the nRVA - }
{
  pObj := nil;
  for i:=1 to nTableLen do
  begin
    GetObject (i, pTempObj);
    if (pTempObj^.nPhysOfs <= nOfs) and (pTempObj^.nPhysSize > 0) then
    begin
      pObj := pTempObj;
      exit;
    end;
  end;
}
  { must be done from back to front!! }
  for i:=nTableLen downto 1 do
  begin
    GetObject (i, pObj);
    if (pObj <> nil) and (nOfs >= pObj^.nPhysOfs) and (pObj^.nPhysSize > 0) then
      exit;
  end;
  pObj := nil;
end;

procedure tPE_ObjectTableHandler.GetPhysOfsByRVA (    nRVA:longint;
                                                  var nOfs:longint);
var pOBJ:pPE_Object;
begin
  GetObjectByRVA (nRVA, pObj);
  if (pObj <> nil) then
    nOfs := pObj^.nPhysOfs + (nRVA - pObj^.nRVA)
  else
    nOfs := nRVA;
end;

procedure tPE_ObjectTableHandler.GetRVAByPhysOfs (    nOfs:longint;
                                                 var nRVA:longint);
var pOBJ:pPE_Object;
begin
  GetObjectByPhysOfs (nOfs, pObj);
  if (pObj <> nil) then
    nRVA := pObj^.nRVA + (nOfs - pObj^.nPhysOfs)
  else
    nRVA := nOfs;
end;

{$ifdef MSDOS}
function NewHeapError (size:word):integer; far;
begin
  if (size > 0) then
    writeln ('Could not allocate ', size, ' bytes!');
  NewHeapError := 1;
  {
  0 - runtime error
  1 - pointer returns NIL
  2 - retry
  }
end;

begin
  HeapError := @NewHeapError;
{$endif}
end.
