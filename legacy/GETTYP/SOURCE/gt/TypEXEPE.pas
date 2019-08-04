{$define extended}
{$i aflags.inc}

unit TypEXEPE;

interface

uses
     GTGlobal,  { - for Win32 - longint = longword - }
     TypOver;   { - for TEXEProc - }

procedure WritePEInfo (aProc:TEXEProc;
                       NE_Offset:longint);

implementation

uses
{$ifdef Windows}
     WinDOS,
{$else}
  {$ifdef DELPHI}
     SysUtils,
  {$else}
     DOS,
  {$endif}
{$endif}
     GTCodes,
     GTBuf, GTCompar, GTFile, GTMem, GTOutput,
     GTDataPE,
     XEXE_PE, XString;

var
    IFH:PE_ImageFileHeader;
    OFH:PE_OptionalFileHeader;
    aProcedure:TEXEProc;

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

function CheckForCode (nEntryPoint:longint;
                       nCalculatedFileSize:longint):boolean;
label Done;
var Found:boolean;
    nPos:longint;
    i:word;
    pObj:pPE_Object;  { - needed for heuristics - }

    procedure __ComparePtrCode (p:PCompPtrRec);
    begin
      if ComparePtrBuf (p) then
        Found := true;
    end;

    procedure CheckASPack;
    var i:byte;
    begin
      nPos := nEntryPoint;
      if (TEXEFile_GetMacroPos ('1k', nPos, PEEXE)) then
      begin
        TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
        __ComparePtrCode (@REC_ASPack1084);
        __ComparePtrCode (@REC_ASPack2000);
        __ComparePtrCode (@REC_ASPack2001);
        __ComparePtrCode (@REC_ASPack2100);
      end;

      nPos := nEntryPoint;
      if (TEXEFile_GetMacroPos ('2r', nPos, PEEXE)) then
      begin
        TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
        __ComparePtrCode (@REC_ASPack104b);
        __ComparePtrCode (@REC_ASPack105b);
      end;

      for i:=3 to 5 do
      begin
        nPos := nEntryPoint;
        {
          skip 3-5 byte,
          do a 32 bit RVA jump, $E9
        }
        if (TEXEFile_GetMacroPos (concat (i2s (i), 'r'), nPos, PEEXE)) then
        begin
          TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
          __ComparePtrCode (@REC_ASPack1061b);
          __ComparePtrCode (@REC_ASPack107b);
        end;
      end;

      for i:=4 to 6 do
      begin
        nPos := nEntryPoint;
        {
          skip 4-6 byte,
          do a 32 bit RVA jump, $E9
        }
        if (TEXEFile_GetMacroPos (concat (i2s (i), 'r'), nPos, PEEXE)) then
        begin
          TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
          __ComparePtrCode (@REC_ASPack108);
          __ComparePtrCode (@REC_ASPack1081);
          __ComparePtrCode (@REC_ASPack1082);
        end;
      end;
    end;

var
    pSCRecord:pSCRecords;
    rec:tSCRecord;
