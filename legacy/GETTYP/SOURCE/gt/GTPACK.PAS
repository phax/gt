{$define extended}
{$i aflags.inc}

{$ifndef VER60}
{$T+}
{$endif}

{$ifdef BORLAND}
{$R-}  { - disable range check for virtual array - }
{$endif}


unit GTPack;

interface

uses
     GTGlobal;

type
     TPacker = (
       NONE,    _777,    ACE,     AIN,     AKT7,    AKT8,
       AMG,     ARC,     ARG,     ARI,
       ARJ,     ARK,     ARQ,     ARS,     ARX,     AR7,     ASD,
       BIX,     BLI,     BOA,     BS2,     BSA,
       BSN,     BZ,      CAB,     CHZ,     COD,     COMP16,  CPAC,    DPA,
       DWC,     ELI,     ESP,     FINISH,  GZ,      HA,      HAP,
       HPA,     HQX,     HYP25,   HYP26,
       IMP,     INS,     ISC,     JAR,     JRC,
       LG,      LIM,     LS,      LZA,     LZH,     LZO,     MAR,
       MSC,     PACK,    PAQ,     PAR,     PHP,     PPMZ,
       Q,       QFC,     RAR0,    RAR,
       RAX,     RKV1,    RKV2,    RNC,
       SAR,     SBX,     SKY,     SOF,     SQWEZ,   SQZ,     STI,
       SWG,     SYN,     SZIP,    TAR,
       UC2,     UFA,     UHA,
       XIE,     XL,      XPA,     YC,
       YZ,      ZET,     ZIP,     ZIP2,    ZOO,     ZPK
     );

const
      P_MIN = _777;
      P_MAX = ZPK;

      PACKER_BUFFER_LEN = 2048;

{$ifdef FARC}
      SPacker:array [NONE..P_MAX] of string [6] = (
      'error', '777',   'ACE',   'AIN',   'AKT7',  'AKT8',
      'AMG',   'ARC',   'ARG',   'ARI',
      'ARJ',   'ARK',   'ARQ',   'ARS',   'ARX',   'AR7',   'ASD',
      'BIX',   'BLI',   'BOA',   'BS2',   'BSA',
      'BSN',   'BZ',    'CAB',   'CHZ',   'COD',   'COMP16','CPAC',  'DPA',
      'DWC',   'ELI',   'ESP',   'FINISH','GZ',    'HA',    'HAP',
      'HPA',   'HQX',   'HYP25', 'HYP26',
      'IMP',   'INS',   'ISC',   'JAR',   'JRC',
      'LG',    'LIM',   'LS',    'LZA',   'LZH',   'LZO',   'MAR',
      'MSC',   'PACK',  'oPAQue','PAR',   'PHP',   'PPMZ',
      'Q',     'QFC',   'RAR0',  'RAR',
      'RAX',   'RKV1',  'RKV2',  'RNC',
      'SAR',   'SBX',   'SKY',   'SOF',   'SQWEZ', 'SQZ',   'STI',
      'SWG',   'SYN',   'SZIP',  'TAR',
      'UC2',   'UFA',   'UHA',
      'XIE',   'XL',    'XPA',   'YC',
      'YZ',    'ZET',   'ZIP',   'ZIP2',  'ZOO',   'ZPK'
      );
{$endif}

      { - needed for EXE overlay packer listing - }
      HYP_ID = $2650481A;
      ZOO_ID = $FDC4A7DC;

      GTPACK_OLDSIZE :longint = 0;
      GTPACK_NEWSIZE :longint = 0;
      GTPACK_FILENAME:string  = '';
      GTPACK_DATETIME:longint = 0;
      GTPACK_PASSWORD:boolean = false;
      GTPACK_ARCNAME :string  = '';
      GTPACK_OldSizeUndefined:boolean = false;
      GTPACK_NewSizeUndefined:boolean = false;

type
     TProcAddFile = procedure;

procedure InitPackBuffer (nStartPos:longint;
                          nMaxLen:longint);
procedure CheckForKnownPackers (nStartPos:longint;
                                nMaxLen:longint;
                                bInitAndRestore:boolean);
procedure RestorePackBuffer;
procedure ShiftPackBuffer;

function  GetPackerID:TPacker;
function  GetPackerName (ID:TPAcker):ShortStr;
{$ifdef FARC}
function  GetPackerStrOfID (ID:TPacker):ShortStr;
function  GetPackerIDOfStr (s:ShortStr):TPacker;
{$endif}
procedure NotePacker;

var
     ArcCount, ArcSumOld, ArcSumNew:longint;
     ProcAddFile:TProcAddFile;
{$ifdef GETTYP}
     bDoShowArcContent:boolean;
{$endif}

implementation

uses
     GTBuf, GTFile, GTMem,
{$ifdef GETTYP}
     GTOutput,
     IOError,
{$endif}
     XDOS, XGlobal, XString;

{---------------------------TYPE---------------------------}
{ need to be public because other methods also use it ... }

type
     ARC_Header = record
       nID:byte;
       nCompressionMethod:byte;
       sFilename:array[0..12] of char;
       nNewSize:longint;
       nDT:longint;
       nCRC16:word;
       nOldSize:longint;
     end;

     GT_ARG_Header = record
       nID:byte; { should be 0 }
       sFilename:array[1..13] of char;
       nDT:longint;
       nOldSize:longint;
       nNewSize:longint;
     end;

     ARI_Filename = record
       case boolean of
         true:(sName:array[1..8] of char;
               sExt:array[1..3] of char);
         false:(sPath:array[1..11] of char);
     end;

     ARI_Header = record
       { path if any }
       aFilename:ARI_Filename;
       nAttr:byte;
       nDT:longint;
       nOldSize:longint;
       nNewSize:longint;
       nCRC32:longint;
       { data }
     end;

     COD_GlobalHeader = record
       nID:word;
       res1:byte;
       nCreationDT:longint;
       nModificationDT:longint;
       res2:array[1..40] of byte;
     end;

     COD_FileHeader = record
       res1:array [1..6] of byte;
       nCompressionDT:longint;
       nOldSize1:longint;
       nOldSize2:longint;
       nNewSize:longint;
       nCRC32:longint;
       res2:array[1..18] of byte;
       nDirNameLen:byte;
       nFilenameLen:byte;
       res3:array[1..4] of byte;
     end;

     HA_GlobalHeader = record
       ID:array[1..2] of char;
       nFilesInArchive:word;
     end;

     HA_LocalHeader = record
       nVersion:byte;
       nNewSize:longint;
       nOldSize:longint;
       nCRC32:longint;
       nDT:longint;
       {
       ..#0
       ..#0
       len
       1..len
       }
     end;

     LIM_Header = record
       res1:word;
       nHeaderSize:byte;
       res2:array[1..10] of byte;
       nOldSize:longint;
       nNewSize:longint;
       res5:longint;
       { .. #0 }
     end;

     LS_Global_Header = record
       nID1:word;
       nID2:word;
       res1:word;
       nHeaderSize:longint;
       nFilesInArc:word;
       res2:array[1..13] of byte;
       nOldSize:longint;
       nNewSize:longint;
       res3:word;
       nCreationDate:longint;
     end;

     Q_GlobalHeader = record
       nID:word;
       nVersionMajor:byte;
       nVersionMinor:byte;
       nFilesInArc:word;
       nTableSize:byte;
       nFlags:byte;
     end;

     Q_LocalHeader = record
       nOldSize:longint;
       nDT:longint;
     end;

     XIE_Header = record
       nOldSize:longint;
       nNewSize:longint;
       sFilename:string[12];
       nDataLen:longint;
     end;

{--------------------------CONST---------------------------}

const
      ARC_H_SIZE  = SizeOf (ARC_Header);
      ARG_H_SIZE  = SizeOf (GT_ARG_Header);  { global !! }
      ARI_H_SIZE  = SizeOf (ARI_Header);
      COD_GH_SIZE = SizeOf (COD_GlobalHeader);
      COD_FH_SIZE = SizeOf (COD_FileHeader);
      HA_GH_SIZE  = SizeOf (HA_GlobalHeader);
      HA_LH_SIZE  = SizeOf (HA_LocalHeader);
      LIM_H_SIZE  = SizeOf (LIM_Header);
      LS_GH_SIZE  = SizeOf (LS_Global_Header);
      Q_GH_SIZE   = SizeOf (Q_GlobalHeader);
      Q_LH_SIZE   = SizeOf (Q_LocalHeader);
      XIE_H_SIZE  = SizeOf (XIE_Header);

      STR_NOT_LIST = 'how to list this type of archive';
      STR_LINE = '-------------------------------------------';

type
     TListProc = procedure (ntartPos:longint);

     TPackerRec = object
       Found:boolean;
       Proc:TListProc;

       procedure init (b:boolean; p:TListProc);
     end;

var
    bHasNoPackedSize:boolean;
    p:array [P_MIN..P_MAX] of TPackerRec;
    PackerPosInFile:longint;
{$ifdef GETTYP}
    LocalSumOld, LocalSumNew, FilesInArchive:longint;
    bExtendedOutput:boolean;
{$endif}

procedure NoteDontKnow;
begin
{$ifdef GETTYP}
  Noteln (concat ('Don''t know ', STR_NOT_LIST, ' :-('));
{$endif}
end;

procedure TArchive_Init (StartPos:longint);
begin
  TFile_Init (StartPos);
  inc (ArcCount);
{$ifdef GETTYP}
  LocalSumOld      := 0;
  LocalSumNew      := 0;
  FilesInArchive   := 0;
  bExtendedOutput  := false;
  bHasNoPackedSize := false;
{$endif}
end;

procedure NoteArchiveVersion (sVer:String25);
begin
{$ifdef GETTYP}
  Noteln (concat ('Archive version: ', sVer));
{$endif}
end;

procedure TArchive_NoteHeader;
begin
{$ifdef GETTYP}
  EmptyLine;
  Noteln ('Original   Packed  Ratio   Name');
  Noteln (STR_LINE);
{$endif}
end;

procedure TArchive_NoteHeaderX;
begin
{$ifdef GETTYP}
  EmptyLine;
  Noteln ('Original   Packed  Ratio   Date     Name');
  Noteln (STR_LINE);
  bExtendedOutput := true;
{$endif}
end;

procedure NoteSizesPercent (OldSize, NewSize:longint);
const
      SPACE_OLD = 8;
      SPACE_NEW = 9;
begin
{$ifdef GETTYP}
  if (bHasNoPackedSize) then
  begin
    Note (concat (i2sf (OldSize, SPACE_OLD), '  --------   ---% '));
  end
  else
  begin
    Note (concat (i2sf (OldSize, SPACE_OLD),
                  i2sf (NewSize, SPACE_NEW)));

    if (OldSize = 0) and (NewSize > 0) then
    begin
      Append ('   ---% ');
      GTPACK_OldSizeUndefined := true;
    end
    else
    begin
      if OldSize > 0 then
        Append (concat (r2sfd (NewSize * 100.0 / OldSize, 6, 1), '% '))
      else
        Append ('   0.0% ');
    end;
  end;
{$endif}
end;

{$ifdef GETTYP}
procedure TArchive_AddFile; far;
begin
  if (GTPACK_NEWSIZE > b.FSize) and (1=2) then
  begin
    TFile_SetError (TFILE_ERR_VALUE);
    exit;
  end;

  inc (FilesInArchive);
  inc (LocalSumOld, GTPACK_OLDSIZE);
  inc (LocalSumNew, GTPACK_NEWSIZE);

  if (not bHasNoPackedSize) then
  begin
    { shr 10 - make KB out of it }
    inc (ArcSumOld, GTPACK_OLDSIZE shr 10);
    inc (ArcSumNew, GTPACK_NEWSIZE shr 10);
  end
  else
  begin
    { new: count onlyold size }
    { old: add GTPACK_OLDSIZE to both counter }
    { shr 10 - make KB out of it }
    inc (ArcSumOld, GTPACK_OLDSIZE shr 10);
    GTPACK_NewSizeUndefined := true;
    {
    inc (ArcSumNew, GTPACK_OLDSIZE shr 10);
    }
  end;

  { - for debugging purposes only - }
  if (GTPACK_OLDSIZE > 15000000) then
    GTPACK_OLDSIZE := GTPACK_OLDSIZE;
  if (GTPACK_NEWSIZE > 15000000) then
    GTPACK_NEWSIZE := GTPACK_NEWSIZE;

  NoteSizesPercent (GTPACK_OLDSIZE, GTPACK_NEWSIZE);

  if (GTPACK_DATETIME <> 0) then
    Append (concat (GetDateStr (GTPACK_DATETIME), ' '))
  else
    if (bExtendedOutput) then
      Append ('invalid  ');

  Append (GTPACK_FILENAME);
  if (GTPACK_PASSWORD) then
    Append (' [password]');
  Finishline;
end;
{$endif}

procedure TArchive_Done;
begin
{$ifdef GETTYP}
  if (TFile_Error <> TFILE_ERR_NONE) and
     (TFile_Error <> TFILE_ERR_USER) then
  begin
    TFile_Error := TFile_Error;
  end;

  if (LocalSumOld = 0) or
     ((LocalSumNew = 0) and (not bHasNoPackedSize)) then
  begin
    TFile_Error := TFile_Error;
  end;

  case TFile_Error of
    TFILE_ERR_NONE: ;  { - do nothing - }
    TFILE_ERR_USER: ;  { - do nothing - }
    TFILE_ERR_EOF:  Noteln ('Error: found unexpected end of file.');
    TFILE_ERR_ID:   Noteln ('Error: found wrong ID. File may be damaged.');
    TFILE_ERR_VALUE:Noteln ('Error: found invalid size value. Maybe GetTyp listing error.');
  else
    NoteIOError (TFile_Error);
    FinishLine;
  end;
{$endif}
end;

procedure TArchive_NoteFooter;
begin
{$ifdef GETTYP}
  TArchive_Done;  { - check 4 errors - }
  Noteln (STR_LINE);
  NoteSizesPercent (LocalSumOld, LocalSumNew);
  Append (concat (i2s (FilesInArchive), ' file'));
  if (FilesInArchive > 1) then
    Appendln ('s')
  else
    FinishLine;
{$endif}
end;

procedure List_Not (nStartPos:longint); far;
begin
  TArchive_Init (0);  { to increase some counter ... }
  NoteDontKnow;
end;

procedure List_Single (nStartPos:longint); far;
begin
  TArchive_Init (0);  { to increase some counter ... }
{$ifdef GETTYP}
  Noteln ('This is a single file archive');
{$endif}
end;

procedure _SetFileInfo (nOldSize:longint;
                        nNewSize:longint;
                        nDT:longint;
                        sFilename:string;
                        bPassword:boolean);
begin
  GTPACK_OLDSIZE  := nOldSIze;
  GTPACK_NEWSIZE  := nNewSIze;
  GTPACK_DATETIME := nDT;
  GTPACK_FILENAME := sFilename;
  GTPACK_PASSWORD := bPassword;
  ProcAddFile;
end;

{---------------------------777---------------------------}

procedure List_777 (nStartPos:longint); far;
type
     GT_777_GlobalHeader = record
       ID:array[1..3] of char;
       res1:array[1..3] of byte;
       nMethod:byte;
       res2:byte;
     end;

     GT_777_SolidGlobalHeader = record
       res1:array[1..13] of byte;
       nDirectoryOffset:longint;
       res2:array[18..19] of byte;
     end;

     GT_777_LocalHeader = record
       aLastAccess:longint;
       nPassword:byte;
       res1:array[6..8] of byte;
       nMethod:byte;
       res2:array[10..13] of byte;
       nAttr:longint;
       nCreationDT:array[1..2] of longint;
       nLastAccessDT:array[1..2] of longint;
       nModificationDT:array[1..2] of longint;

       res3:array[42..46] of byte;

       nNewSize:longint;
       nOldSize:longint;
       nFilenameLen:word;
       { - filename - }
     end;

     GT_777_SolidLocalHeader = record
       res1:array[1..28] of byte;
       nCRC32:longint;
       nMethod:byte;
       nOldSize:longint;
       nFilenameLen:word;
       { - filename - }
     end;

const
      _777_GH_SIZE  = SizeOf (GT_777_GlobalHeader);
      _777_SGH_SIZE = SizeOf (GT_777_SolidGlobalHeader);
      _777_LH_SIZE  = SizeOf (GT_777_LocalHeader);
      _777_SLH_SIZE = SizeOf (GT_777_SolidLocalHeader);
var
    GH:GT_777_GlobalHeader;
    SGH:GT_777_SolidGlobalHeader;
    LH:GT_777_LocalHeader;
    SLH:GT_777_SolidLocalHeader;
    sFilename:string;

    procedure ListSolid;
    begin
      bHasNoPackedSize := true;
      if (TFile_ReadActBuf (SGH, _777_SGH_SIZE) = _777_SGH_SIZE) then
      begin
        TFile_IncFilePos (SGH.nDirectoryOffset);
        repeat
          if (TFile_ReadActBuf (SLH, _777_SLH_SIZE) = _777_SLH_SIZE) then
          begin
            sFilename := TFile_ReadActLongString (SLH.nFilenameLen);
            _SetFileInfo (SLH.nOldSize,
                          0,
                          0,
                          sFilename,
                          false);
          end;
        until (not TFile_IsOkay);
      end;
    end;

    procedure ListDefault;
    begin
      repeat
        if (TFile_ReadActBuf (LH, _777_LH_SIZE) = _777_LH_SIZE) then
        begin
        {
          if (LH.nID <> 1) then
            TFile_SetError (TFILE_ERR_ID)
          else
        }
          begin
            sFilename := TFile_ReadActLongString (LH.nFilenameLen);
            _SetFileInfo (LH.nOldSize,
                          LH.nNewSize,
{$ifdef MSDOS}
 {$ifdef DPMI}
                          0,
 {$else}
                          { function is no longer available - this is the only place used }
                          { WinDT2DOSDT (@LH.nModificationDT), }
                          0,
 {$endif}
{$else}
                          0,
{$endif}
                          sFilename,
                          LH.nPassword and 1 > 0);
            TFile_IncFilePos (LH.nNewSize);
          end;
        end;
        { after the last entry, Read returns 0 -> Finish }
      until (not TFile_IsOkay);
    end;

begin
  TArchive_Init (nStartPos);

{$ifdef MSDOS}
  TArchive_NoteHeaderX;
{$else}
  TArchive_NoteHeader;
{$endif}

  TFile_ReadActBuf (GH, _777_GH_SIZE);

  if (GH.nMethod = 1) then
    LisTSolid
  else
    ListDefault;

  TArchive_NoteFooter;
end;

{---------------------------ACE---------------------------}

procedure List_ACE (nStartPos:longint); far;
type
     GT_ACE_GlobalHeader = record
       res1:word;
       nDataOfs:word;
       res2:array[1..3] of byte;
       nID:array[1..7] of char;
       nCompressVersion:byte;
       nDecompressVersion:byte;
     end;

     ACE_Header = record
       res1:word;
       nHeaderSize:word;
       nType:byte;
       res2:byte;
       nFlags:byte;
       nNewSize:longint;
       nOldSize:longint;
       nDT:longint;
       res3:array[1..14] of byte;
       nFilenameLen:word;
     end;
const
      ACE_GH_SIZE = SizeOf (GT_ACE_GlobalHeader);
      ACE_H_SIZE  = SizeOf (ACE_Header);
var
    GH:GT_ACE_GlobalHeader;
    H:ACE_Header;
    sFilename:string;
    nOldFilePos:longint;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, ACE_GH_SIZE);
{$ifdef GETTYP}
  NoteArchiveVersion (GetVersionStr (GH.nCompressVersion div 10,
                                     GH.nCompressVersion mod 10));
{$endif}
  TFile_GotoFilePos (nStartPos + 4 + GH.nDataOfs);

  TArchive_NoteHeaderX;

  repeat
    nOldFilePos := TFile_GetFilePos;
    if (TFile_ReadActBuf (H, ACE_H_SIZE) = ACE_H_SIZE) then
    begin
      if (H.nType = 1) then
      begin
        sFilename := TFile_ReadActLongString (H.nFilenameLen);
        _SetFileInfo (H.nOldSize,
                      H.nNewSize,
                      H.nDT,
                      sFilename,
                      H.nFlags and $40 > 0);
      end;
      TFile_GotoFilePos (nOldFilePos + H.nHeaderSize + H.nNewSize + 4);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------AIN---------------------------}

