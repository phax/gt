{$i build.inc}   { build counter }
{$i build2.inc}  { numbers for internal info }

const
      KEYFile1 = 'g';

{$ifndef WIN32}
const
      nDirLevel:byte = 0;

  {$ifdef GETTYP}
    {$I msg1.pas}
  {$else}
    {$I msg2.pas}
  {$endif}
  {$i vecball.pas}
{$endif}

var
    FilesFound, FilesIdentified:longint;

{
important note:
this procedure should NOT include any exit.
}
procedure CheckBuffer;
label Done;
var
{$ifdef GETTYP}
    bSave:boolean;
{$endif}
    bNothingFound:boolean;
begin
  if (bVerbose) then writeln ('CheckBuffer');

  ResetOutputParam;
  bNothingFound := false;

{$ifndef GETTYP}
  if (Check_EXE (0, false)) then goto Done;
  if (Check_COM (0))        then goto Done;
{$else}
  if (b.IsText) then
  begin
    if (Check_Textfiles) then goto Done;
    if (Check_Source)    then goto Done;
  end
  else
  begin
    if (Check_TPU)    then goto Done;
    if (Check_OBJ)    then goto Done;
    if (Check_Binary) then goto Done;
  end;

  { no check all that can be both: textfiles and binary files: }

  if (Check_EXE (0, false)) then goto Done;

  { e.g. TAR can be a text file ... }
  CheckForKnownPackers (0, b.FSize, true);
  if (GetPackerID <> NONE) then
  begin
    NotePacker;
    goto Done;
  end;

  if (Check_Image)    then goto Done;
  if (Check_Sound)    then goto Done;  { e.g. a AU sound file can be text }
  if (Check_SYS)      then goto Done;  { Check_SYS after Check_Image because
                                         otherwise PCD_Images are detected
                                         as device drivers :( }
  if (Check_RIFF)     then goto Done;  { e.g. a WAV file can be text }
  if (Check_Name)     then goto Done;
  if (Check_Headtext) then goto Done;
  if (Check_BinText)  then goto Done;  { e.g. PDF }

{ if still the headtext is not found, try checking as a COM file ;-) }
  if (Check_COM (0))  then goto Done;
{$endif}

  bNothingFound := true;

Done:

  inc (FilesFound);
  if (not bNothingFound) then
    inc (FilesIdentified);

{$ifdef GETTYP}
  if (not bDoOnlyStatistics) then
    if (bDoOnlyNotFound or (not bDoOnlyFound)) and
       (bNothingFound) then
    begin
      bSave := bDoOnlyNotFound;
      bDoOnlyNotFound := false;

      if (b.IsText) then
      begin
        if (b.IsUnixText) then
        begin
          if WriteHeader ('Textfile (UNIX format)') then
            FinishLine;
        end
        else
          if WriteHeader ('Textfile (DOS format)') then
            FinishLine;
      end
      else
        if WriteHeader ('Binary file') then
        begin
          FinishLine;
          if (b.Is7Bit) then
            Noteln ('File may be 7 bit only');
        end;

      bDoOnlyNotFound := bSave;
      if (not bListMode) then
        EmptyLine;
    end;
{$endif}

  if (not bNothingFound) and
     (not bDoOnlyNotFound) and
     (not bDoOnlyStatistics) and
     (not bListMode) then
    EmptyLine;
end;

{$ifdef GETTYP}

