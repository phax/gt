{
Copyright (c) 1997, 98 by Philip Helger (philip@dke.univie.ac.at)
}

{$i aflags.inc}

unit GTImage;

interface

function Check_Image:boolean;

implementation

uses
     GTBuf, GTFile, GTGlobal, GTMem, GTOutput,
     XString;

const
      TrueColor = 16777216;

{-------------------------glob--------------------------}

procedure NoteNoResolution;
begin
  Noteln ('Vector data format - no resolution available');
end;

function GetResString (nWidth, nHeight, nColors:longint):string;
var s:string;
begin
  s := concat (i2s (nWidth), 'x', i2s (nHeight), STR_SEPERATOR);
  if (nColors > 0) then
    GetResString := concat (s, i2s (nColors), ' colors')
  else
    GetResString := concat (s, 'no color depth could be found');
end;

procedure NoteResolution (nWidth, nHeight, nColors:longint);
begin
  IncInsLevel;
  Noteln (GetResString (nWidth, nHeight, nColors));
  DecInsLevel;
end;

function WriteResolutionHeader (s:string;
                                nWidth:longint;
                                nHeight:longint;
                                nColors:longint):boolean;
begin
  WriteResolutionHeader := WriteHeader (concat (s, ' ', GetResString (nWidth, nHeight, nColors)));
end;

{-------------------------[BMF]-------------------------}

function Check_BMF:boolean;
type
     BMF_Header = record
       ID:word;
       ver:array[1..2] of char;
       nWidth:word;
       nHeight:word;
     end;
const
      BMF_H_SIZE = SizeOf (BMF_Header);
var
    H:BMF_Header;
begin
  Check_BMF := false;
  if (b.First.w[1] = $8A81) then
  begin
    BUF_GetBuf (H, 1, BMF_H_SIZE);
    Check_BMF := true;
    if WriteResolutionHeader (concat ('BMF ', h.ver[1], '.', h.ver[2], ' image'),
                                      H.nWidth, H.nHeight, 0) then
    begin
      FinishLine;
    end;
  end;
end;

{-------------------------[BMP]-------------------------}

function Check_BMP:boolean;
type
     { Win1 bitmaps }
     Win1_Header = record
       id:word;           { $00 }
       Width:word;
       Height:word;
       ByteWidth:word;
       Planes:byte;       { 1 }
       BitsPerPixel:byte; { 1, 4, 8 }
     end;

     { Win2-95 bitmaps }
     BMP_Main_Header = record
       ID:word;             { $4D42 }
       Size:longint;        { b.FSize if compressed - else 0 }
       Res1:word;           { $00 }
       Res2:word;           { $00 }
       StartOfs:longint;
       HeaderSize:longint;
       { Win2=12; Win3=WinNT=40; Win95=108 }
     end;

     { if BitsPerPixel = 1, 4 or 8
       entries = 1 shl BitsPerPixel
         or
       entries = (StartOfs -
                  SizeOf (HeaderWin2_1) -
                  SizeOf (HeaderWin2_2)) / SizeOf (Win2_Palette)
     }
     Win3_Pal_Entry = record
       Blue:byte;
       Green:byte;
       Red:byte;
     end;

     Win4_Pal_Entry = record
       Blue:byte;
       Green:byte;
       Red:byte;
       Reserved:byte;  { 0 }
     end;

     { if BPP = 16 or 32 else Win3_Palette }
     BMP_Bitfield_Mask = record
       RedMask:longint;
       GreenMask:longint;
       BlueMask:longint;
     end;

     { Win2 bitmaps }
     pWin2_Header = ^Win2_Header;
     Win2_Header = record
       Width:integer;
       Height:integer;
       Planes:word;         { 1 }
       BitsPerPixel:word;   { 1, 4, 8, 24 }
     end;

     { Win3 bitmaps }
     pWin3_Header = ^Win3_Header;
     Win3_Header = record
       Width:longint;
       Height:longint;
       Planes:word;            { 1 }
       BitsPerPixel:word;      { 3.x: 1, 4, 8, 24 }
                               {  NT: 1, 4, 8, 16, 24, 32 }
       Compression:longint;    { 3.x: 0, 1, 2 }
                               {  NT: 0, 1, 2, 3 if (BPP must be 16 or 32) }
       SizeOfBMP:longint;      { 0 if Compression is 0 }
       HorizRes:longint;
       VertRes:longint;
       ColsUsed:longint;       { if ColsUsed = 0 and BitsPerPixel < 16 then
                                   -> ColsUsed = 1 shl BitsPerPixel }
       ColsImportant:longint;  { if ColsImportant = 0 then
                                   -> ColsImportant = ColsUsed }
     end;

     TLongRGBValue = record
       X, Y, Z:longint;
     end;

     TLongRGB = record
       Red, Green, Blue:longint;
     end;

     { NT bitmaps }
     pWinNT_Header = ^WinNT_Header;
     WinNT_Header = Win3_Header;

     { 95 bitmaps }
     pWin95_Header = ^Win95_Header;
     Win95_Header = record
       Width:longint;
       Height:longint;
       Planes:word;            { 1 }
       BitsPerPixel:word;      { 1, 4, 8, 16, 24, 32 }
       Compression:longint;    { 0, 1, 2, 3 (BPP must be 16 or 32) }
       SizeOfBMP:longint;      { 0 if Compression is 0 }
       HorizRes:longint;
       VertRes:longint;
       ColsUsed:longint;       { if ColsUsed = 0 and BitsPerPixel < 16 then
                                   -> ColsUsed = 1 shl BitsPerPixel }
       ColsImportant:longint;  { if ColsImportant = 0 then
                                   -> ColsImportant = ColsUsed }
       Mask:TLongRGB;
       MaskAlpha:longint;
       CSType:longint;
       Red:TLongRGBValue;     { only used if CSType = 0 }
       Green:TLongRGBValue;
       Blue:TLongRGBValue;
       Gamma:TLongRGB;
     end;

     pOS2_Header = ^OS2_Header;
     OS2_Header = record
       Width:longint;
       Height:longint;
       BitPlanes:word;
       BitsPerPixel:word;
       Compression:longint;
       ImageDataSize:longint;
       XRes:longint;
       YRes:longint;
       ColorsUsed:longint;
       ColorsImportant:longint;
       Units:word;
       Res1:word;
       Recording:word;
       Rendering:word;
       Size1:longint;
       Size2:longint;
       ColorEncoding:longint;
       Identifier:longint;
     end;


