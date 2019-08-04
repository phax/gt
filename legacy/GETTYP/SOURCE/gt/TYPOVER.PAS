{$i aflags.inc}

unit TypOver;

interface

uses
     GTGlobal;

var
    nAbsOverlayPos:longint;

procedure WriteOverlayInfo (    aProc:TEXEProc;
                                nRelativeOverlayPos:longint;
                                nOverlaySize:longint;
                            var bFound:boolean;
                                bIgnoreEmptyOverlays:boolean);

implementation

uses
     GTBuf, GTCompar, GTData__, GTFile, GTMem, GTOutput,
{$ifdef GETTYP}
     GTPack,
{$endif}
     XGlobal, XString;

const
      { must be greater than 256 !!
        else difficulties with TOverlayBuffer arouse }
      MAX_OVERLAY_BUFFER = 2048;
      MAX_PACKER_CHECK   = 512;

type
     TOverlayBuffer = record
       case byte of
         0:(data:array[1..MAX_OVERLAY_BUFFER] of byte);
         1:(text:array[1..MAX_OVERLAY_BUFFER] of char);
         2:(First:TBuf256;
            buf2:array[257..MAX_OVERLAY_BUFFER] of byte);
         3:(PGMPAK_ID:byte;
            PGMPAK_Str:array[1..6] of char;
            PGMPAK_res:byte;
            PGMPAK_Version:array[1..4] of char);
     end;

     TOverlayType = (OVR_PGMPAK,
                     OVR_HACKSTOP,
                     OVR_JERUSALEM,
                     OVR_MCAFEE,
                     OVR_BOR_OVR,
                     OVR_ARJ_SFX,
                     OVR_ZEROES,
                     OVR_MS_DEBUG,
                     OVR_PMWLITE,
                     OVR_BOR_DEBUG,
                     OVR_EXE,
                     OVR_MINICHAINER,
                     OVR_UNKNOWN);

var
    OverlayBuffer:TOverlayBuffer;
    nOverlayBufferLen:word;
    OverlayBufStr:string;

function GetTypeOfOverlay (nRelativeOverlayPos:longint):TOverlayType;
var nOvrBufStrLen:word;
begin
  GetTypeOfOverlay := OVR_UNKNOWN;

  TFile_Init (nAbsOverlayPos + nRelativeOverlayPos);

  nOverlayBufferLen := TFile_ReadActBuf (OverlayBuffer, MAX_OVERLAY_BUFFER);
  { - this is possible when the whole overlay is empty - }
  if (nOverlayBufferLen = 0) then
  begin
    Noteln ('internal error GTOO');
    exit;
  end;

  nOvrBufStrLen := min_w (nOverlayBufferLen, word (255));
  move (OverlayBuffer.data [1], OverlayBufStr [1], nOvrBufStrLen);
  SetLength (OverlayBufStr, nOvrBufStrLen);

  { possible problem: SizeOf (CompareBuf) > nOverlayBufLen }
  move (OverlayBuffer.data [1], CompareBuf [0], SizeOf (CompareBuf));

