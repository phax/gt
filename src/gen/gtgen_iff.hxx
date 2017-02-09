MAKE_ANALYZER (IFF);

namespace IFF {

/*
procedure AnalyzeIFF;
type
     IFFHeader = record
       ID:array[1..4] of char;
       Size:longint;
     end;

     IFF_BMHD = record
       Width:word;
       Height:word;
       Left:word;
       Top:word;
       BitPlanes:byte;
       Masking:byte;
       Compress:byte;
       Padding:byte;
       Transparency:word;
       XAspectRatio:byte;
       YAspectRatio:byte;
       PageWidth:word;
       PageHeight:word;
     end;
const
      IFF_LH_SIZE = SizeOf (IFFHeader);
      IFF_BMHD_SIZE = SizeOf (IFF_BMHD);
var H:IFFHeader;
    nSize, p:longint;

    procedure BMHD;
    var h:IFF_BMHD;
    begin
      if (TFile_ReadActBuf (h, IFF_BMHD_SIZE) <> IFF_BMHD_SIZE) then;

      Append (concat (i2s (swap (h.Width)), 'x', i2s (swap (h.Height)),
                       STR_SEPERATOR, i2s (Exp2 (h.BitPlanes)), ' colors'));
      if (h.Compress > 0) then
        Append (concat (STR_SEPERATOR, ' compressed'));
      FinishLine;
    end;

    procedure CMAP;
    var nSize:longint;
    begin
      SwapLong (h.Size, nSize);
      Appendln (concat ('Color table with ', i2s (nSize div 3), ' entries'));
    end;

begin
  TFile_Init (0);
{ read global header }
  if (TFile_ReadActBuf (H, IFF_LH_SIZE) <> IFF_LH_SIZE) then;
  TFIle_IncFilePos (4);
  NoteFormat;

  IncInsLevel;
  p := IFF_LH_SIZE + 4;
  repeat
    if (TFile_ReadActBuf (H, IFF_LH_SIZE) = IFF_LH_SIZE) then
    begin
      NoteFormat;
      SwapLong (h.Size, nSize);
      inc (p, IFF_LH_SIZE + nSize);
      TFile_GotoFilePos (p);
    end;
  until (not TFile_IsOkay);
  DecInsLevel;
end;
*/

//--------------------------------------------------------------------
static LPCTSTR _GetFieldName (const gtuint32 nID)
//--------------------------------------------------------------------
{
  LPTSTR pReal = NULL;
  switch (nID)
  {
    // textual data
    /*(C) */ case 0x20294328: GetStringResBuf (HRC_GENLIB_IFF__C_,  &pReal); break;
    /*ANNO*/ case 0x4F4E4E41: GetStringResBuf (HRC_GENLIB_IFF_ANNO, &pReal); break;
    /*DOC */ case 0x20434F44: GetStringResBuf (HRC_GENLIB_IFF_DOC,  &pReal); break;
    /*FOOT*/ case 0x544F4F46: GetStringResBuf (HRC_GENLIB_IFF_FOOT, &pReal); break;
    /*HEAD*/ case 0x44414548: GetStringResBuf (HRC_GENLIB_IFF_HEAD, &pReal); break;
    /*PAGE*/ case 0x45474150: GetStringResBuf (HRC_GENLIB_IFF_PAGE, &pReal); break;
    /*PARA*/ case 0x41524150: GetStringResBuf (HRC_GENLIB_IFF_PARA, &pReal); break;
    /*PDEF*/ case 0x46454450: GetStringResBuf (HRC_GENLIB_IFF_PDEF, &pReal); break;
    /*TABS*/ case 0x53424154: GetStringResBuf (HRC_GENLIB_IFF_TABS, &pReal); break;
    /*TEXT*/ case 0x54584554: GetStringResBuf (HRC_GENLIB_IFF_TEXT, &pReal); break;
    /*VERS*/ case 0x53524556: GetStringResBuf (HRC_GENLIB_IFF_VERS, &pReal); break;
    /*FTXT*/ case 0x54585446: GetStringResBuf (HRC_GENLIB_IFF_FTXT, &pReal); break;
    /*WORD*/ case 0x44524F57: GetStringResBuf (HRC_GENLIB_IFF_WORD, &pReal); break;
    // graphical data
    /*ACBM*/ case 0x4143424D: GetStringResBuf (HRC_GENLIB_IFF_ACBM, &pReal); break;
    /*DEEP*/ case 0x50454544: GetStringResBuf (HRC_GENLIB_IFF_DEEP, &pReal); break;
    /*DR2D*/ case 0x44325244: GetStringResBuf (HRC_GENLIB_IFF_DR2D, &pReal); break;
    /*FNTR*/ case 0x52544E46: GetStringResBuf (HRC_GENLIB_IFF_FNTR, &pReal); break;
    /*FNTV*/ case 0x56544E46: GetStringResBuf (HRC_GENLIB_IFF_FNTV, &pReal); break;
    /*ILBM*/ case 0x4D424C49: GetStringResBuf (HRC_GENLIB_IFF_ILBM, &pReal); break;
    /*PBM */ case 0x204D4250: GetStringResBuf (HRC_GENLIB_IFF_PBM,  &pReal); break;
    /*PICS*/ case 0x53434950: GetStringResBuf (HRC_GENLIB_IFF_PICS, &pReal); break;
    /*RGB8*/ case 0x38424752: GetStringResBuf (HRC_GENLIB_IFF_RGB8, &pReal); break;
    /*RGBN*/ case 0x4E424752: GetStringResBuf (HRC_GENLIB_IFF_RGBN, &pReal); break;
    /*TDDD*/ case 0x44444454: GetStringResBuf (HRC_GENLIB_IFF_TDDD, &pReal); break;
    /*YUVN*/ case 0x4E565559: GetStringResBuf (HRC_GENLIB_IFF_YUVN, &pReal); break;
    // animation data
    /*ANBM*/ case 0x4D424E41: GetStringResBuf (HRC_GENLIB_IFF_ANBM, &pReal); break;
    /*ANIM*/ case 0x4D494E41: GetStringResBuf (HRC_GENLIB_IFF_ANIM, &pReal); break;
    /*SSA */ case 0x20415353: GetStringResBuf (HRC_GENLIB_IFF_SSA,  &pReal); break;
    // video data
    /*VDEO*/ case 0x4F454456: GetStringResBuf (HRC_GENLIB_IFF_VDEO, &pReal); break;
    // sound data
    /*8SVX*/ case 0x58565338: GetStringResBuf (HRC_GENLIB_IFF_8SVX, &pReal); break;
    /*AIFF*/ case 0x46464941: GetStringResBuf (HRC_GENLIB_IFF_AIFF, &pReal); break;
    /*SAMP*/ case 0x504D4153: GetStringResBuf (HRC_GENLIB_IFF_SAMP, &pReal); break;
    /*UVOX*/ case 0x584F5655: GetStringResBuf (HRC_GENLIB_IFF_UVOX, &pReal); break;
    // music data
    /*GSCR*/ case 0x52435347: GetStringResBuf (HRC_GENLIB_IFF_GSCR, &pReal); break;
    /*SMUS*/ case 0x53554D53: GetStringResBuf (HRC_GENLIB_IFF_SMUS, &pReal); break;
    /*TRAK*/ case 0x4B415254: GetStringResBuf (HRC_GENLIB_IFF_TRAK, &pReal); break;
    /*USCR*/ case 0x52435355: GetStringResBuf (HRC_GENLIB_IFF_USCR, &pReal); break;
    // rest
    /*BODY*/ case 0x59444F42: GetStringResBuf (HRC_GENLIB_IFF_BODY, &pReal); break;
#if 0
    /*BMHD*/ case 0x44484D42: pReal = _T ("]BMHD"); break;
    /*CMAP*/ case 0x50414D43: pReal = _T ("]CMAP"); break;
    /*FORM*/ case 0x4D524F46: pReal = _T ("]FORM"); break;
    /*CRNG*/ case 0x474E5243: pReal = _T ("]CRNG"); break;
    /*DPPS*/ case 0x53505044: pReal = _T ("]DPPS"); break;
#endif
    default:
    {
      static TCHAR x[8];
      char pID[4];
      *((gtuint32*) pID) = nID;
      _stprintf (x, _T ("[%c%c%c%c]"),
                    pID[0] ? pID[0] : ' ',
                    pID[1] ? pID[1] : ' ',
                    pID[2] ? pID[2] : ' ',
                    pID[3] ? pID[3] : ' ');
      pReal = x;
    }
  }

  return pReal;
}

}  // namespace IFF

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (IFF)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x4D524F46;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (IFF)
//--------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_IFF_HEADER));

  if (!m_bListMode)
  {
    out_incindent ();

    IFF_Header aH;
    m_pBuffer->GetBufferX (0, &aH, IFF_H_SIZE);

    LPCTSTR pSub = IFF::_GetFieldName (aH.nID);
    if (pSub)
      out_append (pSub);
    else
    {
      out_format (rc (HRC_GENLIB_IFF_UNK_TYPE),
                  aH.nID);
    }
    out_append (_T ("\n"));

    m_pBuffer->SetActFilePos (IFF_H_SIZE + 4);

    out_incindent ();
    try
    {
      for (;;)
      {
        if (!m_pBuffer->GetBuffer (&aH, IFF_H_SIZE))
          break;

        aH.nSize = LONG_SWAP_BYTE (aH.nSize);
        pSub = IFF::_GetFieldName (aH.nID);

        if (pSub)
          out_append (pSub);
        else
        {
          out_format (rc (HRC_GENLIB_IFF_UNK_SUBTYPE),
                      aH.nID);
        }

        // append size
        out_format (rc (HRC_GENLIB_IFF_BYTES), aH.nSize);

        // align to WORD boundary!!
        m_pBuffer->IncActFilePos (ALIGN_2 (aH.nSize));
      }
    }
    catch (const Excpt&)
    {
      // cleanup
      out_decindent ();
      out_decindent ();
      throw;
    }

    out_decindent ();
    out_decindent ();
  }
}
