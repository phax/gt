{$i aflags.inc}

{ if it is BP 7, disabled the typesafe @ operator in this file }
{$ifdef VER70}
{$T-}
{$endif}
{$ifdef WIN32}
{$T-}
{$endif}

unit GTData__;

interface

uses
     GTCompar;

const

      STR_Empty:string[1] = '';
{
 ############################################################
 ######################### LETTER A #########################
 ############################################################
}

      STR_Alec16 :string[8]= 'Alec 1.6';
      DAT_Alec16:array[1..6] of byte = (
          $B1, $04, $D3, $E3, $8C, $DA
      );
{ original; DataLen:16
          $B1, $04, $D3, $E3, $8C, $DA, $8E, $D2, $8B, $E3, $33, $C0, $50, $F7,
          $D0, $2E,
}
      REC_Alec16:TCompPtrRec = (
        DataLen:6;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_Alec16;
        Data:@DAT_Alec16
      );


      STR_Aluwain809:string [12] = 'Aluwain 8.09';
      DAT_Aluwain809:array[1..80] of byte = (
          $5A, $BE, $04, $00, $33, $C0, $8E, $D8, $8B, $1C, $53, $8B, $5C, $02,
          $53, $8B, $5C, $08, $53, $8B, $5C, $0A, $53, $8B, $DA, $81, $C3, $7D,
          $02, $89, $1C, $8C, $4C, $02, $8B, $DA, $83, $C3, $60, $89, $5C, $08,
          $8C, $4C, $0A, $8B, $DA, $83, $C3, $02, $53, $9C, $80, $4E, $F3, $01,
          $52, $8C, $C0, $8E, $D8, $C3, $55, $8B, $EC, $50, $53, $51, $52, $57,
          $8B, $7E, $02, $E8, $00, $00, $58, $2D, $63, $02
        );
      REC_Aluwain809:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_Aluwain809;
        Data:@DAT_Aluwain809
      );



      STR_BINOBJ:string [16] = 'BINOBJ converted';
      DAT_BINOBJ:array[1..22] of byte = (
          $80, $04, $00, $02, $3A, $3A, $06, $96, $08, $00, $00, $04, $43, $4F,
          $44, $45, $00, $43, $98, $07, $00, $28
      );
      REC_BINOBJ:TCompPtrRec = (
        DataLen:22;
        FileType:0;
        NameType:NORMAL;
        Name:@STR_BINOBJ;
        Data:@DAT_BINOBJ
      );


      STR_Bunny41M:string [24] = 'BUNNY 4.1 / Manipulation';
      DAT_Bunny41M:array[1..80] of byte = (
          $8C, $C0, $2E, $A3, $29, $01, $8C, $D8, $2E, $A3, $2B, $01, $8C, $D0,
          $2E, $A3, $27, $01, $8B, $C4, $2E, $A3, $25, $01, $8C, $C8, $8E, $D8,
          $8E, $C0, $8E, $D0, $BC, $EB, $01, $E8, $9D, $01, $E8, $84, $01, $E8,
          $1C, $01, $86, $C4, $3D, $00, $03, $72, $60, $E8, $D5, $00, $72, $63,
          $A3, $FF, $02, $B8, $00, $00, $A3, $01, $03, $E8, $0A, $01, $72, $5D,
          $B9, $00, $01, $33, $F6, $2E, $8A, $84, $F7, $01
        );
      REC_Bunny41M:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PROTECTOR;
        Name:@STR_Bunny41M;
        Data:@DAT_Bunny41M
      );

      STR_Bunny41I:string [24] = 'BUNNY 4.1 / Installation';
      DAT_Bunny41I:array[1..80] of byte = (
          $8C, $C0, $2E, $A3, $29, $01, $8C, $D8, $2E, $A3, $2B, $01, $8C, $D0,
          $2E, $A3, $27, $01, $8B, $C4, $2E, $A3, $25, $01, $8C, $C8, $8E, $D8,
          $8E, $C0, $8E, $D0, $BC, $95, $01, $E8, $60, $01, $B8, $ED, $09, $BB,
          $B0, $0B, $2B, $D8, $8B, $CB, $33, $F6, $BB, $00, $00, $8B, $C3, $2E,
          $8A, $84, $EC, $09, $03, $D8, $46, $E2, $F6, $2E, $8B, $16, $AF, $03,
          $2E, $89, $1E, $A8, $03, $2B, $DA, $2E, $01, $1E
        );
      REC_Bunny41I:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PROTECTOR;
        Name:@STR_Bunny41I;
        Data:@DAT_Bunny41I
      );

      STR_Bunny41P:string [20] = 'BUNNY 4.1 / Password';
      DAT_Bunny41P:array[1..80] of byte = (
          $8C, $C0, $2E, $A3, $29, $01, $8C, $D8, $2E, $A3, $2B, $01, $8C, $D0,
          $2E, $A3, $27, $01, $8B, $C4, $2E, $A3, $25, $01, $8C, $C8, $8E, $D8,
          $8E, $C0, $8E, $D0, $BC, $9D, $01, $BC, $9D, $01, $E8, $F8, $00, $B8,
          $B2, $07, $BB, $FF, $08, $2B, $D8, $8B, $CB, $33, $F6, $BB, $00, $00,
          $8B, $C3, $2E, $8A, $84, $B1, $07, $03, $D8, $46, $E2, $F6, $2E, $8B,
          $16, $BA, $01, $2E, $89, $1E, $B3, $01, $2B, $DA
        );
      REC_Bunny41P:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PROTECTOR;
        Name:@STR_Bunny41P;
        Data:@DAT_Bunny41P
      );

      STR_C_Crypt102C:string [12] = 'C-Crypt 1.02';
      DAT_C_Crypt102C:array[1..80] of byte = (
          $5D, $83, $ED, $04, $55, $D9, $D0, $9C, $58, $25, $FF, $FE, $50, $9D,
          $50, $57, $BF, $70, $00, $B0, $01, $AA, $5F, $58, $66, $51, $66, $B9,
          $00, $01, $00, $00, $CC, $66, $50, $66, $2E, $A1, $00, $01, $66, $50,
          $BF, $00, $01, $B0, $C3, $88, $05, $57, $FF, $D7, $E2, $FC, $CC, $5F,
          $66, $58, $66, $89, $05, $66, $58, $66, $59, $53, $BB, $EB, $04, $5B,
          $EB, $FB, $9A, $1E, $33, $C0, $8E, $D8, $C7, $06
        );
      REC_C_Crypt102C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_C_Crypt102C;
        Data:@DAT_C_Crypt102C
      );

      STR_Crypt120:string [10] = 'Crypt 1.20';
      DAT_Crypt120:array[1..80] of byte = (
        $8C, $C8, $FA, $8E, $D0, $BC, $3D, $00, $8C, $C0, $BB, $00, $00, $83,
        $E8, $10, $8E, $D8, $33, $F6, $4B, $8C, $CD, $8C, $C0, $2E, $A3, $5A,
        $00, $53, $33, $C0, $8E, $C0, $26, $8C, $0E, $06, $00, $26, $C7, $06,
        $04, $00, $3D, $02, $1F, $8C, $CD, $1E, $B0, $AD, $E6, $64, $2B, $C0,
        $8E, $C0, $05, $00, $04, $8B, $D8, $B8, $25, $1B, $0E, $B9, $37, $01,
        $26, $89, $8F, $0C, $FC, $BE, $43, $51, $26, $89
      );
      REC_Crypt120:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_Crypt120;
        Data:@DAT_Crypt120
      );

      {
          $C9: Noteln (concat (temp, '1.0'));
          $F0: Noteln (concat (temp, '1.0b (1)'));
          $DF: Noteln (concat (temp, '1.0b (2)'));
          $F6: Noteln (concat (temp, '1.01'));
          $05: Noteln (concat (temp, '1.02'));
          $34: Noteln (concat (temp, '1.03'));
      }
      STR_CrackStop10:string[13] = 'CrackStop 1.0';
      DAT_CrackStop10:array[1..80] of byte = (
        { $B4, $48, $BB, $FF, $FF, }
        $B9, $EB, $27, $8B, $EC, $CD, $21, $FA, $FC,
        $03, $46, $FA, $05, $05, $00, $FF, $E0, $9A, $53, $45, $EB, $05, $B8,
        $08, $00, $EB, $EE, $B8, $06, $00, $29, $46, $FA, $EB, $02, $8E, $F8,
        $2B, $E0, $CF, $EA, $11, $0E, $1F, $E8, $03, $00, $24, $00, $24, $5A,
        $B4, $09, $CD, $21, $FA, $33, $C0, $89, $46, $FA, $B8, $D2, $04, $40,
        $2B, $D8, $8B, $46, $FA, $0B, $C0, $75, $B1, $FB,
        $00, $00, $00, $00, $C9
      );
      REC_CrackStop10:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_CrackStop10;
        Data:@DAT_CrackStop10
      );

      STR_CrackStop10b1:string[15] = 'CrackStop 1.0b1';
      DAT_CrackStop10b1:array[1..80] of byte = (
        { $B4, $48, $BB, $FF, $FF, }
        $B9, $EB, $27, $8B, $EC, $CD, $21, $FA, $FC,
        $03, $46, $FA, $05, $05, $00, $FF, $E0, $9A, $53, $45, $EB, $05, $B8,
        $08, $00, $EB, $EE, $B8, $06, $00, $29, $46, $FA, $EB, $02, $8E, $F8,
        $2B, $E0, $CF, $EA, $11, $0E, $1F, $E8, $03, $00, $24, $00, $24, $5A,
        $B4, $09, $CD, $21, $FA, $33, $C0, $89, $46, $FA, $B8, $D2, $04, $40,
        $2B, $D8, $8B, $46, $FA, $0B, $C0, $75, $B1, $FB,
        $00, $00, $00, $00, $F0
      );
      REC_CrackStop10b1:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_CrackStop10b1;
        Data:@DAT_CrackStop10b1
      );

      STR_CrackStop10b2:string[15] = 'CrackStop 1.0b2';
      DAT_CrackStop10b2:array[1..80] of byte = (
        { $B4, $48, $BB, $FF, $FF, }
        $B9, $EB, $27, $8B, $EC, $CD, $21, $FA, $FC,
        $03, $46, $FA, $05, $05, $00, $FF, $E0, $9A, $53, $45, $EB, $05, $B8,
        $08, $00, $EB, $EE, $B8, $06, $00, $29, $46, $FA, $EB, $02, $8E, $F8,
        $2B, $E0, $CF, $EA, $11, $0E, $1F, $E8, $03, $00, $24, $00, $24, $5A,
        $B4, $09, $CD, $21, $FA, $33, $C0, $89, $46, $FA, $B8, $D2, $04, $40,
        $2B, $D8, $8B, $46, $FA, $0B, $C0, $75, $B1, $FB,
        $00, $00, $00, $00, $DF
      );
      REC_CrackStop10b2:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_CrackStop10b2;
        Data:@DAT_CrackStop10b2
      );

      STR_CrackStop101:string[14] = 'CrackStop 1.01';
      DAT_CrackStop101:array[1..80] of byte = (
        { $B4, $48, $BB, $FF, $FF, }
        $B9, $EB, $27, $8B, $EC, $CD, $21, $FA, $FC,
        $03, $46, $FA, $05, $05, $00, $FF, $E0, $9A, $53, $45, $EB, $05, $B8,
        $08, $00, $EB, $EE, $B8, $06, $00, $29, $46, $FA, $EB, $02, $8E, $F8,
        $2B, $E0, $CF, $EA, $11, $0E, $1F, $E8, $03, $00, $24, $00, $24, $5A,
        $B4, $09, $CD, $21, $FA, $33, $C0, $89, $46, $FA, $B8, $D2, $04, $40,
        $2B, $D8, $8B, $46, $FA, $0B, $C0, $75, $B1, $FB,
        $00, $00, $00, $00, $F6
      );
      REC_CrackStop101:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_CrackStop101;
        Data:@DAT_CrackStop101
      );

      STR_CrackStop102:string[14] = 'CrackStop 1.02';
      DAT_CrackStop102:array[1..80] of byte = (
        { $B4, $48, $BB, $FF, $FF, }
        $B9, $EB, $27, $8B, $EC, $CD, $21, $FA, $FC,
        $03, $46, $FA, $05, $05, $00, $FF, $E0, $9A, $53, $45, $EB, $05, $B8,
        $08, $00, $EB, $EE, $B8, $06, $00, $29, $46, $FA, $EB, $02, $8E, $F8,
        $2B, $E0, $CF, $EA, $11, $0E, $1F, $E8, $03, $00, $24, $00, $24, $5A,
        $B4, $09, $CD, $21, $FA, $33, $C0, $89, $46, $FA, $B8, $D2, $04, $40,
        $2B, $D8, $8B, $46, $FA, $0B, $C0, $75, $B1, $FB,
        $00, $00, $00, $00, $05
      );
      REC_CrackStop102:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_CrackStop102;
        Data:@DAT_CrackStop102
      );

      STR_CrackStop103:string[14] = 'CrackStop 1.03';
      DAT_CrackStop103:array[1..80] of byte = (
        { $B4, $48, $BB, $FF, $FF, }
        $B9, $EB, $27, $8B, $EC, $CD, $21, $FA, $FC,
        $03, $46, $FA, $05, $05, $00, $FF, $E0, $9A, $53, $45, $EB, $05, $B8,
        $08, $00, $EB, $EE, $B8, $06, $00, $29, $46, $FA, $EB, $02, $8E, $F8,
        $2B, $E0, $CF, $EA, $11, $0E, $1F, $E8, $03, $00, $24, $00, $24, $5A,
        $B4, $09, $CD, $21, $FA, $33, $C0, $89, $46, $FA, $B8, $D2, $04, $40,
        $2B, $D8, $8B, $46, $FA, $0B, $C0, $75, $B1, $FB,
        $00, $00, $00, $00, $34
      );
      REC_CrackStop103:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_CrackStop103;
        Data:@DAT_CrackStop103
      );


      STR_Crypt121:string [10] = 'Crypt 1.21';
      DAT_Crypt121:array[1..80] of byte = (
          $8C, $C8, $FA, $8E, $D0, $BC, $3D, $00, $FB, $8C, $C0, $BB, $D6, $00,
          $83, $E8, $10, $8E, $D8, $33, $F6, $4B, $8C, $CD, $8C, $C0, $2E, $A3,
          $3F, $00, $33, $C0, $8E, $C0, $26, $A1, $04, $00, $2E, $A3, $AB, $00,
          $26, $A1, $06, $00, $2E, $A3, $AD, $00, $26, $A1, $0C, $00, $2E, $A3,
          $AF, $00, $26, $A1, $0E, $00, $2E, $A3, $B1, $00, $53, $33, $C0, $8E,
          $C0, $26, $8C, $0E, $06, $00, $26, $C7, $06, $04
        );
      REC_Crypt121:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_Crypt121;
        Data:@DAT_Crypt121
      );

      STR_Crypta20:string [12] = 'Cryta II 2.0';
      DAT_Crypta20:array[1..80] of byte = (
          $1E, $06, $8C, $D8, $05, $10, $00, $8E, $D8, $8E, $C0, $2E, $01, $06,
          $91, $05, $2E, $01, $06, $95, $05, $8B, $E8, $2E, $A1, $FE, $01, $2E,
          $8B, $0E, $00, $02, $85, $C0, $74, $03, $49, $E3, $37, $83, $F9, $7C,
          $72, $16, $51, $B9, $00, $F8, $E8, $74, $FE, $59, $83, $E9, $7C, $8C,
          $D8, $05, $80, $0F, $8E, $D8, $8E, $C0, $EB, $E5, $0B, $C9, $74, $18,
          $B8, $00, $02, $F7, $E1, $8B, $C8, $51, $E8, $56
        );
      REC_Crypta20:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_Crypta20;
        Data:@DAT_Crypta20
      );

      STR_CC286x2:string [11] = 'CC286xý 2.1';
      DAT_CC286x2:array[1..80] of byte = (
          $5D, $81, $ED, $5D, $02, $EB, $01, $66, $BF, $00, $01, $EB, $01, $EA,
          $B0, $AD, $EB, $01, $FF, $E6, $64, $EB, $01, $D5, $B0, $C3, $EB, $01,
          $9A, $AA, $EB, $01, $D5, $4F, $8D, $9E, $91, $02, $EB, $01, $66, $53,
          $8B, $DF, $EB, $01, $EA, $53, $EB, $01, $9A, $C3, $EB, $01, $66, $EB,
          $01, $9A, $EB, $01, $EA, $B8, $40, $00, $EB, $01, $66, $8E, $D8, $EB,
          $01, $EA, $A1, $6C, $00, $EB, $01, $9A, $3B, $06
        );
      REC_CC286x2:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_CC286x2;
        Data:@DAT_CC286x2
      );