{ PGMPAK }
  if (nOverlayBufferLen = 12) and  (OverlayBuffer.PGMPAK_Str = 'PGMPAK') then
    GetTypeOfOverlay := OVR_PGMPAK
  else
{ HackStop register information }
  if (copy (OverlayBufStr, 1, 8) = #13#10'RegStr') then
    GetTypeOfOverlay := OVR_HACKSTOP
  else
{ ANTI VIRUS }
  if (copy (OverlayBufStr, 1, 5) = 'MsDos') then
    GetTypeOfOverlay := OVR_JERUSALEM
  else
{ McAfee signature }
  if (copy (OverlayBufStr, 1, 12) = '>>>McA1ph<<<') then
    GetTypeOfOverlay := OVR_MCAFEE
  else
{ Borland overlay }
  if (copy (OverlayBufStr, 1, 4) = 'FBOV') then
    GetTypeOfOverlay := OVR_BOR_OVR
  else
{ Borland overlay }
  if (copy (OverlayBufStr, 1, 7) = 'ARJ_SFX') then
    GetTypeOfOverlay := OVR_ARJ_SFX
  else
{ UNKNOWN }
  case (OverlayBuffer.First.w [1]) of
    $0000:GetTypeOfOverlay := OVR_ZEROES;
    $424E:GetTypeOfOverlay := OVR_MS_DEBUG;
    $4D50:GetTypeOfOverlay := OVR_PMWLITE;
    $52FB:GetTypeOfOverlay := OVR_BOR_DEBUG;
    $5A4D:GetTypeOfOverlay := OVR_EXE;
    $A5B2:GetTypeOfOverlay := OVR_MINICHAINER;
  end;
end;

procedure WriteOverlayInfo (    aProc:TEXEProc;
                                nRelativeOverlayPos:longint;
                                nOverlaySize:longint;
                            var bFound:boolean;
                                bIgnoreEmptyOverlays:boolean);
label Analyze, OverlayEnd;
var
    bWriteOvrHeader:boolean;
{$ifdef GETTYP}
    i, nMaxLoop:word;
    nOldSize:longint;
{$endif}

    procedure FoundOverlayType (s:string);
    begin
      bFound := true;
      NoteOverlay (s);
    end;

    procedure DOSExecutable;
    const SIZE1 = SizeOf (TBuffer);
          SIZE2 = SizeOf (TOverlayBuffer);
    var Old_BufLen:word;
        nRealOverlaySize,
        Old_Overlay_StartPos:longint;
        MS:TMemStruct;
    begin
      bFound := true;

{$ifdef MSDOS}
{$ifndef DPMI}
{$ifdef SSP}
      Noteln (i2s (SPtr));
{$endif}
      if (SPtr < 4000) then
      begin
        Noteln ('DOS executable file');
        EmptyLine;
        Noteln ('Sorry, out of stack - must have incredible overlays ...');
        exit;
      end;
{$endif}
{$endif}

      nRealOverlaySize := nOverlaySize;
      Old_BufLen := b.BufLen;

      {
      NOTE:
      I copy the whole TBuffer object into high memory
      but NOT the content of pointer!!
      The pointer should be unique so far!!
      And the pointer is not needed afterwards ;)
      }
      MS.bInUse := false;
      GetMemory (MS, SIZE1 + SIZE2);
      CopyToMemoryAt (b, MS, SIZE1, 0);
      CopyToMemoryAt (OverlayBuffer, MS, SIZE2, SIZE1);

      b.FSize := nRealOverlaySize;

      Old_Overlay_StartPos := nAbsOverlayPos;
      inc (nAbsOverlayPos, nRelativeOverlayPos);

      TFile_Init (nAbsOverlayPos);
      b.BufLen := TFile_ReadActBuf (b.p^, min_l (Old_BufLen, nRealOverlaySize));

      {
      StartPosInOverlay is needed if their are some zeros in front
      of the overlay
      }
      { RECURSIVE CALL !! }
      if (aProc (nAbsOverlayPos, false)) then;

      CopyFromMemoryAt (MS, b, SIZE1, 0);
      CopyFromMemoryAt (MS, OverlayBuffer, SIZE2, SIZE1);
      FreeMemory (MS);

      nAbsOverlayPos := Old_Overlay_StartPos;
    end;

    procedure PMWLite;
    var nMinor, nMajor:byte;
    begin
      nMajor := OverlayBuffer.First.b[5];
      nMinor := OverlayBuffer.First.b[6];
      Noteln (concat ('Packer: PMWLite ', i2s (nMajor), '.', i2s (nMinor)));
    end;

    function SkipZeroes:word;
    var i:word;
    begin
      bFound := true;
      i := 1;
      while (i <= nOverlayBufferLen) and (OverlayBuffer.data [i] = 0) do
        inc (i);
      dec (i);
      Noteln (concat ('Zero padded (', i2s (i), ' bytes)'));

      inc (nRelativeOverlayPos, i);  { adpat code size }
      dec (nOverlaySize, i);         { adapt overlay size }

      SkipZeroes := i;
    end;

    procedure AppendPercentage (nOffset:longint);
    var n, n2:longint;
    begin
      n := nOffset + nRelativeOverlayPos;
      n2 := n + nOverlaySize;
      Appendln (concat (i2s (n), ' of ', i2s (n2),
                        ' (= ', r2sfd (n / n2 * 100, 0, 1), '%)'));
    end;

    procedure WriteOverlayHeader;
    begin
      if (bWriteOvrHeader) then
      begin
        EmptyLine;

        Noteln (concat ('Found ', i2s (nOverlaySize), ' bytes overlay'));

        IncInsLevel;

        { - if relative and absolute position are identical than output only
            absolute position - }
        if (nAbsOverlayPos > 0) then
        begin
          Note ('Relative position: ');
          AppendPercentage (0);
        end;
        Note ('Absolute position: ');
        AppendPercentage (nAbsOverlayPos);

        DecInsLevel;

        bWriteOvrHeader := false;
      end;
    end;

    function CheckHeadText:boolean;
    { if changing this also change in GTBuf }
    var p:byte;
    begin
      p := 1;
      while (OverlayBuffer.text [p] in TextChars) and (p < 255) do
        inc (p);
      dec (p);

    { - check if a generic string was found - }
      if (p >= 7) then  { min length: 8 }
      begin
        Noteln (concat ('Headtext: "', copy (OverlayBufStr, 1, p), '"'));
        CheckHeadText := true;
      end
      else
        CheckHeadText := false;
    end;

