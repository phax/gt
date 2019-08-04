program GTXtract;

uses
     GTCompar,
     GTData00,
     GTData01,
     GTDataLE,
     GTDataNE,
     GTDataPE;

const
      TYPE_EXE  = 1;
      TYPE_LE   = 2;
      TYPE_NE   = 3;
      TYPE_PE   = 4;
      TYPE_ELSE = 255;

{
8 Sections:
  DOS ""
  DOS "j"
  LE  ""
  LE  "j"
  NE  ""
  NE  "j"
  PE  ""
  PE  "r"
}

function TypeOfSection (i:byte):byte;
begin
  case i of
    1, 2:TypeOfSection := TYPE_EXE;
    3, 4:TypeOfSection := TYPE_LE;
    5, 6:TypeOfSection := TYPE_NE;
    7, 8:TypeOfSection := TYPE_PE;
  end;
end;

function CountOfSection (i:byte):word;
begin
  case i of
    1:CountOfSection := COUNT_00;
    2:CountOfSection := COUNT_01;
    3:CountOfSection := COUNT_LE;
    4:CountOfSection := COUNT_LE_01;
    5:CountOfSection := COUNT_NE;
    6:CountOfSection := COUNT_NE_01;
    7:CountOfSection := COUNT_PE;
    8:CountOfSection := COUNT_PE_01;
  end;
end;

procedure GetPtr (i, j:byte; var rec:TCompPtrRec);
begin
  case i of
    1:rec := REC_00 [j];
    2:rec := REC_01 [j];
    3:rec := REC_LE [j];
    4:rec := REC_LE_01 [j];
    5:rec := REC_NE [j];
    6:;
    7:rec := REC_PE [j];
    8:rec := REC_PE_01 [j];
  end;
end;

var f:file;
    i, j:byte;

    ID1:array[1..4] of char;
    ID2:char;
    nOffset:longint;
    nSections:byte;

    Header:record
      ID:char;
      nType:byte;
      nOffset:word;
      nJumps:byte;
      nCount:word;
    end;

    Data:record
      ID:char;
      nDataLen:byte;
      nType:byte;
      nFilenameLen:byte;
      { ... code }
      { ... name }
    end;

    rec:TCompPtrRec;
    nSavedPos:longint;

begin
  assign (f, 'gtxtract.dat');
  rewrite (f, 1);

  ID1 := 'GTSC';
  blockwrite (f, ID1, 4);

  nSections := 8;
  blockwrite (f, nSections, 1);

  { inserted later }
  nOffset := 0;
  for i:=1 to nSections do
    blockwrite (f, nOffset, 4);

  for i:=1 to nSections do
  begin
    writeln (i);
    nSavedPos := FilePos (f);
    seek (f, 5 + (i - 1) * 4);
    blockwrite (f, nSavedPos, 4);
    seek (f, nSavedPos);

    Header.ID      := 'H';
    Header.nType   := TypeOfSection (i);
    Header.nOffset := 0;
    Header.nJumps  := 0;
    Header.nCount  := CountOfSection (i);
    blockwrite (f, Header, SizeOf (Header));

    for j:=1 to Header.nCount do
    begin
      GetPtr (i, j, rec);
      Data.ID           := 'D';
      Data.nDataLen     := rec.DataLen;
      Data.nType        := rec.FileType;
      Data.nFilenameLen := length (rec.Name^);
      blockwrite (f, Data, SizeOf (Data));

      blockwrite (f, rec.Data^, Data.nDataLen);
      blockwrite (f, rec.Name^[1], Data.nFilenameLen);
    end;
  end;

  close (f);
end.
