const
      FIRST_LEN = 25;
var
    MS:TMemStruct;
    nOldBufLen, nOldAllocLen:TPos;
    LastThree:array[1..3] of char;
    aFirstStr:string[FIRST_LEN];

procedure InitPackBuffer (nStartPos:longint;
                          nMaxLen:longint);
var i:TPacker;
begin
  GTPACK_OLDSIZE  := 0;
  GTPACK_NEWSIZE  := 0;
  GTPACK_FILENAME := '';
  GTPACK_DATETIME := 0;
  GTPACK_PASSWORD := false;
  GTPACK_ARCNAME  := '';

{ save b }
  nOldBufLen := b.BufLen;
  nOldAllocLen := b._AllocLen;

  MS.bInUse := false;
  GetMemory (MS, b._AllocLen);
  CopyToMemory (b.p^, MS, b._AllocLen);

{ set found to NULL - one line is easier to debug }
  for i:=P_MIN to P_MAX do p[i].found := false;

{ init buffers }
  PackerPosInFile := nStartPos;
  TFile_Init (PackerPosInFile);

  b.BufLen := min_l (nMaxLen, PACKER_BUFFER_LEN) and $0000FFFF;

  TFile_ReadActBuf (b.p^, b.BufLen);
  move (b.p^, b.First, min_w (b.BufLen, 256));
  TFile_ReadBufAt (b.FSize - 3, LastThree, 3); { use absolute EOF!! }

  aFirstStr := BUF_GetString (1, FIRST_LEN);
end;

procedure ShiftPackBuffer;
var i:TPacker;
    j:longint;
{$ifndef DELPHI}
    dummy:pointer;
{$endif}
begin
  GTPACK_OLDSIZE  := 0;
  GTPACK_NEWSIZE  := 0;
  GTPACK_FILENAME := '';
  GTPACK_DATETIME := 0;
  GTPACK_PASSWORD := false;
  GTPACK_ARCNAME  := '';

  for i:=P_MIN to P_MAX do p[i].found := false;

{$ifdef MSDOS}
  dummy := pointer (longint (Addr (b.p^)) + 1);
  move (dummy^, b.p^, b.BufLen - 1);
{$else}
  for j:=1 to b.BufLen do
    b.p^ [j - 1] := b.p^ [j];
{$endif}

  dec (b.BufLen);
  inc (PackerPosInFile);

