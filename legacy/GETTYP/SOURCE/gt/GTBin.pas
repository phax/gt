{$i aflags.inc}

unit GTBin;

interface

const
      KEYFile2 = 't';

function Check_BinText:boolean;
function Check_Binary:boolean;
function Check_HeadText:boolean;

implementation

uses
     GTBuf, GTFile, GTGlobal, GTOutput,
     XString;

type
     DBF_LocalHeader = record
       Name:array[0..10] of char;   {  0 - 10 }
       Typ:char;                    { 11 }
       Res1:array[1..4] of byte;    { 12 - 15 }
       Len:byte;                    { 16 }
       Decimals:byte;               { 17 }
       Res2:array[1..2] of byte;    { 18 - 19 }
       WorkspaceID:byte;            { 20 }
       Res3:array[1..11] of byte;   { 21 - 31 }
     end;

     DBF_GlobalHeader = record
       Flag:byte;
       LastMod:array[1..3] of char;
       RecCount:longint;
       HeaderSize:word;
       RecSize:word;
       Res1:word;
       BreakOnOperation:byte;  { 1 = aborted; 0 = okay }
       Encrypted:byte; { 1 = yes; 0 = no }
       Res2:array[1..12] of byte;
       MDXFile:byte;  { 1 = yes; 0 = no }
       Res3:array[1..3] of byte;
       {
       1..RecCount of LocalHeader
       GetByte (x) = 10
       }
     end;

     TTF_GlobalHeader = record
       nID:longint;
       nTables:word;
       nSearchRange:word;
       nEntrySelector:word;
       nRangeShift:word;
     end;

     TTF_LocalHeader = record
       nID:longint;
       nCheckSum:longint;
       nOffset:longint;
       nLength:longint;
     end;

const
      DBF_GH_SIZE = SizeOf (DBF_GlobalHeader);
      DBF_LH_SIZE = SizeOf (DBF_LocalHeader);
      TTF_GH_SIZE = SizeOf (TTF_GlobalHeader);
      TTF_LH_SIZE = SizeOf (TTF_LocalHeader);
      STR_SOURCE = 'Source code name: ';

var
    DBF_GH:DBF_GlobalHeader;
    TTF_GH:TTF_GlobalHeader;

function Check_HeadText:boolean;
var p:word;
{ still nothing found -> scan for headtexts }

    function ID2String (nPos:longint; sID, sMessage:string):boolean;
    begin
      if (BUF_GetString (nPos, length (sID)) = sID) then
      begin
        ID2String := true;
        if WriteHeader (sMessage) then
          FinishLine;
      end
      else
        ID2String := false;
    end;

begin
{ but only if it is a binary file }
  if (b.IsText) then
  begin
    Check_HeadText := false;
    exit;
  end;

  Check_HeadText := true;