{
 ############################################################
 ######################### LETTER D #########################
 ############################################################
}

      STR_DPMI_DJ_1:string [29] = 'DPMI loader by DJ Delorie (1)';
      DAT_DPMI_DJ_1:array[1..80] of byte = (
          $52, $BB, $3B, $05, $E8, $14, $00, $5B, $E8, $10, $00, $BB, $61, $04,
          $E8, $0A, $00, $B8, $FF, $4C, $CD, $21, $43, $B4, $02, $CD, $21, $8A,
          $17, $80, $FA, $24, $75, $F4, $C3, $0D, $0A, $24, $50, $51, $57, $31,
          $C0, $BF, $2A, $06, $B9, $19, $00, $F3, $AB, $5F, $59, $58, $C3, $B8,
          $00, $03, $BB, $21, $00, $31, $C9, $66, $BF, $2A, $06, $00, $00, $CD,
          $31, $C3, $00, $00, $30, $E4, $E8, $5F, $FF, $89
        );
      REC_DPMI_DJ_1:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:EXTENDER;
        Name:@STR_DPMI_DJ_1;
        Data:@DAT_DPMI_DJ_1
      );

      STR_DPMI_DJ_2:string [29] = 'DPMI loader by DJ Delorie (2)';
      DAT_DPMI_DJ_2:array[1..80] of byte = (
          $52, $BB, $3B, $05, $E8, $15, $00, $5B, $E8, $11, $00, $BB, $67, $04,
          $E8, $0B, $00, $B4, $4C, $CD, $21, $43, $50, $B4, $02, $CD, $21, $58,
          $8A, $17, $80, $FA, $24, $75, $F2, $C3, $0D, $0A, $24, $50, $51, $57,
          $31, $C0, $BF, $2A, $06, $B9, $19, $00, $F3, $AB, $5F, $59, $58, $C3,
          $B8, $00, $03, $BB, $21, $00, $31, $C9, $66, $BF, $2A, $06, $00, $00,
          $CD, $31, $C3, $00, $00, $30, $E4, $E8, $4E, $FF
        );
      REC_DPMI_DJ_2:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:EXTENDER;
        Name:@STR_DPMI_DJ_2;
        Data:@DAT_DPMI_DJ_2
      );

{ not used because it's like Protect! 6.0
      STR_DCR12:string [10] = 'DCR II 1.2';
      DAT_DCR12:array[1..80] of byte = (
          $1E, $B4, $30, $CD, $21, $3C, $02, $73, $02, $CD, $20, $BE, $31, $00,
          $E8, $18, $00, $E8, $08, $00, $1F, $8C, $D8, $8E, $C0, $E9, $15, $00,
          $BB, $8C, $07, $B4, $1B, $AC, $04, $8E, $AA, $4B, $75, $F9, $C3, $0E,
          $1F, $FC, $0E, $07, $8B, $FE, $C3, $6E, $74, $A9, $90, $80, $91, $F2,
          $4F, $A7, $9C, $FF, $38, $9B, $80, $F2, $40, $0B, $F3, $6B, $94, $BF,
          $AC, $1E, $E4, $CA, $9C, $60, $F2, $67, $0B, $84
        );
      REC_DCR12:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_DCR12;
        Data:@DAT_DCR12
      );
}


      STR_DarkStop10:string [26] = 'HackStop 1.13/DarkStop 1.0';
      DAT_DarkStop10:array[1..80] of byte = (
          $E5, $83, $C4, $16, $83, $C4, $16, $87, $E5, $50, $E8, $C5, $FF, $53,
          $BB, $EB, $04, $5B, $EB, $FB, $9A, $B4, $30, $CD, $21, $FA, $8B, $EC,
          $8B, $46, $FA, $05, $0C, $00, $FF, $E0, $81, $EB, $02, $EB, $F0, $81,
          $46, $FA, $1A, $00, $83, $EC, $06, $CF, $82, $53, $BB, $EB, $04, $5B,
          $EB, $FB, $9A, $E8, $94, $FF, $53, $BB, $EB, $04, $5B, $EB, $FB, $9A,
          $E8, $6F, $FF, $53, $BB, $EB, $04, $5B, $EB, $FB
        );
      REC_DarkStop10:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_DarkStop10;
        Data:@DAT_DarkStop10
      );

      STR_DeepCrypter01b:string [17] = 'deeP-CRyPTeR 0.1á';
      DAT_DeepCrypter01b:array[1..80] of byte = (
          $59, $8B, $E9, $81, $ED, $6A, $01, $81, $E9, $03, $01, $BE, $00, $01,
          $8D, $BE, $A3, $01, $66, $8B, $05, $66, $89, $04, $8D, $BE, $A7, $01,
          $8B, $D7, $8B, $DF, $83, $C3, $1F, $8A, $04, $32, $05, $F6, $D0, $88,
          $04, $47, $46, $3B, $FB, $76, $02, $8B, $FA, $E2, $EE, $68, $00, $01,
          $C3, $00, $00, $00, $00, $29, $20, $3F, $5E, $4B, $22, $2F, $12, $4F,
          $55, $35, $1A, $0C, $52, $18, $0C, $40, $18, $41
        );
      REC_DeepCrypter01b:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_DeepCrypter01b;
        Data:@DAT_DeepCrypter01b
      );

      STR_DSCRP128:string [11] = 'DS-CRP 1.28';
      DAT_DSCRP128:array[1..80] of byte = (
          $00, $00, $00, $00, $B0, $AD, $E6, $64, $E8, $00, $00, $5D, $81, $ED,
          $95, $02, $9C, $32, $E4, $50, $9D, $9C, $58, $80, $E4, $F0, $80, $FC,
          $F0, $74, $0E, $B4, $70, $50, $9D, $9C, $58, $80, $E4, $70, $74, $03,
          $9D, $EB, $3D, $B4, $09, $8D, $96, $C4, $02, $CD, $21, $B8, $FF, $4C,
          $CD, $21, $54, $68, $69, $73, $20, $70, $72, $6F, $67, $72, $61, $6D,
          $20, $72, $65, $71, $75, $69, $72, $65, $73, $20
        );
      REC_DSCRP128:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_DSCRP128;
        Data:@DAT_DSCRP128
      );