begin
  Found := false;

  if (nEntryPoint > 0) and (nEntryPoint < b.FSize) then
  begin
    TFile_ReadBufAt (nEntryPoint, CompareBuf, SizeOf (CompareBuf));

    _CompareCodesFromFile (Found, EXE_PE_00);
    if (Found) then
      goto Done;
    {
    for i:=1 to COUNT_PE do
      if ComparePtrBuf (@REC_PE [i]) then
      begin
        Found := true;
        goto Done;
      end;
    }
    { - same code for EXE and DLL :( - EXE with jump, DLL without - }
    __ComparePtrCode (@REC_ASPack1081); { DLL }
    __ComparePtrCode (@REC_ASPack1082); { DLL }


    nPos := nEntryPoint;
    { do one long jump ($E9xxyyxxyy) }
    if (TEXEFile_GetMacroPos ('r', nPos, PEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));

      _CompareCodesFromFile (Found, EXE_PE_01);
      if (Found) then
        goto Done;
      {
      for i:=1 to COUNT_PE_01 do
        if ComparePtrBuf (@REC_PE_01 [i]) then
        begin
          Found := true;
          goto Done;
        end;
      }
    end;

    nPos := nEntryPoint;
    { do 2 long jumps ($E9xxyyxxyy) }
    if (TEXEFile_GetMacroPos ('rr', nPos, PEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));

      pSCRecord := aStartupCodes.next;
      while (pSCRecord <> nil) do
      begin
        for i:=0 to pSCRecord^.nCount - 1 do
        begin
          CopyFromMemoryAt (pSCRecord^.aMS, rec, sizeof (tSCRecord), i * sizeof (tSCRecord));
          if ComparePtrBufFile (@rec) then
          begin
            Found := true;
            goto Done;
          end;
        end;
        pSCRecord := pSCRecord^.next;
      end;
      {
      for i:=1 to COUNT_PE_01 do
        if ComparePtrBuf (@REC_PE_01 [i]) then
        begin
          Found := true;
          goto Done;
        end;
      }
    end;

    nPos := nEntryPoint;
    { skip 6 bytes then do 1 jump }
    if (TEXEFile_GetMacroPos ('6j', nPos, PEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
      __ComparePtrCode (@REC_WWPack32_111);
    end;

    nPos := nEntryPoint;
    { skip 6 bytes then do 1 jump and one 32 bit jump }
    if (TEXEFile_GetMacroPos ('6jk', nPos, PEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
      __ComparePtrCode (@REC_WWPack32_109);
    end;

    nPos := nEntryPoint;
    { skip 6 bytes then do 2 jumps }
    if (TEXEFile_GetMacroPos ('6jj', nPos, PEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
      __ComparePtrCode (@REC_WWPack32_100);
      __ComparePtrCode (@REC_WWPack32_112);
    end;

    CheckASPack;

    nPos := nEntryPoint;
    { skip 11 bytes then do a 32 bit jump }
    if (TEXEFile_GetMacroPos ('11k', nPos, PEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
      __ComparePtrCode (@REC_Delphi4);
    end;

    nPos := nEntryPoint;
    { skip 10 bytes then do a 32 bit jump }
    if (TEXEFile_GetMacroPos ('10k', nPos, PEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
      __ComparePtrCode (@REC_VPascal_21);
    end;

    nPos := nEntryPoint;
    { skip 10 bytes then do a 32 bit jump }
    if (TEXEFile_GetMacroPos ('10r', nPos, PEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
      __ComparePtrCode (@REC_Dylan12);
    end;

    nPos := nEntryPoint;
    { skip 15 bytes then do a 32 bit call }
    if (TEXEFile_GetMacroPos ('15k', nPos, PEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
      __ComparePtrCode (@REC_PKLite32_11);
    end;

    nPos := nEntryPoint;
    { skip 20 bytes then do a 32 bit call }
    if (TEXEFile_GetMacroPos ('20k', nPos, PEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
      __ComparePtrCode (@REC_PKLite32_11);
    end;

    nPos := nEntryPoint;
    { skip 29 bytes then do a 32 bit jump }
    if (TEXEFile_GetMacroPos ('29r', nPos, PEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));
      __ComparePtrCode (@REC_VGCrypt_075);
    end;

    if (not Found) then
    begin
      aPE_OTH.GetObject (1, pObj);

      if (pObj^.sName = 'CODE'#0#0#0#0) or
         (pObj^.sName = 'CODE    ') then
      begin
        if (OFH.LinkerMajor = 2) and (OFH.LinkerMinor = 25) then
        begin
          NoteCompiler ('Borland Delphi 3/4 (heuristic)');
          Found := true;
        end
        else
          NoteCompiler ('made by Borland - unknown')
      end
      else
      if (pObj^.sName = '.text'#0#0#0) then
        NoteCompiler ('made by Microsoft - unknown');
    end;
  end
  else
    Noteln ('Entrypoint is out of file bounds.');

Done:
  if (not Found) then
    Noteln (FOUND_NO_MODIFIER);

  CheckForCode := Found;
end;

procedure PrintObjectTable;
var i:byte;
    pObj:pPE_Object;
begin
  Noteln (concat ('Objects (object align = ', hexlong (OFH.SectionAlignment), 'h):'));
  IncInsLevel;

  { - write headline - }
  Noteln ('Name      Virt size     RVA     Phys size  Phys Ofs');

  { - write entries - }
  for i := 1 to IFH.NumberOfSections do
  begin
    aPE_OTH.GetObject (i, pObj);
    Noteln (concat (pObj^.sName,
                    '  ', hexlong (pObj^.nVirtSize), 'h',
                    '  ', hexlong (pObj^.nRVA), 'h',
                    '  ', hexlong (pObj^.nPhysSize), 'h',
                    '  ', hexlong (pObj^.nPhysOfs), 'h'
                    ));
  end;

  DecInsLevel;
end;


procedure CheckForWinzip;
var i:byte;
    dummy:boolean;
    pObj:pPE_Object;
begin
  for i:=1 to aPE_OTH.GetObjectCount do
  begin
    aPE_OTH.GetObject (i, pObj);
    if (pObj^.sName = '_winzip_') then
    begin
      WriteOverlayInfo (aProcedure,
                        pObj^.nPhysOfs,
                        pObj^.nPhysSize,
                        dummy,
                        false);  { do not ignore empty overlays -
                                   archives are not automatically checked! }
      exit;
    end;
  end;
end;

procedure WritePEInfo (aProc:TEXEProc;
                       NE_Offset:longint);
label Done;
var nCalculatedFileSize,
    nEntryPoint:longint;
    dummy:boolean;

    procedure ReadHeaderAndObjectTable;
    begin
      TFile_Init (NE_Offset);  { skip the ID }
      TFile_ReadActBuf (IFH, SIZEOF_PE_IMAGE_FILE_HEADER);
      TFile_ReadActBuf (OFH, SIZEOF_PE_OPTIONAL_FILE_HEADER);

      aPE_OTH.Init (IFH.NumberOfSections);
      aPE_OTH.ReadFromFile (Handle, TFile_GetFilePos);
      aPE_OTH.GetPhysOfsByRVA (OFH.AddressOfEntryPoint, nEntryPoint);
    end;

    procedure PrintFlags;
    begin
    {
    other known IFH.Flags bits:
    #define IMAGE_FILE_BYTES_REVERSED_LO         0x0080  // Bytes of machine word are reversed.
    #define IMAGE_FILE_BYTES_REVERSED_HI         0x8000  // Bytes of machine word are reversed.
    }
      Noteln ('Flags:');
      IncInsLevel;
      if (IFH.Flags and $0001 > 0) then Noteln ('Relocation info stripped from file');
      if (IFH.Flags and $0002 > 0) then Noteln ('File is executable');
      if (IFH.Flags and $0004 > 0) then Noteln ('Line numbers stripped from file');
      if (IFH.Flags and $0008 > 0) then Noteln ('Local symbols stripped from file');
      if (IFH.Flags and $0100 > 0) then Noteln ('32 bit word machine');
      if (IFH.Flags and $0200 > 0) then Noteln ('Debugging info stripped from file in .DBG file');
      if (IFH.Flags and $1000 > 0) then Noteln ('System File');
      if (IFH.Flags and $2000 > 0) then Noteln ('Library image (DLL)');
      DecInsLevel;
    end;

begin
  aProcedure := aProc;

  EmptyLine;
  Noteln (concat ('Portable executable (starting at ', i2s (NE_Offset), ' for ',
                  i2s (b.FSize - NE_Offset), ' bytes)'));
  IncInsLevel;

  ReadHeaderAndObjectTable; { sets aPE_OTH }
  nCalculatedFileSize := aPE_OTH.GetEXESize;

  if (nEntryPoint = 0) then
    Noteln ('File has no entry point (maybe resource DLL)')
  else
    CheckForCode (nEntryPoint, nCalculatedFileSize); { uses aPE_OTH }

  if (bDoEXEHeaderAnyWay) then
  begin
    if (nEntryPoint <> 0) then
    begin
      Noteln (concat ('Calculated entrypoint: ',
                      i2s (nEntryPoint), ' / ',
                      hexlong (nEntryPoint), 'h  (RVA: ',
                      hexlong (OFH.AddressOfEntryPoint), 'h)'));
    end;

    Note ('Required CPU type: ');
    case IFH.CPUType of
      $0000: Appendln ('unknown');
      $014C: Appendln ('80386');
      $014D: Appendln ('80486');
      $014E: Appendln ('80586');
      $0162: Appendln ('MIPS Mark I (R2000, R3000)');
      $0163: Appendln ('MIPS Mark II (R6000)');
      $0166: Appendln ('MIPS Mark III (R4000)');
      $0168: Appendln ('MIPS');
      $0184: Appendln ('Alpha AXP');
      $01F0: Appendln ('IBM PowerPC');
    else
      Appendln (concat ('unknown type ', hexword (IFH.CPUType), 'h'));
    end;

    Note ('Requires ');
    case (OFH.OSMajor) of
      1: Appendln ('any version of Win32');
      4: Appendln ('Win 95 or NT 4');
      5: Appendln ('Win 98 or NT 5');
    else
      Appendln (concat ('Win32 ', GetVersionStr (OFH.OSMajor, OFH.OSMinor)));
    end;

    PrintFlags;
    Noteln (concat ('Linker version: ', GetVersionStr (OFH.LinkerMajor, OFH.LinkerMinor)));
    PrintObjectTable; { uses aPE_OTH }
    DecInsLevel;
  end
  else
    Noteln (USE_ZE);

Done:
  CheckForWinzip; { uses aPE_OTH }
  aPE_OTH.done;

  if (b.FSize > nCalculatedFileSize) then
  begin
    WriteOverlayInfo (aProcedure,
                      nCalculatedFileSize,
                      b.FSize - nCalculatedFileSize,
                      dummy,
                      false);  { do not ignore empty overlays }
  end
  else
    if (b.FSize < nCalculatedFileSize) then
      Noteln (concat (i2s (nCalculatedFileSize - b.FSize), ' bytes are missing in EXE'));

{
do not call aCurrentOTH.Done
This is done in aCurrentOTh.ReInit
a new ExitProc finally deletes aCurrentOTH...
}
end;

end.
