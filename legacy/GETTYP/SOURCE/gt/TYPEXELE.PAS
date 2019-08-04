{$define extended}
{$i aflags.inc}

unit TypEXELE;

interface

uses
     GTGlobal;

procedure WriteLXInfo (aProc:TEXEProc;
                       NE_Offset:longint);

implementation

uses
     GTBuf, GTCompar, GTDataLE, GTFile, GTOutput,
     TypOver,
     XEXE_LE, XGlobal, XString;

function CheckForCode (nEntryPoint:longint):boolean;
label Done;
var Found:boolean;
    i:word;
    nPos:longint;

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

    for i:=1 to COUNT_LE do
      if ComparePtrBuf (@REC_LE [i]) then
      begin
        Found := true;
        goto Done;
      end;

    nPos := nEntryPoint;
    if (TEXEFile_GetMacroPos ('j', nPos, LEEXE)) then
    begin
      TFile_ReadBufAt (nPos, CompareBuf, SizeOf (CompareBuf));

      for i:=1 to COUNT_LE_01 do
        if ComparePtrBuf (@REC_LE_01 [i]) then
        begin
          Found := true;
          goto Done;
        end;
    end;
  end
  else
    Noteln ('Entrypoint is out of file bounds.');

Done:
  if (not Found) then
    Noteln (FOUND_NO_MODIFIER);

  CheckForCode := Found;
end;

procedure WriteLXInfo (aProc:TEXEProc;
                       NE_Offset:longint);
var
    LE_Head:tLE_Header;
    nEntryPoint,
    nCalculatedFileSize,
    nCalculatedFileSize2:longint;
    bFound:boolean;

    procedure ReadHeaderAndObjectTable;
    begin
      TFile_Init (NE_Offset);  { skip the ID }
      TFile_ReadActBuf (LE_Head, LE_HEADER_SIZE);

      aLE_OTH.Init (LE_Head.ObjectTableEntries, @LE_Head);
      aLE_OTH.ReadFromFile (Handle, NE_Offset + LE_Head.ObjectTableOfs);
    end;

    procedure ListNonResidentNames;
    begin
      if (LE_Head.NonResidentNamesTableOfsFromTopOfFile > 0) then
      begin
        TFile_GotoFilePos (LE_Head.NonResidentNamesTableOfsFromTopOfFile);
        Noteln (concat ('"', TFile_ReadActPascalString, '"'));
      end;
    end;

    function ObjectFlagsStr (w:word):string;
    var s:string;
        i:byte;
    begin
      SetLength (s, 16);
      fillchar (s[1], 16, '.');

      for i:=0 to 15 do
        if (w and BITS [i] > 0) then
          s[i + 1] := 'x';
      ObjectFlagsStr := s;
    end;

    procedure WriteObjectTable;
    var i:byte;
        pObj:pLE_Object;
    begin
      Noteln ('Object table entries:');

      Noteln (' #  PhysOfs   EndOfs     Size Ix En VirtSize BaseAddr 0123456789ABCDEF');

      for i:=1 to aLE_OTH.GetEntryCount do
      begin
        aLE_OTH.GetObject (i, pObj);
        Noteln (concat (i2sf (i, 2), ' ',
                        hexlong (aLE_OTH.GetRealAddrOfObject (i)), ' ',
                        hexlong (aLE_OTH.GetRealEndAddrOfObject (i)), ' ',
                        hexlong (aLE_OTH.GetSizeOfObject (i)), ' ',
                        {}
                        hexbyte (pObj^.nPageTableIndex), ' ',
                        hexbyte (pObj^.nPageTableEntries), ' ',
                        {}
                        hexlong (pObj^.nVirtSegmentSize), ' ',
                        hexlong (pObj^.nRelocationBaseAddr), ' ',
                        ObjectFlagsStr (pObj^.nFlags)));
      end;
    end;

    procedure WriteFlagsInfo;
    begin
      Noteln ('Module flags:');
      IncInsLevel;

      if (LE_Head.ModuleType and $00000004 > 0) then
        Noteln ('Per-process library initialization');
      if (LE_Head.ModuleType and $00000010 > 0) then
        Noteln ('Internal fixups for the module have been applied');
      if (LE_Head.ModuleType and $00000020 > 0) then
        Noteln ('External fixups for the module have been applied');
      if (LE_Head.ModuleType and $00000100 > 0) then
        Noteln ('Incompatible with PM windowing');
      if (LE_Head.ModuleType and $00000200 > 0) then
        Noteln ('Compatible with PM windowing');
      if (LE_Head.ModuleType and $00000400 > 0) then
        Noteln ('Uses PM windowing API');
      if (LE_Head.ModuleType and $00002000 > 0) then
        Noteln ('Module is not loadable');

      if (LE_Head.ModuleType and $00038000 = $00000000) then
        Noteln ('Program module');
      if (LE_Head.ModuleType and $00038000 = $00008000) then
        Noteln ('Library module');
      if (LE_Head.ModuleType and $00038000 = $00018000) then
        Noteln ('Protected memory library module');
      if (LE_Head.ModuleType and $00038000 = $00020000) then
        Noteln ('Physical device driver module');
      if (LE_Head.ModuleType and $00038000 = $00028000) then
        Noteln ('Virtual device driver module');

      if (LE_Head.ModuleType and $40000000 > 0) then
        Noteln ('Pre-process library termination');

      DecInsLevel;
    end;

