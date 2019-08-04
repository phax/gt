{$i aflags.inc}

{$ifdef BORLAND}
{$R-}  { - disable range check because of virtual array - }
{$endif}

unit XEXE_NE;

interface

type
     String4 = string[4];

     NE_Base = object
       ID:array[1..2] of char;
       nLinkerMajorVersion:byte;
       nLinkerMinorVersion:byte;
       nEntryTableOfs:word;
       nEntryTableLen:word;

       nFileLoadCRC:longint;  { TPW: $00000000 }
       ProgramFlags:byte;
       ApplicationFlags:byte;
       nAutoDataSegmentIndex:word;

       nInitialLocalHeapSize:word;
       nInitialStackSize:word;
       nIP:word;
       nCS:word;

       nSP:word;
       nSS:word;
       nSegmentCount:word;
       nModuleReferenceCount:word;

       nSizeOfNonresidentNamesTable:word;
       nSegmentTableOfs:word;
       nResourceTableOfs:word;
       nResidentNamesTableOfs:word;

       nModuleReferenceTableOfs:word;
       nImportedNamesTableOfs:word;
       nAbsoluteOfsToNonresidentNamesTable:longint;

       nCountOfMoveableEntryPointsListedInEntryTable:word;
       nFileAlignmentSizeShiftCount:word;
       nResourceSegmentCount:word;
       TargetOS:byte;
       OtherEXEFlags:byte;

       nOffsetToReturnThunksOrStartOfGangload:word;
       nOffsetToSegmentReferenceThunksOrLengthOfGangloadArea:word;
       nMinimumCodeSwapAreaSize:word;
       nExpectedWinVersionMinor:byte;
       nExpectedWinVersionMajor:byte;
     end;

     TEXE_NE = object (NE_Base)
       nStartOffset:longint;

       procedure init (nStart:longint);
       function GetAbsSegmentTablePos:longint;
       function GetAbsResourceTablePos:longint;
       function GetAbsResidentNamesTablePos:longint;
       function GetAbsModuleReferenceTablePos:longint;
       function GetAbsImportedNamesTablePos:longint;
       function GetAbsEntryPointTablePos:longint;
       function GetSectorSize:longint;
       function HasSelfLoader:boolean;
     end;

     NE_Segment = record
       nRelativeOffset:word;
       nLength:word;
       nFlags:word;
       nAllocSize:word;
     end;

     tNE_Segment_Table = array[1..1] of NE_Segment;
     pNE_Segment_Table = ^tNE_Segment_Table;

     NE_Segment_Handler = object
       nSectorSize:longint;
       bHasSelfLoader:boolean;

       constructor init (nSecSize:longint;
                         nEntryCount:word;
                         bSelfLoader:boolean);
       destructor  done;
       procedure   ReadFromFile (var f:file;
                                     nPos:longint);
       function    GetLength (n:word):longint;
       function    GetAllocSize (n:word):longint;
       function    GetStartPos (n:word):longint;
       function    GetEndPos (n:word):longint;
       function    GetType (n:word):String4;
       function    GetFlags (n:word):word;
       function    GetEntryPoint (nCS, nIP:word):longint;
       function    GetTableSize:word;
       function    GetCalculatedFileSize:longint;

     private
       nTableLen:word;
       nTableSize:word;
       pBase:pNE_Segment_Table;
     end;

     NE_TypeInfo = record
       nTypeID:word;
       nResourceCount:word;
       nRes1:longint;
       { NAMEINFO[] }
     end;

     NE_NameInfo = record
       nOffset:word;
       nLength:word;
       nFlags:word;
       nID:word;
       nHandle:word;
       nUsage:word;
     end;

const
      NE_HEADER_SIZE   = SizeOf (NE_Base);     { 64 }
      NE_SEGMENT_SIZE  = SizeOf (NE_Segment);  {  8 }
      NE_TYPEINFO_SIZE = SizeOf (NE_TypeInfo); {  8 }
      NE_NAMEINFO_SIZE = SizeOf (NE_NameInfo); { 12 }

      NE_SEG_IS_DATA            = $0001;
      NE_SEG_ALREADY_ALLOCATED  = $0002;
      NE_SEG_LOADED             = $0004;
      { - $0008  - reserved - }
      NE_SEG_MOVEABLE           = $0010;
      NE_SEG_SHAREABLE          = $0020;
      NE_SEG_PRELOAD            = $0040;
      NE_SEG_EXECUTE_ONLY       = $0080;  { - if CODE - }
      NE_SEG_READONLY           = $0080;  { - if DATA - }
      NE_SEG_RELOCATIONS        = $0100;
      { - $0200  - reserved - }
      { - $0400  - reserved - }
      { - $0800  - reserved - }
      NE_SEG_DISCARDABLE        = $1000;
      { - $2000  - reserved - }
      { - $4000  - reserved - }
      { - $8000  - reserved - }

      NE_SEG_ERR_NO_ENTRYPOINT = -1;
      NE_SEG_ERR_TOO_LARGE     = -2;
      NE_SEG_ERR_EMPTY         = -3;

      RT_CURSOR       = 1;
      RT_BITMAP       = 2;
      RT_ICON         = 3;
      RT_MENU         = 4;
      RT_DIALOG       = 5;
      RT_STRING       = 6;
      RT_FONTDIR      = 7;
      RT_FONT         = 8;
      RT_ACCELERATOR  = 9;
      RT_RCDATA       = 10;
      RT_GROUP_CURSOR = 11 + 1;
      RT_GROUP_ICON   = 11 + 3;