{
 ############################################################
 ######################### LETTER E #########################
 ############################################################
}

      STR_EFP123:string [8] = 'EFP 1.23';
      DAT_EFP123:array[1..80] of byte = (
        $E1, $00, $68, $16, $00, $00, $34, $16, $AC, $D6, $00, $40, $00, $40,
        $FF, $FF, $FF, $FF, $FB, $F0, $95, $51, $FD, $2B, $BA, $9E, $8F, $DE,
        $96, $5F, $8A, $46, $8B, $00, $07, $CB, $93, $12, $D5, $5A, $86, $8D,
        $9C, $CD, $19, $2A, $78, $DA, $0F, $E0, $4E, $8D, $EE, $DC, $A4, $3F,
        $2B, $F9, $AC, $BF, $FC, $7F, $2D, $01, $D6, $E1, $1D, $99, $6D, $7A,
        $46, $7A, $14, $BC, $E3, $3F, $1C, $4F, $1F, $01
      );
      REC_EFP123:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_EFP123;
        Data:@DAT_EFP123
      );

      DAT_FFSE04:array[1..80] of byte = (
        $EB, $01, $69, $60, $25, $EB, $03, $EB, $FC, $83, $8C, $C0, $FA, $66,
        $C1, $CC, $10, $EB, $01, $C0, $44, $BC, $00, $00, $EB, $01, $C6, $2E,
        $A3, $96, $00, $00, $C1, $CC, $10, $E9, $B2, $00, $EB, $01, $C7, $E8,
        $6C, $00, $FC, $0F, $21, $D1, $FA, $68, $00, $00, $07, $06, $1F, $FF,
        $E1, $52, $91, $A7, $B5, $D8, $6A, $50, $A8, $67, $76, $6E, $30, $86,
        $D8, $87, $91, $A7, $40, $88, $92, $71, $25, $EB
      );
      REC_FFSE04:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_Empty;
        Data:@DAT_FFSE04
      );

      STR_FFSE05:string [8] = 'FFSE 0.5';
      DAT_FFSE05:array[1..80] of byte = (
          $00, $54, $F6, $4D, $6A, $4C, $F6, $20, $84, $FF, $D9, $CE, $B1, $48,
          $DC, $A2, $CC, $D0, $0D, $AC, $20, $05, $F9, $B5, $1F, $C2, $F3, $1B,
          $80, $72, $59, $F9, $C0, $61, $21, $3B, $72, $23, $32, $BA, $30, $E8,
          $5B, $97, $36, $0B, $E2, $DB, $84, $98, $51, $B7, $F5, $68, $31, $8B,
          $34, $8B, $8E, $4C, $51, $BA, $C5, $5C, $58, $0D, $4A, $E9, $E9, $8E,
          $40, $2F, $10, $E2, $D2, $5F, $93, $D2, $E4, $11
        );
      REC_FFSE05:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_FFSE05;
        Data:@DAT_FFSE05
      );


      STR_fdscp04:string [10] = 'fds-cp 0.4';
      DAT_fdscp04:array[1..80] of byte = (
          $0E, $1F, $B8, $54, $01, $EB, $01, $EA, $B9, $00, $00, $50, $50, $5F,
          $5E, $AD, $35, $00, $42, $AB, $E2, $F9, $00, $30, $00, $43, $00, $F2,
          $00, $A9, $00, $D8, $00, $26, $00, $43, $00, $1A, $00, $8A, $00, $BD,
          $00, $12, $00, $DE, $00, $F9, $00, $B2, $00, $81, $00, $81, $00, $4C,
          $00, $80, $00, $89, $00, $DF, $00, $1A, $00, $81, $00, $40, $00, $80,
          $00, $B8, $00, $31, $00, $AA, $00, $42, $00, $C1
        );
      REC_fdscp04:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_fdscp04;
        Data:@DAT_fdscp04
      );

      STR_HS102bE:string [20] = 'HackStop 1.02b [EXE]';
      DAT_HS102bE:array[1..80] of byte = (
        $55, $E8, $08, $00, $00, $00, $FE, $17, $00, $00, $00, $00, $50, $B8,
        $EB, $04, $58, $EB, $FB, $9A, $5D, $2E, $89, $46, $00, $2E, $89, $5E,
        $02, $2E, $89, $4E, $04, $58, $2E, $89, $46, $06, $5B, $2E, $8B, $07,
        $83, $C3, $02, $53, $8B, $C8, $32, $ED, $50, $B8, $EB, $04, $58, $EB,
        $FB, $9A, $2E, $30, $27, $50, $B8, $EB, $04, $58, $EB, $FB, $9A, $43,
        $50, $B8, $EB, $04, $58, $EB, $FB, $9A, $E2, $E2
      );
      REC_HS102bE:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS102bE;
        Data:@DAT_HS102bE
      );

      STR_HS112C:string [19] = 'HackStop 1.12 [COM]';
      DAT_HS112C:array[1..80] of byte = (
        $E5, $83, $C4, $16, $83, $C4, $16, $87, $E5, $50, $52, $1E, $B4, $30,
        $CD, $21, $86, $E0, $3D, $FF, $02, $73, $02, $CD, $20, $0E, $1F, $B4,
        $09, $E8, $03, $00, $24, $24, $EA, $5A, $CD, $21, $1F, $5A, $53, $BB,
        $EB, $04, $5B, $EB, $FB, $9A, $EB, $02, $1B, $91, $50, $55, $52, $8C,
        $D2, $FA, $53, $BB, $EB, $04, $5B, $EB, $FB, $9A, $8C, $C8, $8E, $D0,
        $EB, $02, $31, $91, $8B, $EC, $F7, $DC, $BC, $03
      );
      REC_HS112C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS112C;
        Data:@DAT_HS112C
      );

      STR_HS113C:string [19] = 'HackStop 1.13 [COM]';
      DAT_HS113C:array[1..80] of byte = (
        $87, $E5, $83, $C4, $16, $83, $C4, $16, $87, $E5, $50, $E8, $C5, $FF,
        $53, $BB, $EB, $04, $5B, $EB, $FB, $9A, $B4, $30, $CD, $21, $FA, $8B,
        $EC, $8B, $46, $FA, $05, $0C, $00, $FF, $E0, $81, $EB, $02, $EB, $F0,
        $81, $46, $FA, $1A, $00, $83, $EC, $06, $CF, $82, $53, $BB, $EB, $04,
        $5B, $EB, $FB, $9A, $E8, $94, $FF, $53, $BB, $EB, $04, $5B, $EB, $FB,
        $9A, $E8, $6F, $FF, $53, $BB, $EB, $04, $5B, $EB
      );
      REC_HS113C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS113C;
        Data:@DAT_HS113C
      );

      STR_HS114C:string [19] = 'HackStop 1.14 [COM]';
      DAT_HS114C:array[1..51] of byte = (
        $87, $00, $83, $C4, $00, $83, $C4, $00, $87, $00, $50, $E8, $00, $FF,
        $53, $BB, $EB, $04, $5B, $EB, $FB, $9A, $B4, $30, $CD, $21, $FA, $8B,
        $EC, $8B, $46, $FA, $05, $0C, $00, $FF, $E0, $81, $EB, $02, $EB, $F0,
        $81, $46, $FA, $1A, $00, $83, $EC, $06, $CF
      );
      REC_HS114C:TCompPtrRec = (
        DataLen:51;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS114C;
        Data:@DAT_HS114C
      );

      STR_HS115C:string [19] = 'HackStop 1.15 [COM]';
      DAT_HS115C:array[1..80] of byte = (
        $E6, $83, $C4, $10, $83, $C4, $10, $87, $E6, $50, $E8, $A7, $FF, $53,
        $BB, $EB, $04, $5B, $EB, $FB, $9A, $B4, $30, $CD, $21, $FA, $8B, $EC,
        $8B, $46, $FA, $05, $0C, $00, $FF, $E0, $81, $EB, $02, $EB, $F0, $81,
        $46, $FA, $1A, $00, $83, $EC, $06, $CF, $82, $E8, $7E, $FF, $BB, $FF,
        $00, $53, $BB, $EB, $04, $5B, $EB, $FB, $9A, $43, $8D, $84, $30, $00,
        $53, $89, $47, $02, $80, $37, $03, $C3, $81, $53
      );
      REC_HS115C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS115C;
        Data:@DAT_HS115C
      );

      STR_HS117bC:string [21] = 'HackStop 1.17ás [COM]';
      DAT_HS117bC:array[1..80] of byte = (
        $C0, $74, $0B, $53, $BB, $EB, $04, $5B, $EB, $FB, $9A, $E9, $15, $04,
        $87, $E6, $83, $C4, $10, $83, $C4, $10, $87, $E6, $50, $53, $BB, $EB,
        $04, $5B, $EB, $FB, $9A, $E8, $90, $FF, $53, $BB, $EB, $04, $5B, $EB,
        $FB, $9A, $B4, $30, $CD, $21, $FA, $8B, $EC, $8B, $46, $FA, $05, $0C,
        $00, $FF, $E0, $81, $EB, $02, $EB, $F0, $81, $46, $FA, $1A, $00, $83,
        $EC, $06, $CF, $82, $E8, $67, $FF, $BB, $FF, $00
      );
      REC_HS117bC:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS117bC;
        Data:@DAT_HS117bC
      );

      STR_HS117C:string [19] = 'HackStop 1.17 [COM]';
      DAT_HS117C:array[1..80] of byte = (
        $3A, $C0, $74, $0B, $52, $BA, $EB, $04, $5A, $EB, $FB, $9A, $E9, $E9,
        $04, $87, $E6, $83, $C4, $10, $83, $C4, $10, $87, $E6, $50, $56, $52,
        $BA, $EB, $04, $5A, $EB, $FB, $9A, $E8, $6A, $FF, $52, $BA, $EB, $04,
        $5A, $EB, $FB, $9A, $2B, $C0, $80, $C4, $30, $CD, $21, $FA, $8B, $EC,
        $8B, $46, $FA, $05, $0C, $00, $FF, $E0, $81, $EB, $02, $EB, $F0, $81,
        $46, $FA, $1A, $00, $83, $EC, $06, $CF, $82, $E8
      );
      REC_HS117C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS117C;
        Data:@DAT_HS117C
      );

      STR_HS118C:string [19] = 'HackStop 1.18 [COM]';
      DAT_HS118C:array[1..80] of byte = (
        $3A, $C0, $74, $0B, $52, $BA, $EB, $04, $5A, $EB, $FB, $9A, $E9, $6C,
        $05, $87, $E6, $83, $C4, $10, $83, $C4, $10, $87, $E6, $50, $56, $57,
        $51, $E8, $02, $00, $EB, $1A, $E8, $02, $00, $3F, $14, $5F, $B9, $43,
        $00, $B8, $3F, $14, $2E, $87, $05, $2E, $31, $85, $17, $00, $47, $02,
        $E0, $E2, $F6, $C3, $66, $74, $3E, $17, $05, $2B, $2A, $5A, $09, $A9,
        $9C, $94, $09, $65, $C2, $52, $FE, $61, $97, $46
      );
      REC_HS118C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS118C;
        Data:@DAT_HS118C
      );

      STR_HS119b195C:string [29] = 'HackStop 1.19 build 195 [COM]';
      DAT_HS119b195C:array[1..80] of byte = (
        $E6, $83, $C4, $10, $83, $C4, $10, $87, $E6, $50, $56, $57, $51, $E8,
        $02, $00, $EB, $1A, $E8, $02, $00, $1E, $25, $5F, $B9, $43, $00, $B8,
        $1E, $25, $2E, $87, $05, $2E, $31, $85, $17, $00, $47, $D1, $C8, $E2,
        $F6, $C3, $47, $F5, $03, $94, $7E, $CE, $3B, $D9, $E2, $96, $EE, $B3,
        $BE, $F6, $EE, $43, $50, $F4, $BE, $D1, $0F, $30, $A8, $23, $92, $E0,
        $8D, $C5, $BB, $A5, $5B, $A8, $AB, $4A, $D4, $C1
      );
      REC_HS119b195C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS119b195C;
        Data:@DAT_HS119b195C
      );

      STR_HS119b197C:string [29] = 'HackStop 1.19 build 197 [COM]';
      DAT_HS119b197C:array[1..80] of byte = (
        $E6, $83, $C4, $10, $83, $C4, $10, $87, $E6, $50, $56, $57, $51, $E8,
        $02, $00, $EB, $1A, $E8, $02, $00, $E9, $28, $5F, $B9, $43, $00, $B8,
        $E9, $28, $2E, $87, $05, $2E, $31, $85, $17, $00, $47, $D1, $C8, $E2,
        $F6, $C3, $B0, $03, $F8, $E9, $40, $D1, $B4, $1E, $01, $E7, $D6, $2F,
        $70, $91, $5D, $9A, $BC, $02, $45, $AC, $31, $2F, $27, $E4, $71, $91,
        $B5, $59, $75, $C2, $E8, $71, $47, $BC, $2F, $BC
      );
      REC_HS119b197C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS119b197C;
        Data:@DAT_HS119b197C
      );

      STR_HS119b198C:string [29] = 'HackStop 1.19 build 198 [COM]';
      DAT_HS119b198C:array[1..80] of byte = (
        $E6, $83, $C4, $10, $83, $C4, $10, $87, $E6, $50, $56, $57, $51, $E8,
        $02, $00, $EB, $1A, $E8, $02, $00, $FE, $28, $5F, $B9, $43, $00, $B8,
        $FE, $28, $2E, $87, $05, $2E, $31, $85, $17, $00, $47, $D1, $C8, $E2,
        $F6, $C3, $A7, $08, $7D, $2B, $A1, $A1, $0C, $42, $2F, $70, $1D, $CA,
        $02, $28, $01, $B4, $AB, $09, $C0, $6E, $D0, $5F, $9F, $B8, $5F, $06,
        $7E, $BC, $07, $7B, $B4, $5F, $50, $B7, $AA, $7E
      );
      REC_HS119b198C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS119b198C;
        Data:@DAT_HS119b198C
      );

      STR_HS119b204C:string [29] = 'HackStop 1.19 build 204 [COM]';
      DAT_HS119b204C:array[1..80] of byte = (
        $52, $B4, $30, $CD, $21, $86, $C4, $3D, $D6, $02, $73, $02, $CD, $20,
        $0E, $1F, $50, $B8, $EB, $04, $58, $EB, $FB, $9A, $E8, $02, $00, $24,
        $24, $5A, $B4, $09, $CD, $21, $50, $B8, $EB, $04, $58, $EB, $FB, $9A,
        $EB, $02, $D8, $88, $5A, $1F, $87, $E6, $83, $C4, $10, $83, $C4, $10,
        $87, $E6, $50, $56, $57, $51, $E8, $02, $00, $EB, $1A, $E8, $02, $00,
        $5E, $2A, $5F, $B9, $43, $00, $B8, $5E, $2A, $2E
      );
      REC_HS119b204C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS119b204C;
        Data:@DAT_HS119b204C
      );

      STR_HS119b206C:string [29] = 'HackStop 1.19 build 206 [COM]';
      DAT_HS119b206C:array[1..80] of byte = (
        $52, $B4, $30, $CD, $21, $86, $C4, $3D, $D6, $02, $73, $02, $CD, $20,
        $0E, $1F, $E8, $02, $00, $24, $24, $5A, $B4, $09, $CD, $21, $EB, $02,
        $D8, $88, $5A, $1F, $87, $E6, $83, $C4, $10, $83, $C4, $10, $87, $E6,
        $50, $56, $57, $51, $E8, $02, $00, $EB, $1A, $E8, $02, $00, $49, $24,
        $5F, $B9, $2B, $00, $B8, $49, $24, $2E, $87, $05, $2E, $31, $85, $17,
        $00, $47, $D1, $C8, $E2, $F6, $C3, $10, $5F, $68
      );
      REC_HS119b206C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_HS119b206C;
        Data:@DAT_HS119b206C
      );


