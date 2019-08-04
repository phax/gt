{$i aflags.inc}

{$ifdef BORLAND}
{$R-}  { - disable range check - }
{$endif}

unit XEXE_LE;

interface

type
     pLE_Header = ^tLE_Header;
     tLE_Header = record
       ID:array[1..2] of char;
       ByteOrder:byte;
       WordOrder:byte;
       ExeFormatLevel:longint;

       CPUType:word;
       TargetSystem:word;
       ModuleVersion:longint;

       ModuleType:longint;
       MemoryPages:longint;

       InitCS:longint;
       InitEIP:longint;

       InitSS:longint;
       InitESP:longint;

       MemoryPageSize:longint;
       LastPageBytes:longint;

       FixupSize:longint;
       FixupChecksum:longint;

       LoaderSectionSize:longint;
       LoaderChecksum:longint;

       ObjectTableOfs:longint;
       ObjectTableEntries:longint;

       ObjectPageMapOfs:longint;
       ObjectIterateDataMapOfs:longint;

       ResOfs:longint;
       ResEntries:longint;

       ResidentNamesTableOfs:longint;
       EntryTableOfs:longint;

       ModuleDirectivesTableOfs:longint;
       ModuleDirectivesEntries:longint;

       FixupPageTableOfs:longint;
       FixupRecordTableOfs:longint;

       ImportedModulesNameTableOfs:longint;
       ImportedModulesCount:longint;

       ImportedProceduresNameTableOfs:longint;
       PerPageChecksumTableOfs:longint;

       DataPagesOfsFromTopOfFile:longint;
       PreloadPageCount:longint;

       NonResidentNamesTableOfsFromTopOfFile:longint;
       NonResidentNamesTableLength:longint;

       NonResidentNamesTableChecksum:longint;
       AutomaticDataObject:longint;

       DebugInfoOfs:longint;
       DebugInfoLength:longint;

       PreloadInstancePagesNumber:longint;
       DemandInstancePagesNumber:longint;

       ExtraHeapAllocation:longint;
       Unknown:longint;
     end;

     pLE_Object = ^tLE_Object;
     tLE_Object = record
       nVirtSegmentSize:longint;
       nRelocationBaseAddr:longint;
       nFlags:longint;
       nPageTableIndex:longint;
       nPageTableEntries:longint;
       res:longint;
     end;

     pLE_Table = ^tLE_Table;
     tLE_Table = array[1..1] of tLE_Object;

const
      LE_HEADER_SIZE = SizeOf (tLE_Header);         { 176 }
      LE_OBJECT_SIZE = SizeOf (tLE_Object); { 24 }

type
     ObjNameStr = string[8];

     pLE_ObjectTableHandler = ^tLE_ObjectTableHandler;
     tLE_ObjectTableHandler = object
       procedure Init (nEntryCount:byte;
                       pHeader:pLE_Header);
       procedure Done;
       procedure ReadFromFile (var f:file;
                                   nPos:longint);
       function  GetEXESize:longint;
       function  GetEntryPoint:longint;

       procedure GetObject (nPos:byte; var pObj:pLE_Object);
       function  GetRealAddrOfObject (nPos:byte):longint;
       function  GetRealEndAddrOfObject (nPos:byte):longint;
       function  GetSizeOfObject (nPos:byte):longint;

       function  GetEntryCount:byte;

     private
       pLEBase:pLE_Table;
       nTableLen:byte;
       nTableSize:word;
       pHead:pLE_Header;
       nLast:byte;
     end;

implementation

uses
{$ifdef LOG}
     GTbuf, GTDebug,
{$endif}
     GTMem,
     XGlobal, XString;

{ - next class - }

procedure tLE_ObjectTableHandler.Init (nEntryCount:byte;
                                       pHeader:pLE_Header);