const
      BMP_HM_SIZE   = SizeOf (BMP_Main_Header);
      BMP_H1_SIZE   = SizeOf (Win1_Header);
      BMP_H2_SIZE   = SizeOf (Win2_Header);
      BMP_H3_SIZE   = SizeOf (Win3_Header);
      BMP_HNT_SIZE  = SizeOf (WinNT_Header);
      BMP_H95_SIZE  = SizeOf (Win95_Header);
      BMP_PAL3_SIZE = SizeOf (Win3_Pal_Entry);
      BMP_PAL4_SIZE = SizeOf (Win4_Pal_Entry);
      BMP_MASK_SIZE = SizeOf (BMP_Bitfield_Mask);

{ for bitmaps }
      NoBMP = 0;
      Win1  = 1;
      Win2  = 2;
      Win3  = 3;
      WinNT = 4;
      Win95 = 5;
      OS2   = 6;

var
    bBMP:boolean;
    Head:BMP_Main_Header;
    BufLen:word;
    BufPtr:pointer;
    nVersion:byte;

    function GetVersionID:byte;
    begin
      GetVersionID := NoBMP;
      case Head.Headersize of
         12: GetVersionID := Win2;
         40: if (pWin3_Header (BufPtr)^.Compression = 3) then
               GetVersionID := WinNT
             else
               GetVersionID := Win3;
         64: GetVersionID := OS2;
        108: GetVersionID := Win95;
      end;
    end;

    procedure init;
    begin
      BUF_GetBuf (Head, 1, BMP_HM_SIZE);
      bBMP := ((Head.ID = $4D42) or { BM }
               (Head.ID = $4349)    { IC } ) and
              ((Head.HeaderSize = 12) or
               (Head.HeaderSize = 40) or
               (Head.HeaderSize = 64) or
               (Head.HeaderSize = 108));

      if (bBMP) then
      begin
        BufLen := Head.HeaderSize;

        GetMem (BufPtr, BufLen);
        CheckMemoryAllocation (BufPtr, BufLen, 'Check_BMP');

        BUF_GetBuf (BufPtr^, BMP_HM_SIZE + 1, BufLen);

        { BufPtr must be initialized before GetVersionID is called }
        nVersion := GetVersionID;
      end;
    end;

    procedure Done;
    begin
      if (bBMP) then
        freemem (BufPtr, BufLen);
    end;

    function GetWidth:longint;
    begin
      GetWidth := 0;
      case nVersion of
        Win2: GetWidth := pWin2_Header  (BufPtr)^.Width;
        Win3: GetWidth := pWin3_Header  (BufPtr)^.Width;
        WinNT:GetWidth := pWinNT_Header (BufPtr)^.Width;
        Win95:GetWidth := pWin95_Header (BufPtr)^.Width;
        OS2  :GetWidth := pOS2_Header   (BufPtr)^.Width;
      end;
    end;

    function GetHeight:longint;
    begin
      GetHeight := 0;
      case nVersion of
        Win2: GetHeight := pWin2_Header  (BufPtr)^.Height;
        Win3: GetHeight := pWin3_Header  (BufPtr)^.Height;
        WinNT:GetHeight := pWinNT_Header (BufPtr)^.Height;
        Win95:GetHeight := pWin95_Header (BufPtr)^.Height;
        OS2:  GetHeight := pOS2_Header   (BufPtr)^.Height;
      end;
    end;

    function GetBPP:word;
    begin
      GetBPP := 0;
      case nVersion of
        Win2: GetBPP := pWin2_Header  (BufPtr)^.BitsPerPixel;
        Win3: GetBPP := pWin3_Header  (BufPtr)^.BitsPerPixel;
        WinNT:GetBPP := pWinNT_Header (BufPtr)^.BitsPerPixel;
        Win95:GetBPP := pWin95_Header (BufPtr)^.BitsPerPixel;
        OS2:  GetBPP := pOS2_Header   (BufPtr)^.BitsPerPixel;
      end;
    end;

    function GetCompression:longint;
    begin
      GetCompression := 0;
      case nVersion of
        Win2: GetCompression := 0;
        Win3: GetCompression := pWin3_Header  (BufPtr)^.Compression;
        WinNT:GetCompression := pWinNT_Header (BufPtr)^.Compression;
        Win95:GetCompression := pWin95_Header (BufPtr)^.Compression;
        OS2:  GetCompression := pOS2_Header   (BufPtr)^.Compression;
      end;
    end;

    function GetVersionStr:ShortStr;
    begin
      case nVersion of
        Win2: GetVersionStr := '2 - Windows 2.x / OS/2 1.x';
        Win3: GetVersionStr := '3 - Windows 3.x';
        WinNT:GetVersionStr := '3 - Windows NT';
        Win95:GetVersionStr := '4 - Windows 95';
      else
        GetVersionStr := '3 - OS/2 2.x ??';
      end;
    end;