procedure List_AIN (nStartPos:longint); far;
type
     AIN_Header = record
       nID:word; { $1221 }
       res1:array[3..8] of byte;
       nFilesInArc:word;
       res2:array[11..14] of byte;
       nDataSize:longint;
     end;
const
      AIN_H_SIZE = SizeOf (AIN_Header);
var
    H:AIN_Header;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (H, AIN_H_SIZE);
{$ifdef GETTYP}
  Noteln (concat ('Files in archive: ', i2s (H.nFilesInArc)));
{$endif}
  NoteDontKnow;
  TArchive_Done;
end;

{---------------------------AKT7--------------------------}

procedure List_AKT7 (nStartPos:longint); far;
type
     AKT_GlobalHeader = record
       ID:array[1..3] of char;
       nHeaderVersion:byte;
       nFilesInArchive:word;
       nCommentLength:word;
       nCompressionMethod:byte;
       res:longint;
       { comment }
     end;

     AKT_LocalHeader = record
       { nPathLen:byte;
         ..path.. }
       sFilename:array[1..12] of char;
       nDT:longint;
       nAttr:word;
       nStartPos:longint;
       nEndPos:longint;
     end;
const
      AKT_GH_SIZE = SizeOf (AKT_GlobalHeader);
      AKT_LH_SIZE = SizeOf (AKT_LocalHeader);
var
    GH:AKT_GlobalHeader;
    LH:AKT_LocalHeader;
    nPathLength:byte;
    sLastPath, sPath:string;
    i:word;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, AKT_GH_SIZE);
  TFile_IncFilePos (GH.nCommentLength);

  TArchive_NoteHeaderX;

  for i:=1 to GH.nFilesInArchive do
  begin
    nPathLength := TFile_ReadActByte;
    sPath := TFile_ReadActString (nPathLength);

    { if the path is #1 then it is equal to the last path }
    if (sPath = chr (1)) then
      sPath := sLastPath
    else
      sLastPath := sPath;

    TFile_ReadActBuf (LH, AKT_LH_SIZE);
    _SetFileInfo (LH.nEndPos - LH.nStartPos,
                  0,
                  LH.nDT,
                  concat (sPath, LH.sFilename),
                  false);
  end;

  TArchive_Notefooter;
end;

{---------------------------AKT8--------------------------}

procedure List_AKT8 (nStartPos:longint); far;
type
     AKT8_GlobalHeader = record
       ID:array[1..3] of char;
       nHeaderVersion:byte;
       dummy:array[1..5] of byte;
     end;

     AKT8_LocalHeader = record
       sFilename:string[12]; { one length byte and 12 bytes for the name no matter if used or not }
       nNewSize:longint;
       nOldSize:longint;
       dummy:longint;
       nDT:longint;
       dummy2:array[1..3] of byte;
     end;
const
      AKT8_GH_SIZE = SizeOf (AKT8_GlobalHeader);
      AKT8_LH_SIZE = SizeOf (AKT8_LocalHeader);
var
    GH:AKT8_GlobalHeader;
    LH:AKT8_LocalHeader;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, AKT8_GH_SIZE);

  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (LH, AKT8_LH_SIZE) = AKT8_LH_SIZE) then
    begin
      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize,
                    LH.nDT,
                    concat (LH.sFilename),
                    false);
      TFile_IncFilePos (LH.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_Notefooter;
end;

{---------------------------AMG---------------------------}

procedure List_AMG (nStartPos:longint); far;
type
     AMG_GlobalHeader = record
       nID:word;                   {  0 -  1 }
       nExtractVersion:byte;       {  2 }
       res1:array[3..9] of byte;   {  3 -  9 }
       nCommentLen:word;           { 10 - 11 }
     end;

     AMG_LocalHeader = record
       nID:word;                      {  0 -  1 }
       nNewSize:longint;              {  2 -  5 }
       nOldSize:longint;              {  6 -  9 }
       nDT:longint;                   { 10 - 13 }
       nAttr:byte;                    { 14 }
       nFlags:word;                   { 15 - 16 }
       nCRC32:longint;                { 17 - 20 }
       sFileName:array[1..8] of char; { 21 - 28 }
       sFileExt:array[1..3] of char;  { 29 - 31 }
       nPathLen:byte;                 { 32 }
       nFileCommentLen:word;          { 33 - 34 }
     end;
const
      AMG_GH_SIZE = SizeOf (AMG_GlobalHeader);
      AMG_LH_SIZE = SizeOf (AMG_LocalHeader);
var
    GH:AMG_GlobalHeader;
    LH:AMG_LocalHeader;
    Len:TPos;
    VerByte:byte;
    sFilename:string;
begin
  TArchive_Init (nStartPos);

  VerByte := TFile_ReadByteAt (nStartPos + 2);
  NoteArchiveVersion (GetVersionStr (VerByte shr 4, VerByte and $F));

  TArchive_NoteHeaderX;
  TFile_ReadBufAt (nStartPos, GH, AMG_GH_SIZE);

  repeat
    Len := TFile_ReadActBuf (LH, AMG_LH_SIZE);
    if (Len = AMG_LH_SIZE) and (LH.nID = $1C94) then
    begin
      TFile_IncFilePos (LH.nNewSize -
                        AMG_LH_SIZE -
                        LH.nPathLen -
                        LH.nFileCommentLen);
      sFilename := concat (TFile_ReadActString (LH.nPathLen),
                           CopyUntil (LH.sFileName, #0), '.',
                           CopyUntil (LH.sFileExt, #0));

      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize,
                    LH.nDT,
                    sFilename,
                    LH.nFlags and 1 > 0);

      if (LH.nFileCommentLen > 0) then
      begin
{$ifdef GETTYP}
        Noteln ('Comment: ');
        TFile_Print (LH.nFileCommentLen);
        FinishLine;
{$else}
        TFile_Read_To (#0);
{$endif}
      end;
    end;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------ARC---------------------------}

procedure List_ARC (nStartPos:longint); far;
var
    H:ARC_Header;
{$ifdef GETTYP}
    nOldFilePos:longint;
{$endif}
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;

  repeat
    TFile_ReadActBuf (H, ARC_H_SIZE);
    case H.nCompressionMethod of
      0:TFile_Finish;
      1..19:
        begin
          _SetFileInfo (H.nOldSize,
                        H.nNewSize,
                        0,
                        CopyUntil (H.sFilename, #0),
                        false);
        end;
      20..29:
        begin
{$ifdef GETTYP}
          nOldFilePos := TFile_GetFilePos;
          TFile_IncFilePos (3);
          Noteln ('Comment: ');
          TFile_Print_To (#0);
          FinishLine;
          TFile_GotoFilePos (nOldFilePos);
{$endif}
        end;
    end;
    TFile_IncFilePos (H.nNewSize);
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------ARG---------------------------}

procedure List_ARG (nStartPos:longint); far;
var
    H:GT_ARG_Header;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;
  repeat
    if (TFile_ReadActBuf (H, ARG_H_SIZE) = ARG_H_SIZE) then
    begin
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    H.nDT,
                    H.sFilename,
                    false);
      TFile_IncFilePos (H.nNewSize);
    end
    else
      if (H.nID = 1) then
        TFile_Finish;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------ARI---------------------------}

procedure List_ARI (nStartPos:longint); far;
label ReadAgain;
var
    H:ARI_Header;
    sDir, sTemp, sFilename:string;
    nOldFilePos:longint;
    nPos:byte;
    bFirst:boolean;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;
  sDir := '';

  repeat
    nOldFilePos := TFile_GetFilePos;
    sFilename := '';
    bFirst := true;

ReadAgain:
    if (TFile_ReadBufAt (nOldFilePos, H, ARI_H_SIZE) = ARI_H_SIZE) then
    begin
      dec (H.nNewSize, ARI_H_SIZE);
      if (H.nOldSize < 0) or (H.nNewSize < 0) then
        TFile_SetError (TFILE_ERR_VALUE);

      { - check if a path is found - }
      sTemp := H.aFilename.sPath;
      nPos := pos ('\', sTemp);
      if (nPos > 0) then
      begin
        inc (nOldFilePos, nPos);
        sTemp := copy (sTemp, 1, nPos);
        if (bFirst) then
        begin
          sDir := sTemp;
          bFirst := false;
        end
        else
          sDir := concat (sDir, sTemp);

        goto ReadAgain;
      end;

      sFilename := concat (sDir,
                           CopyUntil (H.aFilename.sName, #32), '.',
                           CopyUntil (H.aFilename.sExt, #32));
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    H.nDT,
                    sFilename,
                    false);

      TFile_IncFilePos (H.nNewSize);
    end;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------ARJ---------------------------}

{ not far because it is already in the interface part ... }
procedure List_ARJ (nStartPos:longint); far;
type
     GT_ARJ_GlobalHeader = record
       id:word;
       basic_headersize:word;

       first_headersize:byte;
       pack_version:byte;
       extract_version:byte;
       host_os:byte;

       flags:byte;
       security_version:byte;
       file_type:byte;
       reserved1:byte;

       dt_original:longint;

       dt_modified:longint;

       archive_size:longint;

       security_envelope:longint;

       filespec_pos_in_filename:word;
       security_envelope_length:word;

       reserved2:word;
     end;

     GT_ARJ_LocalHeader = record
       ID:word;
       nHeaderSize:word;
       nFirstHeaderSize:byte;
       nPackVersion:byte;
       nExtractVersion:byte;
       nHostOS:byte;
       nFlags:byte;
       nMethod:byte;
       nFileType:byte;
       res1:byte;
       nDT:longint;
       nNewSize:longint;
       nOldSize:longint;
       nOrigCRC:longint;
       filespec_pos_in_filename:word;
       nFileAccessMode:word;
       nHostData:word;
     end;
const
      FPassword = $01;  { flag }
      FExtFile  = $08;  { flag }

      ARJ_GH_SIZE = SizeOf (GT_ARJ_GlobalHeader);
      ARJ_LH_SIZE = SizeOf (GT_ARJ_LocalHeader);
var GH:GT_ARJ_GlobalHeader;
    LH:GT_ARJ_LocalHeader;
    nSavedPos:longint;
    sFilename:string;
    nFirstExtHeaderSize:word;
begin
  TArchive_Init (nStartPos);

  TFile_ReadActBuf (GH, ARJ_GH_SIZE);
  case GH.pack_version of
      3:NoteArchiveVersion ('2.21');
      4:NoteArchiveVersion ('2.30');
      5:NoteArchiveVersion ('2.39');
      6:NoteArchiveVersion ('2.41');
      7:NoteArchiveVersion ('2.42(c) - 2.50(a)');
      8:NoteArchiveVersion ('2.55 - 2.60');
      9:NoteArchiveVersion ('2.62');
     10:NoteArchiveVersion ('2.63 - 2.70');
     51:NoteArchiveVersion ('ARJZ');
    100:NoteArchiveVersion ('3.00a');
    101:NoteArchiveVersion ('3.03');
  else
    NoteArchiveVersion (concat ('unknown (', i2s (GH.pack_version), ')'));
  end;

{ new header flag in version 2.63 or higher }
  if ((GH.pack_version >= 10) and (GH.pack_version < 50)) or
     (GH.pack_version >= 101) then
    TFile_IncFilePos (4);

  { here we are at position 34 (2 + basic_headersize }
{$ifdef GETTYP}
  Note ('Archive name: "');
  TFile_Print_To (#0);
  Appendln ('"');

  if (TFile_Print_To (#0) > 0) then   { - comment - }
    FinishLine;
{$else}
  TFile_Read_To (#0);
  TFile_Read_To (#0);
{$endif}

  TFile_IncFilePos (4);  { CRC32 }

  nFirstExtHeaderSize := TFile_ReadActWord;
  if (nFirstExtHeaderSize > $0000) then
    TFile_IncFilePos (nFirstExtHeaderSize + 4);  { data + CRC }

  TArchive_NoteHeaderX;

  repeat
    nSavedPos := TFile_GetFilePos;
    TFile_ReadActBuf (LH, ARJ_LH_SIZE);
    if (LH.nHeaderSize > $0000) then
    begin
      if (LH.ID <> 60000) then
        TFile_SetError (TFILE_ERR_ID)
      else
      begin
        { - added - }
        TFile_GotoFilePos (nSavedPos + 4 + LH.nFirstHeaderSize);

        sFilename := TFile_Read_To (#0);
        _SetFileInfo (LH.nOldSize,
                      LH.nNewSize,
                      LH.nDT,
                      sFilename,
                      LH.nFlags and FPassword > 0);

        { - added - }
        TFile_GotoFilePos (nSavedPos +       { old position }
                           4 +               { header start }
                           LH.nHeaderSize +  { header }
                           4);               { local CRC32 }

        nFirstExtHeaderSize := TFile_ReadActWord;
        if (nFirstExtHeaderSize > $0000) then
          TFile_IncFilePos (nFirstExtHeaderSize + 4);  { data + CRC }

        TFile_IncFilePos (LH.nNewSize);
      end;
    end
    else
      TFile_Finish;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------ARK---------------------------}

procedure List_ARK (nStartPos:longint); far;
type
     ARK_GlobalHeader = record
       nID:longint;
       res1:longint;
       res2:byte;
     end;

     ARK_LocalHeader = record
       {
         dummy:word;
         filenamelen:byte;
         filename ...
       }
       nAttr:byte;
       res1:byte;
       nDT:longint;
       nOldSize:longint;
       nNewSize:longint;
       nCRC16:word;
       res2:word;
     end;
const
      ARK_GH_SIZE = SizeOf (ARK_GlobalHeader);
      ARK_LH_SIZE = SizeOf (ARK_LocalHeader);
var
    GH:ARK_GlobalHeader;
    LH:ARK_LocalHeader;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, ARK_GH_SIZE);

  TArchive_NoteHeaderX;

  repeat
    TFile_IncFilePos (2);
    sFilename := TFile_ReadActPascalString;
    if (TFile_ReadActBuf (LH, ARK_LH_SIZE) = ARK_LH_SIZE) then
    begin
      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize,
                    LH.nDT,
                    sFilename,
                    false);
      TFile_IncFilePos (LH.nNewSize);
      if (TFile_GetFilePos > b.FSize) then
        TFile_Finish;
    end;
  until (not TFile_IsOkay);

  TArchive_Notefooter;
end;

{---------------------------ARQ---------------------------}

procedure List_ARQ (nStartPos:longint); far;
type
     ARQHeader = record
       nID:word;
       res1:byte;
       nType:byte;
       nVersion:word;
       nFilenameLen:word;
     end;

     ARQBody = record
       res1:array[1..10] of byte;
       nNewSize:longint;
       nOldSize:longint;
       nCRC32:longint;
       res3:word;
       nFlags:byte;
       res5:longint;
       res6:longint;
     end;
const
      ARQ_H_SIZE = SizeOf (ARQHeader);
      ARQ_B_SIZE = SizeOf (ARQBody);
var
    H:ARQHeader;
    B:ARQBody;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (H, ARQ_H_SIZE) = ARQ_H_SIZE) then
    begin
      if (H.nFilenameLen > 0) and (H.nID = $5767) then
      begin
        case (H.nType) of
          1:begin
              sFilename := TFile_ReadActLongString (H.nFilenameLen);
              TFile_ReadActBuf (B, ARQ_B_SIZE);
              _SetFileInfo (B.nOldSize,
                            B.nNewSize,
                            0,
                            sFilename,
                            B.nFlags and $04 > 0);
              TFile_IncFilePos (B.nNewSize);
            end;
          2:TFile_IncFilePos (4);
{$ifdef GETTYP}
        else
          Noteln (concat ('Unknown type ', i2s (H.nType)));
{$endif}
        end;
      end
      else
        TFile_Finish
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------ARX---------------------------}

procedure List_ARX (nStartPos:longint); far;
type
     ARXHeader = record
       nID1:byte;
       nHeaderSize:byte;
       ID2:array[1..6] of char;
       nNewSize:longint;
       nOldSize:longint;
       nDT:longint;
       nAttr:word;
       nFilenameLen:byte;
     end;
const
      ARX_H_SIZE    = SizeOf (ARXHeader);
var
    H:ARXHeader;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (H, ARX_H_SIZE) = ARX_H_SIZE) then
    begin
      if (H.nNewSize = 0) then
        H.nNewSize := H.nOldSize;

      sFilename := TFile_ReadActLongString (H.nFilenameLen);
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    H.nDT,
                    sFilename,
                    false);

      TFile_IncFilePos (H.nNewSize + 1);
    end
    else
      if (H.nID1 = 0) then
        TFile_Finish;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------AR7---------------------------}

{ not listed }

{---------------------------ASD---------------------------}

procedure List_ASD (nStartPos:longint); far;
type
     ASD_GlobalHeader = record
       ID:array[1..5] of char;
       nSubVersion:byte;
       nFilesInArc:word;
     end;

     ASD_LocalHeader = record
       { ... filename ... }
       nOldSize:longint;
       nCRC32:longint;
       nDT:longint;
       nAttr:word;
     end;

const
      ASD_GH_SIZE = SizeOf (ASD_GlobalHeader);
      ASD_LH_SIZE = SizeOf (ASD_LocalHeader);
var
    GH:ASD_GlobalHeader;
    LH:ASD_LocalHeader;
    sFilename:string;
    i:word;
begin
  TArchive_Init (nStartPos);
  bHasNoPackedSize := true;

  TFile_ReadActBuf (GH, ASD_GH_SIZE);
  TArchive_NoteHeaderX;

  for i:=1 to GH.nFilesInArc do
  begin
    sFilename := TFile_ReadActPascalString;
    TFile_ReadActBuf (LH, ASD_LH_SIZE);
    _SetFileInfo (LH.nOldSize,
                  0,
                  LH.nDT,
                  sFilename,
                  false);
  end;

  TArchive_NoteFooter;
end;

{---------------------------BIX---------------------------}

procedure List_BIX (nStartPos:longint); far;
type
    BIX_GlobalHeader = record
      nID:longint;
      nRes:array[1..20] of byte;
    end;

    BIX_LocalHeader = record
      Res1:array[1..7] of byte;
      nHeaderSize:word;
      nNewSize:longint;
      nOldSize:longint;
      Res2:array[1..10] of byte;
      nMethod:byte;
      Res3:array[1..6] of byte;
      { now th filename ... (not until #0!) }
    end;
const
      BIX_GH_SIZE = SizeOf (BIX_GlobalHeader);
      BIX_LH_SIZE = SizeOf (BIX_LocalHeader);
var
    GH:BIX_GlobalHeader;
    LH:BIX_LocalHeader;
    nFilenameLen:word;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, BIX_GH_SIZE);
  TArchive_NoteHeader;

  repeat
    if (TFile_ReadActBuf (LH, BIX_LH_SIZE) = BIX_LH_SIZE) then
    begin
      { - calculate the string length - }
      nFilenameLen := LH.nHeaderSize - BIX_LH_SIZE;
      sFilename := TFile_ReadActLongString (nFilenameLen);
      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize,
                    0,
                    sFilename,
                    false);
      TFile_IncFilePos (LH.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------BLI---------------------------}

procedure List_BLI (nStartPos:longint); far;
type
    BLI_GlobalHeader = record
      sID:array[0..15] of char;
      nVersion:word;
    end;

    BLI_LocalHeader = record
      case byte of
        1:(nMethod:byte;
           nBytesToSkip:word);
        250:(nDate1:word;
             nTime1:word;
             nOldSize1:longint;
             sFilename1:array[1..13] of char;
             nAttr1:byte);
        251:(sFilename2:array[1..13] of char;
             nAttr2:byte;
             nOldSize2:longint;
             nDate2:word;
             nTime2:word);
    end;

const
      BLI_GH_SIZE = SizeOf (BLI_GlobalHeader);
      BLI_LH_SIZE = SizeOf (BLI_LocalHeader);
var
    GH:BLI_GlobalHeader;
    LH:BLI_LocalHeader;
    nPrevPos, nHeadPos, nNewSize, nOldSize:longint;
    nDT:record
      case boolean of
        true: (nTime, nDate:word);
        false: (nDT:longint);
    end;
    bFirst:boolean;
    sFilename:string;

    procedure GetSizeAndSetInfo;
    begin
      nPrevPos := nHeadPos;
      nHeadPos := TFile_GetFilePos - BLI_LH_SIZE;
      nNewSize := nHeadPos - nPrevPos;

      { just skip the first element ... }
      if (not bFirst) then
      begin
        _SetFileInfo (nOldSize,
                      nNewSize,
                      nDT.nDT,
                      sFilename,
                      false);
      end
      else
        bFirst := false;
    end;

begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, BLI_GH_SIZE);

  NoteArchiveVersion (concat (i2s (GH.nVersion div 100), '.',
                              i2s (GH.nVersion mod 100)));

  TArchive_NoteHeaderX;

  bFirst := true;

  repeat
    if (TFile_ReadActBuf (LH, BLI_LH_SIZE) = BLI_LH_SIZE) then
    begin
      if (LH.nMethod > 1) then
      begin
        GetSizeAndSetInfo;

        case GH.nVersion of
          250:begin
                nOldSize  := LH.nOldSize1;
                nDT.nDate := LH.nDate1;
                nDT.nTime := LH.nTime1;
                sFilename := CopyUntil (LH.sFilename1, #0);
              end;
          251:begin
                nOldSize  := LH.nOldSize2;
                nDT.nDate := LH.nDate2;
                nDT.nTime := LH.nTime2;
                sFilename := CopyUntil (LH.sFilename2, #0);
              end;
        end;
      end
      else
      begin
        TFile_IncFilePos (3 + LH.nBytesToSkip - BLI_LH_SIZE);
      end;
    end;
  until (not TFile_IsOkay);

  GetSizeAndSetInfo;

  TArchive_NoteFooter;
end;

{---------------------------BOA---------------------------}

procedure List_BOA (nStartPos:longint); far;
type
     BOA_GlobalHeader = record
       ID:longint;
       nMinorVersion:byte;
       nMajorVersion:byte;
       nMegaBytesUsed:byte;
     end;

     BOA_LocalHeader = record
       nDT:longint;      { very strange format; not std }
       nCompressionType:byte;  { ?? }
       nOldSize:longint;
       nNewSize:longint;
       nCRC32:longint;
       res1:word;
       nAttr:byte;
     end;
const
      BOA_GH_SIZE = SizeOf (BOA_GlobalHeader);
      BOA_LH_SIZE = SizeOf (BOA_LocalHeader);
var
    GH:BOA_GlobalHeader;
    LH:BOA_LocalHeader;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, BOA_GH_SIZE);
  NoteArchiveVersion (GetVersionStr (GH.nMajorVersion, GH.nMinorVersion));
  TArchive_NoteHeader;

  repeat
    { automatically increases file pointer ... }
    sFilename := TFile_Read_To (#0);

    TFile_ReadActBuf (LH, BOA_LH_SIZE);
    if (sFilename > '') then
    begin
      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize,
                    0,
                    sFilename,
                    false);
      TFile_IncFilePos (LH.nNewSize);
    end
    else
      TFile_Finish;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------BS2---------------------------}

procedure List_BS2 (nStartPos:longint); far;
type
     BS2_Header = record
       res1:word;
       res2:word;
       res3:longint;
       res4:word;
       res5:word;
       res6:longint;
       nOldSize:longint;
       nNewSize:longint;
       res7:longint;
       nFilenameLen:byte;
       { name - Res8:longint; }
     end;
const
      BS2_H_SIZE = SizeOf (BS2_Header);
var
    H:BS2_Header;
    Len:word;
    sFilename:string;
begin
  TArchive_Init (nStartPos + 4);  { skip ID header }
  TArchive_NoteHeader;
  repeat
    Len := TFile_ReadActBuf (H, BS2_H_SIZE);
    if (Len = BS2_H_SIZE) then
    begin
      sFilename := TFile_ReadActLongString (H.nFilenameLen);
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    0,
                    sFilename,
                    false);
      TFile_IncFilePos (4 + H.nNewSize);
    end
    else
      if (Len = 2) then
        TFile_Finish;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------BSA---------------------------}