{
 ############################################################
 ######################### LETTER I #########################
 ############################################################
}


      STR_IBMCRP:string [7] = 'IBM CRP';
      DAT_IBMCRP:array[1..80] of byte = (
          $BE, $00, $01, $8B, $E8, $8B, $D8, $8B, $F8, $83, $C5, $74, $90, $8B,
          $D0, $4A, $52, $87, $D6, $87, $FE, $59, $81, $E9, $05, $01, $52, $4F,
          $4E, $FA, $87, $EC, $49, $FD, $AC, $86, $E0, $AC, $46, $D2, $C0, $2A,
          $C4, $32, $C4, $02, $C1, $F6, $D8, $F6, $D0, $F6, $D8, $32, $47, $76,
          $90, $02, $C6, $AA, $FC, $E3, $22, $EB, $DF, $0D, $0A, $2D, $20, $57,
          $68, $9D, $20, $92, $52, $EE, $20, $9D, $30, $55
        );
      REC_IBMCRP:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_IBMCRP;
        Data:@DAT_IBMCRP
      );

{
 ############################################################
 ######################### LETTER J #########################
 ############################################################
}

      STR_jmtcp05a:string [25] = 'jmt-cp 0.5a / fds-cp 0.4a';
      DAT_jmtcp05a:array[1..22] of byte = (
          $0E, $1F, $B8, $52, $01, $EB, $01, $EA, $B9, $00, $00, $50, $50, $5F,
          $5E, $AD, $35, $00, $00, $AB, $E2, $F9
      );
      REC_jmtcp05a:TCompPtrRec = (
        DataLen:22;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_jmtcp05a;
        Data:@DAT_jmtcp05a
      );


      STR_JMCrypt07g:string [15] = 'JMCryptExe 0.7g';
      DAT_JMCrypt07g:array[1..80] of byte = (
          $1E, $06, $8C, $D8, $05, $10, $00, $8E, $D8, $8E, $C0, $2E, $01, $06,
          $C1, $03, $2E, $01, $06, $C5, $03, $8B, $E8, $2E, $A1, $FE, $01, $2E,
          $8B, $0E, $00, $02, $85, $C0, $74, $03, $49, $E3, $37, $83, $F9, $7C,
          $72, $16, $51, $B9, $00, $F8, $E8, $CE, $FE, $59, $83, $E9, $7C, $8C,
          $D8, $05, $80, $0F, $8E, $D8, $8E, $C0, $EB, $E5, $0B, $C9, $74, $18,
          $B8, $00, $02, $F7, $E1, $8B, $C8, $51, $E8, $B0
        );
      REC_JMCrypt07g:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_JMCrypt07g;
        Data:@DAT_JMCrypt07g
      );

      STR_JMCrypt07i:string [15] = 'JMCryptExe 0.7i';
      DAT_JMCrypt07i:array[1..80] of byte = (
          $1E, $06, $8C, $D8, $05, $10, $00, $8E, $D8, $8E, $C0, $2E, $01, $06,
          $A9, $04, $2E, $01, $06, $AD, $04, $8B, $E8, $2E, $A1, $FE, $01, $2E,
          $8B, $0E, $00, $02, $85, $C0, $74, $03, $49, $E3, $37, $83, $F9, $7C,
          $72, $16, $51, $B9, $00, $F8, $E8, $74, $FE, $59, $83, $E9, $7C, $8C,
          $D8, $05, $80, $0F, $8E, $D8, $8E, $C0, $EB, $E5, $0B, $C9, $74, $18,
          $B8, $00, $02, $F7, $E1, $8B, $C8, $51, $E8, $56
        );
      REC_JMCrypt07i:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_JMCrypt07i;
        Data:@DAT_JMCrypt07i
      );

      STR_JMCrypt07j:string [15] = 'JMCryptExe 0.7j';
      DAT_JMCrypt07j:array[1..80] of byte = (
          $1E, $06, $8C, $D8, $05, $10, $00, $8E, $D8, $8E, $C0, $2E, $01, $06,
          $91, $05, $2E, $01, $06, $95, $05, $8B, $E8, $2E, $A1, $FE, $01, $2E,
          $8B, $0E, $00, $02, $85, $C0, $74, $03, $49, $E3, $37, $83, $F9, $7C,
          $72, $16, $51, $B9, $00, $F8, $E8, $74, $FE, $59, $83, $E9, $7C, $8C,
          $D8, $05, $80, $0F, $8E, $D8, $8E, $C0, $EB, $E5, $0B, $C9, $74, $18,
          $B8, $00, $02, $F7, $E1, $8B, $C8, $51, $E8, $56
        );
      REC_JMCrypt07j:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_JMCrypt07j;
        Data:@DAT_JMCrypt07j
      );

      STR_JMCrypt07:string [24] = 'JMCryptExe 0.7 (general)';
      DAT_JMCrypt07:array[1..80] of byte = (
          $33, $F6, $33, $FF, $2E, $8B, $16, $FC, $01, $FC, $AC, $32, $C2, $AA,
          $02, $D6, $E2, $F8, $1E, $33, $C0, $8E, $D8, $FF, $36, $00, $00, $FF,
          $36, $02, $00, $8C, $0E, $02, $00, $C7, $06, $00, $00, $2F, $02, $F6,
          $F1, $83, $C4, $06, $8F, $06, $02, $00, $8F, $06, $00, $00, $1F, $FA,
          $8B, $D4, $BC, $03, $00, $8B, $E2, $FB, $06, $1E, $56, $57, $33, $FF,
          $8E, $C7, $BF, $84, $00, $26, $8E, $5D, $02, $BE
        );
      REC_JMCrypt07:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_JMCrypt07;
        Data:@DAT_JMCrypt07
      );

{
 ############################################################
 ######################### LETTER K #########################
 ############################################################
}

{
 ############################################################
 ######################### LETTER L #########################
 ############################################################
}




      STR_LockProg05a:string [13] = 'LockProg 0.5a';
      DAT_LockProg05a:array[1..70] of byte = (
          $50, $1E, $33, $C0, $8E, $D8, $A1, $04, $00, $8B, $1E, $06, $00, $3B,
          $06, $0C, $00, $75, $09, $3B, $06, $0E, $00, $74, $03, $EB, $06, $90,
          $1F, $58, $E8, $03, $00, $1F, $58, $C3, $BE, $03, $01, $2E, $8B, $0E,
          $3F, $02, $8B, $EC, $8B, $46, $00, $31, $04, $46, $46, $E2, $F7, $C3,
          $01, $00, $01, $00, $01, $00, $6C, $6F, $70, $72, $6F, $01, $02, $22
      );
      REC_LockProg05a:TCompPtrRec = (
        DataLen:70;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_LockProg05a;
        Data:@DAT_LockProg05a
      );