begin
  Check_BMP := false;
  Init;
  if (bBMP) then
  begin
    Check_BMP := true;
    if WriteResolutionHeader (concat ('Bitmap image (type ', GetVersionStr, ')'),
                              GetWidth, GetHeight, Exp2 (GetBPP)) then
    begin
      FinishLine;
    end;
  end;
  Done;
end;

{-------------------------[GIF]-------------------------}

function Check_GIF:boolean;
type
     GIFHeader = record
       ID:array[1..6] of char;
       nWidth:word;
       nHeight:word;
       nFlags:byte;
     end;

     TGIFLiteKennung = string[7];

const
      GIF_H_SIZE    = SizeOf (GIFHeader);
var s:ShortStr;
    GH:GIFHeader;

    function GetColors:word;
    begin
      GetColors := Exp2 ((GH.nFlags and 7) + 1);  { bits 1-3 }
    end;

    procedure GetLiteKennung (var k:TGIFLiteKennung);
    var StartPos:TPos;
        x:byte;
    begin
      StartPos := 13 + 3 * GetColors + 1;
      while (BUF_GetByte (StartPos) = 33) do { "!" }
      begin
        if (BUF_GetString (StartPos + 3, 7) = 'GIFLITE') then
          exit;
        StartPos := BUF_GetPosOfByte (StartPos, b.BufLen, 0);
        if StartPos = POSERROR then
          StartPos := b.BufLen
        else
          repeat
            inc (StartPos);
            x := BUF_GetByte (StartPos);
          until (x = 33) or (x = 44) or (StartPos >= b.BufLen);
      end;
      k := BUF_GetString (StartPos, 7);
    {
      Da fehlt was !!! (evtl)
    }
    end;

    function IsGIF:boolean;
    begin
      IsGIF := (GH.ID = 'GIF87a') or (GH.ID = 'GIF89a');
    end;

    function IsInterlaced:boolean;
    var StartPos, PalLen:word;
        x:byte;
    begin
      PalLen := 3 * GetColors;
      StartPos := 13 + PalLen + 1;
      while (BUF_GetByte (StartPos) = 33) do { "!" }
      begin
        StartPos := BUF_GetPosOfByte (StartPos, b.BufLen, 0);
        if StartPos = POSERROR then
          StartPos := b.BufLen
        else
          repeat
            inc (StartPos);
            x := BUF_GetByte (StartPos);
          until (x = 33) or (x = 44) or (StartPos >= b.BufLen);
      end;
      inc (StartPos, 9);
      IsInterlaced := (BUF_GetByte (StartPos) and 64 = 64);  { if bit 6 is set }
    end;

    function IsGIFLite:boolean;
    var k:TGIFLiteKennung;
    begin
      GetLiteKennung (k);
      IsGIFLite := (k = 'GIFLITE');
    end;

