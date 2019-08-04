{
TypEXE - Executable files

Copyright (c) 1997, 98 by Philip Helger (philip@dke.univie.ac.at)
}

{$define extended}
{$i aflags.inc}

{$ifdef BORLAND}
{$R-}  { - disable range check for virtual array - }
{$endif}

unit TypEXE;

interface

function Check_COM (StartPos:longint):boolean;

function Check_EXE (StartPos:longint;
                    bExtended:boolean):boolean;

implementation

uses
{$ifdef WINDOWS}
     WinDOS,
{$else}
  {$ifdef WIN32}
    SysUtils,
  {$else}
     DOS,
  {$endif}
{$endif}
{$ifdef GETTYP}
     TypSYS,
{$endif}
     GTCodes,
     GTBuf, GTCompar, GTFile, GTGlobal, GTMem, GTOutput, GTRegist,
     GTData__, GTData00,
     TypEXELE, TypEXENE, TypEXEPE, TypOver,
     XEXE, XGlobal, XString;

const
      NO_NE = '  ';

var
    EXEHeader:TEXE;
    NE_Offset,
    ABS_SP,
    COM_StartPos:longint;

{
---------------------------------------------------------------
  GLOBAL METHODS
---------------------------------------------------------------
}

procedure ReadBuffer (nPos:longint);
begin
  TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
end;

function EvaluateMacro (nStartPos:longint;
                        s:TMacroStr;
                        nTyp:byte):boolean;
var nPos:longint;
begin
  EvaluateMacro := false;
  nPos := nStartPos;
  {  - s: the macro; nPos: returned position; nTyp: typ (COM, EXE, PE) - }
  if (not TEXEFile_GetMacroPos (s, nPos, nTyp)) then
    exit;
  ReadBuffer (nPos);
  EvaluateMacro := true;
end;

function DoJumps (nStartPos:longint; nJumps:byte):boolean;
begin
  DoJumps := false;
  if TEXEFile_GetJumpPos (nStartPos, nJumps) then
  begin
    ReadBuffer (nStartPos);
    DoJumps := true;
  end;
end;

{
---------------------------------------------------------------
  COM SPECIFIC METHODS
---------------------------------------------------------------
}

function Check_EXE2COM_Converter (P:PCompEXECOMRec):boolean;
var nNewStartOfs:longint;
    OldBufLen:word;
    MS:TMemStruct;
begin
  if ComparePtrBuf (@p^.Data) then
  begin
    nNewStartOfs := COM_StartPos + p^.StartPos;

    OldBufLen := b.BufLen;

    { save the buffer }
    MS.bInUse := false;
    GetMemory (MS, OldBufLen);
    CopyToMemory (b.p^, MS, OldBufLen);

    { - if filesize already < MAX; dec buflen - }
    if (b.BufLen < GTBuf_MaxBuffer) then
      dec (b.BufLen, nNewStartOfs)
    else
      { - if newpos + MAX > filesize -> adapted buflen - }
      if (nNewStartOfs + b.BufLen > b.FSize) then
        b.BufLen := b.FSize - nNewStartOfs;

    TFile_GotoFilePos (nNewStartOfs);
    TFile_ReadActBuf (b.p^, b.BufLen);

    { temporary decrease the filesize for the current position }
    dec (b.FSize, p^.StartPos);

    inc (nAbsOverlayPos, nNewStartOfs);

    IncInsLevel;
    Check_EXE (nNewStartOfs, true);  { check extended EXE }
    DecInsLevel;

    { and increase the filesize for the current instance }
    inc (b.FSize, p^.StartPos);

    { restore the saved buffer }
    CopyFromMemory (MS, b.p^, OldBufLen);
    FreeMemory (MS);

    b.BufLen := OldBufLen;

    Check_EXE2COM_Converter := true;
  end
  else
    Check_EXE2COM_Converter := false;
end;

procedure _CompareCodesFromFile (var bFound:boolean;
                                     eType:TEXEType);
var rec:tSCRecord;
    pSCRecord:pSCRecords;
    i:integer;
begin
  pSCRecord := aStartupCodes.next;
  while (pSCRecord <> nil) do
  begin
    for i:=0 to pSCRecord^.nCount - 1 do
    begin
      CopyFromMemoryAt (pSCRecord^.aMS, rec, sizeof (tSCRecord), i * sizeof (tSCRecord));
      if (eType = rec.EXEType) then
        if ComparePtrBufFile (@rec) then
        begin
          bFound := true;
          exit;
        end;
    end;
    pSCRecord := pSCRecord^.next;
  end;