{
 ############################################################
 ######################### LETTER M #########################
 ############################################################
}

      STR_Mess114E:string [15] = 'Mess 1.14 [EXE]';
      DAT_Mess114E:array[1..61] of byte = (
          $1E, $0E, $1F, $BA, $9A, $0A, $81, $EA, $23, $0A, $B4, $09, $CD, $21,
          $1F, $B4, $4C, $CD, $21, $33, $38, $36, $2B, $20, $72, $65, $71, $75,
          $69, $72, $65, $64, $21, $24, $FA, $66, $33, $C0, $EB, $01, $00, $0F,
          $23, $F8, $0F, $23, $C0, $0F, $23, $C8, $0F, $23, $D0, $0F, $23, $D8,
          $FB, $B4, $0D, $CD, $21
        );
      REC_Mess114E:TCompPtrRec = (
        DataLen:61;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_Mess114E;
        Data:@DAT_Mess114E
      );

      STR_Mess115E:string [15] = 'Mess 1.15 [EXE]';
      DAT_Mess115E:array[1..61] of byte = (
          $1E, $0E, $1F, $BA, $01, $0B, $81, $EA, $8A, $0A, $B4, $09, $CD, $21,
          $1F, $B4, $4C, $CD, $21, $33, $38, $36, $2B, $20, $72, $65, $71, $75,
          $69, $72, $65, $64, $21, $24, $FA, $66, $33, $C0, $EB, $01, $00, $0F,
          $23, $F8, $0F, $23, $C0, $0F, $23, $C8, $0F, $23, $D0, $0F, $23, $D8,
          $FB, $B4, $0D, $CD, $21
        );
      REC_Mess115E:TCompPtrRec = (
        DataLen:61;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_Mess115E;
        Data:@DAT_Mess115E
      );

      STR_Mess117E:string [15] = 'Mess 1.17 [EXE]';
      DAT_Mess117E:array[1..19] of byte = (
          $1E, $0E, $1F, $BA, $1C, $0B, $81, $EA, $9A, $0A, $B4, $09, $CD, $21,
          $1F, $B4, $4C, $CD, $21
        );
      REC_Mess117E:TCompPtrRec = (
        DataLen:19;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_Mess117E;
        Data:@DAT_Mess117E
      );

      STR_Mess120E:string [9] = 'Mess 1.20';
      DAT_Mess120E:array[1..19] of byte = (
        $1E, $0E, $1F, $BA, $CC, $01, $81, $EA, $4A, $01, $B4, $09, $CD, $21,
        $1F, $B4, $4C, $CD, $21
      );
      REC_Mess120E:TCompPtrRec = (
        DataLen:19;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_Mess120E;
        Data:@DAT_Mess120E
      );

      {
      byte 49:
      1.14: $36
      1.15: $9D
      1.17: $AD
      }

      STR_Mess114C:string[15] = 'Mess 1.14 [COM]';
      DAT_Mess114C:array[1..80] of byte = (
        $BA, $14, $01, $B4, $09, $CD, $21, $CD, $20, $B4, $30, $CD, $21, $3C,
        $02, $77, $02, $CD, $20, $BD, $4A, $01, $FF, $E5, $FA, $66, $33, $C0,
        $0F, $23, $F8, $0F, $23, $C0, $0F, $23, $C8, $0F, $23, $D0, $0F, $23,
        $D8, $BC, $03, $00, $81, $C4, $36, $08, $8B, $F4, $83, $EE, $02, $33,
        $DB, $FD, $AD, $80, $F4, $00, $C0, $C8, $02, $83, $C3, $13, $33, $C3,
        $50, $81, $FE, $82, $01, $76, $03, $EB, $EB, $00
      );
      REC_Mess114C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_Mess114C;
        Data:@DAT_Mess114C
      );

      STR_Mess115C:string[15] = 'Mess 1.15 [COM]';
      DAT_Mess115C:array[1..80] of byte = (
        $BA, $14, $01, $B4, $09, $CD, $21, $CD, $20, $B4, $30, $CD, $21, $3C,
        $02, $77, $02, $CD, $20, $BD, $4A, $01, $FF, $E5, $FA, $66, $33, $C0,
        $0F, $23, $F8, $0F, $23, $C0, $0F, $23, $C8, $0F, $23, $D0, $0F, $23,
        $D8, $BC, $03, $00, $81, $C4, $9D, $08, $8B, $F4, $83, $EE, $02, $33,
        $DB, $FD, $AD, $80, $F4, $00, $C0, $C8, $02, $83, $C3, $13, $33, $C3,
        $50, $81, $FE, $82, $01, $76, $03, $EB, $EB, $00
      );
      REC_Mess115C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_Mess115C;
        Data:@DAT_Mess115C
      );

      STR_Mess117C:string[15] = 'Mess 1.17 [COM]';
      DAT_Mess117C:array[1..80] of byte = (
        $BA, $14, $01, $B4, $09, $CD, $21, $CD, $20, $B4, $30, $CD, $21, $3C,
        $02, $77, $02, $CD, $20, $BD, $4A, $01, $FF, $E5, $FA, $66, $33, $C0,
        $0F, $23, $F8, $0F, $23, $C0, $0F, $23, $C8, $0F, $23, $D0, $0F, $23,
        $D8, $BC, $03, $00, $81, $C4, $AD, $08, $8B, $F4, $83, $EE, $02, $33,
        $DB, $FD, $AD, $80, $F4, $00, $C0, $C8, $02, $83, $C3, $13, $33, $C3,
        $50, $81, $FE, $82, $01, $76, $03, $EB, $EB, $00
      );
      REC_Mess117C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_Mess117C;
        Data:@DAT_Mess117C
      );

      STR_MessC:string [10] = 'Mess [COM]';
      DAT_MessC:array[1..25] of byte = (
        $BA, $14, $01, $B4, $09, $CD, $21, $CD, $20, $B4, $30, $CD, $21, $3C,
        $02, $77, $02, $CD, $20, $BD, $00, $00, $FF, $E5, $FA
      );
      REC_MessC:TCompPtrRec = (
        DataLen:25;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_MessC;
        Data:@DAT_MessC
      );

      STR_PCrypt350E:string [17] = 'PCrypt 3.50 [EXE]';
      DAT_PCrypt350E:array[1..32] of byte = (
          $50, $43, $52, $59, $50, $54, $20, $76, $33, $2E, $35, $30, $20, $62,
          $79, $20, $4D, $45, $52, $4C, $69, $4E, $00, $00, $00, $30, $0A, $A4,
          $19, $00, $00, $4D
        );
      REC_PCrypt350E:TCompPtrRec = (
        DataLen:32;
        FileType:EXE;
        NameType:ENCRYPTER;
        Name:@STR_PCrypt350E;
        Data:@DAT_PCrypt350E
      );


      STR_PirateStop105:string [16] = 'Pirate Stop 1.05';
      DAT_PirateStop105:array[1..29] of byte = (
          $8D, $9E, $55, $04, $8D, $8E, $99, $06, $2B, $CB, $2E, $8A, $07, $34,
          $00, $34, $00, $34, $00, $FE, $C0, $F6, $D0, $2E, $88, $07, $43, $E2,
          $ED
        );
      REC_PirateStop105:TCompPtrRec = (
        DataLen:29;
        FileType:EXE;
        NameType:PROTECTOR;
        Name:@STR_PirateStop105;
        Data:@DAT_PirateStop105
      );

      STR_PKLite100E:string [24] = 'PKLite 1.00 - 1.05 [EXE]';
      DAT_PKLite100E:array[1..80] of byte = (
          $B8, $00, $00, $BA, $00, $00, $8C, $DB, $03, $D8, $3B, $1E, $02, $00,
          $73, $1D, $83, $EB, $20, $FA, $8E, $D3, $BC, $00, $02, $FB, $83, $EB,
          $00, $8E, $C3, $53, $B9, $00, $00, $33, $FF, $57, $BE, $48, $01, $FC,
          $F3, $A5, $CB, $B4, $09, $BA, $36, $01, $CD, $21, $CD, $20, $4E, $6F,
          $74, $20, $65, $6E, $6F, $75, $67, $68, $20, $6D, $65, $6D, $6F, $72,
          $79, $24, $FD, $8C, $DB, $53, $83, $C3, $00, $03
        );
      REC_PKLite100E:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_PKLite100E;
        Data:@DAT_PKLite100E
      );

      STR_PKLite112E:string [24] = 'PKLite 1.12 / 1.20 [EXE]';
      DAT_PKLite112E:array[1..80] of byte = (
          $B8, $00, $00, $BA, $00, $00, $05, $00, $00, $3B, $06, $02, $00, $73,
          $1A, $2D, $20, $00, $FA, $8E, $D0, $FB, $2D, $00, $00, $8E, $C0, $50,
          $B9, $00, $00, $33, $FF, $57, $BE, $44, $01, $FC, $F3, $A5, $CB, $B4,
          $09, $BA, $32, $01, $CD, $21, $CD, $20, $4E, $6F, $74, $20, $65, $6E,
          $6F, $75, $67, $68, $20, $6D, $65, $6D, $6F, $72, $79, $24, $FD, $8C,
          $DB, $53, $83, $C3, $00, $03, $DA, $8C, $CD, $8B
        );
      REC_PKLite112E:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_PKLite112E;
        Data:@DAT_PKLite112E
      );

      STR_PKLite114E:string [17] = 'PKLite 1.14 [EXE]';
      DAT_PKLite114E:array[1..80] of byte = (
          $B8, $00, $00, $BA, $00, $00, $05, $00, $00, $3B, $06, $02, $00, $72,
          $1B, $B4, $09, $BA, $18, $01, $CD, $21, $CD, $20, $4E, $6F, $74, $20,
          $65, $6E, $6F, $75, $67, $68, $20, $6D, $65, $6D, $6F, $72, $79, $24,
          $2D, $20, $00, $8E, $D0, $2D, $00, $00, $8E, $C0, $50, $B9, $00, $00,
          $33, $FF, $57, $BE, $42, $01, $FC, $F3, $A5, $CB, $FD, $8C, $DB, $53,
          $83, $C3, $00, $03, $DA, $8C, $CD, $8B, $C2, $80
        );
      REC_PKLite114E:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_PKLite114E;
        Data:@DAT_PKLite114E
      );

      STR_PKLite115E:string [17] = 'PKLite 1.15 [EXE]';
      DAT_PKLite115E:array[1..80] of byte = (
          $B8, $00, $00, $BA, $00, $00, $05, $00, $00, $3B, $06, $02, $00, $72,
          $1B, $B4, $09, $BA, $18, $01, $CD, $21, $CD, $20, $4E, $6F, $74, $20,
          $65, $6E, $6F, $75, $67, $68, $20, $6D, $65, $6D, $6F, $72, $79, $24,
          $2D, $20, $00, $8E, $D0, $2D, $00, $00, $90, $8E, $C0, $50, $B9, $00,
          $00, $33, $FF, $57, $BE, $44, $01, $FC, $F3, $A5, $CB, $90, $FD, $8C,
          $DB, $53, $81, $C3, $00, $00, $03, $DA, $8C, $CD
        );
      REC_PKLite115E:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_PKLite115E;
        Data:@DAT_PKLite115E
      );

      STR_PKLite150E:string [24] = 'PKLite 1.50 - 2.01 [EXE]';
      DAT_PKLite150E:array[1..80] of byte = (
          $50, $B8, $00, $00, $BA, $00, $00, $05, $00, $00, $3B, $06, $02, $00,
          $72, $2A, $B4, $09, $BA, $1C, $01, $CD, $21, $B8, $01, $4C, $CD, $21,
          $4E, $6F, $74, $20, $65, $6E, $6F, $75, $67, $68, $20, $6D, $65, $6D,
          $6F, $72, $79, $24, $20, $20, $20, $20, $20, $20, $20, $20, $20, $20,
          $20, $20, $59, $2D, $20, $00, $8E, $D0, $51, $2D, $00, $00, $8E, $C0,
          $50, $B9, $00, $00, $33, $FF, $57, $BE, $54, $01
        );
      REC_PKLite150E:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_PKLite150E;
        Data:@DAT_PKLite150E
      );

      STR_PKLite200Ee:string [24] = 'PKLite 2.00á extra [EXE]';
      DAT_PKLite200Ee:array[1..80] of byte = (
          $50, $B8, $00, $00, $BA, $00, $00, $05, $00, $00, $3B, $06, $02, $00,
          $72, $32, $B4, $09, $BA, $1C, $01, $CD, $21, $B8, $01, $4C, $CD, $21,
          $4E, $6F, $74, $20, $65, $6E, $6F, $75, $67, $68, $20, $6D, $65, $6D,
          $6F, $72, $79, $24, $20, $20, $20, $20, $20, $20, $20, $20, $20, $20,
          $20, $20, $EA, $C0, $ED, $23, $F1, $F3, $A5, $C3, $59, $2D, $20, $00,
          $8E, $D0, $51, $2D, $00, $00, $50, $80, $3E, $41
        );
      REC_PKLite200Ee:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_PKLite200Ee;
        Data:@DAT_PKLite200Ee
      );

      STR_PW10E:string [12] = 'PW 1.0 [EXE]';
      DAT_PW10E:array[1..80] of byte = (
          $8C, $C0, $2E, $A3, $23, $01, $8C, $D8, $2E, $A3, $25, $01, $8C, $D0,
          $2E, $A3, $21, $01, $8B, $C4, $2E, $A3, $1F, $01, $8C, $C8, $8E, $D8,
          $8E, $C0, $8E, $D0, $BC, $85, $01, $E8, $58, $00, $8C, $C8, $2B, $06,
          $08, $01, $03, $06, $06, $01, $A3, $06, $01, $8C, $C8, $2B, $06, $08,
          $01, $03, $06, $21, $01, $8E, $D0, $8B, $26, $1F, $01, $2E, $A1, $23,
          $01, $8E, $C0, $2E, $A1, $25, $01, $8E, $D8, $FB
        );
      REC_PW10E:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PROTECTOR;
        Name:@STR_PW10E;
        Data:@DAT_PW10E
      );

      STR_PW10C:string [12] = 'PW 1.0 [COM]';
      DAT_PW10C:array[1..80] of byte = (
          $8C, $C8, $8E, $D8, $8E, $C0, $8E, $D0, $89, $26, $1A, $01, $BC, $43,
          $01, $E8, $53, $00, $2E, $8B, $26, $1A, $01, $8C, $C8, $2B, $06, $18,
          $01, $A3, $18, $01, $8E, $D8, $8E, $C0, $8E, $D0, $B9, $13, $00, $33,
          $F6, $2E, $8A, $84, $03, $01, $88, $84, $00, $01, $46, $E2, $F4, $2E,
          $FF, $2E, $16, $01, $45, $6E, $74, $65, $72, $20, $50, $61, $73, $73,
          $77, $6F, $72, $64, $3A, $20, $24, $0A, $0D, $49
        );
      REC_PW10C:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_PW10C;
        Data:@DAT_PW10C
      );

      STR_RCCII286113h:string [22] = 'RCC II/286 1.13 (hard)';
      DAT_RCCII286113h:array[1..80] of byte = (
          $B8, $11, $30, $1E, $CD, $21, $86, $C4, $3D, $FF, $02, $73, $02, $CD,
          $20, $B4, $09, $0E, $1F, $E8, $02, $00, $24, $24, $5A, $CD, $21, $EB,
          $02, $D8, $88, $1F, $5A, $FA, $87, $EC, $83, $C4, $28, $4C, $87, $EC,
          $B0, $AD, $E6, $64, $BB, $FF, $00, $8D, $86, $19, $00, $43, $53, $89,
          $47, $02, $80, $37, $03, $C3, $62, $60, $06, $50, $2B, $C0, $8E, $C0,
          $FA, $85, $06, $5C, $E0, $B8, $EB, $03, $EB, $FC
        );
      REC_RCCII286113h:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_RCCII286113h;
        Data:@DAT_RCCII286113h
      );

      STR_RCCII286113m:string [22] = 'RCC II/286 1.13 (mild)';
      DAT_RCCII286113m:array[1..80] of byte = (
          $B8, $11, $30, $1E, $CD, $21, $86, $C4, $3D, $FF, $02, $73, $02, $CD,
          $20, $B4, $09, $0E, $1F, $E8, $02, $00, $24, $24, $5A, $CD, $21, $EB,
          $02, $D8, $88, $1F, $5A, $FA, $87, $EC, $83, $C4, $28, $4C, $87, $EC,
          $B0, $AD, $E6, $64, $BB, $FF, $00, $8D, $86, $19, $00, $43, $53, $89,
          $47, $02, $80, $37, $03, $C3, $62, $1E, $2E, $FF, $36, $00, $01, $BA,
          $05, $04, $2E, $89, $16, $00, $01, $B4, $30, $CD
        );
      REC_RCCII286113m:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_RCCII286113m;
        Data:@DAT_RCCII286113m
      );

      STR_RCCII286114h:string [22] = 'RCC II/286 1.14 (hard)';
      DAT_RCCII286114h:array[1..80] of byte = (
          $87, $EC, $83, $C4, $28, $4C, $87, $EC, $B0, $AD, $E6, $64, $BB, $FF,
          $00, $8D, $86, $29, $00, $43, $53, $89, $47, $02, $80, $37, $03, $C3,
          $62, $60, $57, $51, $E8, $02, $00, $EB, $1A, $E8, $02, $00, $C1, $0F,
          $5F, $B9, $ED, $00, $B8, $C1, $0F, $2E, $87, $05, $2E, $31, $85, $17,
          $00, $47, $D1, $C8, $E2, $F6, $C3, $98, $B0, $71, $EB, $F6, $AE, $B9,
          $DB, $77, $C3, $A5, $37, $08, $4C, $91, $BE, $35
        );
      REC_RCCII286114h:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_RCCII286114h;
        Data:@DAT_RCCII286114h
      );

      STR_RCCII286114m:string [22] = 'RCC II/286 1.14 (mild)';
      DAT_RCCII286114m:array[1..80] of byte = (
          $87, $EC, $83, $C4, $28, $4C, $87, $EC, $B0, $AD, $E6, $64, $BB, $FF,
          $00, $8D, $86, $29, $00, $43, $53, $89, $47, $02, $80, $37, $03, $C3,
          $62, $57, $51, $E8, $02, $00, $EB, $1A, $E8, $02, $00, $07, $0C, $5F,
          $B9, $26, $00, $B8, $07, $0C, $2E, $87, $05, $2E, $31, $85, $17, $00,
          $47, $D1, $C8, $E2, $F6, $C3, $5E, $50, $99, $6D, $DE, $26, $09, $05,
          $B8, $D3, $C4, $CE, $79, $EE, $7D, $3F, $AB, $3F
        );
      REC_RCCII286114m:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_RCCII286114m;
        Data:@DAT_RCCII286114m
      );

      STR_RCCII286115h:string [22] = 'RCC II/286 1.15 (hard)';
      DAT_RCCII286115h:array[1..80] of byte = (
        $87, $EC, $83, $C4, $28, $4C, $87, $EC, $B0, $AD, $E6, $64, $BB, $FF,
        $00, $8D, $86, $29, $00, $43, $53, $89, $47, $02, $80, $37, $03, $C3,
        $62, $60, $57, $51, $E8, $02, $00, $EB, $1A, $E8, $02, $00, $BE, $0F,
        $5F, $B9, $ED, $00, $B8, $BE, $0F, $2E, $87, $05, $2E, $31, $85, $17,
        $00, $47, $D1, $C8, $E2, $F6, $C3, $E7, $8F, $EE, $24, $11, $5D, $40,
        $27, $89, $3C, $5A, $CD, $F7, $B3, $6E, $41, $4A
      );
      REC_RCCII286115h:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_RCCII286115h;
        Data:@DAT_RCCII286115h
      );

      STR_RCCII286115m:string [22] = 'RCC II/286 1.15 (mild)';
      DAT_RCCII286115m:array[1..80] of byte = (
        $87, $EC, $83, $C4, $28, $4C, $87, $EC, $B0, $AD, $E6, $64, $BB, $FF,
        $00, $8D, $86, $29, $00, $43, $53, $89, $47, $02, $80, $37, $03, $C3,
        $62, $57, $51, $E8, $02, $00, $EB, $1A, $E8, $02, $00, $04, $0C, $5F,
        $B9, $26, $00, $B8, $04, $0C, $2E, $87, $05, $2E, $31, $85, $17, $00,
        $47, $D1, $C8, $E2, $F6, $C3, $5D, $51, $19, $AD, $BE, $16, $11, $09,
        $BE, $4D, $05, $AE, $49, $F6, $71, $39, $A8, $3E
      );
      REC_RCCII286115m:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_RCCII286115m;
        Data:@DAT_RCCII286115m
      );

      STR_RCCII286116h:string [22] = 'RCC II/286 1.16 (hard)';
      DAT_RCCII286116h:array[1..80] of byte = (
        $EC, $83, $C4, $28, $4C, $87, $EC, $B0, $AD, $E6, $64, $BB, $FF, $00,
        $8D, $86, $28, $00, $43, $53, $89, $47, $02, $80, $37, $03, $C3, $62,
        $60, $57, $51, $E8, $02, $00, $EB, $1A, $E8, $02, $00, $21, $17, $5F,
        $B9, $BF, $00, $B8, $21, $17, $2E, $87, $05, $2E, $31, $85, $17, $00,
        $47, $D1, $C8, $E2, $F6, $C3, $78, $D8, $A8, $A3, $11, $22, $04, $FF,
        $07, $26, $07, $10, $A5, $C2, $D6, $1A, $57, $6C
      );
      REC_RCCII286116h:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_RCCII286116h;
        Data:@DAT_RCCII286116h
      );

      STR_RCCII286116m:string [22] = 'RCC II/286 1.16 (mild)';
      DAT_RCCII286116m:array[1..80] of byte = (
        $EC, $83, $C4, $28, $4C, $87, $EC, $B0, $AD, $E6, $64, $BB, $FF, $00,
        $8D, $86, $28, $00, $43, $53, $89, $47, $02, $80, $37, $03, $C3, $62,
        $57, $51, $E8, $02, $00, $EB, $1A, $E8, $02, $00, $AC, $12, $5F, $B9,
        $26, $00, $B8, $AC, $12, $2E, $87, $05, $2E, $31, $85, $17, $00, $47,
        $D1, $C8, $E2, $F6, $C3, $F5, $1B, $BC, $7F, $57, $62, $2B, $94, $F0,
        $B8, $5E, $07, $1D, $5C, $24, $13, $3D, $74, $6F
      );
      REC_RCCII286116m:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_RCCII286116m;
        Data:@DAT_RCCII286116m
      );

      STR_RCCII386051:string [15] = 'RCC II/386 0.51';
      DAT_RCCII386051:array[1..80] of byte = (
          $E8, $54, $01, $4C, $55, $05, $B9, $44, $4F, $00, $00, $4F, $4B, $48,
          $49, $46, $47, $C9, $A9, $12, $FB, $AB, $45, $66, $D4, $C7, $A7, $67,
          $15, $B1, $7F, $36, $19, $BD, $6B, $30, $1D, $B9, $7F, $2A, $01, $A5,
          $7B, $22, $73, $06, $A0, $60, $21, $7F, $0B, $A8, $2C, $12, $CC, $5D,
          $4C, $A4, $1C, $1A, $18, $D9, $32, $CE, $47, $AC, $FE, $16, $4B, $FB,
          $EA, $94, $EB, $4C, $07, $CA, $7F, $FA, $27, $8D
        );
      REC_RCCII386051:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_RCCII386051;
        Data:@DAT_RCCII386051
      );

      STR_SCRe2b102:string [11] = 'SCRe2b 1.02';
      DAT_SCRe2b102:array[1..80] of byte = (
          $83, $C2, $10, $0E, $1F, $BE, $93, $00, $BF, $00, $01, $B9, $0E, $00,
          $F3, $A4, $AD, $8B, $C8, $E3, $0D, $AD, $8B, $D8, $AD, $03, $C2, $8E,
          $C0, $26, $01, $17, $E2, $F3, $01, $16, $8B, $00, $01, $16, $91, $00,
          $83, $EA, $10, $8E, $C2, $B4, $4A, $BB, $00, $00, $CD, $21, $73, $38,
          $81, $FB, $00, $00, $73, $32, $0E, $1F, $BA, $4C, $00, $B4, $09, $CD,
          $21, $B8, $FF, $4C, $CD, $21, $50, $72, $6F, $67
        );
      REC_SCRe2b102:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:CONVERTER;
        Name:@STR_SCRe2b102;
        Data:@DAT_SCRe2b102
      );

      STR_SnoopStop115:string [14] = 'SnoopStop 1.15';
      DAT_SnoopStop115:array[1..45] of byte = (
{ original
          $55, $E8, $00, $00, $59, $8B, $E9, $81, $ED, $D5, $02, $66, $60, $8D,
          $9E, $FE, $02, $8D, $8E, $08, $0D, $2B, $CB, $2E, $8A, $07, $34, $00,
          $34, $00, $34, $00, $FE, $C0, $F6, $D0, $2E, $88, $07, $43, $E2, $ED,
          $66, $61, $5D,
}
          $55, $E8, $00, $00, $59, $8B, $E9, $81, $ED, $00, $02, $66, $60, $8D,
          $9E, $00, $02, $8D, $8E, $00, $00, $2B, $CB, $2E, $8A, $07, $34, $00,
          $34, $00, $34, $00, $FE, $C0, $F6, $D0, $2E, $88, $07, $43, $E2, $ED,
          $66, $61, $5D
        );
      REC_SnoopStop115:TCompPtrRec = (
        DataLen:45;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_SnoopStop115;
        Data:@DAT_SnoopStop115
      );

      STR_StonesComcrypt:string [17] = 'Stone''s Comcrypt';
      DAT_StonesComcrypt:array[1..34] of byte = (
          $BB, $05, $01, $B9, $00, $00, $33, $C0, $2E, $80, $2F, $01, $43, $E2,
          $F9, $BB, $05, $01, $B9, $00, $00, $8B, $07, $89, $47, $FB, $43, $E2,
          $F8, $B8, $00, $01, $FF, $E0
        );
      REC_StonesComcrypt:TCompPtrRec = (
        DataLen:34;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_StonesComcrypt;
        Data:@DAT_StonesComcrypt
      );