begin
  Check_GIF := false;
  BUF_GetBuf (GH, 1, GIF_H_SIZE);
  if IsGIF then
  begin
    Check_GIF := true;
    s := GH.ID;
    if WriteResolutionHeader (concat (s, ' image '), GH.nWidth, GH.nHeight, GetColors) then
    begin
      if (IsInterlaced) then
        Appendln (' [interlaced]')
      else
        Appendln (' [non interlaced]');

      if (IsGIFLite) then
        Noteln ('Packed with GIFLITE');
    end;
  end;
end;

{-------------------------[ICO]-------------------------}

function Check_ICO:boolean;
type
     ICOHeader = record
       ID:longint;
       NumberOfImgs:word;
     end;
const
      ICO_H_SIZE = SizeOf (ICOHeader);
var GH:ICOHeader;
begin
  Check_ICO := false;
  BUF_GetBuf (GH, 1, ICO_H_SIZE);
  if (GH.ID = $00010000) and
     (GH.NumberOfImgs > 0) then
  begin
    Check_ICO := true;
    if WriteHeader (concat ('Icon file with ', i2s (GH.NumberOfImgs), ' images')) then
    begin
      FinishLine;
    end;
  end;
end;

{-------------------------[IFF]-------------------------}

procedure AnalyzeIFF;
type
     IFFHeader = record
       ID:array[1..4] of char;
       Size:longint;
     end;

     IFF_BMHD = record
       Width:word;
       Height:word;
       Left:word;
       Top:word;
       BitPlanes:byte;
       Masking:byte;
       Compress:byte;
       Padding:byte;
       Transparency:word;
       XAspectRatio:byte;
       YAspectRatio:byte;
       PageWidth:word;
       PageHeight:word;
     end;
const
      IFF_LH_SIZE = SizeOf (IFFHeader);
      IFF_BMHD_SIZE = SizeOf (IFF_BMHD);