procedure List_BSA (nStartPos:longint); far;
type
     BSA_Header = record
       nHeaderSize:byte;
       nZero:byte;
       nID:word;
       nNewSize:longint;
       nOldSize:longint;
       nDT:longint;
       nCRC16:longint;
       nAttr:word;
       nFilenameLen:byte;
     end;
const
      BSA_H_SIZE = SizeOf (BSA_Header);
var
    H:BSA_Header;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (H, BSA_H_SIZE) = BSA_H_SIZE) then
    begin
      sFilename := TFile_ReadActString (H.nFilenameLen);
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    H.nDT,
                    sFilename,
                    false);
      TFile_IncFilePos (1 + H.nNewSize);
    end
    else
      if (H.nHeaderSize = 0) then
        TFile_Finish;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------BSN---------------------------}

procedure List_BSN (nStartPos:longint); far;
type
     BSN_Header1 = record
       nID:longint;
       nAttr:word;
       res1:longint;
       nDT:longint;
     end;

     BSN_Header2 = record
       nOldSize:longint;
       nNewSize:longint;
       res2:longint;
       res3:longint;
     end;
const
      BSN_H1_SIZE = SizeOf (BSN_Header1);
      BSN_H2_SIZE = SizeOf (BSN_Header2);
var
    H1:BSN_Header1;
    H2:BSN_Header2;
    sFilename:string;
    nOldFilePos:longint;
    Len:word;
begin
  TArchive_Init (nStartPos + 6);
  TArchive_NoteHeaderX;

  repeat
    nOldFilePos := TFile_GetFilePos;
    Len := TFile_ReadActBuf (H1, BSN_H1_SIZE);
    if (Len = BSN_H1_SIZE) then
    begin
      sFilename := TFile_Read_To (#0);
      if (TFile_ReadActBuf (H2, BSN_H2_SIZE) = BSN_H2_SIZE) then
      begin
        SwapLongSelf (H2.nOldSize);
        SwapLongSelf (H2.nNewSize);
        SwapLongSelf (H1.nDT);

        _SetFileInfo (H2.nOldSize,
                      H2.nNewSize,
                      H1.nDT,
                      sFilename,
                      false);

        TFile_GotoFilePos (nOldFilePos +
                           BSN_H1_SIZE +
                           1 + length (sFilename) +
                           BSN_H2_SIZE +
                           H2.nNewSize);
      end
    end
    else
      if (Len = 2) then
        TFile_Finish;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------BZ---------------------------}

{ - not listed - }

{---------------------------CAB---------------------------}

procedure List_CAB (nStartPos:longint); far;
type
     GT_CAB_GlobalHeader = record
       nID:longint;        { 00 }
       null1:longint;      { 04 }
       nFSize:longint;     { 08 }
       null2:longint;      { 12 }
       nDataStart:longint; { 16 }
       r1:longint;         { 20 }
       r2:longint;         { 24 }
       nFileCount:word;    { 28 }
       r3:word;            { 30 }
     end;
const
      CAB_GH_SIZE = SizeOf (GT_CAB_GlobalHeader);
var
    Rec:record
      nOldSize:longint;
      res1:longint;
      nDT:longint;
      res2:longint;
    end;
    sFilename:string;
    G:GT_CAB_GlobalHeader;
    i:word;