procedure OpenDir (sDisplayName, sOpenName:string);
var nPos:byte;
begin
  if (not bDoDirectories) then
    exit;

  nPos := LastPos ('\', sOpenName);
  if (nPos <> 0) then
    b.FName := UpString (copy (sOpenName, nPos + 1, 255))
  else
    b.FName := UpString (sOpenName);
  b.DisplayName := sDisplayName;

  if (WriteHeader ('Directory')) then
    FinishLine;

  if (not bDoOnlyNotFound) and
     (not bDoOnlyStatistics) and
     (not bListMode) then
    EmptyLine;
end;
{$endif}

procedure OpenFile (sDisplayName, sOpenName:string; nSize:longint);
begin
{$ifdef GETTYP}
  aFileTimer.DoContinue;
{$endif}

  BUF_Init (sDisplayName, sOpenName, nSize);
  if (b.BufLen > 0) then
    CheckBuffer;
  BUF_Done;

{$ifdef GETTYP}
  aFileTimer.DoBreak;
{$endif}
end;


procedure Search (sPath, sDisplayPath, sMask:TCmdString);
const NAME_2_LONG = 'Filename length overflow';
      DIR_2_LONG = 'Directory length overflow';
label CheckSubDirs;
var
    nError:integer;
    sr:XSearchRec;
{$ifdef MSDOS}
    sOpenName:string;
    lsr:LONG_SearchRec;
{$endif}

    procedure FindClose;
    begin
      if (bLongNames) then
      begin
{$ifdef MSDOS}
        if (LONG_FindClose (lsr) <> 0) then
          Noteln ('Internal error: LONG_FindClose');
{$else}
        InternalError ('findclose');
{$endif}
      end
      else
        XFindClose (sr);
    end;

    procedure StartSearch;
    begin
      if (bListMode) then
        Note (concat ('Scanning directory ', ExpandPath (sDisplayPath), ' '));

      if (bLongNames) then
      begin
{$ifdef MSDOS}
        nError := LONG_FindFirst (concat (sPath, sMask), XAnyFile, lsr);
        { 2 means "File not found" -> convert to 18 "no more files" ;-) }
        if (nError = 2) then
          nError := 18;
{$else}
        InternalError ('startsearch');
{$endif}
      end
      else
      begin
        nError := XFindFirst (concat (sPath, sMask), XAnyFile, sr);
        { 2 means "File not found" -> convert to 18 "no more files" ;-) }
        if (nError = 2) then
          nError := 18;
      end;
    end;

    procedure SearchOtherDirectories;
    begin
      if (bLongNames) then  { short names ... }
      begin
{$ifdef MSDOS}
        if (LONG_FindFirst (concat (sPath, '*.*'), XDirectory, lsr) = 0) then
          repeat
            { ATTENTION: lsr.name[0] }
            if ((lsr.attrib and XVolumeID = 0) and
                (lsr.attrib and XDirectory <> 0) and
                (lsr.name[0] <> '.')) then
            begin
              { shortname may be #0 -> use std. name }

              sOpenName := StrPas (@lsr.ShortName);
              if (sOpenName = '') then
                sOpenName := StrPas (@lsr.Name);

              if (word (length (sPath)) + word (length (sOpenName)) + 1 > 255) then
                Noteln (DIR_2_LONG);

              Search (concat (sPath, sOpenName, '\'),
                      concat (sDisplayPath, StrPas (@lsr.name), '\'),
                      sMask);
            end;
          until (LONG_FindNext (lsr) <> 0);
{$else}
          InternalError ('searchotherdirs');
{$endif}
      end  { long names ... }
      else
      begin
        if (XFindFirst (concat (sPath, '*.*'), XDirectory, sr) = 0) then
          repeat
            { ATTENTION: sr.name[1] }
            if ((sr.attr and XVolumeID = 0) and
                (sr.attr and XDirectory <> 0) and
                (sr.name[1] <> '.')) then
            begin
              if (word (length (sPath)) + word (length (sr.Name)) + 1 > 255) then
                Noteln (DIR_2_LONG);

              Search (concat (sPath, sr.Name, '\'),
                      concat (sPath, sr.Name, '\'),
                      sMask);
            end;
          until (XFindNext (sr) <> 0);
      end;

      { - and close ... - }
      FindClose;
    end;

label Done;
begin
{$ifndef WIN32}
  inc (nDirLevel);
  if (nDirLevel > 8) then
  begin
    Noteln ('Maximum search depth of 8 is reached.');
    Noteln (concat ('Can not scan ', sDisplayPath));
    {
    goto Done;
    }
  end;
{$endif}

  StartSearch;

  { ignore "no more files" error }
  if (nError <> 0) and (nError <> 18) then
  begin
    { - do not make a FileClose here because nothing was found !! - }
    if not bListMode then
      Note (concat (sDisplayPath, sMask, ' '));

    Append ('- ');
    NoteDOSError (nError);
    goto Done;
  end;

  if bListMode then
    FinishLine;

  if (nError <> 0) then
    goto CheckSubDirs;

  if (bLongNames) then
  begin
{$ifdef MSDOS}
    repeat
      { ATTENTION: lsr.name[0] }
      if (lsr.attrib and XVolumeID = 0) and
         (lsr.name[0] <> '.') then
      begin
        if (lsr.attrib and XDirectory = 0) then
        begin
          sOpenName := StrPas (@lsr.ShortName);
          if (sOpenName = '') then
            sOpenName := StrPas (@lsr.Name);

          if (word (length (sPath)) + word (length (sOpenName)) > 255) then
            Noteln (NAME_2_LONG);

          OpenFile (concat (sDisplayPath, StrPas (@lsr.Name)),
                    concat (sPath, sOpenName),
                    lsr.lowfilesize);

          if (bMaximumFiles) and (FilesIdentified >= nMaximumFiles) then
          begin
            FindClose;
            goto Done;
          end;
        end
{$ifdef GETTYP}
        else
          OpenDir (concat (sDisplayPath, StrPas (@lsr.Name)),
                   concat (sPath, sOpenName));
{$endif}
      end;
    until (LONG_FindNext (lsr) <> 0);
{$endif}
  end  { long names ... }
  else
  begin
    repeat
      { ATTENTION: sr.name[1] }
      if (sr.attr and XVolumeID = 0) and
         (sr.name[1] <> '.') then
      begin
        if (sr.attr and XDirectory = 0) then
        begin
          if (word (length (sPath)) + word (length (sr.Name)) > 255) then
            Noteln (NAME_2_LONG);

          OpenFile (concat (sPath, sr.Name),
                    concat (sPath, sr.Name),
                    sr.size);

          if (bMaximumFiles) and (FilesIdentified >= nMaximumFiles) then
          begin
            FindClose;
            goto Done;   { - do not check subdirs - }
          end;
        end
{$ifdef GETTYP}
        else
          OpenDir (concat (sPath, sr.Name),
                   concat (sPath, sr.Name));
{$endif}
      end;
    until (XFindNext (sr) <> 0);
  end;

  { - close both LONG and X - }
  FindClose;

CheckSubDirs:
  if bListMode then
    EmptyLine;

  if (not bDoSubs) then
    goto Done;

  SearchOtherDirectories;

Done:
{$ifndef WIN32}
  dec (nDirLevel);
{$endif}
end;

procedure GetKeyString;
var f:file;
    s:string;
    nRead:readtype;
begin
  assign (f, concat (GetPath (ParamStr (0)),
                     KeyFile1,
                     KeyFile2,
                     KeyFile3,
                     KeyFile4,
                     KeyFile5,
                     KeyFile6));
  reset (f, 1);
  if (ioresult <> 0) then
    exit;
  blockread (f, s[1], 255, nRead);
  close (f);
  SetLength (s, nRead);
  DecodeString (s);
end;

procedure CopyrightNote;
begin
{$ifdef GETTYP}
  Noteln (concat ('--- # GetTyp 2.', LeadingZero (BuildNr, 2), ' # ----------------- # Copyright (c) 1997-99 by PHaX # ---'));
{$else}
  Noteln (concat ('--- # GetEXETyp 2.', LeadingZero (BuildNr, 2), ' # -------------- # Copyright (c) 1997-99 by PHaX # ---'));
{$endif}
  Noteln ('--- # phax@writeme.com # ---------------------- # http://surf.to/phax # ---');

  PrintRegisteredString;

  if (bLongNames) then
    Noteln ('Using long filenames ...');

  FinishLine;
end;

procedure DrawSyntax;
begin
  bDoPause := true;
  FinishLine;

{$ifdef GETTYP}

  Noteln ('Syntax of GetTyp: GT [options...] [filename] [options...]');
  EmptyLine;
  Noteln ('Some options (starting with "-" or "/"):');
  Noteln ('  /Cn     - set length of internal EXE cache to n bytes (5-80)');
  Noteln ('  /F      - display only identified files');
  Noteln ('  /H, /?  - this help screen');
  Noteln ('  /I      - show internal information');
  Noteln ('  /L      - write output in list form');
  Noteln ('  /Mn     - Stop after n found files');
  Noteln ('  /NA     - do not list archive contents');
  Noteln ('  /NC     - do not use the configuration file');
  Noteln ('  /ND     - do not show directories as identified');
  Noteln ('  /NH     - do not use heuristics for startup code detection');
  Noteln ('  /NL     - do not use long filenames under Windows 95/98');
  Noteln ('  /NS     - do not print final summary');
{$ifdef MSDOS}
  Noteln ('  /NX     - do not use XMS');
{$endif}
  Noteln ('  /OUTfoo - save output to file "foo"');
  Noteln ('  /OVER   - overwrite file given with /OUT without asking');
  Noteln ('  /Pn     - pause after n lines of output (default is 24)');
  Noteln ('  /S      - search subdirectories too');
  if (bInitialized) then
  begin
    Noteln ('  /XO     - extended OBJ listing');
    Noteln ('  /ZE     - show LE/NE/PE header information anyway');
    Noteln ('  /ZH     - suppress output of header');
    Noteln ('  /ZN     - show only not identified files');
    Noteln ('  /ZS     - show only final statistics');
  end;
  Noteln ('Please refer to the documentation for all switches.');

{$else}

  Noteln ('Syntax of GetEXE: GE [options...] [filename] [options...]');
  Noteln ('GetEXE is a small program to detect modifier of executables.');
  EmptyLine;
  Noteln ('Options (starting with "-" or "/"):');
  Noteln ('  /I      - show internal information');
  Noteln ('  /L      - write (less) output in list form');
  Noteln ('  /Mn     - Stop after n found files');
  Noteln ('  /NC     - do not use the configuration file');
  Noteln ('  /OUTfoo - save output to file "foo"');
  Noteln ('  /OVER   - overwrite file given with /OUT without asking');
  Noteln ('  /NL     - do not use long filenames under Windows 95/98');
{$ifdef MSDOS}
  Noteln ('  /NX     - do not use XMS');
{$endif}
  Noteln ('  /Pn     - pause after n lines of output (default is 24)');
  Noteln ('  /S      - search subdirectories too');
  if (bInitialized) then
  begin
    Noteln ('  /ZE     - show extended EXE header anyway');
    Noteln ('  /ZH     - show no startup logo');
  end;

{$endif}

  halt (1);
end;

procedure GetPathFileSpec (Nr:byte; var path, filespec:string);
var p:byte;
begin
{ init vars: path is empty; filespec is all }
  path := '';
  filespec := upstring (CmdLine.GetNonOptionString (Nr));

{ move all the '...\' parts to the path }
  repeat
    p := GetCharPos ('\', filespec);
    if (p > 0) then
    begin
      path := concat (path, copy (filespec, 1, p));
      delete (filespec, 1, p);
    end;
  until p = 0;

{ check for x:*.bla }
  p := GetCharPos (':', filespec);
  if (p > 0) then
  begin
    path := concat (copy (filespec, 1, p), path);
    delete (filespec, 1, p);
  end;

{ check if filemask is empty }
  if filespec = '' then
    filespec := '*.*'
  else
{ auto extend e.g. a* to a*.* }
    if (lastchar (filespec) = '*') and (GetCharPos ('.', filespec) = 0) then
      filespec := concat (filespec, '.*');

{ insert a default path - only if it is no UNC path!! }
{$ifdef WIN32}
  if (length (path) > 2) and (path[1] <> '\') and (path[2] <> '\') then
{$else}
  if (length (path) > 2) then
{$endif}
    path := ExpandPath (path);
end;

{ still in ANSI - but that's okay - }
procedure ShowInternalInformation;
begin
  IncInsLevel;  { - to avoid the "-" chars - }
  Noteln (        'ÚÄ--ú Internal information ú--Ä-ÄÄ-ÄÄ-ÄÄ-ÄÄ-ÄÄ-ÄÄ-ÄÄ-Ä--ú');
  Noteln (concat ('³ Version 2.', LeadingZero (BuildNr, 2)));
{$ifdef VER60}
  Noteln (concat ('³ Built on ', BuildDate, ', ', BuildTime, ' with Turbo Pascal 6'));
{$endif}
{$ifdef VER70}
  Noteln (concat ('³ Built on ', BuildDate, ', ', BuildTime, ' with Turbo Pascal 7'));
{$endif}
{$ifdef VER100}
  Noteln (concat ('³ Built on ', BuildDate, ', ', BuildTime, ' with Borland Delphi 3'));
{$endif}
{$ifdef VER120}
  Noteln (concat ('³ Built on ', BuildDate, ', ', BuildTime, ' with Borland Delphi 4'));
{$endif}
  Noteln (        'ÃÍ--ú');
  Noteln (        '³ GreetinX to Ado of B.O.C. - ADONIS RuleZ!!!');
  Noteln (        ':');
  Noteln (        '³ This version is able to:');
  Noteln (        'ÃÍ--ú');
{$ifdef GETTYP}
  Noteln (concat ('³ detect ', i2s (ARC_DETECT), ' archive formats and list ', i2s (ARC_LIST), ' of them'));
  Noteln (concat ('³ detect ', i2s (IMAGE_DETECT), ' different image formats'));
  Noteln (        'ÃÍ--ú');
{$endif}
  Noteln (concat ('³ detect ', i2s (EXE_MODIFIER), ' DOS EXE and DOS COM modifier/compiler'));
  Noteln (        'ÃÍ--ú');
  Noteln (concat ('³ detect ', i2s (PE_MODIFIER), ' PE EXE modifier/compiler'));
  Noteln (        'ÃÍ--ú');
  Noteln (        '³ Btw. you can copy this program freely.');
  Noteln (        ':');
  DecInsLevel;
  halt (1);
end;

{$ifdef GETTYP}
procedure FinalStatistics;
var dMilliSec:real;
    nMin, nSec:word;

    function GetSecondsStr:string;
    const SEC = 1000;
          MIN = 60 * SEC;
    begin
      if (dMilliSec > MIN) then
      begin
        nMin := Trunc (dMilliSec / MIN);
        dMilliSec := dMilliSec - (longint (nMin) * MIN);
        nSec := Trunc (dMilliSec / SEC);
        GetSecondsStr :=  concat (i2s (nMin), ':', LeadingZero (nSec, 2), ' min');
      end
      else
        if (dMilliSec > SEC) then
          GetSecondsStr := concat (r2sfd (dMilliSec / SEC, 0, 2), ' secs')
        else
          GetSecondsStr := concat (r2sfd (dMilliSec, 0, 1), ' ms');
    end;

begin
{ - totaly reset insert level - }
  InsLevel := 1;

{ - note the number of files found - }
  Note ('Files identified: ');

{ - note the number of files identified - }
  if (FilesFound > 0) then  { - else div by zero! - }
  begin
    Appendln (concat (i2s (FilesIdentified), ' of ',
                      i2s (FilesFound),
                      ' (',
                      r2sfd (FilesIdentified / FilesFound * 100, 0, 2),
                      '%) '));
  end
  else
    Appendln ('0 of 0');

{ - note the absolute time needed - }
  dMilliSec := aTotalTimer.GetMilliSeconds;
  Note (concat ('Total time:       ', GetSecondsStr));

  if (FilesFound > 0) then
  begin
{ - note the number of files identified - }
    if (FilesIdentified > 0) then
    begin
      dMilliSec := aFileTimer.GetMilliSeconds / FilesFound;
      Append (concat (' (', GetSecondsStr, '/file)'));
    end;

{ - note the time that was lost due to search and opne ... - }
    dMilliSec := aTotalTimer.GetMilliSeconds - aFileTimer.GetMilliSeconds;
    if (dMilliSec > 0) then
      Append (concat (' (', GetSecondsStr, ' lost)'));
    FinishLine;
  end
  else
    FinishLine;

{ do archive statistics }
  if (ArcCount > 0) then
  begin
    EmptyLine;
    Noteln (concat ('Archives found:   ', i2s (ArcCount)));
    Note   (concat ('Original size:    ', i2s (ArcSumOld), ' KB = ',
                                          r2sfd (ArcSumOld / 1024, 0, 2), ' MB'));
    if (GTPACK_OldSizeUndefined) then
      Append (' + ???');
    FinishLine;

    Note   (concat ('Packed size:      ', i2s (ArcSumNew), ' KB = ',
                                          r2sfd (ArcSumNew / 1024, 0, 2), ' MB'));
    if (GTPACK_NewSizeUndefined) then
      Append (' + ???');
    FinishLine;
  end;
end;
{$endif}

procedure Main;
var
    where, what:string;
    i:byte;
{$ifndef WIN32}
    MsgPtr:pointer;
{$endif}
begin
  { - open output handle - set to screen - }
  OpenHandle ('', true);

{$ifdef GETTYP}
  aTotalTimer.Start;
{$endif}

{$ifndef WIN32}  { the hidden message ;-) }
  MsgPtr := @InlineMessage1;
{$endif}

{$ifndef WIN32}
  if (cmdline.IsFlag ('PHAX')) then
  begin
    InlineMessage4711;
    halt;
  end;
{$endif}

  GetKeyString;

  if (not cmdline.IsFlag ('NC')) then
    ReadConfigFile;
  EvaluateCommandLine;

  if (bInitialized) and (USER_NAME = '') then
    halt;

{ - init variables - }
  FilesFound := 0;
  FilesIdentified := 0;

{ globally mark all files as readonly... }
  FileMode := FILEMODE_READ;

{
  the copyright notice may only appear
  after cmdline was checked and cfg file was read.
}
  if bDoHeader then
    CopyrightNote;

  if bDoHelp then
    DrawSyntax;

  if bDoInternal then
    ShowInternalInformation;

  if (bInitialized) and (USER_NAME = '') then
    halt;

  if bDoOnlyStatistics then
  begin
    Noteln ('Working, please stand by ...');
    EmptyLine;
  end;

  InitStartupCodes;

{$ifdef GETTYP}
  aFileTimer.Start;
  aFileTimer.DoBreak;
{$endif}

  if cmdline.GetNonOptionStringCount = 0 then
    Search ('', '', '*.*')
  else
    for i:=1 to cmdline.GetNonOptionStringCount do
    begin
      GetPathFileSpec (i, where, what);
{$ifndef WIN32}
      nDirLevel := GetCharCount ('\', where) + GetCharCount ('/', where);
{$endif}
      Search (where, where, what);
    end;

{$ifdef GETTYP}
  aFileTimer.Stop;
  aTotalTimer.Stop;

  if (bDoOnlyStatistics) or (bDoStatistics) then
    FinalStatistics;
{$endif}
end;