var H:IFFHeader;
    nSize, p:longint;

    procedure BMHD;
    var h:IFF_BMHD;
    begin
      if (TFile_ReadActBuf (h, IFF_BMHD_SIZE) <> IFF_BMHD_SIZE) then;

      Append (concat (i2s (swap (h.Width)), 'x', i2s (swap (h.Height)),
                       STR_SEPERATOR, i2s (Exp2 (h.BitPlanes)), ' colors'));
      if (h.Compress > 0) then
        Append (concat (STR_SEPERATOR, ' compressed'));
      FinishLine;
    end;

    procedure CMAP;
    var nSize:longint;
    begin
      SwapLong (h.Size, nSize);
      Appendln (concat ('Color table with ', i2s (nSize div 3), ' entries'));
    end;

    procedure NoteFormat;
    begin
      { otherwise problems with FPC }
      Note (h.ID);
      Note (' ð ');
      { textual data }
           if (h.ID = '(C) ') then Appendln ('Copyright notice and license')
      else if (h.ID = 'ANNO') then Appendln ('Annotation or comment')
      else if (h.ID = 'DOC ') then Appendln ('Document formating information')
      else if (h.ID = 'FOOT') then Appendln ('Document footer information')
      else if (h.ID = 'HEAD') then Appendln ('Document header information')
      else if (h.ID = 'PAGE') then Appendln ('Page break indicator')
      else if (h.ID = 'PARA') then Appendln ('Paragraph formatting information')
      else if (h.ID = 'PDEF') then Appendln ('Deluxe Print page definition')
      else if (h.ID = 'TABS') then Appendln ('Tab positions')
      else if (h.ID = 'TEXT') then Appendln ('Text for a paragraph')
      else if (h.ID = 'VERS') then Appendln ('File version')
      else if (h.ID = 'FTXT') then Appendln ('Formatted text')
      else if (h.ID = 'WORD') then Appendln ('Pro-write word processing')

      { graphical data }
      else if (h.ID = 'ACBM') then Appendln ('Amiga Contiguous Bitmap (MS Basic for Amiga)')
      else if (h.ID = 'DEEP') then Appendln ('IFF Deep (24-bit color image)')
      else if (h.ID = 'DR2D') then Appendln ('2D object standard format (vector data)')
      else if (h.ID = 'FNTR') then Appendln ('Raster font')
      else if (h.ID = 'FNTV') then Appendln ('Vector font')
      else if (h.ID = 'ILBM') then Appendln ('InterLeaved Bitmap (interleaved planar bitmap data)')
      else if (h.ID = 'PBM ') then Appendln ('Portable bitmap')
      else if (h.ID = 'PICS') then Appendln ('Macintosh picture')
      else if (h.ID = 'RGB8') then Appendln ('24-bit color image (Impulse)')
      else if (h.ID = 'RGBN') then Appendln ('12-bit color image (Impulse)')
      else if (h.ID = 'TDDD') then Appendln ('Turbo3d renderung data (3D objects)')
      else if (h.ID = 'YUVN') then Appendln ('YUV image data (V-Lab)')

      { animation data }
      else if (h.ID = 'ANBM') then Appendln ('Animated bitmap')
      else if (h.ID = 'ANIM') then Appendln ('Cel animations')
      else if (h.ID = 'SSA ') then Appendln ('Super smooth animation (ProDAD)')

      { video data }
      else if (h.ID = 'VDEO') then Appendln ('Deluxe Video Construction Set video')

      { sound data }
      else if (h.ID = '8SVX') then Appendln ('8-bit sampled voice')
      else if (h.ID = 'AIFF') then Appendln ('Audio interchange file format')
      else if (h.ID = 'SAMP') then Appendln ('Sampled sound')
      else if (h.ID = 'UVOX') then Appendln ('Uhuru Sound Software Macintosh voice')

      { music data }
      else if (h.ID = 'GSCR') then Appendln ('General use musical score')
      else if (h.ID = 'SMUS') then Appendln ('Simple musical score')
      else if (h.ID = 'TRAK') then Appendln ('MIDI music data')
      else if (h.ID = 'USCR') then Appendln ('Uhuru Sound Software musical score')

      { rest data }
      else if (h.ID = 'BMHD') then BMHD
      else if (h.ID = 'CMAP') then CMAP
      else if (h.ID = 'BODY') then Appendln ('image data')

      else Appendln ('unknown type');
    end;

begin
  TFile_Init (0);
{ read global header }
  if (TFile_ReadActBuf (H, IFF_LH_SIZE) <> IFF_LH_SIZE) then;
  TFIle_IncFilePos (4);
  NoteFormat;

  IncInsLevel;
  p := IFF_LH_SIZE + 4;
  repeat
    if (TFile_ReadActBuf (H, IFF_LH_SIZE) = IFF_LH_SIZE) then
    begin
      NoteFormat;
      SwapLong (h.Size, nSize);
      inc (p, IFF_LH_SIZE + nSize);
      TFile_GotoFilePos (p);
    end;
  until (not TFile_IsOkay);
  DecInsLevel;
end;

function Check_IFF:boolean;
begin
  Check_IFF := false;
  if (b.First.l [1] = $4D524F46) then { "FORM" }
  begin
    Check_IFF := true;
    if WriteHeader ('IFF file') then
    begin
      FinishLine;
      AnalyzeIFF;
    end;
  end;
end;

{-------------------------[JPG]-------------------------}