var aType:TOverlayType;
begin
  bWriteOvrHeader := true;
  bFound := false;

  if (nOverlaySize > 0) and (nRelativeOverlayPos > 0) then
  begin
Analyze:
    if (nOverlaySize = 0) then
      exit;

    aType := GetTypeOfOverlay (nRelativeOverlayPos);
    if (aType = OVR_UNKNOWN) and (bIgnoreEmptyOverlays) then
      exit;

    WriteOverlayHeader;

    IncInsLevel;

{ EFP }
    if ComparePtrBuf (@REC_EFP123) then
      bFound := true
    else
{ PGMPAK }
    if (aType = OVR_PGMPAK) then
    begin
      NoteEXEPacker (concat ('PGMPAK ', OverlayBuffer.PGMPAK_Version));
      bFound := true;
    end
    else
{ HackStop register information }
    if (aType = OVR_HACKSTOP) then
      FoundOverlayType ('HackStop registration info')
    else
{ ANTI VIRUS }
    if (aType = OVR_JERUSALEM) then
      FoundOverlayType ('Virus protection: Jerusalem Virus')
    else
{ McAfee signature }
    if (aType = OVR_MCAFEE) then
      FoundOverlayType ('McAfee checksum')
    else
{ Borland overlay }
    if (aType = OVR_BOR_OVR) then
      FoundOverlayType ('Borland overlay')
    else
{ Borland overlay }
    if (aType = OVR_ARJ_SFX) then
    begin
      FoundOverlayType ('ARJ SFX header');
      inc (nRelativeOverlayPos, 16);  { adpat code size }
      dec (nOverlaySize, 16);         { adapt overlay size }
      DecInsLevel;
      goto Analyze;
    end
    else
{ UNKNOWN }
    if (aType = OVR_ZEROES) then
    begin
      if (SkipZeroes > 0) then  { increases StartPosInOverlay }
      begin
        DecInsLevel;
        goto Analyze;
      end;
    end
    else
    if (aType = OVR_MS_DEBUG) then
      FoundOverlayType ('Microsoft debug info')
    else
    if (aType = OVR_PMWLITE) then
      PMWLite
    else
    if (aType = OVR_BOR_DEBUG) then
      FoundOverlayType ('Borland debug info')
    else
    if (aType = OVR_EXE) then
      DOSExecutable
    else
    if (aType = OVR_MINICHAINER) then
      FoundOverlayType ('Mini Chainer')
    else
    begin
{$ifdef GETTYP}
      { known bug: b should be adopted all the time ... }

      InitPackBuffer (nAbsOverlayPos + nRelativeOverlayPos,
                      nOverlaySize);
      { check for packers in the first MAX_PACKER_CHECK bytes
        but only if the overlay size is also okay.
        to be sure to compare no empty array, leave at least 4 bytes rest }

      if (nOverlayBufferLen > MAX_PACKER_CHECK) then
        nMaxLoop := nOverlayBufferLen
      else
        nMaxLoop := (nOverlayBufferLen and $0000FFFF) - 1;

      for i:=0 to nMaxLoop do
      begin
        CheckForKnownPackers (nAbsOverlayPos + nRelativeOverlayPos + i,
                              nOverlaySize - i,
                              false);

        if (GetPackerID <> NONE) then
        begin
          { save file size }
          nOldSize := b.FSize;
          b.FSize := nOverlaySize - i;

          Noteln (concat ('Found archive at position ', i2s (i), ' in overlay'));
          NotePacker;
          { restore filesize }
          b.FSize := nOldSize;
          bFound := true;
          RestorePackBuffer;  { free XMS handle ...!! }
          goto OverlayEnd;
        end;

        ShiftPackBuffer; { shift b.p^ one to the left -
                           don't have to read it from the file -
                           faster! }
      end;
      RestorePackBuffer;

      if (CheckHeadText) then
      begin
        bFound := true;
        goto OverlayEnd;
      end;

{$endif}
      { this is only displayed if no archive was found. if an archive was
        found the method jumps to the label OverlayEnd }
      NoteOverlay (concat ('unknown (', hexword (OverlayBuffer.first.w [1]), 'h)'));
    end;

OverlayEnd:
    DecInsLevel;
  end;
end;

begin
  nAbsOverlayPos := 0;
end.
