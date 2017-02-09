MAKE_ANALYZER (POIFS);

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (POIFS)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetFileSize () > 512 &&
         aGI.m_pBuffer->GetInt32 (0) == gtint32 (0xE011CFD0) &&
         aGI.m_pBuffer->GetInt32 (4) == gtint32 (0xE11AB1A1);
}

static pstring sTempName;

typedef std::set <gtuint32> PropertIndexSet;

//--------------------------------------------------------------------
void TraversePropertyTree (      FileBuffer*       pBuffer,
                           const file_t            nPropertyStartPos,
                           const gtuint32          nPropertyIndex,
                                 PropertIndexSet&  aHandledProperties)
//--------------------------------------------------------------------
{
  POIFS_Property aP;

  if (!pBuffer->GetBuffer (nPropertyStartPos + (nPropertyIndex * POIFS_P_SIZE), &aP, POIFS_P_SIZE))
  {
    out_error_format (_T ("Failed to read property at index %u\n"), nPropertyIndex);
    return;
  }

  // invalid entry (Note: sName[0] can be != 0xffff whereas nNameSize == 0xffff)
  // Sometypes just the nodee color is wrong
  // "trial and error"
  if (aP.sName [0] == 0xffff)
  {
    out_error_append (_T ("aP.sName is invalid\n"));
    return;
  }
  if (aP.nNameSize == 0xffff)
  {
    out_error_append (_T ("aP.nNameSize is invalid\n"));
    return;
  }
  if (aP.nNodeColor != 0 && aP.nNodeColor != 1)
  {
    out_error_append (_T ("aP.nNodeColor is invalid\n"));
    return;
  }
  if (aP.nNameSize > 0x40)
  {
    out_error_format (_T ("aP.nNameSize is > 0x40 (0x%04X)\n"), aP.nNameSize);
    return;
  }
  if (!aHandledProperties.insert (nPropertyIndex).second)
  {
    out_error_format (_T ("Error: the property [%d] was already handled\n"), nPropertyIndex);
    return;
  }

  // prev node on same level?
  if (aP.nPrevPropIndex != gtuint32 (-1))
  {
    TraversePropertyTree (pBuffer,
                          nPropertyStartPos,
                          aP.nPrevPropIndex,
                          aHandledProperties);
  }

  {
    // name is always stored as Unicode!
    // use global var to avoid stack clash
    str_assign (sTempName, aP.sName, aP.nNameSize);

    out_format (_T ("[%d] "), nPropertyIndex);

    if (aP.nPropertyType == 1)
      out_append (_T ("Directory"));
    else
    if (aP.nPropertyType == 2)
      out_append (_T ("File"));
    else
    if (aP.nPropertyType == 5)
      out_append (_T ("Root"));
    else
      out_append (_T ("[unknown]"));

    out_format (_T (" '%s'"), sTempName.c_str ());

    out_format (_T (" [%d %d] -> %d @BAT[%d]"),
                aP.nPrevPropIndex,
                aP.nNextPropIndex,
                aP.nFirstChildPropIndex,
                aP.nStartBATIndex);

    out_append (_T ("\n"));
  }

  // next node on same level?
  if (aP.nNextPropIndex != gtuint32 (-1))
  {
    TraversePropertyTree (pBuffer,
                          nPropertyStartPos,
                          aP.nNextPropIndex,
                          aHandledProperties);
  }

  // child node
  if (aP.nFirstChildPropIndex != gtuint32 (-1))
  {
    out_append (_T ("{\n"));
    out_incindent ();
    TraversePropertyTree (pBuffer,
                          nPropertyStartPos,
                          aP.nFirstChildPropIndex,
                          aHandledProperties);
    out_decindent ();
    out_append (_T ("}\n"));
  }
}

//--------------------------------------------------------------------
MAKE_DISPLAY (POIFS)
//--------------------------------------------------------------------
{
  POIFS_Header aH;

  out_append (rc (HRC_GENLIB_POIFS_HEADER));

  // read header (512 bytes)
  m_pBuffer->GetBufferX (0, &aH, POIFS_H_SIZE);

  if (!m_bListMode)
  {
    out_incindent ();

    out_info_append (_T ("This is experimental output - work in progress!\n"));
    out_format (_T ("Big block size: %u\n"),   1 << aH.nLog2BigBlockSize);
    out_format (_T ("Small block size: %u\n"), 1 << aH.nLog2SmallBlockSize);
    out_format (_T ("Property start: %u\n"), aH.nPropStart);
    out_format (_T ("BAT  count: %u\n"), aH.nBATCount);
    out_format (_T ("SBAT count: %u\n"), aH.nSBATCount);
    out_format (_T ("XBAT count: %u\n"), aH.nXBATCount);

    // goto property table
    out_incindent ();
    PropertIndexSet aHandledProperties;
    TraversePropertyTree (m_pBuffer,
                          POIFS_H_SIZE + (aH.nPropStart * (1 << aH.nLog2BigBlockSize)),
                          0,
                          aHandledProperties);
    out_decindent ();

    out_decindent ();
  }
}
