{
TypOBJ - Borland / Microsoft object file ...

Copyright (c) 1997, 98 by Philip Helger (philip@dke.univie.ac.at)
}

{$i aflags.inc}

unit TypOBJ;

interface

const
      KEYFile5 = 'e';

function Check_OBJ:boolean;

implementation

uses
     GTBuf, GTCompar, GTFile, GTGlobal, GTOutput, GTRegist,
     GTData__,
     XString;

const
      THEADR  = $80;                { x T-Module Header Record             }

function CompareByteMask:boolean;
begin
  move (b.First, CompareBuf, 256);
  if ComparePtrBuf (@REC_BINOBJ) then
  begin
    CompareByteMask := true;
    Noteln ('BINOBJ converted object file');
    Noteln (concat ('Exported procedure: "', BUF_GetPascalString (34), '"'));
  end
  else
    CompareByteMask := false;
end;

function Check_OBJ:boolean;
begin
  Check_OBJ := false;
  if (b.First.b [1] = THEADR) and
      ((BUF_GetWord (2) = b.First.b [4] + 3) or
       (BUF_GetWord (2) = b.First.b [4] + 2)) then
  begin
    Check_OBJ := true;
    if WriteHeader ('Object file') then
    begin
      FinishLine;

      if (CompareByteMask) then;
    end;
  end;
end;

end.