end;

procedure WriteCOMInfo (var found:boolean);
var b0,  b1,  b7,  b8,
    b18, b27, b34,
    b36, b41, b61:boolean;

    procedure __ComparePtrCode (p:PCompPtrRec);
    begin
      if ComparePtrBuf (p) then
        Found := true;
    end;

{ the main methods where the buffer is read }

    function DoMOVJump (nStartPos:longint):boolean;
    begin
      DoMOVJump := false;

      if TEXEFile_GetMOVJumpPos (nStartPos, COM) then
      begin
        ReadBuffer (nStartPos);
        DoMOVJump := true;
      end;
    end;

    function DoFarJump (nStartPos, nOffset:longint):boolean;
    begin
      DoFarJump := false;

      if TEXEFile_GetFarJumpPos (nStartPos, nOffset, COM) then
      begin
        ReadBuffer (nStartPos);
        DoFarJump := true;
      end
    end;

    function EvaluateCOMMacro (s:TMacroStr):boolean;
    begin
      EvaluateCOMMacro := EvaluateMacro (COM_StartPos, s, COM);
    end;

    procedure CheckCompressed;
    const MAX_CODE = 2000;
    var nPos:longint;
    begin
      {
        if file contains a jump first which leads to
        the last MAX_CODE bytes of a file I declare irt
        compressed :-)
      }
      nPos := COM_StartPos;
      if (b0) and (b.FSize > MAX_CODE) then
        if (TEXEFile_GetJumpPos (nPos, 1)) then
          if (nPos >= b.FSize - MAX_CODE) then
          begin
            Noteln ('Propably compressed ...');
            Found := true;
          end;
    end;

var i:integer;
begin
{ direct buffer }
  TFile_Init (0);
  TFile_ReadBufAt (COM_StartPos, CompareBuf, SizeOf (CompareBuf));

  if Check_EXE2COM_Converter (@REC_DoP102) or
     Check_EXE2COM_Converter (@REC_EXETOCOM) or
     Check_EXE2COM_Converter (@REC_XLoader200) then
  begin
    found := true;
    exit;
  end;

  b0  := IsJump (0);
  b1  := IsJump (1);
  b7  := IsJump (7);
  b8  := IsJump (8);
  b18 := IsJump (18);
  b27 := IsJump (27);
  b34 := IsJump (34);
  b36 := IsJump (36);
  b41 := IsJump (41);
  b61 := IsJump (61);

  if b0 and DoJumps (COM_StartPos, 1) then  { "j" }
  begin
    _CompareCodesFromFile (found, EXE_DOS_01);
    {
    for i:=1 to COUNT_01 do
      if ComparePtrBuf (@REC_01 [i]) then
      begin
        Found := true;
        goto LoopEnd1;
      end;
LoopEnd1:
    }

    if DoJumps (COM_StartPos, 2) then       { "jj" }
    begin
      __ComparePtrCode (@REC_C_Crypt102C);
      __ComparePtrCode (@REC_VSS);
    end;

    if DoJumps (COM_StartPos, 3) then      { "jjj" }
    begin
      __ComparePtrCode (@REC_CC286x2);
      __ComparePtrCode (@REC_LockProg05a);
    end;
  end;

  if b1 and DoJumps (COM_StartPos + 1, 2) then  { "1jj" }
  begin
    __ComparePtrCode (@REC_DeepCrypter01b);
    __ComparePtrCode (@REC_SnoopStop115);
  end;

  if b7 and DoJumps (COM_StartPos + 7, 1) then   { "7j" }
  begin
    __ComparePtrCode (@REC__Synopsis);
  end;

  if b8 and DoJumps (COM_StartPos + 8, 1) then   { "8j" }
  begin
    __ComparePtrCode (@REC_Com4Mail);
  end;

  if b18 and DoJumps (COM_StartPos + 18, 1) then   { "18j" }
  begin
    __ComparePtrCode (@REC_Mess114C);
    __ComparePtrCode (@REC_Mess115C);
    __ComparePtrCode (@REC_Mess117C);
    if (not Found) then
      __ComparePtrCode (@REC_MessC);
  end;

  if b27 and DoJumps (COM_StartPos + 27, 1) then   { "27j" }
  begin
    __ComparePtrCode (@REC_XComOr099g);
  end;

  if b34 and DoJumps (COM_StartPos + 34, 1) then   { "34j" }
  begin
    __ComparePtrCode (@REC_jmtcp05a);
  end;

  if b36 and DoJumps (COM_StartPos + 36, 1) then   { "36j" }
  begin
    __ComparePtrCode (@REC_fdscp04);
  end;

  if b41 and DoJumps (COM_StartPos + 41, 2) then   { "41jj" }
  begin
    __ComparePtrCode (@REC_XcomOr099h);
  end;

  if b61 and DoJumps (COM_StartPos + 61, 2) then   { "61jj" }
  begin
    __ComparePtrCode (@REC_XcomOr099i);
  end;

  if Found then
    exit;

  TFile_ReadBufAt (COM_StartPos, CompareBuf, SizeOf (CompareBuf));

  _CompareCodesFromFile (found, EXE_DOS_00);
{
  for i := 1 to COUNT_00 do
    if ComparePtrBuf (@REC_00 [i]) then
    begin
      Found := true;
      goto LoopEnd2;
    end;
LoopEnd2:
}

