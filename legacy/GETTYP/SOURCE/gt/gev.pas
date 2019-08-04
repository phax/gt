program GEV;

{$i aflags.inc}

uses
     XWrite,  { should be first unit! - last ExitProc!! }
     WinDOS,
     SysUtils,
     GTBuf, GTConfig, GTGlobal, GTOutput, GTRegist,
     XCmdLine, XDOS, XGlobal, XString,
     IOError,
     GTCodes,
{ just for dependencies }
     TypEXE;

{$i gtmain.pas}

begin
  Main;
end.
