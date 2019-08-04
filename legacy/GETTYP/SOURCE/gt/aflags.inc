{$define registered}

{$ifdef MSDOS}
{$ifndef DPMI}
{$define XMS}
{$define LOG_}
{$endif}
{$endif}

{$ifdef DPMI}
{$define MSDOS}
{$endif}

{$ifdef FPK}
{$undef MSDOS}
{$endif}

{$ifdef MSDOS}

  {$ifndef exe}
    {$define debug}
  {$endif}

  {$a-,b-,i-,v-}

  {$ifndef VER50}
    {$ifndef __TMT__}
      {$g+}
    {$endif}
    {$ifdef extended} {$x+} {$else} {$x-} {$endif}
  {$endif}

  {$ifdef far} {$f+} {$else} {$f-} {$endif}

  {$ifndef windows}
    {$ifdef overlay} {$o+} {$else} {$o-} {$endif}
  {$endif}

  {$ifdef debug}
    {$ifdef exe}
      {$ifdef exedebug} {$d+} {$else} {$d-} {$endif}
    {$else}
      {$d+}
    {$endif}
    {$l+,s+,r+}
  {$else}
    {$d-,l-,s-,r-}
  {$endif}

  {$ifdef double}
    {$n+}
    {$ifdef emul} {$e+} {$else} {$e-} {$endif}
  {$else}
    {$ifndef __TMT__}
      {$n-,e-}
    {$endif}
  {$endif}

{$else}
  {$ifdef extended}
    {$x+}
  {$else}
    {$x-}
  {$endif}
{$endif}  { gnu pascal }

{$ifdef WIN32}
  {$ifdef IDE}
    {$a-,b-,c-,h-,o+}
  {$else}
    {$a-,b-,c-,d-,h-,i-,l-,o+,q-,r-,y-}
  {$endif}
{$endif}

{$ifdef VER50}
{$define DOS}
{$define BORLAND}
{$define BORLAND_DOS}
{$endif}

{$ifdef VER60}
{$define DOS}
{$define STD}
{$define BORLAND}
{$define BORLAND_DOS}
{$endif}

{$ifdef VER70}
{$define DOS}
{$define STD}
{$define BORLAND}
{$define BORLAND_DOS}
{$endif}

{$ifdef WINDOWS}
{$define WIN}
{$endif}

{ delphi 3 }
{$ifdef VER100}
{$define DELPHI}
{$define WIN}
{$define BORLAND}
{$endif}

{ delphi 4 }
{$ifdef VER120}
{$define DELPHI}
{$define WIN}
{$define BORLAND}
{$endif}

{ do not use this before the main ifdef MSDOS because these
  flags are not supported }

{$ifdef __TMT__}
{$define MSDOS}
{$undef XMS}
{$define GETTYP}
{$define DPMI}
{$define __CON__}
{$define SLOW_STRINGPOSINBUFFER}
{$define SLOW_COMPARE}
{$endif}

{$ifdef VIRTUALPASCAL}
 {&Use32+}
 {$undef MSDOS}
 {$undef XMS}
 {$ifdef WIN32}
  {&Delphi+} 
 {$endif}
 {$ifdef OS2}
  {$define WIN32}
 {$endif}
{$endif}

{$ifdef WIN32}
{$define SLOW_STRINGPOSINBUFFER}
{$define SLOW_COMPARE}
{$endif}