function Check_JPG:boolean;
type
     JPG_Block = record
       nID:byte;
       nType:byte;
       nSize:word; { 68K! without nID and nType }
     end;

     JPG_SOF0 = record
       nID:byte;
       nType:byte;
       nSize:word;
       nPrecision:byte;
       nHeight:word;
       nWidth:word;
       nComponents:byte;
     end;
const
      JPG_B_SIZE    = SizeOf (JPG_Block);
      JPG_SOF0_SIZE = SizeOf (JPG_SOF0);

      SOF0 = $C0;  { start of image }
      SOF1 = $C1;  { start of image }
      SOF2 = $C2;  { start of image }
      DHT  = $C4;  { define Huffman table }
      DAC  = $CC;  { define Arithmetic table }

      RST0 = $D0;  { RSTn are used for resync }
      RST1 = $D1;
      RST2 = $D2;
      RST3 = $D3;
      RST4 = $D4;
      RST5 = $D5;
      RST6 = $D6;
      RST7 = $D7;

      SOI  = $D8;  { start of image }
      EOI  = $D9;  { end of image }
      SOS  = $DA;  { start of scan }
      DQT  = $DB;  { define quantization table }
      DNL  = $DC;
      DRI  = $DD;  { define restart interval }
      DHP  = $DE;
      EXP  = $DF;

      APP0 = $E0;  { JFIF APP0 segment marker }
      APPF = $EF;

      JPG0 = $F0;
      JPGD = $FD;
      COM  = $FE;  { comment }

var
    GB:JPG_Block;
    GSOF0:JPG_SOF0;
    sColors:String25;
    nNextPos:longint;
    bFound:boolean;
begin

  { check if image is a JPG or not ... }
  if (b.First.w [1] = $D8FF) then
    Check_JPG := true
  else
  begin
    Check_JPG := false;
    exit;
  end;

  if (WriteHeader ('JPG image')) then
  begin
    FinishLine;

    { skip first two bytes }
    nNextPos := 2;
    bFound := false;
    TFile_Init (nNextPos);

    repeat
      if (TFile_ReadBufAt (nNextPos, GB, JPG_B_SIZE) = JPG_B_SIZE) then
      begin
        if (GB.nID <> $FF) then
          TFile_SetError (TFILE_ERR_ID)
        else
        begin
          case GB.nType of
            SOF0,
            SOF1,
            SOF2:begin
                   if (TFile_ReadBufAt (nNextPos, GSOF0, JPG_SOF0_SIZE) <> JPG_SOF0_SIZE) then;

                   case GSOF0.nComponents of
                     1:sColors := 'Greyscaled';
                     3:sColors := 'TrueColor (24 Bit)';
                     4:sColors := 'TrueColor (32 Bit)';
                   else
                     sColors := 'unknown';
                   end;

                   IncInsLevel;
                   Noteln (concat (i2s (swap (GSOF0.nWidth)), 'x',
                                   i2s (swap (GSOF0.nHeight)), STR_SEPERATOR,
                                   sColors));
                   DecInsLevel;

                   bFound := true;
                 end;
          end;
          { 2 for nID and nType; not included in nSize }
          inc (nNextPos, 2 + swap (GB.nSize));
        end;
      end;
    until (bFound) or (not TFile_IsOkay);

    if (not bFound) then
      Noteln ('Cannot find appropriate JPG segment');
  end;
end;

{-------------------------[PCD]-------------------------}

function Check_PCD:boolean;
type
     PCD_Header = record
       ID:array[1..7] of char;
     end;
const
      PCD_H_SIZE = SizeOf (PCD_Header);
var
    GH:PCD_Header;
begin
  Check_PCD := false;
  BUF_GetBuf (GH, 2049, PCD_H_SIZE);
  if (GH.ID = 'PCD_IPI') then
  begin
    Check_PCD := true;
    if (WriteHeader ('Kodak PhotoCD image')) then
    begin
      FinishLine;
    end;
  end;
end;

{-------------------------[PCX]-------------------------}

function Check_PCX:boolean;
type
     PCXHeader = record
       ID:byte;                        { 0A }
       Version:byte;
       Encoding:byte;
       BitsPerPixel:byte;
       XMin:word;
       YMin:word;
       XMax:word;
       YMax:word;
       HRes:word;
       VRes:word;
       EGAPal:array [0..47] of byte;
       Res1:byte;                      { 00 }
       NumBitPlanes:byte;
       BytesPerLine:word;
       PalType:word;
       Res2:array [0..57] of byte;     { all 00 }
     end;
