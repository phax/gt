{
GTText - textfile detection unit

Copyright (c) 1997, 98 by Philip Helger (philip@dke.univie.ac.at)
}

{$i aflags.inc}

unit GTText;

interface

const
      KEYFile3 = '.';

function Check_Textfiles:boolean;

implementation

uses
     GTBuf, GTGlobal, GTOutput,
     XGlobal, XString;

function IsPL:boolean;
var p:longint;
    s15:string[15];
    s21:string[21];
begin
  p := BUF_GetPosOfChar (1, 100, '#', CASEINSENSITIVE);
  if (p <> POSERROR) then
  begin
    s15 := BUF_GetString (p, 15);
    s21 := BUF_GetString (p, 21);
    IsPL := ((UpString (s21) = '#!/USR/LOCAL/BIN/PERL') or
             (UpString (s15) = '#!/USR/BIN/PERL'));
  end
  else
    IsPL := false;
end;

function IsText:boolean;
begin
  IsText := (b.FExt = 'DOC') or (b.FExt = 'DOK') or (b.FExt = 'TXT');
end;

function IsHTM:boolean;
begin
  IsHTM := (BUF_GetPosOfStringICS (1, 200, '<HTML>') > 0) or
           (BUF_GetPosOfStringICS (1, 600, '<TITLE>') > 0) or
           (BUF_GetPosOfStringICS (1, 600, '<BODY') > 0);  { no close tag }
end;

function GetHTMLTitle:string;
const StartStr = '<TITLE>';
      EndStr   = '</TITLE>';
var sP, eP:TPos;
    Start, Len:TPos;
    s:string;
begin
  sP := BUF_GetPosOfStringICS (1, 1024, StartStr);
  if (sP <> POSERROR) then
    eP := BUF_GetPosOfStringICS (1, 1024, EndStr)
  else
    eP := POSERROR;

  if (sP = POSERROR) or (eP = POSERROR) then
    GetHTMLTitle := ' - found no title'
  else
  begin
    Start := sP + 7;         { 7 == length (StartStr); }
    Len := eP - sP + 1 - 8;  { 8 == length (EndStr); }
    if (Len > 255) then
      s := concat (BUF_GetString (Start, 252), '...')
    else
      if (Len > 0) then
        s := BUF_GetString (Start, Len)
      else
        s := '<empty title>';
    GetHTMLTitle := concat ('"', StripSpaces (s), '"');
  end;
end;

function GetPGPVersion:string;
var pStart, pEnd:TPos;
begin
  pStart := BUF_GetPosOfByte (28, 48, 58);  { ':' }
  if (pStart <> POSERROR) then
  begin
    pEnd := BUF_GetPosOfByte (pStart + 1, pStart + 20, 13);
    if (pEnd <> POSERROR) then
    begin
      GetPGPVersion := BUF_GetString (pStart + 2, pEnd - pStart - 2);
      exit;
    end;
  end;
  GetPGPVersion := 'error'
end;

function GetINIPartName:string;
var p:TPos;
begin
  p := BUF_GetPosOfChar (1, 255, ']', CASESENSITIVE);
  GetINIPartName := BUF_GetString (1, p);
end;

function IsINI:boolean;
var i, p:TPos;
begin
  IsINI := false;

  if (b.First.c[1] <> '[') then
    exit;

  p := BUF_GetPosOfChar (1, 255, ']', CASESENSITIVE);
  if (p = POSERROR) then
    exit;

  for i:=2 to p - 1 do
    if (b.First.b[i] < 32) or (b.First.b[i] > 127) then
      exit;

  IsINI := true;
end;

function GetPostScriptCreator:string;
var p, p2:TPos;
begin
  p := BUF_GetPosOfStringCS (16, 100, '%%Creator: ');
  if (p <> POSERROR) then
  begin
    inc (p, 11); { length of '%%Creator ' }
    p2 := BUF_GetPosOfByte (p, p + 200, $0D); { find enter }
    if (p2 <> POSERROR) then
    begin
      GetPostScriptCreator := Buf_GetString (p, p2 - p);
      exit;
    end;
  end;

  GetPostScriptCreator := '';
end;

function Check_Textfiles:boolean;
var s:string;
    sFirst:string [100];
begin
  sFirst := BUF_GetString (1, 100);

  if (BUF_GetPosOfStringICS (1, 20, 'ECHO OFF') <> POSERROR) then
    s := 'BAT/CMD batch file'
  else
  if (IsPL) then
    s := 'Perl script file'
  else
  if (IsText) then
  begin
    if b.IsUnixText then
      s := 'Text file (Unix style)'
    else
      s := 'Text file (DOS style)';
  end
  else
  if (IsHTM) then
    s := concat ('HTML file ', GetHTMLTitle)
  else
  if (copy (sFirst, 1, 8) = 'REGEDIT4') then
    s := 'Windows registry file (textual)'
  else
  if (copy (sFirst, 4, 5) = 'Start') then
    s := 'Borland linker mapping file'
  else
  if (copy (sFirst, 1, 27) = '-----BEGIN PGP MESSAGE-----') then
    s := concat ('PGP message (Version: ', GetPGPVersion, ')')
  else
  if (IsINI) then
    s := concat ('INI file ', GetINIPartName)
  else
  if (pos ('begin 644', sFirst) > 0) or
     (pos ('begin 666', sFirst) > 0) then
    s := 'UUEnocded file'
  else
  if (copy (sFirst, 1, 8) = '<MIFFile') then
    s := 'Framemaker interchange file'
  else
  if (copy (sFirst, 1, 10) = '%!PS-Adobe') then
  begin
    s := GetPostScriptCreator;
    if (s > '') then
      s := concat (' (creator "', s, '")');
    s := concat ('Adobe PostScript file', s)
  end
  else
  if (copy (sFirst, 1, 7) = '^TITLE^') then
    s := 'aPatch data file'
  else
  if (pos ('#!/bin/sh', sFirst) > 0) then
    s := 'Bourne Shell Script'
  else
  if (UpString (copy (sFirst, 1, 11)) = 'BEGIN:VCARD') then
    s := 'Virtual Card File (VCF)'
  else
  if (copy (sFirst, 1, 6) = '{\rtf1') then
    s := 'RTF document (rich text format)'
  else
  if (copy (sFirst, 1, 16) = 'ibo Ablauf-Profi') then
    s := 'ibo Ablauf-Profi file'
  else
    s := '';

  if (s > '') then
  begin
    Check_Textfiles := true;
    if WriteHeader (s) then
      FinishLine;
  end
  else
    Check_Textfiles := false;
end;

end.