{
 ############################################################
 ######################### LETTER T #########################
 ############################################################
}

      DAT_TinyProg2_3:array[1..80] of byte = (
          $83, $EC, $10, $83, $E4, $E0, $8B, $EC, $50, $BE, $05, $01, $03, $36,
          $01, $01, $8C, $D2, $8C, $D8, $03, $44, $0C, $33, $C2, $75, $CF, $FC,
          $8E, $C2, $33, $FF, $BB, $21, $10, $33, $D2, $8B, $C2, $B9, $08, $00,
          $D1, $E0, $73, $02, $33, $C3, $E2, $F8, $AB, $FE, $C6, $75, $EE, $8C,
          $DA, $52, $56, $01, $54, $18, $01, $54, $14, $01, $54, $1C, $8B, $54,
          $2E, $00, $00, $0E, $FF, $74, $10, $C4, $7C, $16
          { pos. 72:
            v2: $74FF
            v3: $4C8B
          }
        );
      REC_TinyProg2_3:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_Empty;
        Data:@DAT_TinyProg2_3
      );

      STR_TP4_1:string [18] = 'Turbo Pascal 4 (1)';
      DAT_TP4_1:array[1..65] of byte = (
          $BA, $00, $00, $8E, $DA, $8C, $06, $00, $00, $8B, $C4, $05, $13, $00,
          $B1, $04, $D3, $E8, $8C, $D2, $03, $C2, $A3, $00, $00, $A3, $00, $00,
          $26, $A1, $00, $00, $2D, $00, $10, $A3, $00, $00, $C7, $06, $00, $00,
          $00, $00, $8C, $0E, $00, $00, $C7, $06, $00, $00, $00, $00, $8C, $0E,
          $00, $00, $C6, $06, $00, $00, $00, $33, $C0
        );
      REC_TP4_1:TCompPtrRec = (
        DataLen:65;
        FileType:EXE;
        NameType:COMPILER;
        Name:@STR_TP4_1;
        Data:@DAT_TP4_1
      );

      STR_TP4_2:string [18] = 'Turbo Pascal 4 (2)';
      DAT_TP4_2:array[1..80] of byte = (
          $BA, $00, $01, $8E, $DA, $8C, $06, $00, $02, $8B, $C4, $05, $04, $00,
          $A3, $00, $02, $8C, $16, $00, $02, $05, $0F, $00, $B1, $04, $D3, $E8,
          $8C, $D2, $03, $C2, $A3, $00, $02, $A3, $00, $02, $26, $A1, $02, $00,
          $2D, $00, $10, $A3, $00, $02, $C7, $06, $00, $02, $D2, $00, $8C, $0E,
          $00, $02, $C7, $06, $00, $02, $D7, $00, $8C, $0E, $00, $02, $C6, $06,
          $00, $02, $02, $33, $C0, $A3, $00, $02, $A3, $00
        );
      REC_TP4_2:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:COMPILER;
        Name:@STR_TP4_2;
        Data:@DAT_TP4_2
      );

      STR_TP5:string [14] = 'Turbo Pascal 5';
      DAT_TP5:array[1..60] of byte = (
          $BA, $00, $00, $8E, $DA, $8C, $06, $00, $00, $33, $ED, $8B, $C4, $05,
          $13, $00, $B1, $04, $D3, $E8, $8C, $D2, $03, $C2, $A3, $00, $00, $A3,
          $00, $00, $03, $06, $00, $00, $A3, $00, $00, $A3, $00, $00, $A3, $00,
          $00, $26, $A1, $00, $00, $2D, $00, $00, $A3, $00, $00, $BF, $00, $00,
          $BE, $00, $00, $B9
        );
      REC_TP5:TCompPtrRec = (
        DataLen:60;
        FileType:EXE;
        NameType:COMPILER;
        Name:@STR_TP5;
        Data:@DAT_TP5
      );

      STR_TP6:string [14] = 'Turbo Pascal 6';
      DAT_TP6:array[1..72] of byte = (
          $BA, $00, $00, $8E, $DA, $8C, $06, $00, $00, $33, $ED, $8B, $C4, $05,
          $13, $00, $B1, $04, $D3, $E8, $8C, $D2, $03, $C2, $A3, $00, $00, $A3,
          $00, $00, $03, $06, $00, $00, $A3, $00, $00, $A3, $00, $00, $A3, $00,
          $00, $A3, $00, $00, $26, $8B, $16, $00, $00, $89, $16, $00, $00, $C7,
          $06, $00, $00, $00, $00, $8C, $0E, $00, $00, $BF, $00, $00, $BE, $00,
          $00, $B9
        );
      REC_TP6:TCompPtrRec = (
        DataLen:72;
        FileType:EXE;
        NameType:COMPILER;
        Name:@STR_TP6;
        Data:@DAT_TP6
      );

      STR_TP7:string [22] = 'Turbo/Borland Pascal 7';
      DAT_TP7:array[1..74] of byte = (
          $BA, $00, $00, $8E, $DA, $8C, $06, $00, $00, $33, $ED, $E8, $00, $00,
          $E8, $00, $00, $8B, $C4, $05, $13, $00, $B1, $04, $D3, $E8, $8C, $D2,
          $03, $C2, $A3, $00, $00, $A3, $00, $00, $03, $06, $00, $00, $A3, $00,
          $00, $A3, $00, $00, $A3, $00, $00, $A3, $00, $00, $8E, $06, $00, $00,
          $26, $A1, $00, $00, $A3, $00, $00, $C7, $06, $00, $00, $00, $00, $8C,
          $0E, $00, $00, $BF
        );
      REC_TP7:TCompPtrRec = (
        DataLen:74;
        FileType:EXE;
        NameType:COMPILER;
        Name:@STR_TP7;
        Data:@DAT_TP7
      );

      STR_TP7Juffa:string [45] = 'Turbo/Borland Pascal 7 (RTL by Norbert Juffa)';
      DAT_TP7Juffa:array[1..80] of byte = (
          $F8, $60, $F9, $72, $1F, $8B, $DC, $36, $C5, $1F, $83, $C3, $05, $80,
          $7F, $FB, $9A, $74, $F7, $80, $7F, $FB, $C8, $75, $0C, $BB, $D2, $00,
          $E8, $0B, $02, $B0, $FE, $E9, $FE, $01, $61, $BA, $00, $00, $8E, $DA,
          $8C, $06, $00, $00, $33, $ED, $E8, $00, $00, $E8, $B2, $00, $8B, $C4,
          $05, $13, $00, $B1, $04, $D3, $E8, $8C, $D2, $03, $C2, $A3, $00, $00,
          $A3, $00, $00, $03, $06, $00, $00, $A3, $00, $00
        );
      REC_TP7Juffa:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:COMPILER;
        Name:@STR_TP7Juffa;
        Data:@DAT_TP7Juffa
      );


      STR_VSS:string [3] = 'VSS';
      DAT_VSS:array[1..80] of byte = (
        $5B, $81, $EB, $B2, $01, $1E, $06, $50, $51, $52, $53, $54, $55, $56,
        $57, $06, $1E, $8B, $EB, $B4, $30, $CD, $21, $86, $E0, $3D, $00, $03,
        $73, $03, $E9, $E9, $01, $E8, $4F, $02, $0B, $C0, $74, $03, $E8, $A1,
        $01, $26, $8E, $06, $2C, $00, $33, $FF, $8B, $C7, $FC, $2E, $88, $86,
        $1E, $05, $47, $26, $39, $05, $75, $FA, $83, $C7, $04, $2E, $89, $BE,
        $1A, $05, $2E, $8C, $86, $1C, $05, $8B, $D7, $8C
      );
      REC_VSS:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:PROTECTOR;
        Name:@STR_VSS;
        Data:@DAT_VSS
      );