const
      PCX_H_SIZE    = SizeOf (PCXHeader);
var
    GH:PCXHeader;

    function IsPCX:boolean;
    begin
      IsPCX := (GH.ID = 10) and  { ID }
               (GH.Res1 = 0) and
               (GH.xMax > GH.xMin) and
               (GH.yMax > GH.yMin);
    end;

begin
  Check_PCX := false;
  BUF_GetBuf (GH, 1, PCX_H_SIZE);
  if IsPCX then
  begin
    Check_PCX := true;
    if WriteResolutionHeader ('ZSoft PCX image',
                              GH.XMax - GH.XMin + 1,
                              GH.YMax - GH.YMin + 1,
                              Exp2 (GH.BitsPerPixel * GH.NumBitPlanes)) then
    begin
      FinishLine;
    end;
  end;
end;

{-------------------------[PNG]-------------------------}

function Check_PNG:boolean;
type
     PNGHeader = record
       ID:array[1..8] of char;
       Res1:array[1..8] of byte;
       Width:longint;
       Height:longint;
       ColorDepth:byte;
       ColorType:byte;
       Res2:word;
       Flags:byte;
     end;

     PNGBlockHeader = record
       Following_Bytes:longint;
       Chunktype:array[1..4] of char;
       width:longint;
       height:longint;
       bit_depth:byte;
       color_type:byte;
       compression_type:byte;
       filter_type:byte;
       interlace_type:byte;
     end;
const
      PNG_H_SIZE    = SizeOf (PNGHeader);
var s:ShortStr;
    H:PNGHeader;
    nColors, nWidth, nHeight:longint;
