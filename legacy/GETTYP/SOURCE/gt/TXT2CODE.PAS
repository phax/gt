const STR_HEXARRAY:array[0..15] of char = '0123456789ABCDEF';

function hexbyte(b:byte):string;
begin
  hexbyte := concat (STR_HEXARRAY[b shr 4], STR_HEXARRAY[b and $f]);
end;

procedure e (s:string);
begin
  writeln (s);
  halt;
end;

const MAX_BUF = 1024;
var o:text;
    i:file;
    c, j:byte;
    buf:array[1..MAX_BUF] of byte;
    buflen:word;
begin
  if (paramcount < 2) then
    e ('Syntax: txt2code messagefile outputfile');

  assign (i, paramstr (1));
  reset (i, 1);
  if ioresult <> 0 then
    e ('Error opening input file');

  assign (o, paramstr (2));
  rewrite (o);
  if ioresult <> 0 then
    e ('Error opening output file');

  writeln (o, 'procedure InlineMessage', paramstr (3), ';');
  writeln (o, 'begin');
  writeln (o, '  inline (');
  write   (o, '  ');

  c := 0;
  repeat
    blockread (i, buf, MAX_BUF, buflen);
    for j:= 1 to buflen do
    begin
      inc (c);
      write (o, '$', hexbyte (buf [j]));
      if (j <> buflen) or (buflen = MAX_BUF) then
        write (o, '/');
      if (c = 19) then
      begin
        c := 0;
        write (o, #13#10, '  ');
      end;
    end;
  until eof (i);
  if (c > 0) then
    writeln (o);
  writeln (o, '  );');
  writeln (o, 'end;');
  close (o);
  close (i);
end.
