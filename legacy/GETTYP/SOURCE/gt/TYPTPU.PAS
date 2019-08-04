{
TypTPU - Turbo Pascal Unit files

Copyright (c) 1997, 98 by Philip Helger (philip@dke.univie.ac.at)
}

{$i aflags.inc}

unit typTPU;

interface

function Check_TPU:boolean;

implementation

uses
     GTBuf, GTFile, GTGlobal, GTOutput,
     XString;

type
     TTPU_Type = (NONE, TP4, TP5, TP55, TP6, TP7, DELPHI);

var
    aType:TTPU_Type;

procedure CheckForUnit;
begin
       if (b.First.L [1] = $30555054) then aType := TP4
  else if (b.First.L [1] = $35555054) then aType := TP5
  else if (b.First.L [1] = $36555054) then aType := TP55
  else if (b.First.L [1] = $39555054) then aType := TP6
  else if (b.First.L [1] = $51555054) then aType := TP7
  else if (b.First.L [1] = $31554344) then aType := DELPHI
  else atype := NONE;
end;

function GetVersion:string;
var s:ShortStr;
begin
  case aType of
    TP4:  s := '4 unit';
    TP5:  s := '5 unit';
    TP55: s := '5.5 unit';
    TP6:  s := '6 unit';
  else
    if (aType = TP7) then
    begin
      s := '7 unit '; { Turbo Pascal }

      if (b.First.w [24] = $1218) then
        s := concat (s, Enbraced ('Windows'))
      else
        if (b.First.w [24] = $1118) then
          s := concat (s, Enbraced ('Protected mode'))
        else
          s := concat (s, Enbraced ('Real mode'));
    end;
  end;

  if (aType = DELPHI) then
    GetVersion := concat ('Borland Delphi unit (old)')
  else
    GetVersion := concat ('Turbo Pascal ', s);
end;

function GetString (nPos:TPos):string;
var len:byte;
begin
  if (nPos + 1) >= b.BufLen then
  begin
    len := BUF_GetByte (nPos + 1);
    if (nPos + 1 + len) < b.BufLen then
    begin
      GetString := BUF_GetString (nPos + 2, len);
      exit;
    end;
    { else read from file ... }
  end;
  GetString := TFile_ReadPascalString (nPos);
end;

procedure NoteSourceName;
var w:TPos;
begin
  if (aType = TP4) then
    w := b.First.w[9] + 1      { BUF_GetWord (19) + 1 }
  else
    if (aType = TP5) or (aType = TP55) then  { das mit TP55 ???? }
      w := b.First.w [12] + 5  { BUF_GetWord (23) + 5 }
    else  { TP6, TP7, DELPHI }
      w := b.First.w [13] + 7; { BUF_GetWord (25) + 7; }

  Noteln (concat ('Source name: ', GetString (w)));
end;

procedure NoteUnitName;
var w:TPos;
begin
  w := b.First.w [5];  { BUF_GetWord (9); }

  if (aType = TP4) or (aType = TP5) or (aType = TP55) then { TP55 ??? }
    inc (w, 2)
  else  { TP6, TP7, DELPHI }
    inc (w, 3);

  Noteln (concat ('Unit name: ', GetString (w)));
end;

procedure ListDependencies;
const EODep = $0004;
var Count:byte;
begin
  if (aType = TP4) or (aType = TP5) then
    Noteln ('Don''t know how to list dependencies')
  else
  begin
    TFile_GotoFilePos (b.First.w [11]);  { b.GetWord (21) }
    if (TFile_GetFilePos = 0) or (TFile_GetFilePos > b.FSize) then
    begin
      Noteln ('Unit is damaged/invalid');
      exit;
    end;

    Noteln ('Dependencies:');
    IncInsLevel;

    Count := 0;
    while (TFile_ReadActWord <> EODep) do  { increases FPos by 2 }
    begin
      inc (Count);
      if (aType = TP7) or (aType = DELPHI) then
        TFile_IncFilePos (2);              { increases FPos by 2 }

      Noteln (TFile_ReadActPascalString);   { increases FPos by actbyte + 1 }
    end;

    if (Count = 0) then
      Noteln ('No dependencies were found.');

    DecInsLevel;
  end;
end;

function Check_TPU:boolean;
begin
  Check_TPU := false;
  CheckForUnit;
  if (aType <> NONE) then
  begin
    Check_TPU := true;
    if WriteHeader (GetVersion) then
    begin
      FinishLine;

      TFile_Init (0);
      IncInsLevel;
      NoteUnitName;
      NoteSourceName;
      ListDependencies;
      DecInsLevel;
    end;
  end;
end;

end.