{ also used in DoJumps (1) section
  __CompareCode (@Protect60UCRec);
}

  if DoMOVJump (COM_StartPos) then   { "m" }
  begin
    __ComparePtrCode (@REC_DSCRP128);
    __ComparePtrCode (@REC_IBMCRP);
    __ComparePtrCode (@REC_StonesComcrypt);
  end;

  if DoMOVJump (COM_StartPos + 1) then   { "1m" }
  begin
    __ComparePtrCode (@REC_DarkStop10);
    __ComparePtrCode (@REC_HS112C);
    __ComparePtrCode (@REC_HS113C);
    __ComparePtrCode (@REC_HS114C);
    __ComparePtrCode (@REC_HS115C);
    __ComparePtrCode (@REC_HS117bC);
    __ComparePtrCode (@REC_HS117C);
    __ComparePtrCode (@REC_HS118C);
    __ComparePtrCode (@REC_HS119b195C);
    __ComparePtrCode (@REC_HS119b197C);
    __ComparePtrCode (@REC_HS119b198C);
    __ComparePtrCode (@REC_HS119b204C);
    __ComparePtrCode (@REC_HS119b206C);
    __ComparePtrCode (@REC_RCCII286113h);
    __ComparePtrCode (@REC_RCCII286113m);

    if EvaluateCOMMacro ('1m50') then  { otherwise I had problems because the
                                         code is too similar -> differences
                                         start at byte 80 ;) }
    begin
      __ComparePtrCode (@REC_RCCII286114h);
      __ComparePtrCode (@REC_RCCII286114m);
      __ComparePtrCode (@REC_RCCII286115h);
      __ComparePtrCode (@REC_RCCII286115m);
      __ComparePtrCode (@REC_RCCII286116h);
      __ComparePtrCode (@REC_RCCII286116m);
    end
  end;

  if DoFarJump (COM_StartPos, 6) then   { "6f" }
  begin
    __ComparePtrCode (@REC_SCRe2b102);
  end;

  if EvaluateCOMMacro ('1m6j') then
  begin
    __ComparePtrCode (@REC_RCCII386051);
  end;

  if EvaluateCOMMacro ('15fj') then
  begin
    __ComparePtrCode (@REC_PW10C);
  end;

  if Found then
    exit;

  { - if nothing was found - generic check - }
  CheckCompressed;
end;

function Check_COM (StartPos:longint):boolean;
var found:boolean;

    function IsCOM (StartPos:longint):boolean;
    begin
      COM_StartPos := StartPos;
      IsCOM := (COM_StartPos > 0) or
               (
                (COM_StartPos = 0) and
                ((b.FExt = 'COM') or (b.FExt = 'EXE')) and
                (b.First.w [1] <> $5A4D) and
                (b.First.w [1] <> $4D5A)
               );
    end;

begin
  { check registration info somewhere in the code ... }
  if (bInitialized) and (USER_NAME = '') then
    halt;
  { end check }
  if IsCOM (StartPos) then
  begin
    Check_COM := true;
    if WriteHeader ('DOS COM file') then
    begin
      Appendln (concat (' - ', i2s (b.FSize), ' bytes'));

      found := false;

      if (b.FSize > 65535) then
        Noteln ('File is too large to be a COM file')
      else
      begin
        WriteCOMInfo (found);
        if (not found) then
          Noteln ('May be pure assembler output (???)');
      end;
    end;
  end
  else
    Check_COM := false;
end;

{
---------------------------------------------------------------
  EXE SPECIFIC METHODS
---------------------------------------------------------------
}