begin
  TArchive_Init (nStartPos);
  bHasNoPackedSize := true;

  TFile_ReadActBuf (G, CAB_GH_SIZE);
{$ifdef GETTYP}
  if (G.nFSize < b.FSize) then
    Noteln (concat ('Additional space: ', i2s (b.FSize - g.nFSize), ' bytes'));
{$endif}

  TFile_GotoFilePos (nStartPos + G.nDataStart);
  TArchive_NoteHeaderX;

  for i:=1 to G.nFileCount do
  begin
    TFile_ReadActBuf (Rec, 4 * 4);
    sFilename := TFile_Read_To (#0);
    _SetFileInfo (Rec.nOldSize,
                  0,
                  Rec.nDT,
                  sFilename,
                  false);
  end;

  TArchive_NoteFooter;
end;

{---------------------------CHZ---------------------------}

procedure List_CHZ (nStartPos:longint); far;
type
     CHZ_GlobalHeader = record
       nID:array[1..3] of byte;
       nHeaderType:byte;
     end;

     CHZ_DirHeader = record
       res:array[1..5] of byte;
       nDirNameLen:byte;
       { directory name }
     end;

     CHZ_FileHeader = record
       nNewSize:longint;
       nOldSize:longint;
       res1:longint;
       res2:longint;
       res3:word;
       nFilenameLen:word;
       { file name }
     end;
const
      CHZ_GH_SIZE = SizeOf (CHZ_GlobalHeader);
      CHZ_DH_SIZE = SizeOf (CHZ_DirHeader);
      CHZ_FH_SIZE = SizeOf (CHZ_FileHeader);
var
    GH:CHZ_GlobalHeader;
    DH:CHZ_DirHeader;
    FH:CHZ_FileHeader;
    nOldFilePos:longint;
    sTemp, sDirectory:string;
    p:byte;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;

  sDirectory := '';

  repeat
    nOldFilePos := TFile_GetFilePos;
    if (TFile_ReadActBuf (GH, CHZ_GH_SIZE) = CHZ_GH_SIZE) then
    begin
      if (GH.nID [1] = $53) and
         (GH.nID [2] = $43) and
         (GH.nID [3] = $68) then
      begin
        case (GH.nHeaderType) of
          { - directory - }
          $44:begin
                TFile_ReadActBuf (DH, CHZ_DH_SIZE);
                sTemp := TFile_ReadActString (DH.nDirNameLen);
                sDirectory := concat (sDirectory, sTemp, '\');
              end;

          { - file - }
          $46:begin
                TFile_ReadActBuf (FH, CHZ_FH_SIZE);
                sTemp := TFile_ReadActLongString (FH.nFilenameLen);
                _SetFileInfo (FH.nOldSize,
                              FH.nNewSize,
                              0,
                              concat (sDirectory, sTemp),
                              false);
                TFile_GotoFilePos (nOldFilePos + FH.nNewSize);
              end;

          { - end of directory }
          $64:begin
                { delete last char ("\") }
                sDirectory := copy (sDirectory, 1, Length (sDirectory) - 1);
                p := LastPos ('\', sDirectory);
                if (p > 0) then
                  sDirectory := copy (sDirectory, 1, p)
                else
                  sDirectory := '';
              end;
{$ifdef GETTYP}
        else
          SendFileToMe;
{$endif}
        end;
      end
      else
        TFile_SetError (TFILE_ERR_ID);
    end;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------COD---------------------------}

procedure List_COD (nStartPos:longint); far;
var
    GH:COD_GlobalHeader;
    FH:COD_FileHeader;
    sDirectory, sFilename:string;
    nOldFilePos:longint;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;

  TFile_ReadActBuf (GH, COD_GH_SIZE);

  repeat
    nOldFilePos := TFile_GetFilePos;
    if (TFile_ReadActBuf (FH, COD_FH_SIZE) = COD_FH_SIZE) then
    begin
      sDirectory := TFile_ReadActString (FH.nDirnameLen);
      sDirectory := ReplaceChar (sDirectory, #1, '\');
      sFilename  := TFile_ReadActString (FH.nFilenameLen);

      dec (FH.nOldSize1, COD_FH_SIZE);

      _SetFileInfo (FH.nOldSize1,
                    FH.nNewSize,
                    0,
                    concat (sDirectory, sFilename),
                    false);
      TFile_GotoFilePos (nOldFilePos + FH.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------CPAC---------------------------}

procedure List_CPAC (nStartPos:longint); far;
{$ifdef GETTYP}
var c:char;
{$endif}
begin
{$ifdef GETTYP}
  TArchive_Init (nStartPos + 6);

  IncInsLevel;
  c := TFile_ReadActChar;
  case c of
    'A':Noteln ('Text mode');
    'B':Noteln ('ASCII mode');
    'C':Noteln ('Binary mode');
  else
    Noteln (concat ('Unknown mode "', c, '"'));
  end;
  DecInsLevel;

  NoteDontKnow;
  TArchive_Done;
{$endif}
end;

{---------------------------DPA---------------------------}

procedure List_DPA (nStartPos:longint); far;
type
     DPA_Header = record
       ID:array[1..20] of char;
       res1:word;
       nOldSize:longint;
       nNextOfs:longint;
       sFilename:array[0..13] of char;
     end;
const
      DPA_H_SIZE = SizeOf (DPA_Header);
var
    H:DPA_Header;
    nOldFilePos:longint;
begin
  TArchive_Init (nStartPos);
  NoteArchiveVersion (GetVersionStr (TFile_ReadByteAt (nStartPos + 21),
                                     TFile_ReadByteAt (nStartPos + 20)));
  TArchive_NoteHeader;
  { - because of ReadByteAt - }
  TFile_GotoFilePos (nStartPos);

  repeat
    nOldFilePos := TFile_GetFilePos;
    if (TFile_ReadActBuf (H, DPA_H_SIZE) = DPA_H_SIZE) then
    begin
      _SetFileInfo (H.nOldSize,
                    H.nNextOfs - nOldFilePos,
                    0,
                    CopyUntil (H.sFilename, #0),
                    false);
      TFile_GotoFilePos (H.nNextOfs);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------DWC---------------------------}

procedure List_DWC (nStartPos:longint); far;
type
     DWC_GlobalHeader = record
       nHeaderSize:word;
       nEntrySize:word;
       res1:longint;
       res2:longint;
       res3:longint;
       res4:longint;
       nFileCount:longint;
       ID:array[1..3] of char;
     end;

     DWC_LocalHeader = record
       sFilename:array[0..12] of char;
       nOldSize:longint;
       res1:longint;
       nNewSize:longint;
       res2:longint;
       res3:longint;
       res4:byte;
     end;
const
      DWC_GH_SIZE = SizeOf (DWC_GlobalHeader);
      DWC_LH_SIZE = SizeOf (DWC_LocalHeader);
var
    GH:DWC_GlobalHeader;
    LH:DWC_LocalHeader;
    i, HeadStartPos:longint;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;

  HeadStartPos := nStartPos + b.FSize - DWC_GH_SIZE;
  TFile_ReadBufAt (HeadStartPos, GH, DWC_GH_SIZE);
  TFile_GotoFilePos (HeadStartPos - (GH.nFileCount * GH.nEntrySize));

  for i:=1 to GH.nFileCount do
  begin
    TFile_ReadActBuf (LH, DWC_LH_SIZE);
    _SetFileInfo (LH.nOldSize,
                  LH.nNewSize,
                  0,
                  CopyUntil (LH.sFilename, #0),
                  false);
  end;

  TArchive_NoteFooter;
end;

{---------------------------ELI---------------------------}

procedure List_ELI (nStartPos:longint); far;
type
     ELI_LocalHeader = record
       nID:longint;
       nOldSize:longint;
       nDT:longint;
       nAttribute:byte;
       nCRC16:word;
       nNewSize:longint;
       res1:longint;
       nMethod:byte;
       res2:word;
       nFilenameLen:byte;
       { filename }
     end;
const
      ELI_LH_SIZE = SizeOf (ELI_LocalHeader);
var
    LH:ELI_LocalHeader;
    sFilename:string;
    nTmp:word;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (LH, ELI_LH_SIZE) = ELI_LH_SIZE) then
    begin
      if (LH.nID <> $2061724F) then
        TFile_SetError (TFILE_ERR_ID)
      else
      begin
        sFilename := TFile_ReadActString (LH.nFilenameLen);
        _SetFileInfo (LH.nOldSize,
                      LH.nNewSize,
                      LH.nDT,
                      sFilename,
                      false);

        if (LH.nMethod = 3) then
        begin
          nTmp := TFile_ReadActWord;
          if (nTmp and $F000 = $E000) then
            TFile_IncFilePos (2);
        end;

        TFile_IncFilePos (LH.nNewSize);
      end;
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------ESP---------------------------}

procedure List_ESP (nStartPos:longint); far;
type
     ESP_GlobalHeader = record
       nID:longint;
       nVersion:byte;
     end;
const
      ESP_GH_SIZE = SizeOf (ESP_GlobalHeader);
var
    GH:ESP_GlobalHeader;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, ESP_GH_SIZE);
  NoteArchiveVersion (GetVersionStr (GH.nVersion shr 4,
                                     GH.nVersion and $0F));
  NoteDontKnow;
  TArchive_Done;
end;

{-------------------------FINISH--------------------------}

procedure List_FINISH (nStartPos:longint); far;
type
     FINISH_Header = record
       nOldSize:longint;
       nDT:longint;
       nCRC32:longint;
       res2:word;
       nNewSize:longint;
       res3:word;
     end;
const
      FINISH_H_SIZE = SizeOf (FINISH_Header);
label ReadFilename;
var
    H:FINISH_Header;
    sDir, sFilename:string;
begin
  TArchive_Init (nStartPos + 10);
  TArchive_NoteHeader;
  sDir := '';

  repeat
  ReadFilename:
    sFilename := TFile_ReadActPascalString;
    if (pos ('\', sFilename) > 0) then
    begin
      sDir := sFilename;
      TFile_IncFilePos (1);
      goto ReadFilename;
    end;

    if (TFile_ReadActBuf (H, FINISH_H_SIZE) = FINISH_H_SIZE) then
    begin
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    0,
                    concat (sDir, sFilename),
                    false);

      TFile_IncFilePos (H.nNewSize);
      if (TFile_GetFilePos >= b.FSize) then
        TFile_Finish;
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------GZ---------------------------}

procedure List_GZ (nStartPos:longint); far;
var
    nNewSize:longint;
    sFilename:string;
    i:byte;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;

  nNewSize := TFile_ReadLongAt (b.FSize - 4);

  if (TFile_ReadByteAt (3) = 8) then
  begin
    TFile_GotoFilePos (nStartPos + 10);
    sFilename := TFile_Read_To (#0);
  end
  else
    sFilename := '???';

  _SetFileInfo (nNewSize,
                b.FSize - nStartPos,
                0,
                sFilename,
                false);

  TArchive_NoteFooter;
end;

{---------------------------HA---------------------------}

procedure List_HA (nStartPos:longint); far;
var
    LH:HA_LocalHeader;
    nSpecial:byte;
    sPathname, sFilename:string;
begin
  TArchive_Init (nStartPos + 4);
  TArchive_NoteHeader;

  repeat
    if (TFile_ReadActBuf (LH, HA_LH_SIZE) = HA_LH_SIZE) then
    begin
      if (LH.nNewSize < b.FSize) then
      begin
        sPathname := TFile_Read_To (#0);
        sPathName := ReplaceChar (sPathname, #255, '\');
        sFilename := TFile_Read_To (#0);

        _SetFileInfo (LH.nOldSize,
                      LH.nNewSize,
                      0,
                      concat (sPathName, sFilename),
                      false);

        nSpecial := TFile_ReadActByte;
        TFile_IncFilePos (nSpecial + LH.nNewSize);
      end
      else
        TFile_SetError (TFILE_ERR_VALUE);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------HAP---------------------------}

{ - not listed - }
procedure List_HAP (nStartPos:longint); far;
type
     HAP_LocalHeader = record
       nID:longint;
       nNewSize:longint;
       res1:array[1..9] of byte;
       nAttr:byte;
       DT:longint;
       nOldSize:longint;
       sFilename:array[1..13] of char;
     end;
const
      HAP_LH_SIZE = SizeOf (HAP_LocalHeader);
var
    LH:HAP_LocalHeader;
begin
  TArchive_Init (nStartPos + 15);
  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (LH, HAP_LH_SIZE) = HAP_LH_SIZE) then
    begin
      if (LH.nID = $574A688E) then
      begin
        _SetFileInfo (LH.nOldSize,
                      LH.nNewSize,
                      LH.DT,
                      CopyUntil (LH.sFilename, #0),
                      false);

        TFile_IncFilePos (LH.nNewSize + 1);
      end
      else
        TFile_SetError (TFILE_ERR_ID);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------HPA---------------------------}

{ - not listed - }

{--------------------------HYP25--------------------------}

procedure List_HYP25 (nStartPos:longint); far;
type
     HYPHeader = record
       n26:byte;
       nID:word;
       nVersion:byte;
       nNewSize:longint;
       nOldSize:longint;
       nDT:longint;
       nCheckSum:longint;
       nFileAttr:byte;
       nFilenameLen:byte;
     end;
const
      HYP_H_SIZE = SizeOf (HYPHeader);
var
    H:HYPHeader;
    nLen:TPos;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

  repeat
    nLen := TFile_ReadActBuf (H, HYP_H_SIZE);
    if (nLen = HYP_H_SIZE) then
      if (H.n26 = 26) and ((H.nID = $5453) or (H.nID = $5048)) then
      begin
        sFilename := TFile_ReadActString (H.nFilenameLen);
        _SetFileInfo (H.nOldSize,
                      H.nNewSize,
                      H.nDT,
                      sFilename,
                      false);
        TFile_IncFilePos (H.nNewSize);
      end
      else
        TFile_SetError (TFILE_ERR_ID);
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{--------------------------HYP26--------------------------}

procedure List_HYP26 (nStartPos:longint); far;
type
     HYP26_Header = record
       nID:longint;
       nNewSize:longint;
       nOldSize:longint;
       nDT:longint;
       res2:longint;
       nAttr:byte;
       nFilenameLen:byte;
       { filename }
     end;
const
      HYP26_H_SIZE = SizeOf (HYP26_Header);
var
    H:HYP26_Header;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (H, HYP26_H_SIZE) = HYP26_H_SIZE) then
    begin
      if (H.nID = HYP_ID) then
      begin
        sFilename := TFile_ReadActString (H.nFilenameLen);
        _SetFileInfo (H.nOldSize,
                      H.nNewSize,
                      H.nDT,
                      sFilename,
                      false);
        TFile_IncFilePos (H.nNewSize);
      end
      else
        TFile_SetError (TFILE_ERR_ID);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------INS---------------------------}

procedure List_INS (nStartPos:longint); far;
var
    nCounter, nFileCount,
    nOldSize, nNewSize:longint;
    sName1, sName2:string;
    nLen:word;
begin
  TArchive_Init (nStartPos + 78);  { at 78 }
  TArchive_NoteHeader;

  nFileCount := TFile_ReadActLong;  { at 82 }
  TFile_ReadActLong;  { at 86 }
  nCounter := 0;

  repeat
    inc (nCounter);
    if (nCounter > 1) then
      TFile_IncFilePos (4);
    nNewSize := TFile_ReadActLong;
    nOldSize := TFile_ReadActLong;
    TFile_IncFilePos (8);

    nLen := TFile_ReadActWord;
    sName1 := TFile_ReadActLongString (nLen);

    nLen := TFile_ReadActWord;
    sName2 := TFile_ReadActLongString (nLen);

    _SetFileInfo (nOldSize,
                  nNewSize,
                  0,
                  concat (sName2, ' -> ', sName1),
                  false);
  until (nCounter = nFileCount);

  TArchive_NoteFooter;
end;

{---------------------------ISC---------------------------}

{ - not listed - }

{---------------------------JAR---------------------------}

{ - not listed - }

{---------------------------JRC---------------------------}

procedure List_JRC (nStartPos:longint); far;
type
     JRC_Header = record
       res1:byte;
       res2:longint;
       nNewSize:longint;
       { .. 0A - filename }
     end;
const
      JRC_H_SIZE = SizeOf (JRC_Header);
var
    H:JRC_Header;
    Len:TPos;
    FSize:longint;
begin
  TArchive_Init (nStartPos + 9);
  FSize := TFile_ReadActLong;
  if (FSize <> b.FSize) then
  begin
{$ifdef GETTYP}
    Noteln ('Archive may be damaged - size of archive is not equal');
{$endif}
    exit;
  end;

  TArchive_NoteHeader;

  repeat
    Len := TFile_ReadActBuf (H, JRC_H_SIZE);
    if (Len = JRC_H_SIZE) then
    begin
      _SetFileInfo (0,
                    H.nNewSize,
                    0,
                    TFile_Read_To (#10),
                    false);
      TFile_IncFilePos (H.nNewSize);
    end
    else
      if (Len = 1) then
        TFile_Finish;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------LG----------------------------}

procedure List_LG (nStartPos:longint); far;
type
     LG_Header = record
       nFlags:byte;  { $F0: HSC - $0F: method }
       nNewSize:longint;
       nOldSize:longint;
       nCRC32:longint;
       nDT:longint;
       { directoryname till #0 }
       { filename till #0 }
       { 2 bytes more }
     end;
const
      LG_H_SIZE = SizeOf (LG_Header);
var
    H:LG_Header;
    sDirectory:string;
    sFilename:string;
begin
  TArchive_Init (nStartPos + 4);
  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (H, LG_H_SIZE) = LG_H_SIZE) then
    begin
      sDirectory := TFile_Read_To (#0);
      sFilename := TFile_Read_To (#0);
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    H.nDT,
                    concat (sDirectory, sFilename),
                    false);

      TFile_IncFilePos (2);

      if (H.nFlags and $F0 > 0) then
      begin
        if (H.nFlags <> 81) then
          TFile_IncFilePos (3);
      end;

      TFile_IncFilePos (H.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------LS----------------------------}

procedure List_LS (nStartPos:longint); far;
type
     LS_Local_Header = record
       sFilename:array [1..13] of char;
       nOffset:longint;
     end;
const
      LS_LH_SIZE = SizeOf (LS_Local_Header);
var
    GH:LS_Global_Header;
    LH:LS_Local_Header;
    nLastOffset:longint;
    sFilename:String12;
    n:word;
label LoopEnd;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;
  if (TFile_ReadActBuf (GH, LS_GH_SIZE) = LS_GH_SIZE) then;

  nLastOffset := GH.nHeaderSize;

  for n:=1 to GH.nFilesInArc do
  begin
    if (TFile_ReadBufAt (nLastOffset, LH, LS_LH_SIZE) = LS_LH_SIZE) then
    begin
      { - store last offset in directory - }
      nLastOffset := TFile_GetFilePos;
      sFilename := CopyUntil (LH.sFilename, #0);

      if (TFile_ReadBufAt (LH.nOffset, GH, LS_GH_SIZE) = LS_GH_SIZE) then
      begin
        _SetFileInfo (GH.nOldSize,
                      GH.nNewSize,
                      0,
                      sFilename,
                      false);
      end
      else
        goto LoopEnd;
    end
    else
      goto LoopEnd;
  end;
LoopEnd:

{$ifdef GETTYP}
  if (n <= GH.nFilesInArc) then
    Noteln ('Error reading archive data');
{$endif}

  TArchive_NoteFooter;
end;

{---------------------------LIM---------------------------}

procedure List_LIM (nStartPos:longint); far;
var
    H:LIM_Header;
    nLen:TPos;
    sFilename:string;
begin
  TArchive_Init (nStartPos + 13);
  TArchive_NoteHeader;

  repeat
    nLen := TFile_ReadActBuf (H, LIM_H_SIZE);
    if (nLen = LIM_H_SIZE) then
    begin
      sFilename := TFile_Read_To (#0);
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    0,
                    sFilename,
                    false);
      TFile_IncFilePos (H.nNewSize);
    end
    else
      if (H.nHeaderSize = nLen) then
        TFile_Finish;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------LZA---------------------------}

procedure List_LZA (nStartPos:longint); far;
type
     LZA_GlobalHeader = record
       ID:array[1..10] of byte;
       res1:array[1..13] of byte;
       nFilesInArc:longint;
       res2:array[1..99] of byte;
     end;

     LZA_LocalHeader = record
       nRes1:array[1..15] of byte;
       nNewSize:longint;
       nOldSize:longint;
       nDateTime:longint;
       nAttr:longint;
       nCRC32:longint;
       nFilenameLen:byte;
       nRes2:longint;
       nRes3:byte;
     end;
const
      LZA_GH_SIZE = SizeOf (LZA_GlobalHeader);
      LZA_LH_SIZE = SizeOf (LZA_LocalHeader);
var
    GH:LZA_GlobalHeader;
    LH:LZA_LocalHeader;
    nFilenamePos, i:longint;
    sFilename:string;

    procedure GetFilename;
    var i, len:byte;
        nSavedPos:longint;
    begin
      { store current position }
      nSavedPos := TFile_GetFilePos;

      { read string }
      sFilename := TFile_ReadString (nFilenamePos, LH.nFilenameLen);

      { return to saved position }
      TFile_GotoFilePos (nSavedPos);

      { inc internal name position }
      inc (nFilenamePos, LH.nFilenameLen);

      { decrypt it }
      len := length (sFilename);
      for i:=1 to len do
        sFilename [i] := chr (ord (sFilename [i]) - 3 - i);
    end;

begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

  TFile_ReadActBuf (GH, LZA_GH_SIZE);
  nFilenamePos := TFile_GetFilePos + (GH.nFilesInArc * LZA_LH_SIZE);

  for i:=1 to GH.nFilesInArc do
    if (TFile_ReadActBuf (LH, LZA_LH_SIZE) = LZA_LH_SIZE) then
    begin
      { read and decrypt filename }
      GetFilename;

      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize,
                    LH.nDateTime,
                    sFilename,
                    false);
    end;

  TArchive_NoteFooter;
end;

{---------------------------LZH---------------------------}

procedure List_LZH (nStartPos:longint); far;
type
     LZH_Header = record
       nHeaderSize:byte;
       nCheckSum:byte;
       ID:array[1..5] of char;
       nNewSize:longint;
       nOldSize:longint;
       nDT:longint;
       nAttr:byte;
       nHeaderVersion:byte;
     end;

const
      LZH_H_SIZE = SizeOf (LZH_Header);
var
    H:LZH_Header;
    nLen:TPos;
    sDirname, sFilename:string;
    nExtType:byte;
    nNextHeaderSize:word;

    procedure HandleExtensions;
    begin
      repeat
        nExtType := TFile_ReadActByte;
        case (nExtType) of
          $00:begin
                TFile_IncFilePos (2); { common }
              end;
          $01:begin
               { the -3 is for the nExtType and the nNextHeaderSize }
               sFilename := TFile_ReadActString (nNextHeaderSize - 3);
               if (sFilename > '') then
                 _SetFileInfo (H.nOldSize,
                               H.nNewSize,
                               UnixDT2DOSDT (H.nDT), { H.nDT, is unix style }
                               concat (sDirname, sFilename),
                               false);
              end;
          $02:begin { directory }
                sDirname := TFile_ReadActString (nNextHeaderSize - 3);
                sDirname := ReplaceChar (sDirname, #255, '\');
              end;
          $3F:begin { comment }
                TFile_IncFilePos (nNextHeaderSize - 3);
              end;
          $40:begin { MSDOS: attributes }
                TFile_IncFilePos (2);
              end;
          else
          begin
            TFile_SetError (TFILE_ERR_ID);
          end;
        end;
        nNextHeaderSize := TFile_ReadActWord;
      until (nExtType =  0);
    end;

begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;
  sDirname := '';

  repeat
    nLen := TFile_ReadActBuf (H, LZH_H_SIZE);

    if (nLen = 1) then
      TFile_Finish
    else
      if (nLen = LZH_H_SIZE) then
      begin
        if (H.nHeaderSize = 0) then
          TFile_Finish
        else
        begin
          case H.nHeaderVersion of
            0:begin
                sFilename := TFile_ReadActPascalString;
                _SetFileInfo (H.nOldSize,
                              H.nNewSize,
                              H.nDT,
                              sFilename,
                              false);
                TFile_IncFilePos (2);  { CRC 16 }
                TFile_IncFilePos (H.nNewSize);
              end;
            1:begin
                sFilename := TFile_ReadActPascalString;
                _SetFileInfo (H.nOldSize,
                              H.nNewSize,
                              H.nDT,
                              sFilename,
                              false);
                TFile_IncFilePos (3);  { CRC 16 + OS ID }
                TFile_IncFilePos (H.nNewSize);
                nNextHeaderSize := TFile_ReadActWord;
                {
                HandleExtensions;
                }
              end;
            2:begin
                TFile_IncFilePos (3);
                nNextHeaderSize := TFile_ReadActWord;
                HandleExtensions;
                TFile_IncFilePos (H.nNewSize);
              end;
          else
            TFile_SetError (TFILE_ERR_USER);
          end;
        end;
      end
      else
        TFile_SetError (TFILE_ERR_USER);
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------LZO---------------------------}

procedure List_LZO (nStartPos:longint); far;
type
     LZO_Header = record
       res1:longint;
       nOldSize:longint;
       nNewSize:longint;
       res2:array [1..9] of byte;
     end;
const
      LZO_H_SIZE = SizeOf (LZO_Header);
var
    H:LZO_Header;
    sFilename:string;
begin
  TArchive_Init (nStartPos + $21);
  TArchive_NoteHeader;

  repeat
    sFilename := TFile_ReadActPascalString;
    if (TFile_ReadActBuf (H, LZO_H_SIZE) = LZO_H_SIZE) then
    begin
      SwapLongSelf (H.nOldSize);
      SwapLongSelf (H.nNewSize);

      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    0,
                    sFilename,
                    false);
      TFile_IncFilePos (H.nNewSize + $20);
      if (TFile_GetFilePos > b.FSize) then
        TFile_Finish;
    end;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------MAR---------------------------}

procedure List_MAR (nStartPos:longint); far;
type
     MAR_Header = record
       nHeaderSize:byte;
       res1:byte;
       ID:array[1..5] of char;
       nNewSize:longint;
       nOldSize:longint;
       res2:longint;
       nAttr:byte;
       res3:byte;
       nFilenameLen:byte;
       {
       filename
       }
     end;
const
      MAR_H_SIZE = SizeOf (MAR_Header);
var
    H:MAR_Header;
    nLen:TPos;
    nOldFilePos:longint;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;

  repeat
    nOldFilePos := TFile_GetFilePos;
    nLen := TFile_ReadActBuf (H, MAR_H_SIZE);
    if (nLen = MAR_H_SIZE) then
    begin
      sFilename := TFile_ReadActString (H.nFilenameLen);
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    0,
                    sFilename,
                    false);
      TFile_GotoFilePos (nOldFilePos + H.nHeaderSize + H.nNewSize + 2);
    end
    else
      if (H.nHeaderSize = 0) then
        TFile_Finish;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------MSC---------------------------}

