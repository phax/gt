{$i aflags.inc}

unit GTSound;

interface

function Check_Sound:boolean;

implementation

uses
     GTBuf, GTFile, GTOutput,
     XGlobal, XString;

function Check_AU:boolean;
begin
  if (b.First.l [1] = $646E732E) then
  begin
    Check_AU := true;
    if (WriteHeader ('SUN sound file')) then
    begin
      FinishLine;
    end;
  end
  else
    Check_AU := false;
end;

function Check_MIDI:boolean;
type
     MIDIHeader = record
       ID:array[1..4] of char;
       HeaderLen:longint;
       Format:word;
       TrackBlocks:word;
       DeltaTime:integer;
     end;
const
      MIDI_H_SIZE = SizeOf (MIDIHeader);
var H:MIDIHeader;
begin
  BUF_GetBuf (H, 1, MIDI_H_SIZE);
  if (H.ID = 'MThd') then
  begin
    Check_Midi := true;
    if WriteHeader ('MIDI music file') then
    begin
      FinishLine;

      IncInsLevel;
      case (H.Format and $00FF) of
        0:Note ('One multi-channel track');
        1:Note ('One or more simultaneous tracks');
        2:Note ('One or more sequentially independent single-track patterns');
      else
        Note (concat ('Unknown track format ', i2s (H.Format and $00FF)));
      end;
      Appendln (concat (' with ', i2s (h.TrackBlocks), ' blocks'));
      DecInsLevel;
    end;
  end
  else
    Check_Midi := false;
end;

function Check_VOC:boolean;

    function Byte2SR (b:byte):longint;
    begin
      Byte2SR := 1000000 div (256 - b);
    end;

type
     VOCHeader = record
       ID:array [1..19] of char;
       Res1:byte;  { $1A }
       DataOfs:word;
       VerLow:byte;
       VerHigh:byte;
       Comp2:word;  { + $1234 }
     end;

     DataRecord = longint;

const
      VOC_H_SIZE = SizeOf (VOCHeader);
      DATA_SIZE  = SizeOf (DataRecord);

var Head:VOCHeader;
    Data:DataRecord;
    DataLen:longint;
    DataID:byte;
    w1:word;
    b1, b2:byte;
begin
  BUF_GetBuf (Head, 1, VOC_H_SIZE);
  if (Head.ID = 'Creative Voice File') then
  begin
    Check_VOC := true;
    if WriteHeader (concat ('Creative Voice ',
                             GetVersionStr (Head.VerHigh, Head.VerLow),
                            ' VOC file')) then
    begin
      FinishLine;

      Noteln ('Listing block types:');
      IncInsLevel;

      TFile_Init (Head.DataOfs);
      repeat
        if (TFile_ReadActBuf (Data, DATA_SIZE) <> DATA_SIZE) then;

        DataLen := Data shr 8;
        DataID  := Data and $FF;

        case DataID of
          0:begin
              if (DataLen <> 0) then
                Noteln ('May be error in VOC file');
            end;
          1:begin
              b1 := TFile_ReadActByte;
              b2 := TFile_ReadActByte;
              Note (concat ('Sound data: ', i2s (Byte2SR (b1)), ' Hz, '));
              case b2 of
                0: Append ('8 bits');
                1: Append ('4 bits');
                2: Append ('2.6 bits');
                3: Append ('2 bits');
              else
                Append (concat ('multi DAC with ', i2s (b2 - 3), ' channels'));
              end;
              FinishLine;
              TFile_IncFilePos (DataLen - 2);
            end;
          2:begin
              Noteln ('Sound continue');
              TFile_IncFilePos (DataLen);
            end;
          3:begin
              w1 := TFile_ReadActWord;
              if (TFile_ReadActByte = 0) then;   { 0 is just a dummy ... }
              Noteln (concat ('Silence for ', r2sfd (w1 / 1000, 0, 1), ' sec'));
              TFile_IncFilePos (DataLen - 3);
            end;
          4:begin
              w1 := TFile_ReadActWord;
              Noteln (concat ('Marker ', i2s (w1)));
            end;
          5:begin
              Noteln ('ASCII');
              TFile_IncFilePos (DataLen);
            end;
          6:begin
              w1 := TFile_ReadActWord;
              if (w1 = $FFFF) then
                Noteln ('Repeat endless')
              else
                Noteln (concat ('Repeat ', i2s (w1 - 1), ' times'));
              IncInsLevel;
            end;
          7:begin
              DecInsLevel;
              Noteln ('End repeat');
            end;
          8:begin
              Noteln ('Extended');
              TFile_IncFilePos (DataLen);
            end;
        else
          Noteln (concat ('Unknown block type ', i2s (DataID)));
          TFile_IncFilePos (DataLen);
        end;

      until (not TFile_IsOkay) or (DataID = 0);

      DecInsLevel;
    end;
  end
  else
    Check_VOC := false;
end;

function Check_MP3:boolean;
type
     TMP3SingleTable = array[0..14] of byte;
     TMP3Table = array[1..2, 1..3] of TMP3SingleTable;
const
      MP3Table:TMP3Table = (
      (
      { Tab448 } ( 0, 4, 8,12,16,20,24,28,32,36,40,44,48,52,56),
      { Tab384 } ( 0, 4, 6, 7, 8,10,12,14,16,20,24,28,32,40,48),
      { Tab320 } ( 0, 4, 5, 6, 7, 8,10,12,14,16,20,24,28,32,40)
      ),
      (
      { Tab256 } ( 0, 4, 6, 7, 8,10,12,14,16,18,20,22,24,28,32),
      { Tab160 } ( 0, 1, 2, 3, 4, 5, 6, 7, 8,10,12,14,16,18,20),
      { Tab160 } ( 0, 1, 2, 3, 4, 5, 6, 7, 8,10,12,14,16,18,20)
      ));

var nLayer, nVersion, nTableIndex, nMode:byte;
begin
  if (b.First.b [1] = $FF) and
     (b.First.b [2] shr 4 = $0F) and
     (b.First.l [1] <> longint ($FFFFFFFF)) then  { FFFFFFFF is a driver .. }
  begin
    Check_MP3 := true;

    (*
    take bits 5 and 6
    nLayer := (b.First.b [2] shr 5) and BITS[1];
    *)

    { take bits 2 and 3 }
    nLayer   := 4 - ((b.First.b [2] shr 1) and 3);
    nVersion := 2 - ((b.First.b [2] shr 3) and 1);

    if WriteHeader (concat ('MPEG ', i2s (nVersion), ' Layer ', i2s (nLayer), ' file')) then
    begin
      FinishLine;

      nTableIndex := b.First.b[3] shr 4;
      if (nTableIndex <= 14) and (nLayer <= 3) and (nVersion <= 2) then
        Noteln (concat ('Bitrate: ', i2s (MP3Table [nVersion, nLayer, nTableIndex] * 8), ' KB/sec'));

      Note ('Error protection: ');
      if (b.First.b [2] and BITS[7] = 0) then
        Appendln ('yes')
      else
        Appendln ('no');

      nMode := b.First.b[4] shr 6;
      case (nMode) of
        0:Noteln ('Stereo');
        1:Noteln ('Joint stereo');
        2:Noteln ('Two channels');
        3:Noteln ('Mono');
      end;
    end;
  end
  else
    Check_MP3 := false;
end;

function Check_Sound:boolean;
begin
  Check_Sound := true;
  if (not Check_AU) then
  if (not Check_MIDI) then
  if (not Check_MP3) then
  if (not Check_VOC) then
    Check_Sound := false;
end;

end.
