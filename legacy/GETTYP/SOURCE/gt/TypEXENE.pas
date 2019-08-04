{$define extended}
{$i aflags.inc}

unit TypEXENE;

interface

uses
     GTGlobal;

procedure WriteNEInfo (aProc:TEXEProc;
                       NE_Offset:longint);

implementation

uses
     GTBuf, GTCompar, GTDataNE, GTFile, GTOutput, GTRegist,
     TypOver,
     XEXE_NE, XGlobal, XString;

function CheckForCode (nEntryPoint:longint):boolean;
label Done;
var Found:boolean;
    i:word;

    procedure __ComparePtrCode (p:PCompPtrRec);
    begin
      if ComparePtrBuf (p) then
        Found := true;
    end;

begin
  Found := false;

  if (nEntryPoint > 0) and (nEntryPoint < b.FSize) then
  begin
    TFile_ReadBufAt (nEntryPoint, CompareBuf, SizeOf (CompareBuf));

    for i:=1 to COUNT_NE do
      if ComparePtrBuf (@REC_NE [i]) then
      begin
        Found := true;
        goto Done;
      end;
  end;

Done:
  CheckForCode := Found;
end;

procedure WriteNEInfo (aProc:TEXEProc;
                       NE_Offset:longint);
var H:TEXE_NE;
    S:NE_Segment_Handler;
    nCalculatedFileSize, nEntryPoint:longint;
    dummy:boolean;

    function CalcEntryPoint:longint;
    var n:longint;
    begin
      n := S.GetEntryPoint (H.nCS, H.nIP);

      { - cannot use case .. of because it is a longint - }
      if (n = NE_SEG_ERR_NO_ENTRYPOINT) then
        Noteln ('File has no entrypoint')
      else
        if (n = NE_SEG_ERR_TOO_LARGE) then
          Noteln ('Error: CS is greater than the segment count.')
        else
          if (n = NE_SEG_ERR_EMPTY) then
            Noteln ('Error: segment itself is empty')
          else
            if (n < 0) or (n > b.FSize) then
            begin
              Noteln ('Error: entrypoint is out of file bounds.');
              n := -1;
            end;

      CalcEntryPoint := n;
    end;

    procedure ListNonResidentNames;
    begin
      TFile_GotoFilePos (H.nAbsoluteOfsToNonresidentNamesTable);
      Noteln (concat ('"', TFile_ReadActPascalString, '"'));
    end;

    procedure WriteInfo;
    var i:byte;
    begin
      i := H.ApplicationFlags and 7;  { bits 0 - 2 }
      case i of
        1:Note ('Full screen application.');
        2:Note ('Application is compatible with Windows API.');
        0,
        3:Note ('Standard Windows application');
      else
        Note (concat ('Unknown type ', i2s (i)));
      end;
      if (H.ApplicationFlags and BITS [7] > 0) then
        Append (' (DLL or driver)');
      FinishLine;

      if (H.ApplicationFlags and BITS [3] > 0) then
        Noteln ('File has a selfloader (unpacker???)');

      Note ('Target operating system: ');
      i := H.TargetOS;
      case i of
        $00: Append ('unknown');
        $01: Append ('OS/2');
        $02: Append ('Windows ' + GetVersionStr (H.nExpectedWinVersionMajor, H.nExpectedWinVersionMinor));
        $03: Append ('European MS-DOS 4.x');
        $04: Append ('Windows 386');
        $05: Append ('BOSS (Borland Operating System Services)');
        $81: Append ('Phar Lap DOS Extender 286 OS/2');
        $82: Append ('Phar Lap DOS Extender 286 Windows');
      else
        Append (concat ('unknwown (', i2s (i), ')'));
      end;
      FinishLine;

      { check registration info somewhere in the code ... }
      if (bInitialized) and (USER_NAME = '') then
        halt;
      { end check }

      i := H.ProgramFlags;
      if (i and BITS[0] > 0) then Noteln ('File is a dynamic-link library (DLL)');
      if (i and BITS[1] > 0) then Noteln ('File is a Windows application');
      if (i and BITS[3] > 0) then Noteln ('Protected mode only');
      if (i and BITS[4] > 0) then Noteln ('8086 instructions');
      if (i and BITS[5] > 0) then Noteln ('80286 instructions');
      if (i and BITS[6] > 0) then Noteln ('80386 instructions');
      if (i and BITS[7] > 0) then Noteln ('80x87 instructions');

      if (H.TargetOS = 1) then  { OS/2 }
      begin
        i := H.OtherEXEFlags;
        if (i and BITS[0] > 0) then Noteln ('Long filename support');
        if (i and BITS[1] > 0) then Noteln ('2.x protected mode');
        if (i and BITS[2] > 0) then Noteln ('2.x proportional fonts');
        if (i and BITS[3] > 0) then Noteln ('Has fast-load area');
      end;

      {
      w := H.ExpectedWinVersion;
      if (w > 0) then
        Noteln (concat ('Expected Windows version: ', GetVersionStr (hi (w), lo (w))));
      }

      Noteln (concat ('Linker version: ', GetVersionStr (H.nLinkerMajorVersion, H.nLinkerMinorVersion)));

      if (nEntryPoint > 0) then
      begin
        Note   (concat ('Entrypoint: ', i2s (nEntryPoint), ' / ', hexlong (nEntryPoint), 'h (CS:IP = '));
        if (H.HasSelfLoader) then
          Appendln ('0001h:0000h)')
        else
          Appendln (concat (hexword (H.nCS), 'h:', hexword (H.nIP), 'h)'));
      end;

      if (nCalculatedFileSize > 0) then
        Noteln (concat ('Calculated length: ', i2s (nCalculatedFileSize)));
    end;

    procedure ListSegments;
    var i:word;
    begin
      Noteln ('Segment listing:');
      IncInsLevel;

      { - headline - }
      Noteln ('Nr  Type  StartPos    EndPos     Size   Alloc');
      { - list segments  - }
      for i:=1 to H.nSegmentCount do
      begin
        Noteln (concat (i2sf (i, 2), '  ',
                        S.GetType (i), '  ',
                        hexlong (S.GetStartPos (i)), 'h - ', hexlong (S.GetEndPos (i)),'h  ',
                        hexlong (S.GetLength (i)), 'h  ',
                        hexlong (S.GetAllocSize (i)), 'h'
                        ));
      end;

      DecInsLevel;
    end;