procedure List_MSC (nStartPos:longint); far;
type
     MSC_Header = record
       res1:longint;
       res2:byte;
       cLastChar:char;
       nOldSize:longint;
     end;

const
      MSC_H_SIZE = SizeOf (MSC_Header);
var
    H:MSC_Header;
    sFilename:string;
    p:byte;
begin
  TArchive_Init (nStartPos + 4);
  TArchive_NoteHeader;
  TFile_ReadActBuf (H, MSC_H_SIZE);

  sFilename := b.FName;
  sFilename[length (sFilename)] := upcase (H.cLastChar);

  p := LastPos ('\', sFilename);
  if (p > 0) then
    sFilename := copy (sFilename, p + 1, 255);

  _SetFileInfo (H.nOldSize,
                b.FSize,
                0,
                sFilename,
                false);
  TArchive_NoteFooter;
end;

{---------------------------PACK---------------------------}

procedure List_PACK (nStartPos:longint); far;
type
     PACK_Header = record
       nID:word;
       nID2:word;
       nFileDate:word;
       nFileTime:word;
       res1:array [9..16] of byte;
       nOldSize:longint;
       nNextOffset:longint;
       sIDStr:array[1..6] of char;
       res2:array[1..9] of byte;
       nFilenameLen:word;
       { ... filename #0 ... }
     end;
const
      PACK_H_SIZE = sizeof (PACK_Header);
var
    H:PACK_Header;
    sFilename:string;
    nOldPos, nNewSize:longint;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;

  repeat
    nOldPos := TFile_GetFilePos;
    if (TFile_ReadActBuf (H, PACK_H_SIZE) = PACK_H_SIZE) then
    begin
      sFilename := TFile_ReadActLongString (H.nFilenameLen);

      { If H.NextOffset is 0 it means that the archive is ended }
      if (H.nNextOffset = 0) then
        nNewSize := b.FSize - nOldPos
      else
      begin
        nNewSize := H.nNextOffset - nOldPos;
        TFile_GotoFilePos (H.nNextOffset);
      end;

      _SetFileInfo (H.nOldSize,
                    nNewSize,
                    0,
                    sFilename,
                    false);
    end;
  until (not TFile_IsOkay) or (H.nNextOffset = 0);

  TArchive_NoteFooter;
end;

{---------------------------PAQ---------------------------}

procedure List_PAQ (nStartPos:longint); far;
type
     PAQ_GlobalHeader = record
       nID1:longint;
       nID2:longint;
       res1:array[1..13] of byte;
     end;

     PAQ_LocalHeader = record
       res1:byte;
       nFilenameLength:byte;
       sFilename:array[1..12] of char;
       nOriginalSize:longint;
       nPackedSize:longint;
       res2:longint;
       res3:array[1..3] of byte;
     end;
const
      PAQ_GH_SIZE = SizeOf (PAQ_GlobalHeader);
      PAQ_LH_SIZE = SizeOf (PAQ_LocalHeader);
var
    LH:PAQ_LocalHeader;
begin
  TArchive_Init (nStartPos + PAQ_GH_SIZE + 3);
  TArchive_NoteHeader;

  repeat
    if (TFile_ReadActBuf (LH, PAQ_LH_SIZE) = PAQ_LH_SIZE) then
    begin
      _SetFileInfo (LH.nOriginalSize,
                    LH.nPackedSize,
                    0,
                    copy (LH.sFilename, 1, LH.nFilenameLength),
                    false);
      TFile_IncFilePos (LH.nPackedSize + 3);
    end;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------PAR---------------------------}

{ - not listed - }

{---------------------------PHP---------------------------}

procedure List_PHP (nStartPos:longint); far;
type
     PHP_Header = record
       nVersion:byte;
       sFilename:string[12];
       nSize:longint;
       nDT:longint;
       nAttr:word;
       res1:word;
     end;
const
      PHP_H_SIZE = SizeOf (PHP_Header);
var
    H:PHP_Header;
begin
  TArchive_Init (nStartPos + 2);
  NoteArchiveVersion (GetVersionStr (TFile_ReadByteAt (nStartPos + 2), 0));
  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (H, PHP_H_SIZE) = PHP_H_SIZE) then
    begin
      _SetFileInfo (H.nSize,
                    H.nSize,
                    H.nDT,
                    H.sFilename,
                    false);
      TFile_IncFilePos (H.nSize);
    end;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------PPZ---------------------------}

procedure List_PPZ (nStartPos:longint); far;
var nOldSize:longint;
begin
  TArchive_Init (nStartPos);
  NoteArchiveVersion (GetVersionStr (b.First.b[5], b.First.b[6]));
  TArchive_NoteHeader;

  nOldSize := b.First.l[3];
  SwapLongSelf (nOldSize);

  _SetFileInfo (nOldSize,
                b.FSize,
                0,
                'total',
                false);

  TArchive_NoteFooter;
end;

{---------------------------Q---------------------------}

procedure List_Q (nStartPos:longint); far;
var
    GH:Q_GlobalHeader;
    LH:Q_LocalHeader;
    i:byte;
    sFilename,
    sComment:string;

    procedure GetXString (var s:string);
    var b:byte;
        w:word;
    begin
      b := TFile_ReadActByte;
      if (b > 127) then
        w := TFile_ReadActByte shl 8 + b
      else
        w := b;
      s := TFile_ReadActLongString (w);
    end;

begin
  TArchive_Init (nStartPos);
  bHasNoPackedSize := true;

  TFile_ReadActBuf (GH, Q_GH_SIZE);
  NoteArchiveVersion (GetVersionStr (GH.nVersionMajor, GH.nVersionMinor));

  TArchive_NoteHeaderX;

  for i:=1 to GH.nFilesInArc do
  begin
    GetXString (sFilename);
    GetXString (sComment);

    if (TFile_ReadActBuf (LH, Q_LH_SIZE) = Q_LH_SIZE) then
      _SetFileInfo (LH.nOldSize,
                    0,
                    LH.nDT,
                    sFilename,
                    false);
  end;
  TArchive_NoteFooter;
end;

{---------------------------QFC---------------------------}

procedure List_QFC (nStartPos:longint); far;
type
     QFC_Header = record
       nID:byte;
       nSize:TripleLong;
       nDT:longint;
       nAttr:byte;
       nFilenameLen:byte;
     end;
const
      QFC_LH_SIZE = SizeOf (QFC_Header);
var
    LH:QFC_Header;
    sFilename:string;
    nSize:longint;
begin
  TArchive_Init (nStartPos + 3);  { - skip first byte - }
  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (LH, QFC_LH_SIZE) = QFC_LH_SIZE) then
    begin
      if (LH.nID <> $1A) then
        TFile_SetError (TFILE_ERR_ID)
      else
      begin
        { - fucking 3 byte size format - }
        nSize := LH.nSize.AsLong;
        sFileName := TFile_ReadActString (LH.nFilenameLen);
        _SetFileInfo (nSize,
                      nSize,
                      LH.nDT,
                      sFilename,
                      false);
        TFile_IncFilePos (nSize);
      end;
    end;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------RAR0--------------------------}

procedure List_RAR0 (nStartPos:longint); far;
type
     RAR_GlobalHeader = record
       nID:longint;
       nArchiveHeaderSize:word;
       nFlags:byte;
     end;

     RAR_LocalHeader = record
       nNewSize:longint;
       nOldSize:longint;
       nCRC16:word;
       nHeaderSize:word;
       nDT:longint;
       nAttr:byte;
       nFlags:byte;
       nVersion:byte;
       nFilenameLen:byte;
       nMethod:byte;
       { filename }
     end;
const
      RAR_GH_SIZE = SizeOf (RAR_GlobalHeader);
      RAR_LH_SIZE = SizeOf (RAR_LocalHeader);
var
    GH:RAR_GlobalHeader;
    LH:RAR_LocalHeader;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, RAR_GH_SIZE);
  TFile_GotoFilePos (nStartPos + GH.nArchiveHeaderSize);

  TArchive_NoteHeaderX;

{$ifdef GETTYP}
  if (GH.nFlags and $02 > 0) then
    Noteln ('[archive comment was found]');
{$endif}

  repeat
    if (TFile_ReadActBuf (LH, RAR_LH_SIZE) = RAR_LH_SIZE) then
    begin
      sFilename := TFile_ReadActString (LH.nFilenameLen);
{$ifdef GETTYP}
      if (LH.nFlags and $08 > 0) then
        sFilename := concat (sFilename, ' [comment]');
{$endif}

      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize,
                    LH.nDT,
                    sFilename,
                    LH.nFlags and $04 > 0);

      TFile_IncFilePos (LH.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------RAR---------------------------}

procedure List_RAR (nStartPos:longint); far;
const
      MARKER      = $72;
      ARCHIV      = $73;
      FILEH       = $74;
      COMMENT     = $75;
      EXTRA       = $76;
      SUB         = $77;
      RECOVERY    = $78;

      FLAG_ArcComment  = $0002;
      FLAG_Encrypted   = $0004;
      FLAG_FileComment = $0008;
      FLAG_HasAddSize  = $8000;

type
     RAR_BasicHeader = record
       nHeadCRC:word;
       nHeadType:byte;
       nHeadFlags:word;
       nHeadSize:word;
     end;

     RAR_ArchiveHeader = record
       res1:word;
       res2:longint;
     end;

     RAR_FileHeader = record
       nNewSize:longint;
       nOldSize:longint;
       nHostOS:byte;
       nFileCRC:longint;
       nDT:longint;
       nExtractVersion:byte;
       nMethod:byte;
       nFileNameLen:word;
       nFileAttr:longint;
       {
       filename
       comment
       }
     end;

     RAR_CommentHeader = record
       nOldSize:word;
       nExtractVersion:byte;
       nMethod:byte;
       nCommentCRC:word;
       {
       comment
       }
     end;

     RAR_ExtraHeader = record
       {
       other data
       }
     end;

     RAR_SubHeader = record
       nDataSize:longint;
       nSubType:word;
       res:byte; { $00 }
       {
       other fields
       }
     end;

     RAR_OS2SubHeader = record
       nDataSize:longint;
       nSubType:word; { $0100 }
       res:byte;     { $00 }
       nOldSize:longint;
       nExtractVersion:byte;
       nMethod:byte;
       nEACRC:longint;
     end;

const
      RAR_BH_SIZE = SizeOf (RAR_BasicHeader);
      RAR_AH_SIZE = SizeOf (RAR_ArchiveHeader);
      RAR_FH_Size = SizeOf (RAR_FileHeader);
      RAR_CH_SIZE = SizeOf (RAR_CommentHeader);
      RAR_EH_SIZE = SizeOf (RAR_ExtraHeader);
      RAR_SH_Size = SizeOf (RAR_SubHeader);
      RAR_OH_SIZE = SizeOf (RAR_OS2SubHeader);
