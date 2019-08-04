{$i aflags.inc}

unit TypRIFF;

interface

function Check_RIFF:boolean;

implementation

uses
     GTBuf, GTFile, GTOutput,
     XString;

var
    sFormat:string;

procedure Check_Wave;
type
     WAVHeader = record
       sound_size:longint;
       ID_WAVE:array[1..4] of char;
       WAV_Length:longint;
       l:longint;
       Typ:word;
       Channels:word;
       SamplesPerSec:longint;
       AverageBytesPerSec:longint;
       BlockAlign:word;
       Bits:word;
       ID_data:array[1..4] of char;
       data_size:longint;
     end;

const
      WAV_H_SIZE = SizeOf (WAVHeader);

var H:WAVHeader;
    bCompressed:boolean;
    PlayTime:real;
    min, sec, hsec:word;
begin
  move (b.First.l[2], H, sizeof (WAVHeader));
  if WriteHeader ('Wave music file (WAV)') then
  begin
    bCompressed := (H.Typ > 1);
    if bCompressed then
      Append (concat (' (compressed - type ', i2s (H.Typ), ')'));
    FinishLine;

    IncInsLevel;

    Noteln (concat ('Channels: ', i2s (H.Channels)));
    Noteln (concat ('Sampling rate: ', i2s (H.SamplesPerSec)));

    Note ('Data size: ');
    if bCompressed then
    begin
      if H.Bits = 0 then
        Appendln ('found error in data structure')
      else
        Appendln (concat (i2s (H.sound_size), ' bytes (decompressed: ',
                  r2sfd (16 / H.Bits * (H.sound_size - WAV_H_SIZE), 0, 0),' bytes)'));
    end
    else
      Appendln (concat (i2s (H.data_size), ' bytes'));

    Note (concat ('Bits: ', i2s (H.Bits)));
    if bCompressed then
    begin
      if (H.AverageBytesPerSec > 0) then
        Append (concat (' (decompressed: ', r2sfd (4.0 * H.bits * H.SamplesPerSec / H.AverageBytesPerSec, 0, 0), ' bytes)'))
      else
        Append (' (error in data)');
    end;
    FinishLine;

    PlayTime := H.sound_size / H.AverageBytesPerSec;
    if PlayTime > 60 then
    begin
      min  := Trunc (PlayTime / 60);
      sec  := Trunc (PlayTime - (min * 60));
      hsec := Trunc ((PlayTime - (min * 60) - sec) * 100);
    end
    else
    begin
      min  := 0;
      sec  := Trunc (PlayTime);
      hsec := Trunc ((PlayTime - sec) * 100);
    end;
    Noteln (concat ('Playtime: ', i2s (min), ':',
                                  LeadingZero (sec, 2), ',',
                                  LeadingZero (hsec, 2), ' min'));
    DecInsLevel;
  end;
end;

{

gehe zu startposition
lese 8 byte
wenn erweiterte ID, dann
  lese 4 byte
  analysiere naechste stufe

}

type
      TStringLong = record
        case byte of
          1: (aName:array[1..4] of char);
          2: (nType:longint);
      end;

procedure Check_Generic;
const MAX_OUTPUT = 20;
var sLine:string;
    nOutputLines:byte;

    function IsAnExtendedID (n:longint):boolean;
    begin
      IsAnExtendedID := (n = $46464952) or { "RIFF" }
                        (n = $5453494C);   { "LIST" }
    end;

    procedure Analyze (nStartPos:longint;
                       nMaxSize:longint);
    var nAbsPos, nUsed, nRead:longint;
        aMainType:record
          aID:TStringLong;
          nSize:longint;
        end;
        cSpace:char;
        bTooLarge:boolean;
    const
          MAIN_SIZE = sizeof (aMainType);
          SUB_SIZE = sizeof (TStringLong);
    begin
      nAbsPos := nStartPos;
      nUsed := 0;
      repeat
        { go to start position }
        TFile_GotoFilePos (nAbsPos);
        { read 8 bytes }
        nRead := TFile_ReadActBuf (aMainType, MAIN_SIZE);
        if (nRead = MAIN_SIZE) then
        begin
          { inc the size of the currently used bytes }
          if (nUsed + aMainType.nSize > nMaxSize) then
          begin
            Noteln (concat (sLine, 'Data (', i2s (nMaxSize - nUsed), ' bytes)'));
            exit;
          end;

          { inc the counter with the number of bytes read }
          inc (nUsed, MAIN_SIZE);
          inc (nAbsPos, MAIN_SIZE);

          bTooLarge := ((nAbsPos + aMainType.nSize - nStartPos) >= nMaxSize);

{$ifdef MSDOS}
          if (bTooLarge) then
            cSpace := 'À'
          else
            cSpace := 'Ã';
          Noteln (concat (sLine, cSpace, 'Ä', aMainType.aID.aName));
{$else}
          if (bTooLarge) then
            cSpace := '+'
          else
            cSpace := '+';
          Noteln (concat (sLine, cSpace, '-', aMainType.aID.aName));
{$endif}

          inc (nOutputLines);

          { check if too much output was made ... }
          if (nOutputLines >= MAX_OUTPUT) then
          begin
            Noteln ('Truncating listing ...');
            exit;
          end;

          { check if it is an extended ID or not }
          if (IsAnExtendedID (aMainType.aID.nType)) then
          begin
            { because the next 4 bytes are and ID }
            inc (nAbsPos, SUB_SIZE);
            inc (nUsed, SUB_SIZE);

            if (bTooLarge) then
              sLine := concat (sLine, '  ')
            else
{$ifdef MSDOS}
              sLine := concat (sLine, '³ ');
{$else}
              sLine := concat (sLine, '| ');
{$endif}

            Analyze (nAbsPos, aMainType.nSize - SUB_SIZE);
            SetLength (sLine, length (sLine) - 2);

            { check if too much output was made ... }
            if (nOutputLines >= MAX_OUTPUT) then
              exit;

            { because the size is within the 4 bytes for the extended ID }
            dec (nAbsPos, SUB_SIZE);
            dec (nUsed, SUB_SIZE);
          end;
          inc (nUsed, aMainType.nSize);
          inc (nAbsPos, aMainType.nSize);
        end;
      until ((nAbsPos - nStartPos) >= nMaxSize) or (nRead < MAIN_SIZE);
    end;

begin
  if (WriteHeader (concat ('RIFF file: ', sFormat))) then
  begin
    FinishLine;

    sLine := '';
    nOutputLines := 0;
    TFile_Init (0);
    Analyze (0, b.FSize);
  end;
end;

function Check_RIFF:boolean;
begin
  Check_RIFF := false;
  if (b.First.l [1] = $46464952) then { "RIFF" }
  begin
    Check_RIFF := true;
    if (b.First.l [3] = $45564157) then { "WAVE" }
      Check_Wave
    else { generic }
    begin
      { "CDR6" - "CDR8" }
      if (b.First.l [3] = $36524443) then
        sFormat := concat ('Corel Draw 6 graphic file (CDR)')
      else
      if (b.First.l [3] = $37524443) then
        sFormat := concat ('Corel Draw 7 graphic file (CDR)')
      else
      if (b.First.l [3] = $38524443) then
        sFormat := concat ('Corel Draw 8 graphic file (CDR)')
      else
      if (b.First.l [3] = $6B626673) then
        sFormat := 'EMU SoundFont bank'
      else
      if (b.First.l [3] = $20495641) then
        sFormat := 'Animation Video (AVI)'
      else
        sFormat := concat ('Unknown RIFF type "', BUF_GetString (9, 4), '"');

      Check_Generic;
    end;
  end;
end;

end.