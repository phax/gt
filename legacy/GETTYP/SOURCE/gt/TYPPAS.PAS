{
TypPAS - Pascal/Modual/C++/ADL source file detection

Copyright (c) 1997, 98 by Philip Helger (philip@dke.univie.ac.at)
}

{$i aflags.inc}

unit TypPAS;

interface

const
      KEYFile4 = 'k';

function Check_Source:boolean;

implementation

uses
     GTBuf, GTGlobal, GTOutput, GTRegist,
     XString;

var
    bPascal,
    bProgram,
    bLibrary,
    bUnit,
    bProcedure,
    bFunction,
    bModule,
    bModulaDef,
    bModulaImp:boolean;

    bProlog,
    bADL,
    bJava,
    bJavaScript,
    bC:boolean;

function GetNextString (StartPos:TPos;
                        csOkay:Charset;
                        csSpace:CharSet;
                        cEnd:char):string;
var s:string;
    ch:char;
    done:boolean;
begin
{ skip the first few spaces }
  ch := BUF_GetChar (StartPos);
  while (ch in csSpace) do
  begin
    inc (StartPos);
    ch := BUF_GetChar (StartPos);
  end;

  s := '';
  repeat
    ch := BUF_GetChar (StartPos);
    done := (ch = #0) or
            (not (ch in csOkay)) or
            (ch = cEnd);
    inc (StartPos);
    if not done then
      s := concat (s, ch);
  until done;
  GetNextString := s;
end;

function InitPascal:string;
const NameSet = ['0'..'9', 'a'..'z', 'A'..'Z', '_', '.'];
      SpaceSet = [#9, #10, #13, #32];
      MAXLEN = 14;
type TPASStr = String[MAXLEN];

    function GetStartPos_Pascal:TPos;
    var p:TPos;
        done:boolean;
        c:char;
    begin
      p := 0;
      done := false;
      repeat
        inc (p);
        c := BUF_GetChar (p);
        if (not IsSpaceChar (c)) then  { UString, skip easy comments }
          case c of
            '{':p := BUF_GetPosOfChar (p, b.BufLen, '}', CASESENSITIVE);
            '(':case BUF_GetChar (p + 1) of
                  '*':begin
                        p := BUF_GetPosOfStringCS (p + 1, b.BufLen, '*)');
                        if p > POSERROR then
                          inc (p);
                      end;
                else
                  done := true;
                end;
            else
              done := true;
          end;

        { check for buffer overflow }
        if (p >= b.BufLen) then
          p := POSERROR;
      until (done) or (p = POSERROR);
      GetStartPos_Pascal := p;
    end;

    function GetStartPos_C:TPos;
    var p:TPos;
        done:boolean;
        c:char;
    begin
      p := 0;
      done := false;

      repeat
        inc (p);
        c := BUF_GetChar (p);
        if (not IsSpaceChar (c)) then
          case c of
            '/':case BUF_GetChar (p + 1) of
                  '/':p := BUF_GetPosOfChar (p, b.BufLen, #10, CASESENSITIVE);
                  '*':begin
                        p := BUF_GetPosOfStringCS (p + 1, b.BufLen, '*/');
                        if p > POSERROR then
                          inc (p, 2);
                      end;
                else
                  done := true;
                end;
          else
            done := true;
          end;

        if (p >= b.BufLen) then
          p := POSERROR;
      until (done) or (p = POSERROR);

      GetStartPos_C := p;
    end;

var nStart_Pascal, nStart_C:TPos;
    s, temp:string;

    function CheckString1 (nStart:TPos; x:TPASStr):boolean;
    var len:byte;
    begin
      len := length (x);
      if (copy (temp, 1, len) = x) then
      begin
        CheckString1 := true;
        s := GetNextString (nStart + len, NameSet, SpaceSet, ';');
      end
      else
        CheckString1 := false;
    end;

    function CheckString2 (nStart:TPos; s1, s2:TPasStr):boolean;
    var len:byte;
    begin
      len := length (s1);
      CheckString2 := false;
      if (copy (temp, 1, len) = s1) then
        if (upstring (GetNextString (nStart + len, NameSet, SpaceSet, ' ')) = s2) then
          CheckString2 := true;
    end;

var p1, p2:byte;
begin
  bPascal    := false;
  bProgram   := false;
  bLibrary   := false;
  bUnit      := false;
  bProcedure := false;
  bFunction  := false;
  bModule    := false;
  bModulaDef := false;
  bModulaImp := false;

  bProlog     := false;
  bC          := false;
  bADL        := false;
  bJava       := false;
  bJavaScript := false;

  s := '';

  nStart_Pascal := GetStartPos_Pascal;
  if (nStart_Pascal > POSERROR) then
  begin
    { check registration info somewhere in the code ... }
    if (bInitialized) and (USER_NAME = '') then
      halt;
    { end check }

    temp := upstring (BUF_GetString (nStart_Pascal, 14));

         if CheckString1 (nStart_Pascal, 'PROGRAM')   then bProgram   := true
    else if CheckString1 (nStart_Pascal, 'LIBRARY')   then bLibrary   := true
    else if CheckString1 (nStart_Pascal, 'UNIT')      then bUnit      := true
    else if CheckString1 (nStart_Pascal, 'PROCEDURE') then bProcedure := true
    else if CheckString1 (nStart_Pascal, 'FUNCTION')  then bFunction  := true
    else if CheckString1 (nStart_Pascal, 'CONST')     then bPascal    := true
    else if CheckString1 (nStart_Pascal, 'TYPE')      then bPascal    := true
    else if CheckString1 (nStart_Pascal, 'USES')      then bPascal    := true
    else if CheckString1 (nStart_Pascal, 'VAR')       then bPascal    := true
    else if CheckString1 (nStart_Pascal, 'MODULE')    then bModule    := true
    else if CheckString2 (nStart_Pascal, 'DEFINITION', 'MODULE') then
    begin
      bModulaDef := true;
      { get startpos of "MODULE" }
      nStart_Pascal := BUF_GetPosOfChar (nStart_Pascal + 10, b.BufLen, 'M', CASEINSENSITIVE);
      { get startpos of next word }
      s := GetNextString (nStart_Pascal + 6, NameSet, SpaceSet, ';');
    end
    else if CheckString2 (nStart_Pascal, 'IMPLEMENTATION', 'MODULE') then
    begin
      bModulaImp := true;
      { get startpos of "MODULE" }
      nStart_Pascal := BUF_GetPosOfChar (nStart_Pascal + 14, b.BufLen, 'M', CASEINSENSITIVE);
      { get startpos of next word }
      s := GetNextString (nStart_Pascal + 6, NameSet, SpaceSet, ';');
    end;

    { check registration info somewhere in the code ... }
    if (bInitialized) and (USER_NAME = '') then
      halt;
    { end check }
  end;

  nStart_C := GetStartPos_C;
  if (nStart_C > POSERROR) then
  begin
    temp := upstring (BUF_GetString (nStart_C, 14));
         if CheckString1 (nStart_C, 'PREDICATES') then bProlog     := true
    else if CheckString1 (nStart_C, 'DOMAINS')    then bProlog     := true

    else if CheckString1 (nStart_C, '#INCLUDE')   then bC          := true
    else if CheckString1 (nStart_C, '#DEFINE')    then bC          := true
    else if CheckString1 (nStart_C, '#IF')        then bC          := true
    else if CheckString1 (nStart_C, '#IFDEF')     then bC          := true
    else if CheckString1 (nStart_C, '#IFNDEF')    then bC          := true
    else if CheckString1 (nStart_C, '#PRAGMA')    then bC          := true
    else if CheckString1 (nStart_C, 'TYPEDEF')    then bC          := true
    else if CheckString1 (nStart_C, 'EXTERN')     then bC          := true

    else if CheckString1 (nStart_C, 'IMPORT')     then bJava       := true
    else if CheckString1 (nStart_C, 'PACKAGE')    then bJava       := true
    else if CheckString1 (nStart_C, 'PRIVATE')    then bJava       := true
    else if CheckString1 (nStart_C, 'PROTECTED')  then bJava       := true
    else if CheckString1 (nStart_C, 'PUBLIC')     then bJava       := true

    else if CheckString1 (nStart_C, 'VAR')        then bJavaScript := true
    else if CheckString1 (nStart_C, 'FUNCTION')   then bJavaScript := true

    else if CheckString2 (nStart_C, 'WORKING',  'ENVIRONMENT') then bADL    := true
    else if CheckString2 (nStart_C, 'BUSINESS', 'PROCESS')     then bADL    := true

    else if CheckString2 (nStart_C, 'GLOBAL',   'PREDICATES')  then bProlog := true
    else;

    if bADL then
    begin
      s := BUF_GetString (nStart_C + 7, 255);  { get maximum string }
      p1 := pos ('<', s);
      p2 := pos ('>', s);
      temp := copy (s, p2 + 1, length (s));
      temp := copy (temp, 1, pos (#13, temp));  { only till end of line }
      s := copy (s, p1 + 1, p2 - p1 - 1);
      if (temp[2] <> ':') or
         (pos ('<', temp) = 0) or
         (pos ('>', temp) = 0) then
      begin
        bADL := false;
        s := '';
      end;
    end;
  end;

  InitPascal := s;
end;

function Check_Source:boolean;
const PASSRC = 'Pascal source: ';
var name:string;
begin
  { check registration info somewhere in the code ... }
  if (bInitialized) and (USER_NAME = '') then
    halt;
  { end check }
  name := InitPascal;
  if (name > '') or
     (bC) or
     (bJava) or
     (bProlog) or
     (bPascal) then
  begin
    Check_Source := true;
    name := concat ('"', name, '"');

    if bProgram then
      if WriteHeader (concat (PASSRC, 'program ', name)) then
        FinishLine;

    if bLibrary then
      if WriteHeader (concat (PASSRC, 'library ', name)) then
        FinishLine;

    if bUnit then
      if WriteHeader (concat (PASSRC, 'unit ', name)) then
        FinishLine;

    if bProcedure then
      if WriteHeader (concat (PASSRC, 'procedure ', name)) then
        FinishLine;

    if bFunction then
      if WriteHeader (concat (PASSRC, 'function ', name)) then
        FinishLine;

    if bModule then
      if WriteHeader (concat ('Modula program ', name)) then
        FinishLine;

    if bModulaDef then
      if WriteHeader (concat ('Modula definition file ', name)) then
        FinishLine;

    if bModulaImp then
      if WriteHeader (concat ('Modula implementation file ', name)) then
        FinishLine;

    if bPascal then
      if WriteHeader ('Pascal source code') then
        FinishLine;

    if bProlog then
      if WriteHeader ('Prolog source code') then
        FinishLine;

    if bC then
      if WriteHeader ('C/C++ source code') then
        FinishLine;

    if bADL then
      if WriteHeader (concat ('ADONIS export file ', name)) then
        FinishLine;

    if bJava then
      if WriteHeader ('Java source code') then
        FinishLine;

    if bJavaScript then
      if WriteHeader ('JavaScript source code') then
        FinishLine;
  end
  else
    Check_Source := false;
end;

end.
