{$i aflags.inc}

unit GTConfig;

interface

const
      KEYFile6 = 'y';

procedure EvaluateCommandLine;
procedure ReadConfigFile;

implementation

uses
     GTBuf, GTGlobal, GTMem, GTOutput,
{$ifdef GETTYP}
     GTPack,
{$endif}
     INIFile,
     XCmdLine, XString, XWrite;

const
{$ifdef GETTYP}
      CFGFile  = 'GT.CFG';
      CFGFile2 = 'GETTYP.CFG';
{$else}
      CFGFile  = 'GE.CFG';
      CFGFile2 = 'GETEXE.CFG';
{$endif}

      VALUE_ERR1 = 'Value of switch /';
      VALUE_ERR2 = ' is invalid. Should be ';

procedure ReadConfigFile;
var I:TINIFile;
    sName:string;
    aEntry:TINIEntry;

    procedure SwitchValueError (s:string);
    begin
      I.PrintError (concat (VALUE_ERR1, sName, VALUE_ERR2, s));
    end;

begin
{ check if INI file exists }
  if (not I.init (CFGFile2)) then
    if (not I.init (CFGFile)) then  { - because of Delphi :( - }
      if (not I.Init (concat (GetPath (ParamStr (0)), CFGFile))) then
        exit;

  while (I.GetNextEntry (sName, aEntry)) do
    if ((sName[1] = '/') or (sName[1] = '-')) and (length (sName) > 1) then
    begin
      Delete (sName, 1, 1); { skip "/" or "-" }
      sName := Upstring (sName);
      if (sName[1] = 'C')  then
      begin
        nMaximumCompare := s2i (copy (sName, 2, 255)) and $000000FF;
        if (nMaximumCompare < 5) or (nMaximumCompare > 80) then
          SwitchValueError ('between 5 and 80.');
      end
{$ifdef GETTYP}
      else if (sName = 'F')    then bDoOnlyFound       := true
      else if (sName = 'NA')   then bDoShowArcContent  := false
      else if (sName = 'ND')   then bDoDirectories     := false
      else if (sName = 'NS')   then bDoStatistics      := false
{$endif}
      else if (sName = 'L')    then bListMode := true
      else if (sName[1] = 'M')    then
      begin
        bMaximumFiles := true;
        nMaximumFiles := s2i (copy (sName, 2, 255));
        if (nMaximumFiles <= 0) then
          SwitchValueError ('greater than 0.');
      end
      else if (sName[1] = 'P')    then
      begin
        bDoPause := true;
        nPauseLines := s2i (copy (sName, 2, 255));
        if (nPauseLines <= 0) then
          nPauseLines := 24;
      end
      else if (sName = 'NH')   then bHeuristics        := false
      else if (sName = 'NL')   then bLongNames         := false
{$ifdef XMS}
      else if (sName = 'NX')   then GTMem_UseXMS       := false
{$endif}
      else if (sName = 'OVER') then bOverwriteFile     := true
      else if (sName = 'S')    then bDoSubs            := true
      else if (sName = 'V')    then bVerbose           := true
      else if (sName = 'YB')   then bOutputBreak       := true
      else if (sName = 'ZA')   then bSearchARI         := true
      else if (sName = 'ZH')   then bDoHeader          := false
      else if (sName = 'ZN')   then bDoOnlyNotFound    := true
      else if (sName = 'ZE')   then bDoEXEHEaderAnyway := true
      else I.PrintError (concat ('Unknown switch "/', sName, '"'));
    end
    else  { neither "=" nor a switch }
      I.PrintError ('Entry is no switch (start with "-" or "/")');
  I.done;
end;

procedure EvaluateCommandLine;
var nDummy:longint;
    sDummy:string;

    procedure SwitchValueError (cSwitch:char;
                                s:string);
    begin
      Noteln (concat (VALUE_ERR1, cSwitch, VALUE_ERR2, s));
      halt (2);
    end;

    procedure CheckPlusFlag (var b:boolean;
                                 s:TParamStr);
    begin
      b := b or CmdLine.IsFlag (s);
    end;

    procedure CheckMinusFlag (var b:boolean;
                                  s:TParamStr);
    begin
      b := b and (not  CmdLine.IsFlag (s));
    end;

begin
{$ifdef GETTYP}
  CheckPlusFlag  (bDoOnlyFound,      'f');
  CheckMinusFlag (bDoShowArcContent, 'na');
  CheckMinusFlag (bDoDirectories,    'nd');
  CheckMinusFlag (bDoStatistics,     'ns');
{$endif}

  CheckPlusFlag  (bVerbose,           'v');
  CheckPlusFlag  (bDoHelp,            'h');
  CheckPlusFlag  (bDoHelp,            '?');
  CheckPlusFlag  (bDoInternal,        'i');
  CheckPlusFlag  (bListMode,          'l');
  CheckMinusFlag (bHeuristics,        'nh');
  CheckMinusFlag (bLongNames,         'nl');
{$ifdef XMS}
  CheckMinusFlag (GTMem_UseXMS,       'nx');
{$endif}
  CheckPlusFlag  (bOverwriteFile,     'over');
  CheckPlusFlag  (bDoPause,           'p');
  CheckPlusFlag  (bDoSubs,            's');

{ undcoumented ... }
  CheckPlusFLag  (bOutputBreak,       'yb');

  CheckPlusFlag  (bSearchARI,         'za');
{ undocumented ... }
  CheckMinusFlag (bDoHeader,          'zh');
  CheckPlusFlag  (bDoOnlyNotFound,    'zn');
  CheckPlusFlag  (bDoOnlyStatistics,  'zs');
  CheckPlusFlag  (bDoEXEHeaderAnyway, 'ze');

  { - set max number of files to scan - }
  if (cmdline.IsFlag ('M')) and (nMaximumFiles = 0) then
  begin
    bMaximumFiles := true;
    nMaximumFiles := cmdline.GetInteger ('M');
    if (nMaximumFiles <= 0) then
      SwitchValueError ('M', 'greater than 0.');
  end;

  { - set compare length - }
  if (cmdline.IsFlag ('C')) then
  begin
    nDummy := cmdline.GetInteger ('C') and $000000FF;
    if (nDummy < 5) or (nDummy > 80) then
      SwitchValueError ('C', 'between 5 and 80.');
    nMaximumCompare := nDummy;
  end;

  { - set max number of files to scan - }
  if (cmdline.IsFlag ('P')) then
  begin
    bDoPause    := true;
    nPauseLines := cmdline.GetInteger ('P');
    if (nPauseLines <= 0) then
      nPauseLines := 24;
  end;

  { - redirect output to file - }
  if (cmdline.IsFlag ('OUT')) then
  begin
    bOutputToFile := true;
    sDummy := cmdline.GetString ('OUT');
    if (sDummy > '') then
    begin
      if (HandleCanBeOpened (sDummy, bOverwriteFile)) then
        Noteln (concat ('Saving output to the file "', sDummy, '".'))
      else
        Noteln (concat ('The file "', sDummy, '" already exists.'));

      CloseHandle;
      OpenHandle (sDummy, bOverwriteFile);
    end;
  end;

{$ifndef WIN32}
  if (bOutputRedirected) then
    bDoPause := false;
{$endif}

  if (bOutputToFile) then
    bDoPause := false;
end;

end.
