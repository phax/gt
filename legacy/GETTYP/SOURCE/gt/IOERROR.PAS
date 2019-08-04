{$i aflags.inc}

unit IOError;

interface

type
     String67 = string[67];

const
      bShowIOErrors:boolean = true;

procedure CheckIOError  (io:integer; s:String67);
procedure NoteIOError   (io:integer);

procedure CheckDOSError (io:integer);
procedure NoteDOSError (io:integer);

procedure CheckXMSError (io:integer; s:String67);

implementation

uses
     XString, XWrite;

procedure NoteIOError (io:integer);
begin
  if (io <> 0) and (bShowIOErrors) then
  begin
    { if (io = 1) .. else ... is 32 bytes larger :( }
    FastPrint ('IO error: ');
    case io of
        1:FastPrint ('Invalid function number');
        2:FastPrint ('File not found');
        3:FastPrint ('Path not found');
        4:FastPrint ('Too many open files');
        5:FastPrint ('File access denied');
        6:FastPrint ('Invalid file handle');
       12:FastPrint ('Invalid file access code');
       15:FastPrint ('Invalid drive number');
       16:FastPrint ('Cannot remove current directory');
       17:FastPrint ('Cannot rename across drives');
       18:FastPrint ('No more files');
       32:FastPrint ('Cannot open file for reading');
      100:FastPrint ('Disk read error');
      101:FastPrint ('Disk write error');
      102:FastPrint ('File not assigned');
      103:FastPrint ('File not open');
      104:FastPrint ('File not open for input');
      105:FastPrint ('File not open for output');
      106:FastPrint ('Invalid numeric format');
      150:FastPrint ('Disk is write-protected');
      151:FastPrint ('Bad drive request struct length');
      152:FastPrint ('Drive not ready');
      153:FastPrint ('Cannot read from file');
      154:FastPrint ('CRC error in data');
      156:FastPrint ('Disk seek error');
      157:FastPrint ('Unknown media type');
      158:FastPrint ('Sector not found');
      159:FastPrint ('Printer out of paper');
      160:FastPrint ('Device write fault');
      161:FastPrint ('Device read fault');
      162:FastPrint ('Hardware failure');
    else
      FastPrint (concat ('Unknown IO Error #', i2s (io)));
    end;
  end;
end;

procedure CheckIOError (io:integer; s:String67);
begin
  if (io <> 0) then
  begin
    if (bShowIOErrors) then
    begin
      NoteIOError (io);
      if (s > '') then
        FastPrint (concat (' - ', s));
      FastPrint (#13#10);
    end;
    halt (io);
  end;
end;

procedure NoteDOSError (io:integer);
begin
  if (bShowIOErrors) then
  begin
    FastPrint ('DOS error: ');
    case io of
       2:FastPrint ('File not found');
       3:FastPrint ('Path not found');
       5:FastPrint ('Access denied');
       6:FastPrint ('Invalid handle');
       8:FastPrint ('Not enough memory');
      10:FastPrint ('Invalid environment');
      11:FastPrint ('Invalid format');
      18:FastPrint ('No more files');
    else
      FastPrint (concat ('Unknown DOS Error #', i2s (io)));
    end;
    FastPrint (#13#10);
  end;
end;

procedure CheckDOSError (io:integer);
begin
  if (io > 0) then
  begin
    NoteDOSError (io);
    halt (io);
  end;
end;

procedure CheckXMSError (io:integer; s:String67);
begin
  if (io <> 0) and (bShowIOErrors) then
  begin
    FastPrint (concat ('XMS error (', s, '): '));
    case io of
      $80:FastPrint ('Invalid function number');
      $81:FastPrint ('Detected VDISK-RAMDISK');
      $82:FastPrint ('Error on gate A20');
      $8E:FastPrint ('General driver error');
      $8F:FastPrint ('Unexpected error');
      $90:FastPrint ('HMA was not found');
      $91:FastPrint ('HMA is already in use');
      $92:FastPrint ('The value given in DX is too small');
      $93:FastPrint ('HMA is not allocated');
      $94:FastPrint ('Gate A20 is still enabled');
      $A0:FastPrint ('No XMS available');
      $A1:FastPrint ('All XMS handles are in use');
      $A2:FastPrint ('Invalid handle');
      $A3:FastPrint ('Source handle is invalid');
      $A4:FastPrint ('Source offset is invalid');
      $A5:FastPrint ('Destination handle is invalid');
      $A6:FastPrint ('Destination offset is invalid');
      $A7:FastPrint ('Invalid length for Move function');
      $A8:FastPrint ('Forbidden overlapping of Move function');
      $A9:FastPrint ('Parity error');
      $AA:FastPrint ('UMB is not locked');
      $AB:FastPrint ('UMB is still locked');
      $AC:FastPrint ('Overflow of the UMB lock counter');
      $AD:FastPrint ('Cannot lock UMB');
      $B0:FastPrint ('Only smaller UMB is available');
      $B1:FastPrint ('No UMB available');
      $B2:FastPrint ('UMB segment adress is invalid');
    else
      FastPrint (concat ('Unknown XMS Error #', i2s (io)));
    end;
    FastPrint (#13#10);
    halt (io);
  end;
end;

end.