begin
  EmptyLine;
  Noteln (concat ('Linear executable (starting at ', i2s (NE_Offset), ' for ',
                  i2s (b.FSize - NE_Offset), ' bytes)'));

  ReadHeaderAndObjectTable; { inits aLE_OTH }
  IncInsLevel;
  { ... }
  { ... }

  nCalculatedFileSize := aLE_OTH.GetEXESize;
  nEntryPoint := aLE_OTH.GetEntryPoint;

  {
  Noteln (concat ('Calculated filesize: ', hexlong (nCalculatedFileSize)));
  Noteln (concat ('Entrypoint:          ', hexlong (nEntryPoint)));
  }

  ListNonResidentNames;
  if (nEntryPoint = 0) then
    Noteln ('File has no entrypoint')
  else
    CheckForCode (nEntryPoint);

  if (bDoEXEHeaderAnyWay) then
  begin
    WriteObjectTable; { uses aLE_OTH }
    WriteFlagsInfo;
  end
  else
    Noteln (USE_ZE);

  aLE_OTH.done;

  DecInsLevel;

  if (nCalculatedFileSize = LE_Head.NonResidentNamesTableOfsFromTopOfFile) then
  begin
    Emptyline;
    Noteln (concat ('Non-resident names table is appended to the file for ',
                    i2s (LE_Head.NonResidentNamesTableLength), ' bytes.'));
    inc (nCalculatedFileSize, LE_Head.NonResidentNamesTableLength);
    if (b.FSize < nCalculatedFileSize) then
      Noteln ('But it could not be found at the end of the file');
  end;

  if (b.FSize > nCalculatedFileSize) then
  begin
    if (nCalculatedFileSize mod $100 <> 0) then
      nCalculatedFileSize2 := ((nCalculatedFileSize div $100) + 1) * $100
    else
      nCalculatedFileSize2 := 0;

    { ignore empty overlay if the first approach was successful }
    WriteOverlayInfo (aProc,
                      nCalculatedFileSize,
                      b.FSize - nCalculatedFileSize,
                      bFound,
                      false);

    { align nCalculatedFileSize to $100 and check overlay again }
    if (not bFound) and (b.FSize > nCalculatedFileSize2) then
    begin
      WriteOverlayInfo (aProc,
                        nCalculatedFileSize2,
                        b.FSize - nCalculatedFileSize2,
                        bFound,
                        true);  { ignore empty overlays }
    end;
  end
  else
    if (b.FSize < nCalculatedFileSize) then
      Noteln (concat ('Calculated filesize (', i2s (nCalculatedFileSize),
                      ') is greater than the filesize (', i2s (b.FSize), ')'));

end;

end.