{
  ... overwrites its own memory area :( ...
}
  move (b.p^, b.First, min_w (b.BufLen, $0100));
  aFirstStr := BUF_GetString (1, FIRST_LEN);
end;

procedure RestorePackBuffer;
begin
  CopyFromMemory (MS, b.p^, nOldAllocLen);
  FreeMemory (MS);
  b._AllocLen := nOldAllocLen;
  b.BufLen    := nOldBufLen;
  move (b.p^, b.First, min_w (b.BufLen, 256));
end;

function _FilenameIsText (dummy:string; len:byte):boolean;
var a, i, dotcount:byte;
begin
  _FilenameIsText := false;
  i := 1;
  dotcount := 0;
  repeat
    a := ord (dummy[i]);
    if (a <> 0) and
       (a <> 32) and  { ' ' }
       (a <> 45) and  { '-' }
       (a <> 46) and  { '.' }
       ((a < 48) or (a >  57)) and   { '0' .. '9' }
       ((a < 65) or (a >  90)) and   { 'A' .. 'Z' }
       (a <> 95) and  { '_' }
       ((a < 97) or (a > 122)) then  { 'a' .. 'z' }
      exit;  { invalid ... }
    { check whether too many dots are in the filename }
    if (a = 46) then
    begin
      inc (dotcount);
      if (dotcount > 1) then
        exit;
    end;
    inc (i);
  until (i > len);
  { is okay, if the last char is a space or a #0 }
  _FilenameIsText := true;
end;

{
CheckForKnownPackers is independent of "b" except b.FSize.
So it is not necessary to adopt anything in "b".
}
procedure CheckForKnownPackers (nStartPos:longint;
                                nMaxLen:longint;
                                bInitAndRestore:boolean);
var sTemp:string;

    function IsARC:boolean;
    var AH:ARC_Header;
        p:byte;
        nFilesFound, nBufPos:TPos;
        bOkay:boolean;
    begin
      nBufPos := 1;
      nFilesFound := 0;

      repeat
        BUF_GetBuf (AH, nBufPos, ARC_H_SIZE);
        inc (nBufPos, ARC_H_SIZE);

        bOkay := (AH.nID = $1A);
        if (bOkay) then
          case (AH.nCompressionMethod) of
            0:begin
                nBufPos := b.BufLen;
                if (nFilesFound = 0) then
                  bOkay := false;
              end;
            { data }
            1..19:begin
                    p := pos (#0, AH.sFilename);
                    bOkay := (AH.nID = $1A) and
                             (AH.nOldSize >= AH.nNewSize) and
                             (AH.nNewSize <= b.FSize) and
                             (AH.nNewSize >= 0) and
                             (p > 2) and
                             (_FilenameIsText (AH.sFilename, p));
                    inc (nFilesFound);
                    inc (nBufPos, AH.nNewSize);
                  end;
            { comment }
            20..29:begin
                     if ((nStartPos + nBufPos - 1) + AH.nNewSize > b.FSize) or
                        (AH.nNewSize < 1) then
                       bOkay := false;
                     inc (nBufPos, AH.nNewSize);
                   end;
          else
            bOkay := false;
          end;
      until (not bOkay) or (nBufPos > b.BufLen);

      IsARC := bOkay;
    end;

    function IsARG:boolean;
    var AH:GT_ARG_Header;
        p:byte;
        nBufPos:TPos;
        bOkay:boolean;
    begin
      nBufPos := 1;

      repeat
        BUF_GetBuf (AH, nBufPos, ARG_H_SIZE);
        p := pos (#0, AH.sFilename);
        bOkay := (AH.nID = 0) and
                 (AH.nOldSize >= AH.nNewSize) and
                 (AH.nNewSize <= b.FSize) and
                 (AH.nNewSize >= 0) and
                 (p > 2) and
                 (AH.sFilename[1] <> '.') and
                 (_FilenameIsText (AH.sFilename, p));
        inc (nBufPos, ARG_H_SIZE + AH.nNewSize);
      until (not bOkay) or (nBufPos > b.BufLen);

      IsARG := bOkay;
    end;

    function IsARI:boolean;
    label ReadAgain;
    var bOkay:boolean;
        H:ARI_Header;
        nBufPos:longint;
        nPos:byte;
    begin
      nBufPos := 1;
      bOkay := true;

      repeat
      ReadAgain:
        BUF_GetBuf (H, nBufPos, ARI_H_SIZE);

        { it's the directory, but less stack ... }
        sTemp := H.aFilename.sPath;
        nPos := pos ('\', sTemp);
        if (nPos > 0) then
        begin
          inc (nBufPos, nPos);
          if (_FilenameIsText (sTemp, nPos - 1)) then
            goto ReadAgain
          else
            bOkay := false;
        end;

        if (bOkay) then
        begin
          nPos := pos (#0, H.aFilename.sName);
          if (nPos = 0) then
            nPos := length (H.aFilename.sName);
          bOkay := (_FilenameIsText (H.aFilename.sPath, 11)) and
                   { the filename should be only chars }
                   (_FilenameIsText (H.aFilename.sName, nPos)) and
                   { first char can not be a #0 }
                   (H.aFilename.sName [1] <> #0) and
                   { filename can not contain a dot }
                   (pos ('.', H.aFilename.sName) = 0) and
                   (pos ('.', H.aFilename.sExt) = 0) and
                   { H.sExt [1] maybe #0! - e.g. "makefile." }
                   (H.nNewSize <= b.FSize) and
                   { maybe violated (H.nOldSize >= H.nNewSize) and }
                   (H.nNewSize >= ARI_H_SIZE) and
                   (nStartPos + H.nNewSize <= b.FSize);
          if (bOkay) then
            bOkay := bOkay;
        end;
        inc (nBufPos, H.nNewSize);
      until (not bOkay) or (nBufPos > b.BufLen);
      IsARI := bOkay;
    end;

    function IsCOD:boolean;
    var bOkay:boolean;
        FH:COD_FileHeader;
        nBufPos:longint;
    begin
      if (b.First.w [1] <> $FF76) then
      begin
        IsCOD := false;
        exit;
      end;

      nBufPos := COD_GH_SIZE;
      repeat
        BUF_GetBuf (FH, nBufPos, COD_FH_SIZE);

        { it's the directory, but less stack ... }
        sTemp := BUF_GetString (nBufPos, FH.nDirnameLen);
        { it's the filename, but less stack ... }
        sTemp := BUF_GetString (nBufPos, FH.nFilenameLen);

        bOkay := (_FilenameIsText (sTemp, FH.nFilenameLen)) and
                 (FH.nFilenameLen > 0) and
                 (FH.nNewSize <= b.FSize) and
                 { maybe violated (H.nOldSize >= H.nNewSize) and }
                 (FH.nOldSize1 >= 0);
        inc (nBufPos, FH.nNewSize);
      until (not bOkay) or (nBufPos >= b.BufLen);
      IsCOD := bOkay;
    end;

    function IsHA:boolean;
    var bOkay:boolean;
        LH:HA_LocalHeader;
        nBufPos:longint;
        nLen:byte;
        nAdditional:byte;
    begin
      if (b.First.w [1] <> $4148) or (b.IsText) then
      begin
        IsHA := false;
        exit;
      end;

      nBufPos := HA_GH_SIZE + 1;
      repeat
        BUF_GetBuf (LH, nBufPos, HA_LH_SIZE);
        inc (nBufPos, HA_LH_SIZE);

        { in reality it's the directory but I wanted to save stack }
        sTemp := BUF_GetStringUntil (nBufPos, 0);
        inc (nBufPos, 1 + length (sTemp));

        { it's the filename, but less stack ... }
        sTemp := BUF_GetStringUntil (nBufPos, 0);
        nLen := length (sTemp);
        inc (nBufPos, 1 + nLen);

        nAdditional := BUF_GetByte (nBufPos);
        inc (nBufPos);

        bOkay := (_FilenameIsText (sTemp, nLen)) and
                 (LH.nNewSize < b.FSize) and
                 (LH.nOldSize >= LH.nNewSize);
        inc (nBufPos, nAdditional + LH.nNewSize);
      until (not bOkay) or (nBufPos > b.BufLen);
      IsHA := bOkay;
    end;

    function IsLIM:boolean;
    var bOkay:boolean;
        H:LIM_Header;
        nBufPos:longint;
        nLen:byte;
    begin
      if (b.First.w [1] <> $4D4C) then
      begin
        IsLIM := false;
        exit;
      end;

      nBufPos := 14;
      repeat
        BUF_GetBuf (H, nBufPos, LIM_H_SIZE);
        inc (nBufPos, LIM_H_SIZE);

        { it's the directory, but less stack ... }
        sTemp := BUF_GetStringUntil (nBufPos, 0);
        nLen := length (sTemp);
        inc (nBufPos, 1 + nLen);

        bOkay := (_FilenameIsText (sTemp, nLen)) and
                 (H.nNewSize < b.FSize) and
                 (H.nOldSize >= H.nNewSize);
        inc (nBufPos, H.nNewSize);
      until (not bOkay) or (nBufPos > b.BufLen);
      IsLIM := bOkay;
    end;

    function IsLS:boolean;
    var GH:LS_Global_Header;
    begin
      IsLS := false;

      if (b.First.w [1] <> $534C) then
        exit;

      BUF_GetBuf (GH, 1, LS_GH_SIZE);
      if (GH.nHeaderSize <> 55) or
         (GH.nFilesInArc <= 0) or
         (GH.nOldSize < 0) or
         (GH.nNewSize < 0) then
        exit;

      IsLS := true;
    end;

    function IsMSXIE:boolean;
    var bOkay:boolean;
        H:XIE_Header;
        nBufPos:longint;
    begin
      if (b.First.w [1] <> $534D) or (b.IsText) then
      begin
        IsMSXIE := false;
        exit;
      end;

      nBufPos := 4 + 1;
      repeat
        BUF_GetBuf (H, nBufPos, XIE_H_SIZE);
        bOkay := (H.sFilename [0] <= #12) and
                 (H.nNewSize < b.FSize) and
                 (H.nOldSize >= H.nNewSize) and
                 (H.nNewSize >= 0);
        inc (nBufPos, XIE_H_SIZE + H.nNewSize);
      until (not bOkay) or (nBufPos > b.BufLen);
      IsMSXIE := bOkay;
    end;

    function IsQ:boolean;
    var bOkay:boolean;
        GH:Q_GlobalHeader;
        LH:Q_LocalHeader;
        nBufPos:longint;
        sFilename, sComment:string;
        nCount:word;

        procedure GetXString (var s:string);
        var b:byte;
            nLen:word;
        begin
          b := BUF_GetByte (nBufPos);
          inc (nBufPos);

          if (b > 127) then
          begin
            nLen := BUF_GetByte (nBufPos) shl 8 + b;
            inc (nBufPos);
          end
          else
            nLen := b;

          s := BUF_GetLongString (nBufPos, nLen);
          inc (nBufPos, nLen);
        end;

    begin
      BUF_GetBuf (GH, 1, Q_GH_SIZE);

      if (GH.nID <> $5344) or
         (GH.nVersionMajor <> 0) or
         (b.IsText) then
      begin
        IsQ := false;
        exit;
      end;

      nBufPos := Q_GH_SIZE + 1;
      nCount := 0;

      repeat
        GetXString (sFilename);
        GetXString (sComment);
        BUF_GetBuf (LH, nBufPos, Q_LH_SIZE);
        inc (nBufPos, Q_LH_SIZE);
        bOkay := (_FilenameIsText (sFilename, length (sFilename))) and
                 (LH.nOldSize >= 0);
        inc (nCount);
      until (not bOkay) or (nCount >= GH.nFilesInArc);
      IsQ := bOkay;
    end;

begin
  if (bInitAndRestore) then
    InitPackBuffer (nStartPos, nMaxLen);

{ check data }
  p[_777].init  ((copy (aFirstStr, 1, 3) = '777'),                List_777);
  p[ACE].init   ((copy (aFirstStr, 8, 7) = '**ACE**'),            List_ACE);
  p[AIN].init   ((b.First.w [1] = $1221),                         List_AIN);
  p[AKT7].init  ((b.First.l [1] = $07544B41),                     List_AKT7);
  p[AKT8].init  ((b.First.l [1] = $08544B41),                     List_AKT8);
  p[AMG].init   ((b.First.w [1] = $36AD),                         List_AMG);
  p[ARC].init   (IsARC,                                           List_ARC);
  p[ARG].init   (IsARG,                                           List_ARG);
  { -----------------------------------------------------------------------}
  if (bSearchARI) then
    p[ARI].init   (IsARI,                                           List_ARI);
  { -----------------------------------------------------------------------}
  p[ARQ].init   ((b.First.w [1] = $5767),                         List_ARQ);
  p[ARJ].init   ((b.First.w [1] = $EA60) and
                 (b.First.b[6] > 0),                              List_ARJ);
  p[ARK].init   ((b.First.l [1] = $00100437),                     List_ARK);
  p[ARS].init   ((copy (aFirstStr, 1, 14) = #13'(C) STEPANYUK'),  List_Not);
  p[ARX].init   ((copy (aFirstStr, 3, 6) = '-lh1-'#0),            List_ARX);
  p[AR7].init   ((b.First.l [1] = $3752412C),                     List_Not);
  p[ASD].init   ((b.First.l [1] = $30445341),                     List_ASD);
  p[BIX].init   ((b.First.l [1] = $30584942),                     List_BIX);
  p[BLI].init   ((copy (aFirstStr, 1, 15) = 'Blink by D.T.S.'),   List_BLI);
  p[BOA].init   ((b.First.l [1] = $00414F42),                     List_BOA);
  p[BS2].init   ((b.First.l [1] = $425303D4),                     List_BS2);
  p[BSA].init   ((b.First.b [1] > 0) and (b.First.w [2] = $07AE), List_BSA);
  p[BSN].init   ((copy (aFirstStr, 1, 6) = #$FF#$42#83#71#0#0),
                                                                  List_BSN);
  p[BZ].init    ((b.First.w [1] = $5A42),                         List_Not);
  p[CAB].init   ((b.First.l [1] = $4643534D),                     List_CAB);
  p[CHZ].init   ((b.First.l [1] = $46684353),                     List_CHZ);
  p[COD].init   (IsCOD,                                           List_COD);
  p[COMP16].init((b.First.w [1] = $9D1F),                         List_Single);
  p[CPAC].init  ((copy (aFirstStr, 1, 6) = 'DSIGDC'),             List_CPAC);
  p[DPA].init   ((copy (aFirstStr, 1, 10) = 'Dirk Paehl'),        List_DPA);
  p[DWC].init   ((LastThree = 'DWC'),                             List_DWC);
  p[ELI].init   ((b.First.l [1] = $2061724F),                     List_ELI);
  p[ESP].init   ((b.First.l [1] = $3E505345),                     List_ESP);
  p[FINISH].init((b.First.l [1] = $14005000),                     List_FINISH);
  p[GZ].init    ((b.First.w [1] = $8B1F),                         List_GZ);
  p[HA].init    (IsHA,                                            List_HA);
  p[HAP].init   ((b.First.l [1] = $46483391),                     List_HAP);
  p[HPA].init   ((b.First.l [1] = $00415048),                     List_Not);
  p[HQX].init   ((copy (aFirstStr, 1, FIRST_LEN) = '(This file must be conver'),
                                                                  List_Not);
  p[HYP25].init ((copy (aFirstStr, 1, 3) = #26'HP') or
                 (copy (aFirstStr, 1, 3) = #26'ST'),              List_HYP25);
  p[HYP26].init ((b.First.l [1] = HYP_ID),                        List_HYP26);
  p[IMP].init   ((b.First.l [1] = $0A504D49),                     List_Not);
  p[INS].init   ((b.First.l [1] = longint ($D879AB2A)),           List_INS);
  p[ISC].init   ((b.First.l [1] = $28635349),                     List_Not);
  p[JAR].init   ((copy (aFirstStr, 15, 6) = #26'Jar'#27#0),       List_Not);
  p[JRC].init   ((copy (aFirstStr, 1, 7) = 'JRchive'),            List_JRC);
  p[LG].init    ((b.First.w [1] = $474C),                         List_LG);
  p[LS].init    (IsLS,                                            List_LS);
  p[LIM].init   (IsLIM,                                           List_LIM);
  p[LZA].init   ((copy (aFirstStr, 4, 7) = 'ChfLZ_2'),            List_LZA);
  p[LZH].init   ((not p [ARX].found) and
                 (copy (aFirstStr, 3, 2) = '-l') and
                 ((b.First.c [5] = 'h') or (b.First.c [5] = 'z')) and
                  (b.First.c [7] = '-'),                          List_LZH);
  p[LZO].init   ((b.First.l [1] = $4F5A4C89),                     List_LZO);
  p[MAR].init   ((copy (aFirstStr, 3, 5) = '-ah0-'),              List_MAR);
  p[MSC].init   ((b.First.l [1] = $44445A53),                     List_MSC);
  p[PACK].init  ((b.First.w [1] = $96A5),                         List_PACK);
  p[PAQ].init   ((b.First.l [1] = $5A2B1527),                     List_PAQ);
  p[PAR].init   ((copy (aFirstStr, 1, 7) = 'PHILIPP'),            List_Not);
  p[PHP].init   ((b.First.w [1] = $4850) and (b.First.b [3] < 2), List_PHP);
  p[PPMZ].init  ((b.First.l [1] = $5A4D5050),                     List_PPZ);
  p[Q].init     (IsQ,                                             List_Q);
  p[QFC].init   ((b.First.l [1] = $1A46511A),                     List_QFC);
  p[RAR0].init  ((b.First.l [1] = $5E7E4552),                     List_RAR0);
  p[RAR].init   ((b.First.l [1] = $21726152),                     List_RAR);
  p[RAX].init   ((b.First.l [1] = $42454C55),                     List_RAX);
  p[RKV1].init  ((b.First.b [2] = $80) and (b.First.b [3] = $0F), List_Not);
  p[RKV2].init  ((b.First.b [2] = $C0) and (b.First.b [3] = $07), List_Not);
  p[RNC].init   ((b.First.l [1] = $41434E52),                     List_RNC);
  p[SAR].init   ((b.First.w [2] = $4C20) and (b.First.w [3] = $3548),
                                                                  List_SAR);
  p[SBX].init   ((b.First.l [1] = $00314253),                     List_SBX);
  p[SKY].init   ((b.First.l [1] = $001040BC),                     List_SKY);
  p[SOF].init   ((b.First.l [1] = $06034B50),                     List_SOF);
  p[SQWEZ].init ((b.First.l [1] = $45575153),                     List_SQWEZ);
  p[SQZ].init   ((copy (aFirstStr, 1, 5) = 'HLSQZ'),              List_SQZ);
  p[STI].init   ((b.First.l [1] = longint ($8C655D13)),           List_STI);
  p[SWG].init   ((copy (aFirstStr, 3, 5) = '-sw1-'),              List_SWG);
  p[SYN].init   ((b.First.l [1] = $53594E46),                     List_SYN);
  p[SZIP].init  ((b.First.l [1] = $00010601) or
                 (b.First.l [1] = $040A5A53),                     List_Single);

  { TAR archives have at least a size of 512 }
  if (nMaxLen > 512) then
  begin
    sTemp := BUF_GetString (258, 5);
    p[TAR].init  ((sTemp = 'ustar'),                                List_TAR);
  end;

  p[UC2].init  ((copy (aFirstStr, 1, 3) = #85#67#50),             List_Not);
  p[UFA].init  ((b.First.l [1] = longint ($C6414655)),            List_UFA);
  p[UHA].init  ((b.First.l [1] = $02414855),                      List_Not);
  p[XIE].init  (IsMSXIE,                                          List_XIE);
  p[XL].init   ((b.First.l [1] = longint ($82024C58)),            List_XL);
  p[XPA].init  ((b.First.l [1] = $00617078),                      List_XPA);
  p[YC].init   ((b.First.l [4] = $43590100),                      List_YC);
  p[YZ].init   ((b.First.w [1] = $7A79),                          List_Not);
  p[ZET].init  ((b.First.w [1] = $5A4F),                          List_ZET);
  p[ZIP].init  ((b.First.l [1] = $04034B50),                      List_ZIP);
  p[ZIP2].init ((b.First.l [1] = $08074B50),                      List_ZIP2);
  p[ZOO].init  ((b.First.l [6] = longint (ZOO_ID)),               List_ZOO);
  p[ZPK].init  ((b.First.l [1] = $4B505A01),                      List_ZPK);

  if (bInitAndRestore) then
    RestorePackBuffer;
end;