var
    NE_ID:array[1..2] of char;
    HSRec:record
      ID1:array [1..2] of char;
      VHi:byte;
      VLo:byte;
      ID2:array [1..5] of char;
    end;

function IsNoNewEXE:boolean;
begin
  IsNoNewEXE := (NE_ID <> 'LE') and
                (NE_ID <> 'LX') and
                (NE_ID <> 'NE') and
                (NE_ID <> 'PE');
end;

procedure CheckCompBuf;
var l:longint;
    p, MAX_COMP:TPos;
begin
  { check registration info somewhere in the code ... }
  if (bInitialized) and (USER_NAME = '') then
    halt;
  { end check }
  p := 1;

  { the minus 4 is needed because of the following move command }
  MAX_COMP := sizeof (CompareBuf) - 4;
  while (p < MAX_COMP) and (CompareBuf [p - 1] < $20) do
    inc (p);

  { if nothing valid was found, exit }
  if (p >= MAX_COMP) then
    exit;

  move (CompareBuf [p], l, 4);

  if (l = $21CD4CB4) then
    Noteln ('First command is program abortion');

  if (l = $21CD30B4) then
    Noteln ('First command is question for DOS version');
end;

{
the return value simply determines wether the file should
be analyzed on or if it is a COM file (false)
}
function AnalyzeStartupCode (var found:boolean):boolean;
var _IP, nPos:longint;
    temp:string [50];

    procedure __ComparePtrCode (p:PCompPtrRec);
    begin
      if ComparePtrBuf (p) then
        Found := true;
    end;

    procedure CheckFFSE04;
    var w1, w2:word;
    begin
      if ComparePtrBuf (@REC_FFSE04) then
      begin
        Found := true;
        temp := 'Encrypter: FFSE 0.4';
        move (CompareBuf [22], w1, 2);
        move (CompareBuf [29], w2, 2); { guess it should be 30 }
             if (w1 = $08B2) and (w2 = $0896) then Noteln (concat (temp, 'E'))
        else if (w1 = $0837) and (w2 = $081B) then Noteln (concat (temp, 'F'))
        else if (w1 = $0699) and (w2 = $067D) then Noteln (concat (temp, '+'))
        else Noteln (concat (temp, ' (unknown)'));
      end;
    end;

    procedure CheckPKLite;
    var bLo, bHi:byte;
    begin
      if (not Found) then
      begin
        __ComparePtrCode (@REC_PKLite100E);
        __ComparePtrCode (@REC_PKLite112E);
        __ComparePtrCode (@REC_PKLite114E);
        __ComparePtrCode (@REC_PKLite115E);
        __ComparePtrCode (@REC_PKLite150E);
        __ComparePtrCode (@REC_PKLite200Ee);

        if Found then
        begin
          bLo := b.First.b [29];
          bHi := b.First.b [30];
          Note (concat ('According to header: PKLite ', i2s (bHi and $0F), '.', LeadingZero (bLo, 2)));
          if (bHi and BITS [4] > 0) then
            Append (' [extra compression]');
          FinishLine;
        end;
      end;
    end;

    procedure CheckAlec16;
    var n:byte;
    begin
      n := 16;
      repeat
        dec (n);
        move (CompareBuf [1], CompareBuf [0], SizeOf (CompareBuf) - 1);
        { - the last byte does not matter - compare length is only 6!! - }
        if ComparePtrBuf (@REC_Alec16) then
          Found := true;
      until (Found) or (n = REC_Alec16.DataLen);
    end;

    procedure CheckWWPack;
    const WWPExt:array[1..4] of string[2] = ('PR', 'P ', 'PU', 'PP');
    var Version:byte;
        s:string[12];
    begin
      if ComparePtrBuf (@REC_WWPack300PR) then
      begin
        if CompareBuf [1] = 9 then
          Version := 9
        else
          Version := 2;
      end
      else
      if ComparePtrBuf (@REC_WWPack30xPR) then
      begin
        Version := CompareBuf [1] + 14;
        if (Version <> 14) and
           (Version <> 17) and
           (Version <> 21) and
           (Version <> 25) then
          Version := 2;
      end
      else
      if ComparePtrBuf (@REC_WWPack30xP) then
      begin
        Version := CompareBuf [1];
        if (Version < 10) or
           (Version = 14) or
           (Version = 17) or
           (Version = 21) or
           (Version = 25) or
           (Version > 28) then
          Version := 2;
      end
      else
        exit;

      case Version of
             2:s := '???';
             9:s := '3.00/3.01 PR';
        10..11:s := concat ('3.00 ',  WWPExt [Version -  8]);
        12..13:s := concat ('3.01 ',  WWPExt [Version - 10]);
        14..16:s := concat ('3.02 ',  WWPExt [Version - 13]);
        17..20:s := concat ('3.03 ',  WWPExt [Version - 16]);
        21..24:s := concat ('3.04 ',  WWPExt [Version - 20]);
        25..28:s := concat ('3.05á ', WWPExt [Version - 24]);
      end;
      Found := true;
      Noteln (concat ('Packer: WWPack ', s));
    end;

    procedure CheckTinyProg;
    var s:string;
        w:word;
    begin
      if ComparePtrBuf (@REC_TinyProg2_3) then
      begin
        Found := true;
        move (CompareBuf [71], w, 2);
        s := 'Packer: TinyProg ';
             if (w = $74FF) then s := concat (s, '1.0')
        else if (w = $4C8B) then s := concat (s, '3.3 - 3.9')
        else s := concat (s, '(unknown)');
        Noteln (s);

       if (CompareBuf[1] = $18) and
          (CompareBuf[2] = $01) then
          Noteln ('Addon protector: TinyProt 1.0e');
      end;
    end;

    procedure CheckDJ;
    const X1 = 20;
          X2 = 26;
    var b:array [X1..X2] of boolean;
        i:byte;
    begin
      { read the vars at CS:IP }
      for i:=X1 to X2 do
        b[i] := IsJump (i);

      for i:=X1 to X2 do
        if (b[i]) and DoJumps (_IP + i, 1) then
        begin
          __ComparePtrCode (@REC_DPMI_DJ_1);
          __ComparePtrCode (@REC_DPMI_DJ_2);
          if (Found) then
            exit;
        end;
    end;

    function EvaluateEXEMacro (s:TMacroStr):boolean;
    begin
      EvaluateEXEMacro := EvaluateMacro (_IP, s, EXE);
    end;

    procedure DoCOMCompare;
    var nDelta:longint;
    begin
      nDelta := EXEHeader.GetAbsoluteIP;

      dec (b.FSize, nDelta);
      IncInsLevel;

      Check_COM (ABS_SP + nDelta);

      DecInsLevel;
      inc (b.FSize, nDelta);
    end;

