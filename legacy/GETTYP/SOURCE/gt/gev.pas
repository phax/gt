program GEV;

{$i aflags.inc}

uses
     XWrite,  { should be first unit! - last ExitProc!! }
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