label Done;
begin
  H.init (NE_Offset);

  EmptyLine;
  Noteln (concat ('New executable (starting at ', i2s (NE_Offset), ' for ',
                  i2s (b.FSize - NE_Offset), ' bytes)'));
  TFile_Init (NE_Offset);

  { - read NE header - }
  TFile_ReadActBuf (H, NE_HEADER_SIZE);

  { - init NE segment handler - }
  S.init (H.GetSectorSize, H.nSegmentCount, H.HasSelfLoader);

  { - read segments - }
  S.ReadFromFile (Handle, H.GetAbsSegmentTablePos);

  { - now it is safe to calc the entry point - }
  nEntryPoint := CalcEntryPoint;

  { if no segment table was found -> nCalculated size is NULL }
  nCalculatedFileSize := S.GetCalculatedFileSize;

  IncInsLevel;

  ListNonResidentNames;
  { if the entry point is invalid, it is -1, so check for <> 0!! }
  if (nEntryPoint > 0) then
  begin
    if (not CheckForCode (nEntryPoint)) then
    begin
      Noteln (FOUND_NO_MODIFIER);
      if (not bDoEXEHeaderAnyWay) then
      begin
        Noteln (USE_ZE);
        goto Done;
      end;
    end;
  end;

  WriteInfo;
  if (H.nSegmentCount > 0) then
    ListSegments;

Done:
  DecInsLevel;

  if (nCalculatedFileSize > 0) and
     (b.FSize > nCalculatedFileSize) then
  begin
    WriteOverlayInfo (aProc,
                      nCalculatedFileSize,
                      b.FSize - nCalculatedFileSize,
                      dummy,
                      false); { do not ignore empty overlays }
  end;

  { - important: release memory - }
  S.done;
end;

end.
