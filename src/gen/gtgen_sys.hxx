MAKE_ANALYZER (SYS);

/*! Binary files only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (SYS)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  SYS_Header aHeader;
  if (!aGI.m_pBuffer->GetBuffer (0, &aHeader, SYS_H_SIZE))
    return false;

  return (aHeader.nAdrOfNextDriver == (gtint32) 0xFFFFFFFF &&
          aHeader.nStrategyOfs < aGI.m_pBuffer->GetFileSize ());
// caused problems with UPX packed EXE-SYS file:
//     && aHeader.nInterruptOfs < aGI.m_pBuffer->GetFileSize ());
}

//------------------------------------------------------------------
MAKE_DISPLAY (SYS)
//------------------------------------------------------------------
{
  out_append (rc (HRC_GENLIB_SYS_HEADER));

  if (!m_bListMode)
  {
    out_incindent ();

    SYS_Header aHeader;
    m_pBuffer->GetBuffer (0, &aHeader, SYS_H_SIZE);

    const bool bBlockDeviceDriver = (HI_BYTE (aHeader.nAttr) & 0x80);
    gtuint8 aBuffer[EXE_BUFFER_SIZE];

    if (bBlockDeviceDriver)
    {
      out_format (rc (HRC_GENLIB_SYS_BLOCK_DD),
                  int (aHeader.sName[0]));
    }
    else
    {
      TCHAR s8[9];
      memset (s8, 0, sizeof (s8));
      for (int i = 0; i < 8; i++)
        if (aHeader.sName[i] < 32)
          break;
        else
          s8[i] = aHeader.sName[i];
      out_format (rc (HRC_GENLIB_SYS_CHAR_DD),
                  s8);
    }

    // compare only if all bytes could be read!
    if (m_pBuffer->GetBuffer (0, aBuffer, EXE_BUFFER_SIZE))
    {
      SignatureCmp aCompare (aBuffer,
                             GT_COMPARE_MAGIC_SYS,
                             CmdlineParam_GetLong (GT_FLAG_FTS),
                             m_pBuffer,
                             NULL);
      ESignatureCmpErr cCode;

      // compare the header!!
      for (int i = 0; i < GT_SYS_Count (); i++)
      {
        const SYSEntry *pEntry = GT_SYS_Index (i);
        ASSERT (pEntry);

        cCode = aCompare.ExplicitMatches (pEntry->pData, pEntry->nDataLen);

        if (cCode != GTCOMP_NO_MATCH)
        {
          // print "found modifier bla bla bla"
          SignatureCmp::DisplayOutput (MODIFIER, cCode, pEntry->sName);
          if (cCode == GTCOMP_MATCH)
            break;
        }
      }
    }

    out_decindent ();
  }
}