{
 ############################################################
 ######################### LETTER W #########################
 ############################################################
}

{ to implement }
      DAT_WWPack300PR:array[1..12] of byte = (
        $00, $00, $BE, $00, $00, $BA, $00, $00, $BF, $00, $00, $B9
      );
      REC_WWPack300PR:TCompPtrRec = (
        DataLen:12;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_Empty;
        Data:@DAT_WWPack300PR
      );

      DAT_WWPack30xPR:array[1..19] of byte = (
        $00, $00, $BE, $00, $00, $BF, $00, $00, $B9, $00, $00, $8C, $CD, $81,
        $ED, $00, $00, $8B, $DD
      );
      REC_WWPack30xPR:TCompPtrRec = (
        DataLen:19;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_Empty;
        Data:@DAT_WWPack30xPR
      );

      DAT_WWPack30xP:array[1..16] of byte = (
        $00, $00, $B8, $00, $00, $8C, $CA, $03, $D0, $8C, $C9, $81, $00, $00,
        $00, $51
      );
      REC_WWPack30xP:TCompPtrRec = (
        DataLen:16;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_Empty;
        Data:@DAT_WWPack30xP
      );

{ NOT IN USE !! too less IDs
      STR_WWPMut10:string [10] = 'WWPMut 1.0';
      DAT_WWPMut10:array[1..63] of byte = (
          $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
          $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
          $00, $00, $00, $00, $00, $03, $D0, $00, $00, $00, $00, $00, $00, $00,
          $00, $00, $00, $00, $00, $00, $00, $00, $48, $79, $EE, $00, $00, $00,
          $00, $00, $00, $00, $00, $00, $CB
        );
      REC_WWPMut10:TCompPtrRec = (
        DataLen:63;
        FileType:EXE;
        NameType:PROTECTOR;
        Name:@STR_WWPMut10;
        Data:@DAT_WWPMut10
      );
}


{
 ############################################################
 ######################### LETTER X #########################
 ############################################################
}

      STR_XcomOR099g:string [14] = 'XcomOR 0.99f/g';
      DAT_XcomOR099g:array[1..80] of byte = (
        $1E, $6A, $00, $1F, $81, $36, $05, $00, $97, $87, $81, $36, $0D, $00,
        $97, $87, $8B, $1E, $84, $00, $A1, $86, $00, $50, $1F, $81, $3F, $90,
        $90, $75, $0E, $90, $90, $1F, $C7, $06, $00, $01, $CD, $20, $BE, $00,
        $01, $FF, $E6, $1F, $83, $C5, $04, $83, $06, $1C, $01, $43, $C7, $06,
        $00, $01, $EB, $19, $B8, $00, $01, $50, $50, $FF, $E0, $5B, $5F, $90,
        $BB, $00, $00, $81, $F3, $F0, $99, $8B, $4D, $1C
      );
      REC_XcomOR099g:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_XcomOR099g;
        Data:@DAT_XcomOR099g
      );

      STR_XcomOR099h:string [12] = 'XcomOR 0.99h';
      DAT_XcomOR099h:array[1..80] of byte = (
        $6A, $00, $1F, $E8, $01, $00, $EA, $8C, $C3, $66, $C1, $E3, $10, $5B,
        $83, $C3, $18, $66, $87, $1E, $80, $00, $41, $EB, $01, $EA, $6A, $00,
        $C3, $D9, $66, $89, $1E, $80, $00, $81, $36, $05, $00, $0D, $F0, $81,
        $36, $0D, $00, $ED, $BE, $8B, $1E, $84, $00, $A1, $86, $00, $50, $1F,
        $83, $3F, $87, $75, $10, $90, $90, $1F, $26, $C7, $06, $00, $01, $CD,
        $20, $BE, $00, $01, $FF, $E6, $EA, $83, $C4, $02
      );
      REC_XcomOR099h:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_XcomOR099h;
        Data:@DAT_XcomOR099h
      );

      STR_XcomOR099i:string [12] = 'XcomOR 0.99i';
      DAT_XcomOR099i:array[1..80] of byte = (
        $8C, $C3, $66, $C1, $E3, $10, $5B, $83, $C3, $1C, $90, $6A, $00, $1F,
        $66, $87, $1E, $80, $00, $41, $EB, $01, $EA, $6A, $00, $C3, $D9, $FB,
        $66, $89, $1E, $80, $00, $81, $36, $0D, $00, $ED, $BE, $EB, $01, $EA,
        $83, $C4, $02, $0E, $1F, $8C, $C1, $68, $00, $B8, $07, $66, $BB, $10,
        $00, $00, $00, $EB, $01, $0F, $66, $26, $67, $FF, $33, $8E, $C1, $83,
        $06, $3E, $01, $6A, $90, $FC, $C7, $06, $00, $01
{ orig; DataLen:65
        $4D, $4D, $58, $66, $81, $36, $03, $01, $92, $EB, $04, $00, $EB, $F5,
        $D3, $5A, $81, $C2, $0E, $01, $B9, $87, $00, $EB, $01, $DF, $0F, $23,
        $F8, $0F, $23, $D8, $0F, $23, $F8, $0F, $23, $C8, $E2, $FB, $9C, $6A,
        $00, $0F, $A9, $16, $B4, $09, $17, $65, $FF, $1E, $84, $00, $83, $EC,
        $02, $9D, $EB, $01, $D9, $E8, $00, $00, $24,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
}
      );
      REC_XcomOR099i:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR_XcomOR099i;
        Data:@DAT_XcomOR099i
      );

      STR_XPack10jE:string [16] = 'XPack 1.0j [EXE]';
      DAT_XPack10jE:array[1..80] of byte = (
        $2E, $C7, $06, $04, $01, $00, $00, $8B, $EC, $EB, $01, $E8, $2E, $C6,
        $06, $FD, $03, $FA, $EB, $00, $9A, $BC, $0A, $04, $2E, $C6, $06, $09,
        $04, $C3, $EB, $00, $9A, $30, $04, $06, $09, $95, $19, $99, $26, $03,
        $62, $E3, $02, $62, $04, $7D, $04, $07, $05, $3D, $05, $E2, $00, $51,
        $05, $CF, $00, $39, $05, $AC, $00, $3C, $05, $AA, $00, $30, $05, $FC,
        $00, $33, $E4, $8E, $D4, $83, $C4, $10, $2E, $8F
      );
      REC_XPack10jE:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_XPack10jE;
        Data:@DAT_XPack10jE
      );

      STR_XPack10mE:string [23] = 'XPack 1.0m - 1.20 [EXE]';
      DAT_XPack10mE:array[1..80] of byte = (
          $2E, $C7, $06, $04, $01, $00, $00, $8B, $EC, $EB, $01, $E8, $2E, $C6,
          $06, $06, $04, $FA, $EB, $00, $9A, $BC, $13, $04, $2E, $C6, $06, $12,
          $04, $C3, $EB, $00, $9A, $39, $04, $06, $09, $95, $19, $99, $26, $03,
          $62, $EC, $02, $6B, $04, $86, $04, $10, $05, $46, $05, $E2, $00, $5A,
          $05, $CF, $00, $42, $05, $AC, $00, $45, $05, $AA, $00, $39, $05, $FC,
          $00, $33, $E4, $8E, $D4, $83, $C4, $10, $2E, $8F
        );
      REC_XPack10mE:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_XPack10mE;
        Data:@DAT_XPack10mE
      );


      STR_XPack129E:string [23] = 'XPack 1.29 - 1.30 [EXE]';
      DAT_XPack129E:array[1..80] of byte = (
          $2E, $C7, $06, $04, $01, $00, $00, $8B, $EC, $EB, $01, $E8, $2E, $C6,
          $06, $42, $04, $FA, $EB, $00, $9A, $BC, $4F, $04, $2E, $C6, $06, $4E,
          $04, $C3, $EB, $00, $9A, $75, $04, $06, $09, $95, $19, $99, $26, $03,
          $62, $28, $03, $A7, $04, $C2, $04, $4C, $05, $82, $05, $E2, $00, $96,
          $05, $CF, $00, $7E, $05, $AC, $00, $81, $05, $AA, $00, $75, $05, $FC,
          $00, $33, $E4, $8E, $D4, $83, $C4, $10, $2E, $8F
        );
      REC_XPack129E:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_XPack129E;
        Data:@DAT_XPack129E
      );

      STR_XPack131E:string [16] = 'XPack 1.31 [EXE]';
      DAT_XPack131E:array[1..80] of byte = (
          $2E, $C7, $06, $04, $01, $00, $00, $8B, $EC, $EB, $01, $E8, $2E, $C6,
          $06, $43, $04, $FA, $EB, $00, $9A, $BC, $50, $04, $2E, $C6, $06, $4F,
          $04, $C3, $EB, $00, $9A, $76, $04, $06, $09, $95, $19, $99, $26, $03,
          $62, $29, $03, $A8, $04, $C3, $04, $4E, $05, $84, $05, $E2, $00, $98,
          $05, $CF, $00, $80, $05, $AC, $00, $83, $05, $AA, $00, $77, $05, $FC,
          $00, $33, $E4, $8E, $D4, $83, $C4, $10, $2E, $8F
        );
      REC_XPack131E:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_XPack131E;
        Data:@DAT_XPack131E
      );

      STR_XPack133E:string [16] = 'XPack 1.33 [EXE]';
      DAT_XPack133E:array[1..80] of byte = (
          $2E, $C7, $06, $04, $01, $00, $00, $8B, $EC, $EB, $01, $E8, $2E, $C6,
          $06, $CD, $04, $FA, $EB, $00, $9A, $BC, $DA, $04, $2E, $C6, $06, $D9,
          $04, $C3, $EB, $00, $9A, $00, $05, $06, $09, $95, $19, $99, $26, $03,
          $62, $B3, $03, $32, $05, $4D, $05, $D8, $05, $0E, $06, $E2, $00, $22,
          $06, $CF, $00, $0A, $06, $AC, $00, $0D, $06, $AA, $00, $01, $06, $FC,
          $00, $33, $E4, $8E, $D4, $83, $C4, $10, $2E, $8F
        );
      REC_XPack133E:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_XPack133E;
        Data:@DAT_XPack133E
      );

      STR_XPack134E:string [16] = 'XPack 1.34 [EXE]';
      DAT_XPack134E:array[1..80] of byte = (
          $2E, $C7, $06, $04, $01, $00, $00, $8B, $EC, $EB, $01, $E8, $2E, $C6,
          $06, $FF, $04, $FA, $EB, $00, $9A, $BC, $0C, $05, $2E, $C6, $06, $0B,
          $05, $C3, $EB, $00, $9A, $32, $05, $06, $09, $95, $19, $99, $26, $03,
          $62, $E5, $03, $64, $05, $7F, $05, $0A, $06, $40, $06, $E2, $00, $54,
          $06, $CF, $00, $3C, $06, $AC, $00, $3F, $06, $AA, $00, $33, $06, $FC,
          $00, $33, $E4, $8E, $D4, $83, $C4, $10, $2E, $8F
        );
      REC_XPack134E:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_XPack134E;
        Data:@DAT_XPack134E
      );

