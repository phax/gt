MAKE_ANALYZER (ELF);

/*! Binary only.
 */
//--------------------------------------------------------------------
MAKE_IS_A (ELF)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return aGI.m_pBuffer->GetInt32 (0) == 0x464C457F;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (ELF)
//--------------------------------------------------------------------
{
  // read header
  ELF_Header aH;
  m_pBuffer->GetBufferX (0, &aH, ELF_H_SIZE);

  // write header
  out_append (rc (HRC_GENLIB_ELF_UNIX));

  // display target system of file
  switch (aH.nClass)
  {
    case 0x01: out_append (rc (HRC_GENLIB_ELF_32_BIT)); break;
    case 0x02: out_append (rc (HRC_GENLIB_ELF_64_BIT)); break;
  }

  // display type of file
  switch (aH.nType)
  {
    case 0x0000: out_append (rc (HRC_GENLIB_ELF_INVALID)); break;
    case 0x0001: out_append (rc (HRC_GENLIB_ELF_RELOCATABLE)); break;
    case 0x0002: out_append (rc (HRC_GENLIB_ELF_EXECUTABLE)); break;
    case 0x0003: out_append (rc (HRC_GENLIB_ELF_SHARED_OBJECT)); break;
    case 0x0004: out_append (rc (HRC_GENLIB_ELF_CORE)); break;
    default:
    {
      if (aH.nType >= 0xFF00 /* && aH.nType <= 0xFFFF */)
        out_append (rc (HRC_GENLIB_ELF_PROC_SPECIFIC));
      else
        out_append (rc (HRC_GENLIB_ELF_UNKNOWN));
      break;
    }
  }
  out_append (rc (HRC_GENLIB_ELF_FILE));

  if (!m_bListMode)
  {
    out_incindent ();
    out_append (rc (HRC_GENLIB_ELF_DATA_ENCODING));

    switch (aH.nDataEncoding)
    {
      case 0x00: out_append (rc (HRC_GENLIB_ELF_DE_INVALID)); break;
      case 0x01: out_append (rc (HRC_GENLIB_ELF_DE_LITTLE_ENDIAN)); break;
      case 0x02: out_append (rc (HRC_GENLIB_ELF_DE_BIG_ENDIAN)); break;
      default:   out_append (rc (HRC_GENLIB_ELF_DE_UNKNOWN)); break;
    }

    out_append (rc ("\n"));
    out_append (rc (HRC_GENLIB_ELF_MACHINE_TYPE));

    switch (aH.nMachine)
    {
      case 0x0000: out_append (rc (HRC_GENLIB_ELF_MT_NO_MACHINE)); break;
      case 0x0001: out_append (rc (HRC_GENLIB_ELF_MT_ATT)); break;
      case 0x0002: out_append (rc (HRC_GENLIB_ELF_MT_SPARC)); break;
      case 0x0003: out_append (rc (HRC_GENLIB_ELF_MT_INTEL_80386)); break;
      case 0x0004: out_append (rc (HRC_GENLIB_ELF_MT_MOTOROLA_68000)); break;
      case 0x0005: out_append (rc (HRC_GENLIB_ELF_MT_MOTOROLA_88000)); break;
      /* no 0x0006! */
      case 0x0007: out_append (rc (HRC_GENLIB_ELF_MT_INTEL_80860)); break;
      case 0x0008: out_append (rc (HRC_GENLIB_ELF_MT_MIPS_RS32000)); break;
    }
    out_append (rc ("\n"));

    // entry point present?
    if (aH.nEntryRVA == 0)
    {
      // info only on executable images
      if (aH.nType == 0x0002)
        out_info_append (rc (HRC_GENLIB_ELF_NO_EP));
    }

    out_decindent ();
  }
}
