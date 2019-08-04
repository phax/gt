program GT;

{$i aflags.inc}

{ if it is BP 7, disabled the typesafe @ operator in this file }
{$ifdef VER70}
{$T-}
{$endif}

{$ifndef VIRTUALPASCAL}
{$m 50000, 0, 655360}
{$endif}

uses
     XWrite,  { should be first unit! - last ExitProc!! }
     WinDOS,
     SysUtils,
     GTBuf, GTConfig, GTGlobal, GTOutput,
     IOError,
     GTCodes,
     XCmdLine, XDOS, XGlobal, XString,
     GTRegist,
{$ifdef GETTYP}
     XTimer,
     GTBin, GTFile, GTImage, GTName,
     GTPack, GTSound, GTText,
     TypOBJ,
     TypPAS,
     TypSYS,
     TypRIFF,
     TypTPU,
{$endif}
{ just for dependencies }
     TypEXE;

{$i gtmain.pas}

begin
  Main;
end.