begin
  nTableLen := nEntryCount;
  nTableSize := nTableLen * LE_OBJECT_SIZE;
  pHead := pHeader;

  if (nEntryCount > 0) then
  begin
{$ifdef LOG}
    Log (concat ('LE.init: ', b.FName));
    Log (concat ('LE.init; 1: ', i2s (getusedheapsize)));
{$endif}
    GetMem (pLEBase, nTableSize);
    CheckMemoryAllocation (pLEBase, nTableSize, 'LEObjectTableHandler.init');
{$ifdef LOG}
    Log (concat ('LE.init; 2: ', i2s (getusedheapsize)));
{$endif}
  end
  else
    pLEBase := nil;

  nLast := 0;
end;

procedure tLE_ObjectTableHandler.Done;
begin
  if (pLEBase <> nil) then
  begin
{$ifdef LOG}
    Log (concat ('LE.done: 1: ', i2s (getusedheapsize)));
{$endif}
    freemem (pLEBase, nTableSize);
{$ifdef LOG}
    Log (concat ('LE.done: 2: ', i2s (getusedheapsize)));
{$endif}
    pLEBase := nil;
  end;
end;

procedure tLE_ObjectTableHandler.ReadFromFile (var f:file;
                                                   nPos:longint);
var i:byte;
begin
  seek (f, nPos);
  blockread (f, pLEBase^, nTableSize);

  nLast := 0;
  for i:=1 to nTableLen do
    if (pLEBase^ [i].nPageTableEntries > 0) then
      nLast := i;
end;

function tLE_ObjectTableHandler.GetEXESize:longint;
(*
var nSize:longint;
    i:byte;
    aObj:tLE_Object;
*)
begin
  GetEXESize := GetRealEndAddrOfObject (nTableLen);
(*
  { add the main object }
  nSize := pHead^.DataPagesOfsFromTopOfFile;

  { add all objects from 1 to nTableLen - 1 }
  for i:=1 to nTableLen - 1 do
    inc (nSize, pLEBase^ [i].nPageTableEntries * pHead^.MemoryPageSize);

  { add the last object }
  aObj := pLEBase^ [nTableLen];
  if (aObj.nPageTableEntries > 1) then
    inc (nSize, (aObj.nPageTableEntries - 1) * pHead^.MemoryPageSize);

  { add the lastpagebytes only if the last entry has at least one page }
  if (aObj.nPageTableEntries > 0) then
    inc (nSize, pHead^.LastPageBytes);

  GetEXESize := nSize;
*)
end;

function tLE_ObjectTableHandler.GetEntryPoint:longint;
begin
  if (pHead^.InitCS > 0) then
    GetEntryPoint :=  GetRealAddrOfObject (pHead^.InitCS) + pHead^.InitEIP
  else
    GetEntryPoint := 0;
end;

procedure tLE_ObjectTableHandler.GetObject (    nPos:byte;
                                            var pObj:pLE_Object);
begin
  pObj := Addr (pLEBase^ [nPos]);
end;

function tLE_ObjectTableHandler.GetRealAddrOfObject (nPos:byte):longint;
begin
  if (nPos > nTableLen) then  { was previously nLast }
    nPos := nTableLen;

  GetRealAddrOfObject := pHead^.DataPagesOfsFromTopOfFile +
                         (pLEBase^ [nPos].nPageTableIndex - 1) * pHead^.MemoryPageSize;
end;

function tLE_ObjectTableHandler.GetRealEndAddrOfObject (nPos:byte):longint;
begin
  GetRealEndAddrOfObject := GetRealAddrOfObject (nPos) +
                            GetSizeOfObject (nPos);
end;

function tLE_ObjectTableHandler.GetSizeOfObject (nPos:byte):longint;
begin
  if (nPos > nTableLen) then  { was previously nLast }
    nPos := nTableLen;

  if (nPos = nTableLen) then
  begin
    if (pLEBase^ [nPos].nPageTableEntries > 1) then
      GetSizeOfObject := (pLEBase^ [nPos].nPageTableEntries - 1) * pHead^.MemoryPageSize +
                         pHead^.LastPageBytes
    else
      GetSizeOfObject := 0;
  end
  else
    GetSizeOfObject := pLEBase^ [nPos].nPageTableEntries * pHead^.MemoryPageSize;
end;

function tLE_ObjectTableHandler.GetEntryCount:byte;
begin
  GetEntryCount := nTableLen;
end;

end.