{ check for fixed shortcuts }
  if ID2String (1, '[ver]'#13#10#9, 'Lotus AmiPro document') then exit;
  if ID2String (1, 'ARMV0006', 'Armadillo project file') then exit;
  if ID2String (1, 'IDAIDS',   'IDA DLL description file') then exit;
  if ID2String (1, 'IImGxP',   'Pax Imperia 2 save game') then exit;
  if ID2String (1, 'MS-PROF',  'Microsoft profiler file (PBI)') then exit;
  if ID2String (1, 'PNCIUNDO', 'Norton Utilities Disk Doctor undo file') then exit;
  if ID2String (1, 'PS-X EXE', 'Sony PlayStation executable') then exit;
  if ID2String (1, 'VCPCH0',   'Microsoft precompiled header file (PCH)') then exit;
  if ID2String (1, 'WordPro',  'Lotus WordPro document') then exit;
  if ID2String (5, 'Standard Jet DB', 'Microsoft Access database file (MDB)') then exit;

{ - seek for generic characters sequence - }
  p := 1;
  while (b.first.c [p] in TextChars) and (p < 256) do
    inc (p);
  dec (p);

{ - check if a generic string was found - }
  if (p >= 7) then  { min length: 8 }
  begin
    if WriteHeader (concat ('Headtext: "', BUF_GetString (1, p), '"')) then
      FinishLine;
  end
  else
    Check_HeadText := false;
end;

function GetGRPName:string;
begin
  TFile_Init (b.First.w [12]);
  GetGrpName := TFile_Read_To (#0);
end;

function GetPPUVersion:VersionStr;
var s:VersionStr;
begin
  s := BUF_GetString (4, 3);
  insert ('.', s, 2);
  GetPPUVersion := s;
end;

function WP_GetType:String25;
begin
  case b.First.b[10] of
    $01: WP_GetType := 'macro';
    $02: WP_GetType := 'help';
    $03: WP_GetType := 'keyboard definition';
    $0A: WP_GetType := 'document';
    $0B: WP_GetType := 'dictionary';
    $0C: WP_GetType := 'thesaurus';
    $0D: WP_GetType := 'block';
    $0E: WP_GetType := 'rectangular block';
    $0F: WP_GetType := 'column block';
    $10: WP_GetType := 'printer resource (PRS)';
    $11: WP_GetType := 'setup';
    $12: WP_GetType := 'prefix information';
    $13: WP_GetType := 'printer resource (ALL)';
    $14: WP_GetType := 'display resource (DRS)';
    $15: WP_GetType := 'overlay (WP.FIL)';
  { $16 is explicitly handled in GTIMAGE.PAS - "graphics (WPG)"
    saving space :-)  }
    $17: WP_GetType := 'hyphenation code module';
    $18: WP_GetType := 'hyphenation data module';
    $19: WP_GetType := 'macro resource (MRS)';
    $1A: WP_GetType := 'graphics driver (WPD)';
    $1B: WP_GetType := 'hyphenation lex module';

  { the following are not described in the docu; self formulated }
    $46: WP_GetType := 'PostScript font';
  else
    WP_GetType := concat ('unknown (', hexbyte (b.First.b[10]), 'h)');
  end;
end;

function GetPIFApp:string;
begin
  GetPIFApp := StripSpaces (BUF_GetString (3, 30));
end;

procedure PrintBGIComment (EndByte:byte);
var p:TPos;
begin
  p := BUF_GetPosOfByte (5, 1024, EndByte) - 1;
  if (p > POSERROR) then
    Noteln (BUF_GetString (5, p - 4))
  else
    Noteln ('No copyright notice was found :-(');
end;

function IsDBF:boolean;
const EMPTY_FIELD = 13;
var nEndPos:word;
    RecSize:word;
    FieldCount:word;
    nCalculatedFields:longint;

    function GetEndPos:word;
    var nPos:TPos;
        LH:DBF_LocalHeader;
    begin
      nPos := 33;
      while (nPos <= b.BufLen) and
            (BUF_GetByte (nPos) <> EMPTY_FIELD) and
            (FieldCount <= nCalculatedFields) do
      begin
        BUF_GetBuf (LH, nPos, DBF_LH_SIZE);
        inc (RecSize, LH.Len);
        inc (FieldCount);
        inc (nPos, DBF_LH_SIZE);
      end;
      if (nPos > b.BufLen) then
        GetEndPos := POSERROR
      else
        GetEndPos := nPos;
    end;

begin
  BUF_GetBuf (DBF_GH, 1, DBF_GH_SIZE);
  RecSize := 0;
  FieldCount := 0;
  nCalculatedFields := (longint (DBF_GH.HeaderSize) - DBF_GH_SIZE - 1) div DBF_LH_SIZE;

  nEndPos := GetEndPos;

{
fucking casting :( but otherwise there'll be some
runtime errors if it is compiled with BP7
}
  IsDBF := (DBF_GH.HeaderSize = nEndPos) and
           (DBF_GH.RecSize = RecSize + 1) and
           (FieldCount = nCalculatedFields) and
           (BUF_GetByte (nEndPos) = EMPTY_FIELD) and
           (nEndPos + DBF_GH.RecCount * DBF_GH.RecSize + 1 = b.FSize);
end;

function DBF_GetFieldCount:word;
begin
  DBF_GetFieldCount := round ((DBF_GH.HeaderSize - DBF_GH_SIZE - 1.0) / DBF_LH_SIZE);
end;

function DBF_GetFieldStr (c:char):ShortStr;
begin
  case c of
    'C':DBF_GetFieldStr := 'character';
    'D':DBF_GetFieldStr := 'date';
    'F':DBF_GetFieldStr := 'float';
    'L':DBF_GetFieldStr := 'logical';
    'M':DBF_GetFieldStr := 'memo';
    'N':DBF_GetFieldStr := 'numerical';
    'P':DBF_GetFieldStr := 'picture';
  else
    DBF_GetFieldStr := concat ('unknown type "', c, '"');
  end;
end;

procedure DBF_List_Header;
var LH:DBF_LocalHeader;
    i:word;
    sName:String12;
begin
  TFile_Init (DBF_GH_SIZE);
  for i:=1 to DBF_GetFieldCount do
  begin
    if (TFile_ReadActBuf (LH, DBF_LH_SIZE) <> DBF_LH_SIZE) then;
    sName := LH.Name;
    Noteln (concat (sName, STR_SEPERATOR, i2sf (LH.Len,4), STR_SEPERATOR, DBF_GetFieldStr (LH.Typ)));
  end;
end;

function IsTTF:boolean;
begin
  { fill global header :'O }
  SwapLong (b.First.l [1], TTF_GH.nID);
  TTF_GH.nTables        := Swap (b.First.w [3]);
  TTF_GH.nSearchRange   := Swap (b.First.w [4]);
  TTF_GH.nEntrySelector := Swap (b.First.w [5]);
  TTF_GH.nRangeShift    := Swap (b.First.w [6]);

  IsTTF := (b.FSize > 12) and { 6 * 2 }
           (TTF_GH.nID = $00010000) and
           (TTF_GH.nRangeshift = longint (TTF_GH.nTables) * 16 - TTF_GH.nSearchRange);
end;

procedure TTF_Analyze;
type
     TTF_name = record
       nFormat:word;
       nCount:word;
       nOffset:word;
     end;

     TTF_entry = record
       nPlatformID:word;
       nEncodingID:word;
       nLanguageID:word;
       nNameID:word;
       nStringLength:word;
       nRelOffset:word;
     end;
const
      TTF_NAME_SIZE  = sizeof (TTF_name);
      TTF_ENTRY_SIZE = sizeof (TTF_entry);
var
    LH:TTF_LocalHeader;
    LH_name:TTF_name;
    LH_entry:TTF_entry;
    nPos:longint;
    i:word;

    { starting at 1 }
    procedure ReadEntry (nPos:word; var s:string);
    var nStartPos:longint;
    begin
      s := '';
      if (nPos > LH_name.nCount) then
        exit;

      nStartPos := LH.nOffset;
      if (TFile_ReadBufAt (nStartPos + TTF_NAME_SIZE + (nPos - 1) * TTF_entry_SIZE,
                           LH_entry,
                           TTF_ENTRY_SIZE) <> TTF_ENTRY_SIZE) then;
      LH_entry.nPlatformID   := Swap (LH_entry.nPlatFormID);
      LH_entry.nEncodingID   := Swap (LH_entry.nEncodingID);
      LH_entry.nLanguageID   := Swap (LH_entry.nLanguageID);
      LH_entry.nNameID       := Swap (LH_entry.nNameID);
      LH_entry.nStringLength := Swap (LH_entry.nStringLength);
      LH_entry.nRelOffset    := Swap (LH_entry.nRelOffset);

      s := TFile_ReadString (nStartPos + LH_name.nOffset + LH_entry.nRelOffset,
                             LH_entry.nStringLength);
      if (LH_entry.nPlatFormID = 0) or (LH_entry.nPlatFormID = 3) then  { UniCode ?? }
        s := RemoveUniCodeChars (s);
    end;

    procedure CheckName;
    var sFontName, sType:string;
    begin
      TFile_Init (LH.nOffset);
      if (TFile_ReadActBuf (LH_name, TTF_NAME_SIZE) <> TTF_NAME_SIZE) then;
      LH_name.nFormat := Swap (LH_name.nFormat);
      LH_name.nCount  := Swap (LH_name.nCount);
      LH_name.nOffset := Swap (LH_name.nOffset);

      ReadEntry (2, sFontName);
      ReadEntry (3, sType);
      Noteln (concat (sFontName, ' - ', sType));
    end;

begin
  if WriteHeader ('TrueType Font') then
  begin
    FinishLine;

    nPos := TTF_GH_SIZE + 1;
    for i:=1 to TTF_GH.nTables do
    begin
      inc (nPos, 16);
      BUF_GetBuf (LH, nPos, TTF_LH_SIZE);
      SwapLongSelf (LH.nCheckSum);
      SwapLongSelf (LH.nOffset);
      SwapLongSelf (LH.nLength);
      if (LH.nID = $656D616E) then { "name" - do NOT swap }
        CheckName;
    end;
  end;
end;

procedure AnalyzeDelphi3DCU;
var nPos:TPos;
begin
  IncInsLevel;
  nPos := 21 + BUF_GetByte (19);
  Noteln (concat (STR_SOURCE, BUF_GetPascalString (nPos)));
  DecInsLevel;
end;

procedure AnalyzeDelphi4DCU;
begin
  IncInsLevel;
  Noteln (concat (STR_SOURCE, BUF_GetPascalString (22)));
  DecInsLevel;
end;

function GetImpulseTrackerModuleName:string;
var nPos:TPos;
begin
  nPos := BUF_GetPosOfByte (4, b.Buflen, 0);
  GetImpulseTrackerModuleName := BUF_GetString (5, nPos - 5);
end;

function Check_BinText:boolean;
var s:string;
begin
  Check_BinText := true;

  if (b.First.l [1] = $46445025) then
    s := 'Portable Document Format (PDF) file'
  else
    s := '';

  if (s > '') then
  begin
    if WriteHeader (s) then
      FinishLine;
  end
  else
    Check_BinText := false;
end;

function Check_Binary:boolean;
var s:string;
    x:byte;
    w:word;
begin
  { - because so many exits are coming and all are true - }
  Check_Binary := true;

  if (b.IsText) then
  begin
    Check_Binary := false;
    exit;
  end;

  if (b.First.l [1] = $43434D50) then
  begin
    s := GetGRPName;
    if (s > '') then
      s := concat (' "', s, '"');
    s := concat ('Windows group file', s);
  end
  else
  if ((b.First.l [1] and $00FFFFFF) = $00555050) then
    s := concat ('FPK Pascal ', GetPPUVersion, ' unit')
  else
  if (b.First.l [1] = $00000107) then
    s := 'FPK Pascal object file'
  else
  if (b.First.l [1] = $00112BAD) then
    s := 'C++ symbol file'
  else
  if (b.First.l [1] = $00000DF0) then
    s := 'C++ library file'
  else
  if (b.First.l [1] = $464C457F) then
    s := 'Unix/Linux excutable'
  else
  if (b.First.l [1] = $4D535150) then
    s := 'Microsoft Quick Pascal unit'
  else
  if (b.First.l [1] = $435057FF) then
    s := concat ('Wordperfect ', WP_GetType, ' file')
  else
  if (b.First.b [1] = 0) and (BUF_GetString (370, 16) = 'MICROSOFT PIFEX'#0) then
    s := concat ('Windows PIF file for "', GetPIFApp, '"')
  else
  if (b.First.l [1] = longint ($BEBAFECA)) then
    s := 'Compiled Java bytecode'
  else
  if (b.First.l [1] = $47455243) then
    s := 'Windows registry file (binary)'
  else
  if (b.First.l [1] = $544C4150) then
    s := 'Fractal Extreme palette file'
  else
  if (b.First.l [1] = $4D4F4F5A) then
    s := 'Fractal Extreme movie file'
  else
  if (b.First.l [1] = $0000004C) then
    s := 'Windows shortcut file'
  else
  if (b.First.l [1] = $54000AFF) then
    s := 'Borland Delphi 3/4 form module'
  else
  if (b.First.l [1] = $000CC9B8) then
    s := 'Compiled installSHIELD InstallScript'
  else
  if (b.First.l [1] = $564F4246) then
    s := 'Borland overlay file (OVR)'
  else
  if (b.First.l [1] = $4A41574B) then
    s := 'MS Compress 6.22 file'
  else
  if (b.First.l [1] = $33535054) then
    s := 'Borland precompiled header file (PSM)'
  else
  if (b.First.l [1] = $4D504D49) then
    s := concat ('Impulse Tracker Module "', GetImpulseTrackerModuleName, '"')
  else
  if (b.First.l [1] = $46484246) then
    s := 'Turbo Vision help file'
  else
  if (b.First.w [1] = $014C) then
    s := concat ('Common Object File Format (COFF) (', i2s (b.First.w [2]), ' entries)')
  else
  if (b.First.l [1] = $0000BE31) then
    s := 'MS Word for DOS/Write document'
  else
  if (b.First.l [1] = $002DA5DB) then
    s := 'Winword 2 document'
  else
  if (b.First.l [1] = longint ($E011CFD0)) then
    s := 'OLE2 document (Winword, Excel, Starwriter, ...)'
  else
  if (b.First.l [1] = $001A0000) then
    s := 'Lotus 123 worksheet'
  else
  if (b.First.l [1] = $00060409) then
    s := 'MS Excel 4 worksheet'
  else
  if (b.First.l [3] = $0000FFFF) and (b.First.l [4] = $0000FFFF) then
    s := 'Compiled resource file'
  else
  if (b.First.l [1] = $0300001A) then
    s := 'Lotus Notes 4.5 database'
  else
  if (b.First.l [1] = $01045F3F) then
    s := 'MS Visual C++ help file'
  else
  if (b.First.l [1] = $46535449) then
    s := 'MS HTML-Help file'
  else
  if (b.First.l [1] = $00020000) then
    s := 'Windows mouse cursor file'
  else
  if (b.First.l [1] = $01505349) then
    s := 'IBM Help file'
  else
  if (b.First.l [1] = $72613C21) then
    s := 'MS C/C++ library'
  else
  if ((b.First.l [1] and $00FFFFFF) = $00535746) then
    s := 'Macromedia Flash file'
  else
  if (b.First.l [5] = $766F6F6D) then
    s := 'QuickTime move file'
  else
  if (b.First.l [5] = longint ($862A0906)) then
    s := 'PKCS #7 certificate'
  else
  if (b.First.w [1] = $8230) and
     (b.First.w [3] = $8230) then
    s := 'DER encoded binary X.509 certificate'
  else
  if (b.First.l [1] = $0000A871) then
    s := 'installShield uninstall file'
  else
  if (b.First.l [1] = $4C534854) then
    s := 'Links LS shot file'
  else
  if (b.First.l [1] = longint ($FFFFFFDD)) and
     (b.First.l [2] = longint ($A1981FF1)) then
    s := 'Symantec Internet Tools Data File 1.0'
  else
  if (b.First.l [1] = $33495056) then
    s := 'Virtual Pascal Unit v3'
  else
    s := '';

{ check if something was already found }
  if (s > '') then
  begin
    if WriteHeader (s) then
      FinishLine;
  end
  else
  if (IsTTF) then
    TTF_Analyze
  else
  if (b.First.l [1] = $00035F3F) then
  begin
    s := 'Windows ';
    if (b.First.l [4] <> b.FSize) then
      s := concat (s, '3.x')
    else
      s := concat (s, '95/98');
    s := concat (s, ' help file');

    if WriteHeader (s) then
    begin
      FinishLine;
      { check if it is Win 3.x help file }
      if (b.First.l [4] <> b.FSize) then
      begin
        w := BUF_GetWord (40);
        if (w <= 255) then
        begin
          s := BUF_GetString (42, w);
          Noteln (concat ('"', s, '"'));
        end;
      end;
    end;
  end
  else
  if (IsDBF) then
  begin
    if WriteHeader (concat ('dBase database, v', i2s (b.First.b [1] and 7))) then
    begin
      FinishLine;
      Noteln (concat (i2s (DBF_GH.RecCount),   ' records with ',
                      i2s (DBF_GetFieldCount), ' fields   ',
                      i2s (DBF_GH.RecSize),    ' bytes'));
      DBF_List_Header
    end;
  end
  else
  if (b.First.l [1] = $08086B70) or (b.First.l [1] = $44474246) then
  begin
    if WriteHeader ('BGI Device Driver') then
    begin
      FinishLine;
      EmptyLine;
      PrintBGIComment (0);
    end;
  end
  else
  if (b.First.l [1] = $08084B50) then
  begin
    if WriteHeader ('BGI Stroked Font') then
    begin
      FinishLine;
      EmptyLine;
      PrintBGIComment (26);
    end;
  end
  else
  if (b.First.l [1] = $44518641) and (b.First.l [2] = b.FSize) then
  begin
    if WriteHeader ('Borland Delphi 3 unit') then
    begin
      FinishLine;
      AnalyzeDelphi3DCU;
    end;
  end
  else
  if (b.First.l [1] = $4768A6D8) and (b.First.l [2] = b.FSize) then
  begin
    if WriteHeader ('Borland Delphi 4 unit') then
    begin
      FinishLine;
      AnalyzeDelphi4DCU;
    end;
  end
  else
  if ((b.First.l [1] and $FFFFFF00) = $79724300) then
  begin
    x := b.First.b [1];
    if WriteHeader (concat ('Crypt! ', GetVersionStr (x shr 5 and 7, x shr 2 and 7))) then
    begin
      if (b.First.b [5] and 1 > 0) then
        Append (concat (' ', Enbraced ('high security level')));
      FinishLine;
    end;
  end
  else  { - Check_Binary's default is true! - }
    Check_Binary := false;
end;

end.
