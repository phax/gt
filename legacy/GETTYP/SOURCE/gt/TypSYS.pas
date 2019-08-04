{
TypSYS - Device driver ...

Copyright (c) 1997, 98 by Philip Helger (philip@dke.univie.ac.at)
}

{$i aflags.inc}

unit TypSYS;

interface

uses
     GTBuf;

var
    SYS_Header:record
      AdrOfNextDriver:longint;
      Attr:word;
      StrategyOfs:word;
      InterruptOfs:word;
      Name:array[1..8] of char;
    end;

const
      SYS_HEADER_SIZE = SizeOf (SYS_Header);

function Check_SYS:boolean;
procedure SYS_List;

function SYS_IsDeviceDriver:boolean;

implementation

uses
     GTCompar, GTFile, GTGlobal, GTOutput,
     GTData__,
     XGlobal, XString;

{
SYS file handling is a little bit complicated, because
sys drivers maybe standalone files (e.g. HIMEM.SYS) or they
may be included in EXE files (e.g. EMM386.EXE).
So I provide a generic interface for both EXE and SYS files.
}

function SYS_IsDeviceDriver;
begin
  SYS_IsDeviceDriver := (SYS_Header.AdrOfNextDriver = longint ($FFFFFFFF)) and
                        (SYS_Header.StrategyOfs < b.FSize);
{
  problems with UPX packed files :((
                        (SYS_Header.InterruptOfs < b.FSize);
}
end;

function IsSYS:boolean;
begin
  BUF_GetBuf (CompareBuf, 1, SizeOf (CompareBuf));
  move (CompareBuf, SYS_Header, SYS_HEADER_SIZE);
  IsSys := SYS_IsDeviceDriver;
end;

procedure SYS_List;
var bBDD:boolean; { block device driver }
begin
  bBDD := hi (SYS_Header.Attr) and BITS[7] = 0;

  if bBDD then
    Noteln (concat ('Block device driver with ', i2s (ord (SYS_Header.Name[1])), ' device(s)'))
  else
    Noteln (concat ('Character device driver "', SYS_Header.Name, '"'));

{ check for known SYS packers }
  if (not ComparePtrBuf (@REC_SYS_SysPack01)) then
  if (not ComparePtrBuf (@REC_SYS_UPX020_040)) then
  if (not ComparePtrBuf (@REC_SYS_UPX051)) then
  if (not ComparePtrBuf (@REC_SYS_UPX081)) then
  ;
end;

function Check_SYS:boolean;
begin
  Check_SYS := false;
  if IsSYS then
  begin
    Check_SYS := true;
    if WriteHeader ('Device driver') then
    begin
      FinishLine;
      SYS_List;
    end;
  end;
end;

end.