implementation

uses
     GTMem,
     XGlobal;

constructor NE_Segment_Handler.init (nSecSize:longint;
                                     nEntryCount:word;
                                     bSelfLoader:boolean);
begin
  nSectorSize := nSecSize;
  nTableLen   := nEntryCount;
  nTableSize  := nTableLen * NE_SEGMENT_SIZE;
  pBase := nil;
  bHasSelfLoader := bSelfLoader;

  if (nTableSize > 0) then
  begin
    GetMem (pBase, nTableSize);
    CheckMemoryAllocation (pBase, nTableSize, 'NE_Segment_Handler.init');
  end;
end;

destructor NE_Segment_Handler.done;
begin
  if (nTableSize > 0) then
  begin
    FreeMem (pBase, nTableSize);
    pBase := nil;
  end;
end;

procedure NE_Segment_Handler.ReadFromFile (var f:file;
                                               nPos:longint);
begin
  seek (f, nPos);
  blockread (f, pBase^, nTableSize);
end;

function NE_Segment_Handler.GetLength (n:word):longint;
begin
  if (pBase^ [n].nLength = 0) and (pBase^ [n].nRelativeOffset <> 0) then
    GetLength := $10000
  else
    GetLength := pBase^ [n].nLength;
end;

function NE_Segment_Handler.GetAllocSize (n:word):longint;
begin
  if (pBase^ [n].nAllocSize = 0) then
    GetAllocSize := $10000
  else
    GetAllocSize := pBase^ [n].nAllocSize;
end;

function NE_Segment_Handler.GetStartPos (n:word):longint;
begin
  GetStartPos := pBase^ [n].nRelativeOffset * nSectorSize;
end;

function NE_Segment_Handler.GetEndPos (n:word):longint;
begin
  if (pBase^ [n].nRelativeOffset = 0) then
  begin
    GetEndPos := 0;
  end
  else
  begin
    GetEndPos := (pBase^ [n].nRelativeOffset * nSectorSize) + GetLength (n);
  end;
end;

function NE_Segment_Handler.GetType (n:word):String4;
begin
  if (pBase^ [n].nFlags and NE_SEG_IS_DATA > 0) then
    GetType := 'DATA'
  else
    GetType := 'CODE';
end;

function NE_Segment_Handler.GetFlags (n:word):word;
begin
  GetFlags := pBase^ [n].nFlags;
end;

function NE_Segment_Handler.GetEntryPoint (nCS, nIP:word):longint;
begin
  if (nCS = 0) then
    GetEntryPoint := NE_SEG_ERR_NO_ENTRYPOINT
  else
    if (nCS > nTableLen) then
      GetEntryPoint := NE_SEG_ERR_TOO_LARGE
    else
      if (GetStartPos (nCS) = 0) then
        GetEntryPoint := NE_SEG_ERR_EMPTY
      else
        if (bHasSelfLoader) then
          GetEntryPoint := GetStartPos (1)  { CS:IP = 0001:0000 }
        else
          GetEntryPoint := GetStartPos (nCS) + nIP;
end;

function NE_Segment_Handler.GetTableSize:word;
begin
  GetTableSize := nTableSize;
end;

function NE_Segment_Handler.GetCalculatedFileSize:longint;
var n:word;
    nRes:longint;
begin
  if (nTableLen > 0) then
  begin
    n := nTableLen;
    repeat
      nRes := GetEndPos (n);
      if (nRes = 0) then
        dec (n);
    until (nRes > 0) or (n = 0);
  end
  else
    nRes := 0;
  GetCalculatedFileSize := nRes;
end;

{ ------------------------------------------------------- }

procedure TEXE_NE.init (nStart:longint);
begin
  nStartOffset := nStart;
end;

function TEXE_NE.GetAbsSegmentTablePos:longint;
begin
  GetAbsSegmentTablePos := nStartOffset + nSegmentTableOfs;
end;

function TEXE_NE.GetAbsResourceTablePos:longint;
begin
  GetAbsResourceTablePos := nStartOffset + nResourceTableOfs;
end;

function TEXE_NE.GetAbsResidentNamesTablePos:longint;
begin
  GetAbsResidentNamesTablePos := nStartOffset + nResidentNamesTableOfs;
end;

function TEXE_NE.GetAbsModuleReferenceTablePos:longint;
begin
  GetAbsModuleReferenceTablePos := nStartOffset + nModuleReferenceTableOfs;
end;

function TEXE_NE.GetAbsImportedNamesTablePos:longint;
begin
  GetAbsImportedNamesTablePos := nStartOffset + nImportedNamesTableOfs;
end;

function TEXE_NE.GetAbsEntryPointTablePos:longint;
begin
  GetAbsEntryPointTablePos := nStartOffset + nEntryTableOfs;
end;

function TEXE_NE.GetSectorSize:longint;
begin
  GetSectorSize := longint (1) shl nFileAlignmentSizeShiftCount;
end;

function TEXE_NE.HasSelfLoader:boolean;
begin
  HasSelfLoader := (ApplicationFlags and BITS[3] <> 0);
end;

end.