label LoopEnd0, LoopEnd2;
var b0,  b1,  b3,
    b4,  b8,  b13,
    b15, b21:boolean;
    i:integer;
begin
  move (EXEHeader.HeaderSize, CompareBuf, SizeOf (TArray24));

{ check for converted COM file (COM2EXE) }
  for i:=1 to COUNT_C2E do
    if CompareHeader (@REC_C2E [i]) then
    begin
      Found := true;
      goto LoopEnd0;
    end;
LoopEnd0:

  if Found then
  begin
    AnalyzeStartupCode := false;
    DoCOMCompare;
    exit;
  end
  else
    AnalyzeStartupCode := true;

  TFile_Init (0);
  _IP := ABS_SP + EXEHeader.GetAbsoluteIP;

{ check for long call }
  nPos := _IP;
  if (TEXEFile_GetLongCallPos (nPos, ABS_SP + EXEHeader.GetHeaderSize)) then
  begin
    ReadBuffer (nPos);
    __ComparePtrCode (@REC_TP4_1);
    __ComparePtrCode (@REC_TP4_2);
    __ComparePtrCode (@REC_TP5);
    __ComparePtrCode (@REC_TP6);
    __ComparePtrCode (@REC_TP7);
    __ComparePtrCode (@REC_TP7Juffa);

    exit;
  end;

  ReadBuffer (_IP);

  b0  := IsJump (0);
  b1  := IsJump (1);
  b3  := IsJump (3);
  b4  := IsJump (4);
  b8  := IsJump (8);
  b13 := IsJump (13);
  b15 := IsJump (15);
  b21 := IsJump (21);

  if b0 and DoJumps (_IP, 1) then
  begin
    _CompareCodesFromFile (Found, EXE_DOS_01);

    if DoJumps (_IP, 2) then
    begin
      __ComparePtrCode (@REC_Crypt120);
      __ComparePtrCode (@REC_Crypt121);
      __ComparePtrCode (@REC_Crypta20);
      __ComparePtrCode (@REC_JMCrypt07g);
      __ComparePtrCode (@REC_JMCrypt07i);
      __ComparePtrCode (@REC_JMCrypt07j);
      __ComparePtrCode (@REC_XPack10jE);
      __ComparePtrCode (@REC_XPack10mE);
      __ComparePtrCode (@REC_XPack129E);
      __ComparePtrCode (@REC_XPack131E);
      __ComparePtrCode (@REC_XPack133E);
      __ComparePtrCode (@REC_XPack134E);

      CheckTinyProg;

      if (not Found) and EvaluateEXEMacro ('jj48j') then
        __ComparePtrCode (@REC_JMCrypt07);
    end;
    if (Found) then
      goto LoopEnd2;
  end;

  if b1 and DoJumps (_IP + 1, 1) then
  begin
    __ComparePtrCode (@REC_Bunny41I);
    __ComparePtrCode (@REC_Bunny41M);
    __ComparePtrCode (@REC_Bunny41P);
    __ComparePtrCode (@REC_PW10E);
  end;

  if b3 and DoJumps (_IP + 3, 1) then
  begin
    __ComparePtrCode (@REC_Aluwain809);
  end;

  if b4 and DoJumps (_IP + 4, 1) then
  begin
    CheckTinyProg;
  end;

  if b8 and DoJumps (_IP + 8, 1) then
  begin
    __ComparePtrCode (@REC_PirateStop105);
  end;

  if b13 and DoJumps (_IP + 13, 1) then
  begin
    __ComparePtrCode (@REC_Mess117E);
    __ComparePtrCode (@REC_Mess120E);
  end;

  if b15 and DoJumps (_IP + 15, 1) then
  begin
    __ComparePtrCode (@REC_Mess114E);
    __ComparePtrCode (@REC_Mess115E);
  end;

  if b21 and DoJumps (_IP + 21, 1) then
  begin
    __ComparePtrCode (@REC_HS102bE);
  end;

  if Found then
    exit;

  { DoJumps changes the buffer -> re-read }
  ReadBuffer (_IP);

  _CompareCodesFromFile (Found, EXE_DOS_00);
{
  for i:=1 to COUNT_00 do
    if ComparePtrBuf (@REC_00 [i]) then
    begin
      Found := true;
      goto LoopEnd2;
    end;
}
LoopEnd2:

{
Also used in Jumps (_IP, 2) ... ???
  __CompareCode (@Crypt121Rec);
}

{ removed due to problems with normal WWPack
  __CompareCode (WWPMut10Rec);
}

{ all follwoing checks need the buffer from CS:IP }

  CheckFFSE04;
  CheckPKLite;
  CheckCompBuf;

{ needs to be the last because the buffer is modified
  but needs the start buffer at CS:IP }
  CheckDJ;

  if Found then
    exit;

  ReadBuffer (_IP - 16);
  CheckAlec16;

  ReadBuffer (_IP - 2);
  CheckWWPack;

  ReadBuffer (_IP + 5);
  __ComparePtrCode (@REC_CrackStop10);
  __ComparePtrCode (@REC_CrackStop10b1);
  __ComparePtrCode (@REC_CrackStop10b2);
  __ComparePtrCode (@REC_CrackStop101);
  __ComparePtrCode (@REC_CrackStop102);
  __ComparePtrCode (@REC_CrackStop103);

  ReadBuffer (_IP + 750);
  __ComparePtrCode (@REC_YifPress10);

  ReadBuffer (ABS_SP + 32);
  __ComparePtrCode (@REC_PCrypt350E);

  ReadBuffer (ABS_SP + EXEHeader.GetHeaderSize + EXEHeader.GetCS);  { IP == 0 }
  __ComparePtrCode (@REC_FFSE05);

  ReadBuffer (ABS_SP + EXEHeader.GetHeaderSize + 8);
  __ComparePtrCode (@REC_ANTIUPC_102);
end;

function InitEXE (EXE_ABS_SP:longint; bExtended:boolean):boolean;
begin
  ABS_SP := EXE_ABS_SP;

  move (b.p^, EXEHeader, EXE_HEADER_SIZE);

  if (    bExtended and EXEHeader.IsExtendedEXE) or
     (not bExtended and EXEHeader.IsEXE) then
  begin
    TFile_Init (0);

