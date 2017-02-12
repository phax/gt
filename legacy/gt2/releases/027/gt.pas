{ Delphi unit to use for GTDATA.DLL v0.27 }

unit GT;

{$a-,b-,c-,h-,o+}

interface

{ ---------- types ------------ }

const 
      GT_NameType_NORMAL    = 0;
      GT_NameType_PACKER    = 1;
      GT_NameType_ENCRYPTER = 2;
      GT_NameType_COMPILER  = 3;
      GT_NameType_LINKER    = 4;
      GT_NameType_CONVERTER = 5;
      GT_NameType_PROTECTOR = 6;
      GT_NameType_STICKER   = 7;
      GT_NameType_EXTENDER  = 8;
      GT_NameType_PASSWORD  = 9;

type
     GT_C2EEntry = record
       sName:PChar;
       aData:array[0..23] of byte;
     end;
     pGT_C2EEntry = ^GT_C2EEntry;

     GT_ExeEntry = record
       nDataLen:longint;
       pData:^byte;
       nFileType:longint;  { is not of inteterest - ignore it like GT does }
       nNameType:longint;
       sName:PChar;
     end;
     pGT_ExeEntry = ^GT_ExeEntry;

     GT_SysEntry = GT_ExeEntry;
     pGT_SysEntry = ^GT_SysEntry;

     GT_ExtExeEntry = record
       nDataLen:longint;
       pData:^byte;
       nFileType:longint;  { is not of inteterest - ignore it like GT does }
       nNameType:longint;
       sName:PChar;
       sMacro:PChar;       { ask me for details - macro "language" ;-) }
     end;
     pGT_ExtExeEntry = ^GT_ExtExeEntry;

     GT_FixedExtensionEntry = record
       sExtension:PChar;    { without the "." !! }
       sDescription:PChar;
     end;
     pGT_FixedExtensionEntry = ^GT_FixedExtensionEntry;

     GT_FixedNameEntry = record  
       sFilename:PChar;      { all lowercase without a path }
       sDescription:PChar;
     end;
     pGT_FixedNameEntry = ^GT_FixedNameEntry;


{ ---------- decl ------------ }

function GT_GetVersion:PChar; cdecl;

function GT_C2E_Count:integer; cdecl;
function GT_C2E_Index (n:integer):pGT_C2EEntry; cdecl;

function GT_EXE_0J_Count:integer; cdecl;
function GT_EXE_0J_Index (n:integer):pGT_ExeEntry; cdecl;

function GT_EXE_1J_Count:integer; cdecl;
function GT_EXE_1J_Index (n:integer):pGT_ExeEntry; cdecl;

function GT_EXE_1LJ_Count:integer; cdecl;
function GT_EXE_1LJ_Index (n:integer):pGT_ExeEntry; cdecl;

function GT_EXE_2J_Count:integer; cdecl;
function GT_EXE_2J_Index (n:integer):pGT_ExeEntry; cdecl;

function GT_EXT_EXE_Count:integer; cdecl;
function GT_EXT_EXE_Index (n:integer):pGT_ExtExeEntry; cdecl;
                                          
function GT_EXT_PE_Count:integer; cdecl;
function GT_EXT_PE_Index (n:integer):pGT_ExtExeEntry; cdecl;

function GT_NE_0J_Count:integer; cdecl;
function GT_NE_0J_Index (n:integer):pGT_ExeEntry; cdecl;

function GT_PE_0J_Count:integer; cdecl;
function GT_PE_0J_Index (n:integer):pGT_ExeEntry; cdecl;

function GT_SYS_Count:integer; cdecl;
function GT_SYS_Index (n:integer):pGT_SysEntry; cdecl;

function GT_FixedName_Count:integer; cdecl;
function GT_FixedName_Index (n:integer):pGT_FixedNameEntry; cdecl;

function GT_FixedExtension_Count:integer; cdecl;
function GT_FixedExtension_Index (n:integer):pGT_FixedExtensionEntry; cdecl;

{ ---------- impl ------------ }

implementation

function GT_GetVersion; external 'gtdata.dll';
function GT_C2E_Count; external 'gtdata.dll';
function GT_C2E_Index; external 'gtdata.dll';
function GT_EXE_0J_Count; external 'gtdata.dll';
function GT_EXE_0J_Index; external 'gtdata.dll';
function GT_EXE_1J_Count; external 'gtdata.dll';
function GT_EXE_1J_Index; external 'gtdata.dll';
function GT_EXE_1LJ_Count; external 'gtdata.dll';
function GT_EXE_1LJ_Index; external 'gtdata.dll';
function GT_EXE_2J_Count; external 'gtdata.dll';
function GT_EXE_2J_Index; external 'gtdata.dll';
function GT_EXT_EXE_Count; external 'gtdata.dll';
function GT_EXT_EXE_Index; external 'gtdata.dll';
function GT_EXT_PE_Count; external 'gtdata.dll';
function GT_EXT_PE_Index; external 'gtdata.dll';
function GT_NE_0J_Count; external 'gtdata.dll';
function GT_NE_0J_Index; external 'gtdata.dll';
function GT_PE_0J_Count; external 'gtdata.dll';
function GT_PE_0J_Index; external 'gtdata.dll';
function GT_SYS_Count; external 'gtdata.dll';
function GT_SYS_Index; external 'gtdata.dll';
function GT_FixedName_Count; external 'gtdata.dll';
function GT_FixedName_Index; external 'gtdata.dll';
function GT_FixedExtension_Count; external 'gtdata.dll';
function GT_FixedExtension_Index; external 'gtdata.dll';

end.