{
 ############################################################
 ######################### LETTER Y #########################
 ############################################################
}

      STR_YifPress10:string [12] = 'YifPress 1.0';
      DAT_YifPress10:array[1..80] of byte = (
          $54, $B4, $09, $53, $CD, $21, $5B, $C3, $E8, $F4, $FF, $BA, $2C, $02,
          $E8, $EE, $FF, $B0, $02, $B4, $4C, $9C, $BA, $B3, $AB, $29, $00, $E2,
          $FF, $B4, $30, $95, $0A, $C0, $74, $D2, $BE, $4D, $02, $AD, $91, $AC,
          $32, $E4, $50, $51, $0B, $C8, $74, $E0, $AC, $91, $C0, $02, $56, $8B,
          $D6, $B4, $3C, $BB, $01, $00, $80, $F9, $FF, $74, $08, $B8, $18, $02,
          $72, $BF, $93, $1E, $07, $5F, $B9, $64, $00, $33
        )
{ starting at IP + 750 because of collision with Ady's Glue 1.10) };
      REC_YifPress10:TCompPtrRec = (
        DataLen:80;
        FileType:EXE;
        NameType:PACKER;
        Name:@STR_YifPress10;
        Data:@DAT_YifPress10
      );

      STR__Synopsis:string [19] = 'unknown by Synopsis';
      DAT__Synopsis:array[1..40] of byte = (
          $FA, $06, $57, $33, $FF, $57, $07, $8D, $76, $07, $B9, $0F, $00, $80,
          $04, $00, $46, $E2, $FA, $B1, $00, $AC, $26, $8A, $1D, $04, $00, $AA,
          $80, $EB, $00, $88, $5C, $FF, $E2, $F1, $5F, $FB, $07, $C3
        );
      REC__Synopsis:TCompPtrRec = (
        DataLen:40;
        FileType:COM;
        NameType:ENCRYPTER;
        Name:@STR__Synopsis;
        Data:@DAT__Synopsis
      );

{
.SYS file packer
}

      STR_SYS_SysPack01:string [11] = 'SysPack 0.1';
      DAT_SYS_SysPack01:array[1..80] of byte = (
          $FF, $FF, $FF, $FF, $00, $00, $4D, $00, $58, $00, $00, $00, $00, $00,
          $20, $20, $20, $20, $01, $00, $53, $59, $53, $50, $41, $43, $4B, $3A,
          $20, $43, $6F, $70, $79, $72, $69, $67, $68, $74, $20, $62, $79, $20,
          $56, $61, $64, $69, $6D, $20, $56, $2E, $20, $56, $6C, $61, $73, $6F,
          $76, $2C, $20, $4D, $6F, $73, $63, $6F, $77, $2C, $20, $52, $75, $73,
          $73, $69, $61, $2E, $20, $00, $00, $2E, $89, $1E
        );
      REC_SYS_SysPack01:TCompPtrRec = (
        DataLen:80;
        FileType:0;
        NameType:PACKER;
        Name:@STR_SYS_SysPack01;
        Data:@DAT_SYS_SysPack01
      );

      STR_SYS_UPX020_040:string [21] = 'UPX 0.20 - 0.40 [SYS]';
      DAT_SYS_UPX020_040:array[1..36] of byte = (
        $FF, $FF, $FF, $FF, $00, $00, $08, $00, $68, $00, $00, $60, $BE, $00,
        $00, $BF, $00, $00, $89, $F1, $06, $1E, $07, $FD, $F3, $A4, $FC, $F7,
        $E1, $93, $8B, $2F, $87, $F7, $83, $C6
      );
      REC_SYS_UPX020_040:TCompPtrRec = (
        DataLen:36;
        FileType:0;
        NameType:PACKER;
        Name:@STR_SYS_UPX020_040;
        Data:@DAT_SYS_UPX020_040
      );

      STR_SYS_UPX051:string [21] = 'UPX 0.51 - 0.72 [SYS]';
      DAT_SYS_UPX051:array[1..36] of byte = (
        $FF, $FF, $FF, $FF, $00, $00, $0A, $00, $00, $00, $60, $BE, $00, $00,
        $BF, $00, $00, $89, $F1, $06, $1E, $07, $FD, $F3, $A4, $FC, $F7, $E1,
        $93, $87, $F7, $83, $EE, $00, $19, $ED
      );
      REC_SYS_UPX051:TCompPtrRec = (
        DataLen:36;
        FileType:0;
        NameType:PACKER;
        Name:@STR_SYS_UPX051;
        Data:@DAT_SYS_UPX051
      );

      STR_SYS_UPX081:string [23] = 'UPX 0.81 - 1.02 [SYS]';
      DAT_SYS_UPX081:array[1..65] of byte = (
        $FF, $FF, $FF, $FF, $00, $00, $0A, $00, $00, $00, $60, $BE, $00, $00,
        $BF, $00, $00, $89, $F1, $06, $1E, $07, $FD, $F3, $A4, $FC, $BB, $00,
        $80, $87, $F7, $83, $EE, $C2, $19, $ED, $57, $E9, $00, $00, $55, $50,
        $58, $21, $0B, $02, $04, $07, $00, $00, $00, $00, $00, $00, $00, $00,
        $00, $00, $00, $00, $06, $00, $00, $9B, $FF
      );
      REC_SYS_UPX081:TCompPtrRec = (
        DataLen:36;
        FileType:0;
        NameType:PACKER;
        Name:@STR_SYS_UPX081;
        Data:@DAT_SYS_UPX081
      );

      STR_Com4Mail:string [12] = 'Com4Mail 1.0';
      DAT_Com4Mail:array[1..80] of byte = (
        $25, $30, $30, $25, $40, $40, $50, $5A, $2D, $4F, $6E, $2D, $4F, $6E,
        $50, $59, $2D, $4F, $21, $50, $5F, $52, $5E, $21, $35, $51, $58, $2D,
        $77, $50, $50, $5E, $31, $35, $51, $58, $2D, $62, $21, $50, $50, $5F,
        $52, $5E, $21, $35, $47, $47, $21, $35, $47, $47, $21, $35, $47, $47,
        $3D, $0D, $0A, $21, $35, $47, $47, $21, $35, $5F, $52, $58, $2D, $60,
        $4B, $35, $70, $50, $50, $5E, $31, $35, $47, $47
      );
      REC_Com4Mail:TCompPtrRec = (
        DataLen:80;
        FileType:COM;
        NameType:CONVERTER;
        Name:@STR_Com4Mail;
        Data:@DAT_Com4Mail
      );

      { EXE header of COM2EXE }

      { Misc data }

      STR_ANTIUPC_102:string [12] = 'AntiUPC 1.02';
      DAT_ANTIUPC_102:array [1..32] of byte = (
        $40, $2E, $89, $06, $00, $00, $2E, $89, $06, $02, $00, $8B,
        $2E, $02, $00, $8E, $DA, $A3, $00, $00, $8C, $06, $00, $00,
        $89, $1E, $00, $00, $89, $2E, $00, $00
      );
      REC_ANTIUPC_102:TCompPtrRec = (
        DataLen:32;
        FileType:EXE;
        NameType:PROTECTOR;
        Name:@STR_ANTIUPC_102;
        Data:@DAT_ANTIUPC_102
      );

{ EXE 2 COM converter }

      STR_DoP102:string[15] = 'E2C 1.02 - DoP';
      DAT_DoP102:array[1..80] of byte = (
        $BE, $4A, $01, $BF, $50, $FE, $B9, $32, $00, $FC, $57, $F3, $A5, $C3,
        $20, $FE, $20, $45, $32, $43, $20, $76, $31, $2E, $30, $32, $61, $20,
        $62, $79, $20, $54, $68, $65, $20, $44, $6F, $50, $20, $54, $69, $6D,
        $69, $73, $6F, $61, $72, $61, $20, $3C, $63, $62, $72, $65, $64, $69,
        $40, $6D, $62, $2E, $73, $6F, $72, $6F, $73, $74, $6D, $2E, $72, $6F,
        $3E, $20, $FE, $20, $BF, $50, $FE, $8C, $DB, $83
      );
      REC_DoP102:TCompEXECOMRec = (
        data:(
          DataLen:80;
          FileType:COM;
          NameType:CONVERTER;
          Name:@STR_DoP102;
          Data:@DAT_DoP102
        );
        StartPos:174
      );

      STR_EXETOCOM:string[8] = 'EXETOCOM';
      DAT_EXETOCOM:array[1..18] of byte = (
        $E9, $00, $00, $5F, $5F, $5F, $43, $6F, $6E, $76, $65, $72, $74, $65,
        $64, $20, $4D, $5A
      );
      REC_EXETOCOM:TCompEXECOMRec = (
        data:(
          DataLen:18;
          FileType:COM;
          NameType:CONVERTER;
          Name:@STR_EXETOCOM;
          Data:@DAT_EXETOCOM
        );
        StartPos:16
      );

      STR_XLoader200:string [12] = 'XLOADER 2.00';
      DAT_XLoader200:array[1..80] of byte = (
        $FC, $8C, $DB, $33, $C0, $8E, $C0, $B8, $61, $03, $26, $A3, $04, $00,
        $26, $8C, $0E, $06, $00, $50, $52, $FA, $BA, $C4, $03, $B0, $01, $EE,
        $42, $EC, $0C, $20, $EE, $FB, $5A, $58, $50, $B0, $AD, $E6, $64, $58,
        $B4, $30, $CD, $21, $3C, $02, $73, $05, $33, $C0, $06, $50, $CB, $BF,
        $A1, $53, $8B, $36, $02, $00, $2B, $F7, $81, $FE, $00, $10, $EB, $0A,
        $B4, $4A, $CD, $21, $BF, $78, $13, $B9, $80, $17
      );
      REC_XLoader200:TCompEXECOMRec = (
        data:(
          DataLen:80;
          FileType:COM;
          NameType:CONVERTER;
          Name:@STR_XLoader200;
          Data:@DAT_XLoader200
        );
        StartPos:624
      );


implementation

end.