{ check if it is a New Executable }
{$ifdef difficult}
    if EXEHeader.NEOffsetIsValid then
    begin
      { add the startpos!! othwerwise NE_OFFSET would point to the first EXE }
      NE_Offset := ABS_SP + BUF_GetLong ($3D);
      TFile_ReadBufAt (NE_Offset, NE_ID, 2);
    end
    else
    begin
      NE_ID := NO_NE;
      NE_Offset := 0;
    end;
{$else}
    { add the startpos!! othwerwise NE_OFFSET would point to the first EXE }
    NE_Offset := ABS_SP + BUF_GetLong ($3D);
    if (NE_Offset < b.FSize) then
      TFile_ReadBufAt (NE_Offset, NE_ID, 2)
    else
      NE_ID := NO_NE;
{$endif}

{$ifdef GETTYP}
{ - SYS_Header: aus TYPSYS.PAS - }
    TFile_ReadBufAt (EXEHeader.GetHeaderSize, SYS_Header, SYS_HEADER_SIZE);
{$endif}

{ HSRec - TYPEXE.PAS }
    TFile_ReadBufAt (ABS_SP + EXEHeader.GetSizeInHeader - 9, HSRec, 9);

    InitEXE := true;
  end
  else
    InitEXE := false;
end;

{ TLink }

function IsTLink:boolean;
begin
  IsTLink := (b.First.b [31] = $FB);
end;

function GetTLinkVersion:VersionStr;
var t:byte;
begin
  t := b.First.b [32];
  GetTLinkVersion := concat (i2s (t shr 4), '.', i2s (t and $0F));
end;

{ PKLite }

function IsPKLite:boolean;
var bCode, bID:boolean;
begin
  bCode := ((b.First.b [30] and $0F) >= 1) and  { hiversion >= 1 }
           ((b.First.b [30] and $0F) <= 2) and  { hiversion <= 2 }
            (b.First.l [6] = longint ($FFF00100));
  bID := (b.First.w [16] = $4B50);

  if (bID) and (not bCode) then
    Noteln ('Found ID but no code of PKLite');

  if (not bID) and (bCode) then
    Noteln ('Found code but no ID of PKLite');

  IsPKLite := bCode;
end;

function GetPKLiteVersion:VersionStr;
begin
                                                 { bits 0-3 }
  GetPKLiteVersion := concat (i2s (b.First.b[30] and $0F), '.',
                                   LeadingZero (b.First.b[29], 2));
end;

function IsPKLiteExtraCompression:boolean;
begin
  IsPKLiteExtraCompression := (b.First.b [30] and BITS [4] > 0);
end;

{ EXEPack }

function IsExePack:boolean;
const s = 35;
      e = 512;
label LoopEnd;
var IsZero:boolean;
    buf:array[s..e] of byte;
    i:word;
begin
  BUF_GetBuf (buf, s, SizeOf (buf));
  IsZero := true;
  for i:=s to e do
    if buf [i] <> 0 then
    begin
      IsZero := false;
      goto LoopEnd;
    end;
LoopEnd:

  IsExePack := (b.First.b  [9]  = 32) and
               (b.First.b [13] = 255) and
               (b.First.b [14] = 255) and
               (b.First.b [17] = 128) and
               IsZero;
end;

function GetExePackVersion:VersionStr;
var b1, b2:byte;
begin
  b1 := b.First.b [19];
  b2 := b.First.b [21];
       if (b1 = $00) and (b2 = $10) then GetExePackVersion := '3.65'
  else if (b1 = $99) and (b2 = $10) then GetExePackVersion := '4.00'
  else if (b1 = $15) and (b2 = $10) then GetExePackVersion := '4.03'
  else if (b1 = $00) and (b2 = $12) then GetExePackVersion := 'ID_2'
  else GetExePackVersion := '???';
end;

{ SEA AXE }

function IsSEAAXE:boolean;
begin
  IsSEAAXE := (BUF_GetString (33, 8) = 'SEA-AXE ');
end;

function GetSEAAXEVersion:VersionStr;
begin
  GetSEAAXEVersion := BUF_GetString (41, 3);
end;