var { HasComment:boolean; }
    nCommentSize:word;
    BH,
    BH1:RAR_BasicHeader;
    AH:RAR_ArchiveHeader;

    procedure GetHeader (var b:RAR_BasicHeader);
    var nTemp:word;
        nAddSize:longint;
    begin
      TFile_ReadActBuf (b, RAR_BH_SIZE);

      nTemp := b.nHeadFlags and FLAG_HasAddSize;
      if (b.nHeadType <> FILEH) and (nTemp > 0) then
      begin
        TFile_ReadActBuf (nAddSize, 4);
        inc (b.nHeadSize, nAddSize);
      end;
    end;

    procedure DoFileHeader;
    var FH:RAR_FileHeader;
    begin
      TFile_ReadActBuf (FH, RAR_FH_SIZE);

      _SetFileInfo (FH.nOldSize,
                    FH.nNewSize,
                    FH.nDT,
                    TFile_ReadActLongString (FH.nFilenameLen),
                    BH1.nHeadFlags and FLAG_Encrypted > 0);

      TFile_IncFilePos (FH.nNewSize);

      if (BH1.nHeadFlags and FLAG_FileComment > 0) then
      begin
        nCommentSize := BH1.nHeadSize - (RAR_BH_SIZE + RAR_FH_SIZE) - FH.nFilenameLen;
{$ifdef GETTYP}
        Noteln ('[file comment]');
{$endif}
        TFile_IncFilePos (nCommentSize);
      end;
    end;

    procedure DoSimpleSkip;
    begin
      TFile_IncFilePos (BH1.nHeadSize - RAR_BH_SIZE);
    end;

    procedure DoArchiveHeader;
    begin
      TFile_ReadActBuf (AH, RAR_AH_SIZE);

      nCommentSize := BH1.nHeadSize - (RAR_BH_SIZE + RAR_AH_SIZE);
      if (nCommentSize > 0) then
      begin
{$ifdef GETTYP}
        Noteln ('[archive comment]');
{$endif}
        TFile_IncFilePos (nCommentSize);
      end;
    end;

label quit;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

  GetHeader (BH);

  while TFile_IsOkay do
  begin
    GetHeader (BH1);

    case BH1.nHeadType of
      0:          goto quit;
      ARCHIV:     DoArchiveHeader;
      FILEH:      DoFileHeader;
      EXTRA:      begin
{$ifdef GETTYP}
                    Noteln ('[Extra information]');
{$endif}
                    DoSimpleSkip;
                  end;
      RECOVERY:   begin
{$ifdef GETTYP}
                    Noteln ('[Recovery record]');
{$endif}
                    DoSimpleSkip;
                  end;
      COMMENT,
      SUB:        DoSimpleSkip;
    else
{$ifdef GETTYP}
      Noteln (concat ('Unhandled header type #', hexbyte (BH1.nHeadType), 'h'));
{$endif}
      goto quit;
    end;
  end;

quit:
  TArchive_NoteFooter;
end;

{---------------------------RAX---------------------------}

procedure List_RAX (nStartPos:longint); far;
type
     RAX_Header = record
       nFilenameLen:byte;
       res1:byte;
       nDT:longint;
       nOldSize:longint;
       nNewSize:longint;
       res2:longint;
     end;

     RAX_GlobalHeader = record
       nID:longint;
       res1:byte;
       nFlags:byte;
       res2:longint;
       res3:longint;
     end;
const
      RAX_H_SIZE  = SizeOf (RAX_Header);
      RAX_GH_SIZE = SizeOf (RAX_GlobalHeader);
var
    H:RAX_Header;
    GH:RAX_GlobalHeader;
    sFilename:string;
    bPassword:boolean;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

  TFile_ReadActBuf (GH, RAX_GH_SIZE);
  bPassword := (GH.nFlags and 1 > 0);

  repeat
    if (TFile_ReadActBuf (H, RAX_H_SIZE) = RAX_H_SIZE) then
    begin
      sFilename := TFile_ReadActString (H.nFilenameLen);
      dec (H.nNewSize, RAX_H_SIZE + H.nFilenameLen);
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    H.nDT,
                    sFilename,
                    bPassword);
      TFile_IncFilePos (H.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------RKV---------------------------}

{ - not listed  - }

{---------------------------RNC---------------------------}

procedure List_RNC (nStartPos:longint); far;
type
     RNC_GlobalHeader = record
       nID:longint;
       nDataStart:word;
       res1:byte;
       res2:word;
       nNextDirOfs:word;
     end;

     RNC_Header = record
       ID:array[1..3] of char;
       nMethod:byte;
       nOldSize:longint;
       nNewSize:longint;
     end;
const
      RNC_GH_SIZE = SizeOf (RNC_GlobalHeader);
      RNC_LH_SIZE = SizeOf (RNC_Header);
var
    GH:RNC_GlobalHeader;
    H:RNC_Header;
    sFilename:string;
    nNewSize, nSavedPos, nOffset:longint;
    sDir:string;
