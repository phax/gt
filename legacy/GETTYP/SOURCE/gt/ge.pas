program GE;

{$i aflags.inc}

{ if it is BP 7, disabled the typesafe @ operator in this file }
{$ifdef VER70}
{$T-}
{$endif}

{$m 62000, 0, 655360}

uses
     XWrite,  { should be first unit! - last ExitProc!! }
{$ifdef WINDOWS}
     WinDOS,
{$else}
     DOS,
     XMS,
{$endif}
     GTBuf, GTConfig, GTGlobal, GTOutput, GTRegist,
     XCmdLine, XDOS, XGlobal, XString,
     GTCodes,
     IOError,
{ just for dependencies }
     TypEXE;

{$i gtmain.pas}

begin
  Main;
end.