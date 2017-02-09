
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
    length:word;
begin
  TArchive_Init (nStartPos + 6);  { skip file header }
  NoteArchiveVersion (GetVersionStr (TFile_ReadByteAt (nStartPos + 4),
                                     TFile_ReadByteAt (nStartPos + 5)));
  TArchive_NoteHeaderX;

  repeat
    nOldFilePos := TFile_GetFilePos;
    length := TFile_ReadActBuf (H, ZPK_H_SIZE);
    if (length = ZPK_H_SIZE) then
    begin
      _SetFileInfo (H.nOldSize,
                    H.nNewSize,
                    H.nDT,
                    CopyUntil (H.sFilename, #0),
                    false);
      TFile_GotoFilePos (nOldFilePos + ZPK_H_SIZE + H.nNewSize);
    end
    else
      if (length = 8) then
        TFile_Finish;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

*/

/*

      { - needed for EXE overlay packer listing - }
      HYP_ID = $2650481A;
      ZOO_ID = $FDC4A7DC;

      ARCERR_OLDSIZE :longint = 0;
      ARCERR_NEWSIZE :longint = 0;
      ARCERR_FILENAME:string  = '';
      ARCERR_DATETIME:longint = 0;
      ARCERR_PASSWORD:boolean = false;
      ARCERR_ARCNAME :string  = '';
      ARCERR_OldSizeUndefined:boolean = false;
      ARCERR_NewSizeUndefined:boolean = false;

type
     TProcAddFile = procedure;


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
      ARCERR_OldSizeUndefined := true;
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
  if (ARCERR_NEWSIZE > b.FSize) and (1=2) then
  begin
    TFile_SetError (TFILE_ERR_VALUE);
    exit;
  end;

  inc (FilesInArchive);
  inc (LocalSumOld, ARCERR_OLDSIZE);
  inc (LocalSumNew, ARCERR_NEWSIZE);

  if (not bHasNoPackedSize) then
  begin
    { shr 10 - make KB out of it }
    inc (ArcSumOld, ARCERR_OLDSIZE shr 10);
    inc (ArcSumNew, ARCERR_NEWSIZE shr 10);
  end
  else
  begin
    { new: count onlyold size }
    { old: add ARCERR_OLDSIZE to both counter }
    { shr 10 - make KB out of it }
    inc (ArcSumOld, ARCERR_OLDSIZE shr 10);
    ARCERR_NewSizeUndefined := true;
    {
    inc (ArcSumNew, ARCERR_OLDSIZE shr 10);
    }
  end;

  { - for debugging purposes only - }
  if (ARCERR_OLDSIZE > 15000000) then
    ARCERR_OLDSIZE := ARCERR_OLDSIZE;
  if (ARCERR_NEWSIZE > 15000000) then
    ARCERR_NEWSIZE := ARCERR_NEWSIZE;

  NoteSizesPercent (ARCERR_OLDSIZE, ARCERR_NEWSIZE);

  if (ARCERR_DATETIME <> 0) then
    Append (concat (GetDateStr (ARCERR_DATETIME), ' '))
  else
    if (bExtendedOutput) then
      Append ('invalid  ');

  Append (ARCERR_FILENAME);
  if (ARCERR_PASSWORD) then
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
  ARCERR_OLDSIZE  := nOldSIze;
  ARCERR_NEWSIZE  := nNewSIze;
  ARCERR_DATETIME := nDT;
  ARCERR_FILENAME := sFilename;
  ARCERR_PASSWORD := bPassword;
  ProcAddFile;
end;

{---------------------------777---------------------------}

{---------------------------ACE---------------------------}

{---------------------------AIN---------------------------}

{---------------------------AKT7--------------------------}

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

{---------------------------ARI---------------------------}

procedure List_ARI (nStartPos:longint); far;
label ReadAgain;
var
    H:ARI_Header;
    sDir, g_sTemp, sFilename:string;
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
      g_sTemp := H.aFilename.sPath;
      nPos := pos ('\', g_sTemp);
      if (nPos > 0) then
      begin
        inc (nOldFilePos, nPos);
        g_sTemp := copy (g_sTemp, 1, nPos);
        if (bFirst) then
        begin
          sDir := g_sTemp;
          bFirst := false;
        end
        else
          sDir := concat (sDir, g_sTemp);

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
{---------------------------ARX---------------------------}
{---------------------------AR7---------------------------}
{---------------------------ASD---------------------------}
{---------------------------BIX---------------------------}
{---------------------------BLI---------------------------}
{---------------------------BOA---------------------------}
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
    length:word;
    sFilename:string;
begin
  TArchive_Init (nStartPos + 4);  { skip ID header }
  TArchive_NoteHeader;
  repeat
    length := TFile_ReadActBuf (H, BS2_H_SIZE);
    if (length = BS2_H_SIZE) then
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
      if (length = 2) then
        TFile_Finish;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;

{---------------------------BSA---------------------------}

{---------------------------BSN---------------------------}

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
    g_sTemp, sDirectory:string;
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
                g_sTemp := TFile_ReadActString (DH.nDirNameLen);
                sDirectory := concat (sDirectory, g_sTemp, '\');
              end;

          { - file - }
          $46:begin
                TFile_ReadActBuf (FH, CHZ_FH_SIZE);
                g_sTemp := TFile_ReadActLongString (FH.nFilenameLen);
                _SetFileInfo (FH.nOldSize,
                              FH.nNewSize,
                              0,
                              concat (sDirectory, g_sTemp),
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

{---------------------------JAR---------------------------}

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
    length:TPos;
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
    length := TFile_ReadActBuf (H, JRC_H_SIZE);
    if (length = JRC_H_SIZE) then
    begin
      _SetFileInfo (0,
                    H.nNewSize,
                    0,
                    TFile_Read_To (#10),
                    false);
      TFile_IncFilePos (H.nNewSize);
    end
    else
      if (length = 1) then
        TFile_Finish;
  until (not TFile_IsOkay);

  TArchive_NoteFooter;
end;

{---------------------------LG----------------------------}


{---------------------------LS----------------------------}

procedure List_LS (nStartPos:longint); far;
type
     LS_Local_Header = record
       sFilename:array [1..13] of char;
       m_nLocalOffset:longint;
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

      if (TFile_ReadBufAt (LH.m_nLocalOffset, GH, LS_GH_SIZE) = LS_GH_SIZE) then
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

{---------------------------LZO---------------------------}

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
    nNewSize, nSavedPos, m_nLocalOffset:longint;
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
      m_nLocalOffset := TFile_ReadActLong;
      SwapLongSelf (m_nLocalOffset);

      { - goto the position in the archive - }
      nSavedPos := TFile_GetFilePos;
      TFile_GotoFilePos (m_nLocalOffset);

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

  bNormalFormat := ((TFile_ReadByteAt (nStartPos + 7) and BITARRAY [0]) <> 0);

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