begin
  Check_PNG := false;
  BUF_GetBuf (H, 1, PNG_H_SIZE);
  if (H.ID = '‰PNG'#13#10#26#10) then
  begin
    Check_PNG := true;
    if (H.Flags > 0) then
      s := 'interlaced'
    else
      s := 'non interlaced';

    SwapLong (H.Width,  nWidth);
    SwapLong (H.Height, nHeight);

    if (H.ColorDepth = 8) and (H.ColorType = 2) then
      nColors := TrueColor
    else
      nColors := Exp2 (H.ColorDepth);

    if WriteResolutionHeader (concat ('PiNG image ', Enbraced (s)), nWidth, nHeight, nColors) then
    begin
      FinishLine;
    end;
  end;
end;

{-------------------------[RAS]-------------------------}

function Check_RAS:boolean;
type
     RASHeader = record
       ID:longint;
       Width:longint;
       Height:longint;
       Depth:longint;
       Length:longint;
       ImageType:longint;
       ColormapType:longint;
       ColormapLength:longint;
     end;
const
      RAS_H_SIZE = SizeOf (RASHeader);
var H:RASHeader;
    nDepth, nWidth, nHeight:longint;
begin
  Check_RAS := false;
  BUF_GetBuf (H, 1, RAS_H_SIZE);
  if (H.ID = longint ($956AA659)) then
  begin
    Check_RAS := true;
    SwapLong (H.Depth,  nDepth);
    SwapLong (H.Width,  nWidth);
    SwapLong (H.Height, nHeight);
    if WriteResolutionHeader ('Sun Raster image', nWidth, nHeight, Exp2 (nDepth)) then
    begin
      FinishLine;
    end;
  end;
end;

{-------------------------[TIF]-------------------------}

function Check_TIFF:boolean;
type
     TIFF_GlobalHeader = record
       nVendor:word;  { "II" or "MM" }
       nVersion:word;  { 42 }
       nFirstImageDirOffset:longint;
     end;

     TIFF_Tag = record
       nTag:word;
       nUnitType:word;
       nUnitCount:longint;
       nDataOffset:longint;  { if size > 4 then the offset else the data }
     end;

const
      TIFF_GH_SIZE = SizeOf (TIFF_GlobalHeader);
      TIFF_T_SIZE = SizeOf (TIFF_Tag);
var
    GH:TIFF_GlobalHeader;
    i, nTagCount:word;
    T:TIFF_Tag;
    nHeight, nWidth:longint;
begin
  Check_TIFF := false;
  BUF_GetBuf (GH, 1, TIFF_GH_SIZE);
  if ((GH.nVendor = $4949) or (GH.nVendor = $4D4D)) and
      (GH.nVersion = $002A) then
  begin
    Check_TIFF := true;
    { only analyze Intel byte ordered images }
    if (WriteHeader ('TIFF image')) and (GH.nVendor = $4949) then
    begin
      FinishLine;
      IncInsLevel;

      TFile_Init (GH.nFirstImageDirOffset);
      nTagCount := TFile_ReadActWord;

      nWidth := -1;
      nHeight := -1;

      for i:=1 to nTagCount do
        if (TFile_ReadActBuf (T, TIFF_T_SIZE) = TIFF_T_SIZE) then
        begin
          case (T.nTag) of
            256:if (T.nUnitType = 3) then  { word }
                  nWidth := word (T.nDataOffset and $0000FFFF)
                else
                  if (T.nUnitType = 4) then  { dword }
                    nWidth := T.nDataOffset
                  else
                    Noteln ('Error in width tag.');
            257:if (T.nUnitType = 3) then  { word }
                  nHeight := word (T.nDataOffset and $0000FFFF)
                else
                  if (T.nUnitType = 4) then  { dword }
                    nHeight := T.nDataOffset
                  else
                    Noteln ('Error in width tag.');
          end;
        end;

      Noteln (concat (i2s (nWidth), 'x', i2s (nHeight)));

      DecInsLevel;
    end;
  end;
end;

{-------------------------[WMF]-------------------------}

function Check_WMF:boolean;
type
     WMF_Header = record
       ID:longint;
       Handle:word;
       Left:integer;
       Top:integer;
       Right:integer;
       Bottom:integer;
       Inch:word;
       Reserved:longint;
       Checksum:word;
     end;

     WMF_DataHeader = record
       FileType:word;
       HeaderSize:word;  { in WORDS !! }
       Version:word;
       FileSize:longint;
       NumOfObjects:word;
       MaxRecordSize:longint;
       NoPramaters:word;
     end;

const
      WMF_H_SIZE = SizeOf (WMF_Header);

var H:WMF_Header;
begin
  Check_WMF := false;
  BUF_GetBuf (H, 1, WMF_H_SIZE);
  if (H.ID = longint ($9AC6CDD7)) then
  begin
    Check_WMF := true;
    if WriteHeader ('Windows Metafile image ') then
    begin
      FinishLine;
      IncInsLevel;
      Noteln (concat ('Coordinates stored: ',
                    i2s (H.Left), '/', i2s (H.Top), ' - ',
                    i2s (H.Right), '/', i2s (H.Bottom)));
      Noteln (concat ('Size in pixel: ', i2s (H.Right - H.Left + 1), 'x', i2s (H.Top - H.Bottom + 1)));
      DecInsLevel;
    end;
  end;
end;

{-------------------------[WPG]-------------------------}

function Check_WPG:boolean;
type
     WPGHeader = record
       ID:longint;
       DataOffset:longint;
       ProductType:byte;
       FileType:byte;
       MajorVersion:byte;
       MinorVersion:byte;
       EncryptionKey:word;
       Reserved:word;
     end;
const
      WPG_H_SIZE = SizeOf (WPGHeader);
var H:WPGHeader;
begin
  Check_WPG := false;
  BUF_GetBuf (H, 1, WPG_H_SIZE);
  if (H.ID = $435057FF) and   { WPC  }
     (H.DataOffset  = $10) and
     (H.ProductType = $01) and
     (H.FileType    = $16) then
  begin
    Check_WPG := true;
    if WriteHeader (concat ('Wordperfect Graphics ',
                            GetVersionStr (H.MajorVersion, H.MinorVersion),
                            ' image')) then
    begin
      FinishLine;
      NoteNoResolution;
    end;
  end;
end;

{-------------------------[XXX]-------------------------}

function Check_Image:boolean;
begin
  Check_Image := true;
  if (not Check_BMF) then
  if (not Check_BMP) then
  if (not Check_GIF) then
  if (not Check_ICO) then
  if (not Check_IFF) then
  if (not Check_JPG) then
  if (not Check_PCD) then
  if (not Check_PCX) then
  if (not Check_PNG) then
  if (not Check_RAS) then
  if (not Check_TIFF) then
  if (not Check_WMF) then
  if (not Check_WPG) then
    Check_Image := false;
end;

end.