begin
  TArchive_Init (nStartPos);  { +11 ?? }
  TArchive_NoteHeader;

  TFile_ReadActBuf (GH, RNC_GH_SIZE);
  sDir := '';

  { - the archive contains a list with all files and there offsets - }

  repeat
    { - read the filenbame - }
    sFilename := TFile_Read_To (#0);

    { check if a valid string was found }
    if (sFilename = '') then
    begin
      TFile_ReadActWord;
      sDir := TFile_Read_To (#0);
    end
    else
    begin
      { - read the offset in the arc - }
      nOffset := TFile_ReadActLong;
      SwapLongSelf (nOffset);

      { - goto the position in the archive - }
      nSavedPos := TFile_GetFilePos;
      TFile_GotoFilePos (nOffset);

      { - read local header - }
      TFile_ReadActBuf (H, RNC_LH_SIZE);
      if (H.ID = 'RNC') then
      begin
        SwapLongSelf (H.nOldSize);
        if (H.nMethod = 0) then  { - stored - }
          SwapLong (H.nOldSize, nNewSize)
        else
          SwapLong (H.nNewSize, nNewSize);

        _SetFileInfo (H.nOldSize,
                      nNewSize,
                      0,
                      concat (sDir, sFilename),
                      false);

        TFile_GotoFilePos (nSavedPos);
      end
      else
        TFile_SetError (TFILE_ERR_ID);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------SAR---------------------------}

procedure List_SAR (nStartPos:longint); far;
type
     SAR_LocalHeader = record
       aMethod:array[1..4] of char;
       nAttr:byte;
       nNewSize:longint;
       nOldSize:longint;
       res1:longint;
       res2:word;
       nFilenameLen:byte;
     end;
const
      SAR_LH_SIZE = SizeOf (SAR_LocalHeader);
var
    LH:SAR_LocalHeader;
    sFilename:string;
begin
  TArchive_Init (nStartPos + 2);
  TArchive_NoteHeader;

  repeat
    if (TFile_ReadActBuf (LH, SAR_LH_SIZE) = SAR_LH_SIZE) then
    begin
      sFilename := TFile_ReadActString (LH.nFilenameLen);
      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize,
                    0,
                    sFilename,
                    false);
      TFile_IncFilePos (7 + LH.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_Notefooter;
end;


{---------------------------SBX---------------------------}

procedure List_SBX (nStartPos:longint); far;
type
     SBX_LocalHeader = record
       nID:longint;
       nNewSize:longint;
       nDT:longint;
       nAttr:byte;
       nFilenameLen:byte;
       {
         filename
         oldsize
       }
     end;
const
      SBX_LH_SIZE = SizeOf (SBX_LocalHeader);
var
    LH:SBX_LocalHeader;
    sFilename:string;
    nOldFilePos:longint;
    nOldsize:longint;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;

  repeat
    nOldFilePos := TFile_GetFilePos;
    if (TFile_ReadActBuf (LH, SBX_LH_SIZE) = SBX_LH_SIZE) then
    begin
      sFilename := TFile_ReadActString (LH.nFilenameLen);
      nOldSize := TFile_ReadActLong;
      _SetFileInfo (nOldSize,
                    LH.nNewSize,
                    LH.nDT,
                    sFilename,
                    false);
      TFile_GotoFilePos (nOldFilePos + LH.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_Notefooter;
end;


{---------------------------SKY---------------------------}

procedure List_SKY (nStartPos:longint); far;
type
     SKY_GlobalHeader = record
       nID:longint;
       res1:longint;
       res2:longint;
     end;

     SKY_LocalHeader = record
       res1:word;
       nNewSize:longint;
       nOldsize:longint;
       nDT:longint;
       nAttr:byte;
       res2:word;
       nCRC32:longint;
       sName:array[1..8] of char;
       sExt:array[1..3] of char;
       nDirNameLen:byte;
       res3:word;
     end;
const
      SKY_GH_SIZE = SizeOf (SKY_GlobalHeader);
      SKY_LH_SIZE = SizeOf (SKY_LocalHeader);
var
    GH:SKY_GlobalHeader;
    LH:SKY_LocalHeader;
    sDirectory, sFilename:string;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, SKY_GH_SIZE);

  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (LH, SKY_LH_SIZE) = SKY_LH_SIZE) then
    begin
      sFilename := concat (CopyUntil (LH.sName, #0), '.', CopyUntil (LH.sExt, #0));
      sDirectory := TFile_ReadActString (LH.nDirNameLen);

      dec (LH.nNewSize, SKY_LH_SIZE + LH.nDirNameLen);

      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize,
                    LH.nDT,
                    concat (sDirectory, sFilename),
                    false);
      TFile_IncFilePos (LH.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_Notefooter;
end;

{---------------------------SOF---------------------------}

procedure List_SOF (nStartPos:longint); far;
type
     SOF_Header = record
       nExtract_Version:word;
       nFlags:word;
       nCompression_Method:word;
       nDT:longint;
       nCRC32:longint;
       nNewSize:longint;
       nOldSize:longint;
       nFilenameLen:word;
       nExtraLen:word;
     end;

     {
     only if LFH.Flags and bit[3] > 0
     for STDIN data
     }
     SOF_DataDescriptor = record
       nCRC32:longint;
       nNewSize:longint;
       nOldSize:longint;
       nAttr:longint;
     end;

const
      SOF_H_SIZE = SizeOf (SOF_Header);
      SOF_DD_SIZE = SizeOf (SOF_DataDescriptor);
      SOF_DD     = $08;
var
    H:SOF_Header;
    nID:longint;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

  repeat
    nID := TFile_ReadActLong;
    if (nID = $06034B50) then
    begin
      if (TFile_ReadActBuf (H, SOF_H_SIZE) = SOF_H_SIZE) then
      begin
        sFilename := TFile_ReadActString (H.nFilenameLen);
        _SetFileInfo (H.nOldSize,
                      H.nNewSize,
                      H.nDT,
                      sFilename,
                      false);
        TFile_IncFilePos (H.nExtraLen + H.nNewSize);
        if (H.nFlags and SOF_DD > 0) then
          TFile_IncFilePos (SOF_DD_SIZE);
      end;
    end
    else
      if (nID = $04014B50) then
      begin
        TFile_Finish;
      end;
  until (not TFile_IsOkay);

  TArchive_Notefooter;
end;


{--------------------------SQWEZ--------------------------}

procedure List_SQWEZ (nStartPos:longint); far;
type
     SQWEZ_GlobalHeader = record
       sID:array[1..10] of char;
       res:array[1..8] of byte;
     end;

     SQWEZ_LocalHeader = record
       nNewSize:longint;
       nCRC16:word;
       nDT:longint;
       nAttr:byte;
       res1:array[1..3] of byte;
       sFilename:array[1..13] of char;
       nOldSize:longint;
     end;

const
      SQWEZ_GH_SIZE = SizeOf (SQWEZ_GlobalHeader);
      SQWEZ_LH_SIZE = SizeOf (SQWEZ_LocalHeader);
var
    GH:SQWEZ_GlobalHeader;
    LH:SQWEZ_LocalHeader;
    nOldFilePos:longint;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, SQWEZ_GH_SIZE);

  TArchive_NoteHeaderX;

  repeat
    nOldFilePos := TFile_GetFilePos;
    if (TFile_ReadActBuf (LH, SQWEZ_LH_SIZE) = SQWEZ_LH_SIZE) then
    begin
      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize - SQWEZ_LH_SIZE,
                    LH.nDT,
                    CopyUntil (LH.sFilename, #0),
                    false);
      TFile_GotoFilePos (nOldFilePos + LH.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_Notefooter;
end;

{---------------------------SQZ---------------------------}

procedure List_SQZ (nStartPos:longint); far;
type
     SQZ_Header = record
       nHeaderCheckSum:byte;
       nMethod:byte;
       nNewSize:longint;
       nOldSize:longint;
       nDT:longint;
       nAttr:byte;
       nCRC32:longint;
       {
       HeaderSize - SizeOf (SQZHeader): Filename
       }
     end;
const
      SQZ_H_SIZE = SizeOf (SQZ_Header);
var
    bNormalFormat:boolean;
    H:SQZ_Header;
    nHeaderSize:byte;
    nNewSize, nOldSize:longint;
    nTmp:word;
begin
  TArchive_Init (nStartPos + 8);
  TArchive_NoteHeaderX;

  bNormalFormat := ((TFile_ReadByteAt (nStartPos + 7) and BITS [0]) <> 0);

  repeat
    nHeaderSize := TFile_ReadActByte;

    if (nHeaderSize >= 19) then
    begin
      if (TFile_ReadActBuf (H, SQZ_H_SIZE) = SQZ_H_SIZE) then
      begin
        if bNormalFormat then
        begin
          nOldSize := H.nOldSize;
          nNewSize := H.nNewSize;
        end
        else
        begin
          SwapLong (H.nOldSize, nOldSize);
          SwapLong (H.nNewSize, nNewSize);
        end;

        _SetFileInfo (nOldSize,
                      nNewSize,
                      H.nDT,
                      TFile_ReadActLongString (nHeaderSize - 18),
                      false);
        TFile_IncFilePos (nNewSize)
      end;
    end
    else
      if (nHeaderSize = 0) then
        TFile_Finish
      else
      begin
{$ifdef GETTYP}
        if (nHeaderSize = 1) or (nHeaderSize = 7) then
          Noteln (Enbraced ('Comment'));
{$endif}
        nTmp := TFile_ReadActWord;
        if (not bNormalFormat) then
          nTmp := swap (nTmp);
        TFile_IncFilePos (nTmp);
      end;
      {
        1:COMMENT (SQZ <= 1.08.2)
        2:PASSWORD
        3:POSTFIX
        4:SECURITY ENVELOPE
        5:VOLUME LABEL
        6:DIRECTORY
        7:COMMENT (SQZ >= 1.08.3)
      }
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------STI---------------------------}

procedure List_STI (nStartPos:longint); far;
type
     STI_GlobalHeader = record
       nID:longint;
       res1:array[5..12] of byte;
       nFilesInArchive:word;
       res2:longint; { date/time stamp?? }
       nArchiveSize:longint;
       nExtractedSize:longint;
       res3:array[1..15] of byte; { only 00 and FF }
       nDataOffsetWithDirs:longint;
       res4:longint;
       res5:word;
       nDataOffset:longint;
       res6:byte;
     end;

     STI_DirHeader = record
       res1:byte;
       ID:byte;
       nHeaderSize:word;
       nFileNameLen:word;
       {
       1 + 4 bytes
       }
     end;

     STI_FileHeader = record
       nDiskID:byte;
       nDirID:byte;
       nID:byte;                    { $00 }
       nOldSize:longint;
       nNewSize:longint;
       res2:array[1..12] of byte;
       nHeaderSize:word;
       res3:array[1..4] of byte;
       nFilenameLen:byte;
       {
       1 + 12 bytes;
       }
     end;
const
      STI_GH_SIZE = SizeOf (STI_GlobalHeader);
      STI_DH_SIZE = SizeOf (STI_DirHeader);
      STI_FH_SIZE = SizeOf (STI_FileHeader);
      MAX_DIR = 255;
type
     PDirArray = ^TDirArray;
     TDirArray = array[0..MAX_DIR] of string[80];

var
    pArray:PDirArray;
    nOldFilePos:longint;
    GH:STI_GlobalHeader;
    DH:STI_DirHeader;
    FH:STI_FileHeader;
    nFilesFound:word;
    nDirIndex:integer;
{$ifdef GETTYP}
    nDiskNr:byte;
{$endif}
    sDirectory, sFilename:string;
begin
  nDirIndex := -1;
{$ifdef GETTYP}
  nDiskNr := 0;
{$endif}

  TArchive_Init (nStartPos);

  TFile_ReadActBuf (GH, STI_GH_SIZE);

  TFile_GotoFilePos (nStartPos + GH.nDataOffsetWithDirs);

  if (not TFile_IsOkay) then
  begin
{$ifdef GETTYP}
    InternalError ('List_STI');
{$endif}
    exit;
  end;

  new (pArray);
  CheckMemoryAllocation (pArray, SizeOf (TDirArray), 'List_STI');
  FillChar (pArray^, SizeOf (TDirArray), 0);

  TArchive_NoteHeader;
  nFilesFound := 0;

  repeat
    nOldFilePos := TFile_GetFilePos;

    if (TFile_ReadByteAt (nOldFilePos + 2) = $00) then
    begin
      { Note: Act File Pos has changed ... }
      if (TFile_ReadBufAt (nOldFilePos, FH, STI_FH_SIZE) = STI_FH_SIZE) then
      begin
        inc (nFilesFound);

{$ifdef GETTYP}
        if (nDiskNr <> FH.nDiskID) then
        begin
          nDiskNR := FH.nDiskID;
          Noteln (Enbraced (concat ('Disk ', i2s (nDiskNR))));
        end;
{$endif}

        sDirectory := '';
        if (nDirIndex >= FH.nDirID) and (pArray^ [FH.nDirID] > '') then
          sDirectory := concat (pArray^ [FH.nDirID], '\');
        sFilename := TFile_ReadActString (FH.nFilenameLen);

        _SetFileInfo (FH.nOldSize,
                      FH.nNewSize,
                      0,
                      concat (sDirectory, sFilename),
                      false);

        TFile_GotoFilePos (nOldFilePos + FH.nHeaderSize)
      end;
    end
    else  { directory handling }
    begin
      { Note: Act File Pos has changed ... }
      if (TFile_ReadBufAt (nOldFilePos, DH, STI_DH_SIZE) = STI_DH_SIZE) then
      begin
        inc (nDirIndex);
        pArray^ [nDirIndex] := TFile_ReadActLongString (DH.nFileNameLen);

        TFile_GotoFilePos (nOldFilePos + DH.nHeaderSize);

        if (nDirIndex = MAX_DIR) then
        begin
{$ifdef GETTYP}
          Noteln ('Maximum number of directories exceeded.');
          Noteln ('Listing aborted.');
{$endif}
          TFile_Finish;
        end;
      end;
    end;

    { - check manually for end - }
    if (((TFile_GetFilePos - nStartPos) >= GH.nArchiveSize) or
        (nFilesFound >= GH.nFilesInArchive)) and
       (not TFile_IsError) then
      TFile_Finish;
  until (not TFile_IsOkay);

  dispose (pArray);

  TArchive_NoteFooter;

  {
  if (GH.nArchiveSize < b.FSize) then
    WriteOverlayInfo (    aProc:TEXEProc;
                      GH.nArchiveSize,
                      b.FSize - GH.nArchiveSize,
                      dummy);
  }
end;

{---------------------------SWG---------------------------}

procedure List_SWG (nStartPos:longint); far;
type
     SWG_Header = record
       nHeadSize:byte;                  {size of header}
       nHeadChk:byte;                   {checksum for header}
       sHeadID:array [1..5] of char;    {compression type tag}
       nNewSize:longint;                {compressed size}
       nOldSize:longint;                {original size}
       nTime:word;                      {packed time}
       nDate:word;                      {packed date}
       nAttr:word;                      {file attributes and flags}
       nBufCRC:longint;                 {32-CRC of the Buffer }
       sSwag:string[12];                {stored SWAG filename}
       sSubject:string[40];             {snipet subject}
       sContrib:string[35];             {contributor}
       sKeys:string[70];                {search keys, comma deliminated}
       (*
       FName:PathStr;                  {filename (variable length)}
       CRC:word;                       {16-bit CRC (immediately follows FName)}
       *)
     end;
const
      SWG_H_SIZE = SizeOf (SWG_Header);
var
    H:SWG_Header;
    nOldFilePos:longint;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;

  repeat
    nOldFilePos := TFile_GetFilePos;
    TFile_ReadActBuf (H, SWG_H_SIZE);
    if (H.sHeadID = 'SWAG ') then
      TFile_Finish
    else
    begin
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    0,
                    H.sSubject,
                    false);
      TFile_GotoFilePos (nOldFilePos + 2 + H.nHeadSize + H.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------SYN---------------------------}

procedure List_SYN (nStartPos:longint); far;
type
     SYN_GlobalHeader = record
       nID:longint;
       nNumberOfFiles:longint;
       res:array[1..8] of byte;
     end;

     SYN_LocalHeader = record
       sFilename:array[1..24] of char;
       nOffset:longint;
       nSize:longint;
     end;

const
      SYN_GH_SIZE = SizeOf (SYN_GlobalHeader);
      SYN_LH_SIZE = SizeOf (SYN_LocalHeader);

var
    GH:SYN_GlobalHeader;
    LH:SYN_LocalHeader;
    i:longint;
    {
    nStartOffset:longint;
    }
label ExitLoop;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;

  TFile_ReadActBuf (GH, SYN_GH_SIZE);
  {
  nStartOffset := SYN_GH_SIZE + (GH.nNumberOfFiles * SYN_LH_SIZE);
  }

  for i:=1 to GH.nNumberOfFiles do
  begin
    if (TFile_ReadActBuf (LH, SYN_LH_SIZE) <> SYN_LH_SIZE) then
      goto ExitLoop
    else
      _SetFileInfo (LH.nSize,
                    LH.nSize,
                    0,
                    CopyUntil (LH.sFilename, #0),
                    false);
  end;
ExitLoop:

  TArchive_NoteFooter;
end;

{---------------------------SZIP--------------------------}

{ - not listed, single file archive - }

{---------------------------TAR---------------------------}

procedure List_TAR (nStartPos:longint); far;
type
     TAR_Header = record
       Name    :array[1..100] of char;
       Mode    :array[1..  8] of char;
       UID     :array[1..  8] of char;
       GID     :array[1..  8] of char;
       Size    :array[1.. 12] of char;
       MTime   :array[1.. 12] of char;
       ChkSum  :array[1..  8] of char;
       LinkFlag:char;
       LinkName:array[1..100] of char;
       Magic   :array[1..  8] of char;
       UName   :array[1.. 32] of char;
       GName   :array[1.. 32] of char;
       DevMajor:array[1..  8] of char;
       DevMinor:array[1..  8] of char;
       Rest    :array[346..512] of byte;
     end;
const
      TAR_H_SIZE = SizeOf (TAR_Header);
var
    H:TAR_Header;
    nSize:longint;
    bDone:boolean;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;

  repeat
    bDone := true;
    if (TFile_ReadActBuf (H, TAR_H_SIZE) = TAR_H_SIZE) then
    begin
      bDone := (copy (H.magic, 1, 5) <> 'ustar');
      if (not bDone) then
      begin
        nSize := StrOct2Dec (ReplaceChar (copy (H.Size, 1, 11), ' ', '0'));

        sFilename := CopyUntil (H.Name, #0);
        if ((LastChar (sFilename) = '\') or (LastChar (sFilename) = '/')) and (nSize = 0) then
          sFilename := concat (sFilename, ' [directory]');

        _SetFileInfo (nSize,
                      nSize,
                      0,
                      sFilename,
                      false);

        if (nSize mod 512 <> 0) then
          nSize := ((nSize div 512) + 1) * 512;
        TFile_IncFilePos (nSize);
      end;
    end;
  until (not TFile_IsOkay) or (bDone);
  TArchive_NoteFooter;
end;

{---------------------------UC2---------------------------}

{ - not listed - }

{---------------------------UFA---------------------------}

procedure List_UFA (nStartPos:longint); far;
type
     UFA_GlobalHeader = record
       nID:array[1..3] of char;
       res1:array[4..6] of byte;
       nMethod:byte;
       res2:byte;
     end;

     UFA_SolidGlobalHeader = record
       res1:array[1..13] of byte;
       nDirectoryOffset:longint;
       res2:array[18..19] of byte;
     end;

     UFA_SolidLocalHeader = record
       res1:array[1..28] of byte;
       nCRC32:longint;
       nMethod:byte;
       nOldSize:longint;
       nFilenameLen:word;
       { - filename - }
     end;

     UFA_000_Header = record
       Res0:longint;
       nPassword:byte;
       Res1:byte;
       Res2:word;
       Res3:longint;
       Res4:longint;
       Res5:longint;
       nNewSize:longint;
       nOldSize:longint;
       nAttr:byte;
       nFilenameLen:word;
     end;

     UFA_001_Header = record
       Res0:longint;
       nPassword:byte;
       Res1:byte;
       Res2:word;
       Res3:array[1..14] of byte;
       nNewSize:longint;
       nOldSize:longint;
       nAttr:byte;
       nFilenameLen:word;
     end;

     UFA_004_Header = record
       res0:longint;
       nPassword:byte;
       res1:byte;
       res2:word;
       res3:array[9..46] of byte;
       nNewSize:longint;
       nOldSize:longint;
       nFilenameLen:word;
     end;

     UFA_LocalHeader = record
       case byte of
         0:(U0:UFA_000_Header);
         1:(U1:UFA_001_Header);
         4:(U4:UFA_004_Header);
     end;

const
      UFA_GH_SIZE = SizeOf (UFA_GlobalHeader); { 8 }
      UFA_SGH_SIZE = SizeOf (UFA_SolidGlobalHeader); { 19 }
      UFA_SLH_SIZE = SizeOf (UFA_SolidLocalHeader); { 39 + filenamelen }
      UFA_000_H_SIZE = SizeOf (UFA_000_Header);    { 31 }
      UFA_001_H_SIZE = SizeOf (UFA_001_Header);    { 31 }
      UFA_004_H_SIZE = SizeOf (UFA_004_Header);  { 56 }
var
    GH:UFA_GlobalHeader;
    sFilename:string;

    procedure ListSolid;
    var
        SGH:UFA_SolidGlobalHeader;
        SLH:UFA_SolidLocalHeader;
    begin
      NoteArchiveVersion ('solid');
      TArchive_NoteHeader;

      bHasNoPackedSize := true;
      if (TFile_ReadActBuf (SGH, UFA_SGH_SIZE) = UFA_SGH_SIZE) then
      begin
        TFile_IncFilePos (SGH.nDirectoryOffset);
        repeat
          if (TFile_ReadActBuf (SLH, UFA_SLH_SIZE) = UFA_SLH_SIZE) then
          begin
            sFilename := TFile_ReadActLongString (SLH.nFilenameLen);
            _SetFileInfo (SLH.nOldSize,
                          0,
                          0,
                          sFilename,
                          false);
          end;
        until (not TFile_IsOkay);
      end;
    end;

    procedure ListNormal0;
    var
        H:UFA_000_HEADER;
    begin
      NoteArchiveVersion ('0.00');
      TArchive_NoteHeader;

      repeat
        if (TFile_ReadActBuf (H, UFA_000_H_SIZE) = UFA_000_H_SIZE) then
        begin
          sFilename := TFile_ReadActString (H.nFilenameLen);
          _SetFileInfo (H.nOldSize,
                        H.nNewSize,
                        0,
                        sFilename,
                        H.nPassword and $01 > 0);
          TFile_IncFilePos (H.nNewSize);
        end;
      until (not TFile_IsOkay);
    end;

    procedure ListNormal12;
    var
        H:UFA_001_HEADER;
    begin
      NoteArchiveVersion ('0.01/0.02');
      TArchive_NoteHeader;

      repeat
        if (TFile_ReadActBuf (H, UFA_001_H_SIZE) = UFA_001_H_SIZE) then
        begin
          sFilename := TFile_ReadActString (H.nFilenameLen);
          _SetFileInfo (H.nOldSize,
                        H.nNewSize,
                        0,
                        sFilename,
                        H.nPassword and $01 > 0);
          TFile_IncFilePos (H.nNewSize);
        end;
      until (not TFile_IsOkay);
    end;

    procedure ListNormal34;
    var
        H:UFA_004_HEADER;
    begin
      NoteArchiveVersion ('0.03/0.04');
      TArchive_NoteHeader;

      repeat
        if (TFile_ReadActBuf (H, UFA_004_H_SIZE) = UFA_004_H_SIZE) then
        begin
          sFilename := TFile_ReadActString (H.nFilenameLen);
          _SetFileInfo (H.nOldSize,
                        H.nNewSize,
                        0,
                        sFilename,
                        H.nPassword and $01 > 0);
          TFile_IncFilePos (H.nNewSize);
        end;
      until (not TFile_IsOkay);
    end;

begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, UFA_GH_SIZE);

  if (GH.nMethod = 0) then
  begin
    if (b.First.b [20] = $01) or (b.First.b [20] = $02) then
      ListNormal12
    else
      if (b.First.b [20] = $03) or (b.First.b [20] = $04) then
        ListNormal34
      else
        ListNormal0;
  end
  else
    ListSolid;

  TArchive_NoteFooter;
end;

{---------------------------XIE---------------------------}

procedure List_XIE (nStartPos:longint); far;
var
    H:XIE_Header;
begin
  TArchive_Init (nStartPos + 4);
  TArchive_NoteHeader;

  repeat
    if (TFile_ReadActBuf (H, XIE_H_SIZE) = XIE_H_SIZE) then
    begin
      if (H.sFilename > '') then
      begin
        _SetFileInfo (H.nOldSize,
                      H.nNewSize,
                      0,
                      H.sFilename,
                      false);
        TFile_IncFilePos (h.nNewSize);
      end;
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------XL---------------------------}

procedure List_XL (nStartPos:longint); far;
type
     NameArray = array[1..12] of char;

     XLINK_GlobalHeader = record
       nID:longint;
       res1:longint;
       nFiles:word;
       nStart:word;
       nDataOfs:longint;
     end;

     XLINK_LocalHeader = record
       sFilename:NameArray;
       res1:longint;

       nFilesize:longint;
       res2:longint;
       res3:longint;
       res4:longint;
     end;

const
      XL_GH_SIZE = SizeOf (XLINK_GlobalHeader);
      XL_LH_SIZE = SizeOf (XLINK_LocalHeader);
var
    GH:XLINK_GlobalHeader;
    LH:XLINK_LocalHeader;

    procedure DecryptLH (nStart:byte);
    var a:array [0..XL_LH_SIZE - 1] of byte absolute LH;
        i:byte;
    begin
      for i:=0 to XL_LH_SIZE - 1 do
        a [i] := byte (a [i] - (nStart + i));
    end;

var i:byte;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, XL_GH_SIZE);

  TFile_GotoFilePos (nStartPos + GH.nDataOfs);
  TArchive_NoteHeader;

  for i := 1 to GH.nFiles do
  begin
    TFile_ReadActBuf (LH, XL_LH_SIZE);
    DecryptLH ((i - 1) * XL_LH_SIZE);
    _SetFileInfo (LH.nFilesize,
                  LH.nFilesize,
                  0,
                  LH.sFilename,
                  false);
  end;

  TArchive_NoteFooter;
end;

{----------------------------XPA----------------------------}

procedure List_XPA (nStartPos:longint); far;
type
     XPA_GlobalHeader = record
       nID:longint;
       nVersion:byte;
     end;

     XPA_LocalHeader = record
       {
       nFilenameLen:byte;
       .. filename ..
       }
       nOldSize:longint;
       nNewSize:longint;
     end;
const
      XPA_GH_SIZE = SizeOf (XPA_GlobalHeader);
      XPA_LH_SIZE = SizeOf (XPA_LocalHeader);
var
    LH:XPA_LocalHeader;
    sFilename:string;
begin
  TArchive_Init (nStartPos + XPA_GH_SIZE);
  TArchive_NoteHeader;

  repeat
    sFilename := TFile_ReadActPascalString;
    if (TFile_ReadActBuf (LH, XPA_LH_SIZE) = XPA_LH_SIZE) then
    begin
      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize,
                    0,
                    sFilename,
                    false);
      TFile_IncFilePos (LH.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_Notefooter;
end;

{----------------------------YC----------------------------}

procedure List_YC (nStartPos:longint); far;
type
     YC_Header = record
       Res1:longint;
       nSectionSize:longint;
       Res2:longint;
       nID:longint;
       Res3:longint;
       nFilesInSection:word;
       Res4:word;
       nFilesInArchive:word;
       Res5:array[1..29] of byte;
     end;

     YC_FileHeader = record
       nFilenameLen:word;
       nDT:longint;
       nAttr:word;
       nOldSize:longint;
       nCRC16:word;
       nRes1:longint;
       { filename }
     end;
const
      YC_H_SIZE = SizeOf (YC_Header);
      YC_FH_SIZE = SizeOf (YC_FileHeader);
var
    H:YC_Header;
    FH:YC_FileHeader;
    nOldFilePos:longint;
    i:word;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

  bHasNoPackedSize := true;
  nOldFilePos := nStartPos;

  repeat
    if (TFile_ReadActBuf (H, YC_H_SIZE) = YC_H_SIZE) then
    begin
      if (H.nID <> $43590100) then
        TFile_SetError (TFILE_ERR_ID)
      else
      begin
        for i:=1 to H.nFilesInSection do
        begin
          if (TFile_ReadActBuf (FH, YC_FH_SIZE) = YC_FH_SIZE) then
          begin
            sFilename := TFile_ReadActLongString (FH.nFilenameLen);
            _SetFileInfo (FH.nOldSize,
                          0,
                          FH.nDT,
                          sFilename,
                          false);
            TFile_IncFilePos (1);
          end;
        end;

        inc (nOldFilePos, H.nSectionSize);
        TFile_GotoFilePos (nOldFilePos);
      end;
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{----------------------------YZ----------------------------}

{ - not listed - }

{---------------------------ZET----------------------------}

procedure List_ZET (nStartPos:longint); far;
type
     ZET_Header = record
       nID:word;
       nSubType:byte;
       nHeaderSize:byte; { incl. ID }
     end;

     ZET_LocalHeader = record
       res1:word;
       nNewSize:longint;
       res2:longint;
       res3:longint;
       res4:longint;
       nAttr1:byte;
       res5:byte;
       nOldSize:longint;
       { .. #0 }
     end;
const
      ZET_H_SIZE = SizeOf (ZET_Header);
      ZET_LH_SIZE = SizeOf (ZET_LocalHeader);
var
    H:ZET_Header;
    LH:ZET_LocalHeader;
    sCurrentDir, sFilename:string;
    nDummy:word;
    nOldFilePos:longint;
begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeader;
  sCurrentDir := '';

  repeat
    nOldFilePos := TFile_GetFilePos;
    if (TFile_ReadActBuf (H, ZET_H_SIZE) = ZET_H_SIZE) then
    begin
      if (H.nID <> $5A4F) then
        TFile_SetError (TFILE_ERR_ID)
      else
      begin
        case (H.nSubType) of
          $DE:TFile_Finish;
          $E0:begin
                { directory name }
                TFile_IncFilePos (6);
                sCurrentDir := TFile_Read_To (#0);
                TFile_GotoFilePos (nOldFilePos + H.nHeaderSize);
              end;
          $E1:begin
                { data part }
                TFile_IncFilePos (4);
                nDummy := TFile_ReadActWord;
                TFile_GotoFilePos (nOldFilePos + H.nHeaderSize + nDummy);
              end;
          $E2:begin
                { file info }
                if (TFile_ReadActBuf (LH, ZET_LH_SIZE) = ZET_LH_SIZE) then
                begin
                  sFilename := TFile_Read_to (#0);
                  _SetFileInfo (LH.nOldSize,
                                LH.nNewSize,
                                0,
                                concat (sCurrentDir, sFilename),
                                false);
                  TFile_GotoFilePos (nOldFilePos + H.nHeaderSize);
                end;
              end;
          $E3:begin
                TFile_IncFilePos (4);
              end;
        else
          TFile_IncFilePos (H.nHeaderSize - ZET_H_SIZE);
        end;
      end;
    end;
  until (not TFile_IsOkay);

  TArchive_Notefooter;
end;

{---------------------------ZIP---------------------------}

procedure List_ZIP (nStartPos:longint); far;
type
     ZIP_CentralDirectoryStructure = record
       nID:longint;
       nPackVersion:word;
       nExtractVersion:word;
       nFlags:word;
       nCompression_Method:word;
       nLast_Modi_DT:longint;
       nCRC32:longint;
       nNewSize:longint;
       nOldSize:longint;
       nFilenameLen:word;
       nExtraLen:word;
       nCommentLen:word;
       nDiskNumberStart:word;
       nInternalAttr:word;
       nExternalAttr:longint;
       nOffset:longint;
     end;

     ZIP_CentralDirectoryStructureEnd = record
       nID:longint;
       nDiskNumber:word;
       nCentralDirectoryStartDisk:word;
       nCentralDirectoryEntriesOnThisDisk:word;
       nCentralDirectoryEntries:word;
       nCentralDirectorySize:longint;
       nOffsetToFirstDisk:longint;
       nCommentLen:word;
     end;

     ZIP_LocalFileHeader = record
       nID:longint;
       nExtract_Version:word;
       nFlags:word;
       nCompression_Method:word;
       nDT:longint;
       nCRC32:longint;
       nNewSize:longint;
       nOldSize:longint;
       nFilenameLen:word;
       nExtraLen:word;
     end;

     {
     only if LFH.Flags and bit[3] > 0
     for STDIN data
     }
     ZIP_DataDescriptor = record
       nCRC32:longint;
       nNewSize:longint;
       nOldSize:longint;
       nAttr:longint;
     end;
const
      ZIP_LFH_SIZE  = SizeOf (ZIP_LocalFileHeader); { 30 }
      ZIP_DD_SIZE   = SizeOf (ZIP_DataDescriptor);  { 16 }
      ZIP_CDS_SIZE  = SizeOf (ZIP_CentralDirectoryStructure);     { 46 }
      ZIP_CDSE_SIZE = SizeOf (ZIP_CentralDirectoryStructureEnd);  { 22 }

      ZIP_CDS_ID  = $02014B50;
      ZIP_LFH_ID  = $04034B50;
      ZIP_CDSE_ID = $06054B50;
      ZIP_DD      = $08;
var
    LFH:ZIP_LocalFileHeader;
    CDS:ZIP_CentralDirectoryStructure;
    CDSE:ZIP_CentralDirectoryStructureEnd;
{$ifdef GETTYP}
    nCodeCount,
{$endif}
    nDirCount, nZIPDirPos:longint;
    sFilename:string;

    procedure CheckZIPDirectory;
    var nSavePos:longint;
    begin
      nDirCount := 0;
      TFile_GotoFilePos (nZIPDirPos);
      repeat
        nSavePos := TFile_GetFilePos;
        if (TFile_ReadActBuf (CDS, ZIP_CDS_SIZE) = ZIP_CDS_SIZE) then
        begin
          if (CDS.nID = ZIP_CDS_ID) then
          begin
            TFile_IncFilePos (CDS.nFilenameLen +
                             CDS.nExtraLen +
                             CDS.nCommentLen);
            inc (nDirCount);
          end
          else  { check for CDSE }
          begin
            TFile_GotoFilePos (nSavePos);
            TFile_ReadActBuf (CDSE, ZIP_CDSE_Size);
            TFile_IncFilePos (CDSE.nCommentLen);
            TFile_Finish;
          end;
        end
        else  { check for CDSE }
        begin
          TFile_GotoFilePos (nSavePos);
          TFile_ReadActBuf (CDSE, ZIP_CDSE_Size);
          TFile_IncFilePos (CDSE.nCommentLen);
          TFile_Finish;
        end;
      until (not TFile_IsOkay);

{$ifdef GETTYP}
      if (nDirCount <> nCodeCount) then
        Noteln ('Error in ZIP directory');
{$endif}
    end;

begin
  TArchive_Init (nStartPos);
  TArchive_NoteHeaderX;

{$ifdef GETTYP}
  nCodeCount := 0;
{$endif}
  nZIPDirPos := 0;
  repeat
    if (TFile_ReadActBuf (LFH, ZIP_LFH_SIZE) = ZIP_LFH_SIZE) then
    begin
      if (LFH.nID = ZIP_LFH_ID) then
      begin
{$ifdef GETTYP}
        inc (nCodeCount);
{$endif}
        sFilename := TFile_ReadActLongString (LFH.nFilenameLen);
        _SetFileInfo (LFH.nOldSize,
                      LFH.nNewSize,  { - headersize ??? - }
                      LFH.nDT,
                      sFilename,
                      LFH.nFlags and BITS[0] > 0);

        TFile_IncFilePos (LFH.nExtraLen + LFH.nNewSize);
        if (LFH.nFlags and ZIP_DD > 0) then
          TFile_IncFilePos (ZIP_DD_SIZE);
      end
      else
        if (LFH.nID = ZIP_CDS_ID) or
           (LFH.nID = ZIP_CDSE_ID) then
        begin
          nZIPDirPos := TFile_GetFilePos - ZIP_LFH_SIZE;
          CheckZIPDirectory;
          TFile_Finish;
        end
        else
          TFile_SetError (TFILE_ERR_ID);
    end;
  until (not TFile_IsOkay);

{$ifdef GETTYP}
  if (nZIPDirPos = 0) then
    Noteln ('Error: cannot find ZIP directory!');
{$endif}

  TArchive_NoteFooter;
end;

{---------------------------ZIP2--------------------------}

procedure List_ZIP2 (nStartPos:longint); far;
begin
  List_ZIP (nStartPos + 4);
end;

{---------------------------ZOO---------------------------}

procedure List_ZOO (nStartPos:longint); far;
type
     ZOO_GlobalHeader = record
       sHeaderText:array[1..20] of char;  { 0 }
       nID:longint;                       { 20 }
       nOffsetFirstFile:longint;          { 24 }
       res:longint;                       { 28 }
       nPackVersion:byte;                 { 32 }
       nExtractVersion:byte;              { 33 }
     end;

     ZOO_LocalHeader = record
       nID:longint;                  { 0 }
       nTyp:byte;                    { 4 }
       nMethod:byte;                 { 5 }
       nOffsetNextDir:longint;       { 6 }
       nOffsetNextHeader:longint;    { 10 }
       nDT:longint;                  { 14 }
       nCRC16:word;                  { 18 }
       nOldSize:longint;             { 20 }
       nNewSize:longint;             { 24 }
       nPackVersion:byte;            { 28 }
       nExtractVersion:byte;         { 29 }
       nDeleteFlag:byte;             { 30 }
       nOffsetComment:longint;       { 31 } { 0 = null }
       nCommentLen:word;             { 35 }
       res1:byte;                   { 37 }
       {
       filename #0
       }
     end;
const
      ZOO_GH_SIZE = SizeOf (ZOO_GlobalHeader);
      ZOO_LH_SIZE = SizeOf (ZOO_LocalHeader);
var
    GH:ZOO_GlobalHeader;
    LH:ZOO_LocalHeader;
    sFilename:string;
begin
  TArchive_Init (nStartPos);

  TFile_ReadActBuf (GH, ZOO_GH_SIZE);
  NoteArchiveVersion (GetVersionStr (GH.nPackVersion, 0));
  TFile_GotoFilePos (nStartPos + GH.nOffsetFirstFile);

  TArchive_NoteHeader;

  repeat
    if (TFile_ReadActBuf (LH, ZOO_LH_SIZE) = ZOO_LH_SIZE) then
    begin
      if (LH.nOffsetNextDir = 0) then
        TFile_Finish
      else
      begin
        sFilename := TFile_Read_To (#0);
        _SetFileInfo (LH.nOldSize,
                      LH.nNewSize,
                      0,
                      sFilename,
                      false);
        TFile_GotoFilePos (nStartPos + LH.nOffsetNextDir);
      end;
    end;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------ZPK---------------------------}

procedure List_ZPK (nStartPos:longint); far;
type
     ZPK_Header = record
       res1:longint;
       res2:word;
       sFilename:array[0..12] of char;
       nDT:longint;
       nOldSize:longint;
       nNewSize:longint;
       res4:longint;
       res5:longint;
     end;
const
      ZPK_H_SIZE = SizeOf (ZPK_Header);
var
    H:ZPK_Header;
    nOldFilePos:longint;
    Len:word;
begin
  TArchive_Init (nStartPos + 6);  { skip file header }
  NoteArchiveVersion (GetVersionStr (TFile_ReadByteAt (nStartPos + 4),
                                     TFile_ReadByteAt (nStartPos + 5)));
  TArchive_NoteHeaderX;

  repeat
    nOldFilePos := TFile_GetFilePos;
    Len := TFile_ReadActBuf (H, ZPK_H_SIZE);
    if (Len = ZPK_H_SIZE) then
    begin
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    H.nDT,
                    CopyUntil (H.sFilename, #0),
                    false);
      TFile_GotoFilePos (nOldFilePos + ZPK_H_SIZE + H.nNewSize);
    end
    else
      if (Len = 8) then
        TFile_Finish;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{--------------------------GLOBAL-------------------------}

procedure TPackerRec.init (b:boolean; p:TListProc);
begin
  Found := b;
  Proc  := p;
end;

{$i gtpack2.pas}

function GetPackerID:TPacker;
var i:TPacker;
begin
  GetpackerID := NONE;
  for i:=P_MIN to P_MAX do
    if p[i].found then
    begin
      GetPackerID := i;
      exit;
    end;
end;

function GetPackerName (ID:TPacker):ShortStr;
var s:ShortStr;
begin
  case ID of
    _777  :s := '777';
    ACE   :s := 'ACE';
    AIN   :s := 'AIN';
    AKT7  :s := 'AKT7';
    AKT8  :s := 'AKT8';
    AMG   :s := 'AMG';
    ARC   :s := 'ARC/PAK';
    ARG   :s := 'ARG';
    ARI   :s := 'ARI';
    ARJ   :s := 'ARJ';
    ARK   :s := 'Quark';
    ARQ   :s := 'ARQ';
    ARS   :s := 'ARS';
    ARX   :s := 'ARX';
    ASD   :s := 'ASD';
    BIX   :s := 'BIX';
    BLI   :s := 'Blink';
    BOA   :s := 'BOA';
    BS2   :s := 'BS2';
    BSA   :s := 'BSA';
    BSN   :s := 'BSN';
    BZ    :s := 'BZip';
    CAB   :s := 'MS Cabinet';
    CHZ   :s := 'CHZ';
    COD   :s := 'CODEC';
    COMP16:s := 'COMP16';
    CPAC  :s := 'CrossePAC';
    DPA   :s := 'Dirks Packer';
    DWC   :s := 'DWC';
    ELI   :s := 'ELI';
    ESP   :s := 'ESP';
    FINISH:s := 'Finish';
    GZ    :s := 'GZip';
    HA    :s := 'HA';
    HAP   :s := 'HAP';
    HPA   :s := 'HPA';
    HQX   :s := 'BinHex';
    HYP25 :s := 'Hyper 2.5';
    HYP26 :s := 'Hyper 2.6';
    IMP   :s := 'IMP';
    INS   :s := 'installSHIELD';
    ISC   :s := 'installSHIELD Cabinet';
    JAR   :s := 'JAR';
    JRC   :s := 'JRC';
    LG    :s := 'ARHANGEL';
    LS    :s := 'Links LS';
    LIM   :s := 'LIM';
    LZA   :s := 'LZA';
    LZH   :s := 'LHA/LHArc/LHIce/AR';
    LZO   :s := 'lzop';
    MAR   :s := 'MAR';
    MSC   :s := 'MS Compress 5.0';
    PACK  :s := 'IBM Pack';
    PHP   :s := 'PHP';
    Q     :s := 'Quantum';
    QFC   :s := 'QFC';
    PAQ   :s := 'oPAQue';
    PAR   :s := 'PAR';
    PPMZ  :s := 'PPMZ';
    RAR0  :s := 'old RAR';
    RAR   :s := 'RAR';
    RAX   :s := 'RAX';
    RKV1  :s := 'RKive 1.4';
    RKV2  :s := 'RKive 1.92';
    RNC   :s := 'ProPack';
    SAR   :s := 'SAR';
    SBX   :s := 'SBX';
    SKY   :s := 'SKY';
    SOF   :s := 'SOF';
    SQWEZ :s := 'SQWEZ';
    SQZ   :s := 'Squeezer';
    STI   :s := 'Stirling';
    SWG   :s := 'SWAG';
    SYN   :s := 'Synetic';
    SZIP  :s := 'SZip';
    TAR   :s := 'TAR tape';
    UC2   :s := 'UC2';
    UFA   :s := 'UFA';
    UHA   :s := 'UHARC';
    XIE   :s := 'MSXIE';
    XL    :s := 'XLink';
    XPA   :s := 'XPA';
    YC    :s := 'YAC';
    YZ    :s := 'YamazakiZipper';
    ZET   :s := 'ZET';
    ZIP   :s := 'PKZip';
    ZIP2  :s := 'Multi volume PKZip';
    ZOO   :s := 'ZOO';
    ZPK   :s := 'ZPack';
  else
    s := 'GetPackerName - error';
{$ifdef GETTYP}
    InternalError ('packername');
{$endif}
  end;
  GetPackerName := s;
end;

{$ifdef FARC}
function GetPackerStrOfID (ID:TPacker):ShortStr;
begin
  GetPackerStrOfID := SPacker [ID];
end;

function GetPackerIDOfStr (s:ShortStr):TPacker;
var ID:TPacker;
begin
  for ID := NONE to P_MAX do
    if (s = SPacker [ID]) then
    begin
      GetPackerIDOfStr := ID;
      exit;
    end;
  GetPackerIDOfStr := NONE;
end;
{$endif}

procedure NotePacker;
var ID:TPacker;
begin
  ID := GetPackerID;
  if (ID = NONE) then
    exit;

  GTPACK_ARCNAME := GetPackerName (ID);

{$ifdef GETTYP}
  if WriteHeader (concat (GTPACK_ARCNAME, ' archive')) then
  begin
    FinishLine;
    if (bDoShowArcContent) then
      p[ID].Proc (PackerPosInFile)
    else
      { - check if procedure would be "List_" - }
      if (Addr (p[ID].proc) <> Addr (List_Not)) and
         (Addr (p[ID].proc) <> Addr (List_Single)) then
        Noteln ('Archive is not listed because switch /NA is used.');
  end;
{$else}
  p[ID].Proc (PackerPosInFile)
{$endif}
end;

begin
  ArcCount := 0;
  ArcSumOld := 0;
  ArcSumNew := 0;
{$ifdef GETTYP}
  bDoShowArcContent := true;
  ProcAddFile := TArchive_AddFile;
{$endif}
end.
