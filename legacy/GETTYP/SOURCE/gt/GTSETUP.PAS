{$define extended}
{$i aflags.inc}

program GTCfg;

uses
     XWrite,
     GTOutput,
     INIFile,
     XCrt, XString;

type
     TSwitchOutput = record
       name:string[3];
       askstr:string[80];
       cfgstr:string[80];
       descri:string [158];
       mode:(yesno, value);
       { only if mode=value }
       min:longint;
       max:longint;
       select:boolean;
     end;

const
      max_switch = 17;
      CFG_FILENAME = 'GT.CFG';

      switches:array [1..max_switch] of TSwitchOutput = (
        (name:'F';
         askstr:'Would you like to display only the identified files?';
         cfgstr:'/f     ; show only identified files';
         descri:'will only display files that are@' +
                'identified. If you select this option, GT will display unknown files as@' +
                '"text file" or "binary file".';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'L';
         askstr:'Would you like to enable list mode?';
         cfgstr:'/l     ; enable list mode';
         descri:'displays the result in list form.@' +
                'This is especially good for directory or drive listings.@' +
                'Not recommended in the configuration file.';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'P';
         askstr:'Would you like to pause after every screen?';
         cfgstr:'/p@@@  ; pause after every n lines of screen output';
         descri:'will pause after every n lines that@' +
                'are displayed on the screen.';
         mode:value;
         min:1;
         max:2147483647;
         select:false),
        (name:'S';
         askstr:'Would you always like to recurse into subdirectories?';
         cfgstr:'/s     ; recurse into subdirectories';
         descri:'will always search the current directory@' +
                'and all subdirectories.@' +
                'Not recommended in the configuration file.';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'ZH';
         askstr:'Would you like to suppress the header?';
         cfgstr:'/zh    ; suppress logo and copyright';
         descri:'wont display the copyright message.@' +
                'Only recommended if you have the registered version.';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'ZE';
         askstr:'Would you like to see the extended EXE header anyway?';
         cfgstr:'/ze    ; always display extended EXE header info';
         descri:'will display all header infos of LE/NE/PE@' +
                'executables even if a compiler/modifier was detected.@' +
                'Only recommended for professionals.';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'XO';
         askstr:'Would you like to see the extended object file listing?';
         cfgstr:'/xo    ; display extended object file listing';
         descri:'will display some more information about@' +
                'object files (not the COFF format).@' +
                'Not recommended in the configuration file.';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'C';
         askstr:'Would you like to change the EXE compare buffer?';
         cfgstr:'/c@@   ; EXE buffer compare length';
         descri:'will use the given buffer length to@' +
                'compare EXE startup codes for detecting compiler/modifier...@' +
                'Only recommended for professionals.';
         mode:value;
         min:5;
         max:80;
         select:false),
        (name:'M';
         askstr:'Would you always like GT to stop after "n" files?';
         cfgstr:'/m@@@@@@@@@@   ; stop after n found files';
         descri:'will always stop searching after "n"@' +
                'found files. This contains both identified and not identified files.@' +
                'Not recommended in the configuration file.';
         mode:value;
         min:1;
         max:2147483647;
         select:false),
        (name:'NA';
         askstr:'Would you like to disable the listing of archives?';
         cfgstr:'/na    ; do not show archive contents';
         descri:'wont display the content of an archive@' +
                'like ARJ or ZIP. This option may be interesting if you only want to@' +
                'identify the files and want no further information.';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'ND';
         askstr:'Would you like to disable the identification of directories?';
         cfgstr:'/nd    ; no directory identification';
         descri:'wont display directories otherwise it@' +
                'will simply detect them as "directory".';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'NH';
         askstr:'Would you like to disable the heursitic identification?';
         cfgstr:'/nh    ; no heuristic identification';
         descri:'wont display EXE/COM modifier where it@'+
                'is not 100% sure.';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'NS';
         askstr:'Would you like to disable the final summary?';
         cfgstr:'/ns    ; print no final summary';
         descri:'wont display the final summary where GT@' +
                'displays the number of files found, the number of identified files and@' +
                'some times (per file, total, lost).';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'NL';
         askstr:'Would you like to disable long filename support?';
         cfgstr:'/nl    ; no long filenames under Windows 9x';
         descri:'will not use long filenames even if you@' +
                'are running Windows 95 or 98.@' +
                'Only recommended it if you have any problems with the long filenames.';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'NX';
         askstr:'Would you like to disable XMS support?';
         cfgstr:'/nx    ; disable XMS support';
         descri:'wont use XMS (extended memory) only the@' +
                '640K conventional memory.@' +
                'Only recommended it if you have any problems with the XMS.';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'OB';
         askstr:'Would you like to force newlines in output?';
         cfgstr:'/ob    ; force newline in output';
         descri:'will insert a newline after every 80@' +
                'characters. This option is only interpreted if you redirect the output@' +
                'of GT to a file (e.g. "GT *.exe > exefiles.txt").';
         mode:yesno;
         min:0;
         max:0;
         select:false),
        (name:'OVER';
         askstr:'Would you like to overwrite existing files when using /OUT?';
         cfgstr:'/over  ; overwrite without asking';
         descri:'will only be used when using the /OUT@' +
                'parameter. Then GT wil overwrite an existing file without asking.@'+
                'If the file exists an /OVR is not given the output goes to the screen.';
         mode:yesno;
         min:0;
         max:0;
         select:false)
      );