function GetSEAAXEOrigFilename:FNameStr;
begin
  GetSEAAXEOrigFilename := CopyUntil (BUF_GetString (207, 12), #0);
end;

{ Crunch }

function IsCrunch:boolean;
begin
  IsCrunch := (BUF_GetLong (55) = $00510121);
end;

{ HackStop }

function IsUnregHS:boolean;
begin
  IsUnregHS := (upstring (HSRec.ID1) = 'HS') and
               (upstring (HSRec.ID2) = 'MSDOS');
end;

function GetUnregHSVersion:VersionStr;
begin
  GetUnregHSVersion := GetVersionStr (HSRec.VHi, HSRec.VLo);
end;

{ main }
function Check_EXE (StartPos:longint;
                    bExtended:boolean):boolean;
var Found:boolean;

    function NoteWrongHeader (s:String12):boolean;
    begin
      Noteln (concat ('Headersize is too ', s, ': ', i2s (EXEHeader.GetHeaderSize)));
      EXEHeader.CalculateHeadersize;
      Noteln (concat ('Calculated headersize: ', i2s (EXEHeader.GetHeaderSize)));

      if (EXEHeader.GetHeaderSize > b.FSize) then
      begin
        Noteln ('Header is still too large - skipping file');
        NoteWrongHeader := false;
      end
      else
        NoteWrongHeader := true;
    end;

begin
  if InitEXE (StartPos, bExtended) then
  begin
    Check_EXE := true;

    if WriteHeader ('DOS executable file') then
    begin
      Appendln (concat (' - ', i2s (b.FSize), ' bytes'));

{ check for errors in executable }
      if (EXEHeader.IsEuropeanEXE) then
        Noteln ('Note: Reverse "ZM" header was found!');

{ check error messages }
      if (EXEHeader.GetSizeInHeader < 32) then
      begin
        Noteln (concat ('Filesize in header is too small: ', i2s (EXEHeader.GetSizeInHeader),' - aborting'));
        exit;
      end;

      if (EXEHeader.GetHeaderSize < 32) and (StartPos > 0) then
        if not NoteWrongHeader ('small') then
          exit;

      if (EXEHeader.GetHeaderSize > b.FSize) then
        if not NoteWrongHeader ('large') then
          exit;

{ now start comparing EXEHeader }
      Found := false;

      { check if COM or EXE file
        if AnalyzeStartupCode returns false then it was a COM file ... }
      if AnalyzeStartupCode (Found) then
      begin
        { Borland TLink }
        if (IsTLink) then
        begin
          NoteLinker (concat ('TLink ', GetTLinkVersion));
          if (not Found) and (EXEHeader.RelocEntries = 0) then
            NoteCompiler ('maybe Turbo Assembler');
          Found := true;
        end;

{$ifdef GETTYP}
        { EXE/SYS device driver }
        if SYS_IsDeviceDriver then
        begin
          SYS_List; { SYSHeader wird in IsSYS gelesen }
          Found := true;
        end;
{$endif}

        { check registration info somewhere in the code ... }
        if (bInitialized) and (USER_NAME = '') then
          halt;
        { end check }

        if not Found then
        begin
          { PKLite }
          if (IsPKLite) then
          begin
            if (IsPKLiteExtraCompression) then
              NoteEXEPacker (concat ('PKLite ', GetPKLiteVersion, ' extra compression'))
            else
              NoteEXEPacker (concat ('PKLite ', GetPKLiteVersion));
            Found := true;
          end;

          { ExePack }
          if (IsExePack) then
          begin
            NoteEXEPacker (concat ('ExePack v', GetExePackVersion));
            Found := true;
          end;

          { SEA AXE }
          if (IsSEAAXE) then
          begin
            NoteEXEPacker (concat ('SEA-AXE ', GetSEAAXEVersion));
            Noteln (concat ('Original filename: ', GetSEAAXEOrigFilename));
            Found := true;
          end;

          { CRUNCH }
          if (IsCrunch) then
          begin
            NoteEXEPacker ('Crunch 1.0');
            Found := true;
          end;

          { HackStop EXE protector }
          if (IsUnregHS) and (not Found) then
          begin
            Noteln (concat ('Found ID of HackStop ', GetUnregHSVersion));
            Found := true;
          end;
        end;

             if (NE_ID = 'LE') then WriteLXInfo (Check_EXE, NE_Offset)
        else if (NE_ID = 'LX') then WriteLXInfo (Check_EXE, NE_Offset)
        else if (NE_ID = 'NE') then WriteNEInfo (Check_EXE, NE_Offset)
        else if (NE_ID = 'PE') then WritePEInfo (Check_EXE, NE_Offset)
        else
        begin
          { - if still nothing was found check if it may be compressed - }
          if (not Found) and (EXEHeader.RelocEntries = 0) then
              Noteln ('Probably compressed or assembler coded');

          WriteOverlayInfo (Check_EXE,
                            EXEHeader.GetSizeInHeader,
                            EXEHeader.GetOverlaySize (b.FSize),
                            Found,
                            false); { do not ignore empty overlays }
        end;
      end;
    end;
  end
  else
    Check_EXE := false;
end;

end.

