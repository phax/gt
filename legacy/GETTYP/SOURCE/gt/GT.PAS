program GT;

{$i aflags.inc}

{ if it is BP 7, disabled the typesafe @ operator in this file }
{$ifdef VER70}
{$T-}
{$endif}

{$ifndef VIRTUALPASCAL}
{$ifndef __TMT__}
{$m 50000, 0, 655360}
{$endif}
{$endif}

uses
     XWrite,  { should be first unit! - last ExitProc!! }
{$ifdef GETTYP}
     GTFile,  { should be second because of the logfile }
{$endif}
{$ifdef WIN32}
     SysUtils,
{$else}
  {$ifdef WINDOWS}
     WinDOS,
  {$else}
     DOS,
    {$ifdef XMS}
     XMS,
    {$endif}
  {$endif}
{$endif}
     GTBuf, GTConfig, GTGlobal, GTOutput,
     IOError,
     XCmdLine, XDOS, XGlobal, XString,
     GTRegist,
     GTCodes,
{$ifdef GETTYP}
     XTimer,
     GTBin, GTImage, GTName,
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