var
    t:text;

function getch:char;
var c:char;
begin
  c := readkey;
  if (c = #0) then
  begin
    c := readkey;
    c := #0;
  end;
  getch := c;
end;

function GetYN (i:byte):boolean;
var ch:char;
    s:string;
    bYes:boolean;
    bNo:boolean;
    bLeave:boolean;
begin
  s := 'Your choice (';
  if (switches [i].select) then
    s := concat (s, 'Y/n): ')
  else
    s := concat (s, 'y/N): ');

  Note (s);

  repeat
    ch := upcase (getch);
    bYes := (ch = 'Y') or (ch = 'J');
    bNo := (ch = 'N');
    bLeave := (ch = #13) or (ch = #27);
  until (bYes) or (bNo) or (bLeave);

  Appendln (ch);

  if (bLeave) then
    if (switches [i].select) then
      bYes := true
    else
      bNo := true;

  { write to gt.cfg }
  if (bYes) and (switches [i].mode = yesno) then
    writeln (t, switches [i].cfgstr);

  GetYN := bYes;
end;

procedure GetValue (i:byte);
label Loop;
var s:string;
    value:longint;
    len:byte;
begin
  if (GetYN (i)) then
  begin
Loop:
    Note (concat ('Please insert a number between ', i2s (switches [i].min),
                  ' and ', i2s (switches [i].max), ': '));

    readln (s);
    value := s2i (s);
    if (value < switches [i].min) or (value > switches [i].max) then
    begin
      Noteln ('Invalid input');
      goto Loop;
    end;

    s := switches [i].cfgstr;
    len := GetCharCount ('@', s);
    OverwriteString (s, pos ('@', s), LeadingZero (value, len));
    { write to gt.cfg }
    writeln (t, s);
  end;
end;

procedure InitSwitch (s:string);
begin
  EmptyLine;
  Noteln (concat ('SWITCH /', s));
end;

procedure Check;
var i, p:byte;
    s:string;
begin
  for i:=1 to max_switch do
  begin
    InitSwitch (switches [i].name);
    Noteln (switches [i].askstr);
    IncInsLevel;
    s := concat ('If you select this option, GT ', switches [i].descri);
    repeat
      p := pos ('@', s);
      if (p > 0) then
      begin
        Noteln (copy (s, 1, p - 1));
        delete (s, 1, p);
      end;
    until (p = 0);
    Noteln (s);
    DecInsLevel;
    case (switches [i].mode) of
      yesno: GetYN (i);
      value: GetValue (i);
    end;
  end;
end;

procedure ParseConfigFile;
var I:TINIFile;
    Name:string;
    Entry:TINIEntry;
    j:byte;
begin
{ check if INI file exists }
  if (not I.init (CFG_FILENAME)) then
    exit;

  while (I.GetNextEntry (Name, Entry)) do
    if ((Name[1] = '/') or (Name[1] = '-')) and (length (Name) > 1) then
    begin
      delete (Name, 1, 1); { delete "/" or "-" }
      for j:=1 to max_switch do
        if (switches [j].name = Name) then
          switches [j].select := true;
    end;

  I.done;
end;

begin
  { - open output handle - set to screen - }
  OpenHandle ('', true);

  EmptyLine;
  Noteln ('GetTyp 2.x configuration utility - (c) 1999 by PHaX');
  EmptyLine;

  Noteln (concat ('GTSetup will modify the file "', CFG_FILENAME, '" in the current directory.'));
  Noteln ('Everytime you are asked if you want a special option in the configuration');
  Noteln ('file you have to possibilities: yes or no.');
  Noteln ('You can select yes by pressing the "Y", and no by pressing "N".');
  EmptyLine;
  Noteln ('For every entry either Y or N is always uppercase which shows the current');
  Noteln ('value in the config file. "Enter" or "Esc" just keep the current value.');
  EmptyLine;
  Note ('Press Escape to exit this program now, any other key to continue.');

  if (getch = #27) then
  begin
    FinishLine;
    Noteln ('Maybe next time ....');
    halt (1);
  end;
  FinishLine;

  ParseConfigFile;

  assign (t, CFG_FILENAME);
  rewrite (t);
  writeln (t, ';');
  writeln (t, '; Configuration file of GetTyp 2.0 or higher');
  writeln (t, '; It was automatically created by GTSetup on ', GetCurrentDateStr, ' at ', GetCurrentTimeStr);
  writeln (t, ';');

  Check;

  close (t);

  Noteln (concat ('Successfully created ', CFG_FILENAME));
  Noteln ('Copyright (c) 1999 by PHaX - http://surf.to/phax');
end.
