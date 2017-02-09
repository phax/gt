MAKE_ANALYZER (MPC);

/*! Binary only
 */
//--------------------------------------------------------------------
MAKE_IS_A (MPC)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  // compare first three bytes
  return (aGI.m_pBuffer->GetInt32 (0) & 0x00ffffff) == 0x002b504d;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (MPC)
//--------------------------------------------------------------------
{
  const size_t nVersion = m_pBuffer->GetInt32 (0) >> 24;

  out_format (rc (HRC_GENLIB_MPC_HEADER),
              nVersion & 15,
              nVersion >> 4);

  // #f# more to come

  // APE tag present?
  if (!m_bListMode && m_pBuffer->GetFileSize () > APETAG_F_SIZE)
  {
    APETAG_Footer aApeTag;
    m_pBuffer->GetBufferX (m_pBuffer->GetFileSize () - APETAG_F_SIZE, &aApeTag, APETAG_F_SIZE);
    if (memcmp (aApeTag.sID, "APETAGEX", 8) == 0)
    {
      // okay, it's an ape tag, so set overlay offset!
      // Comment in "mppdec-1.95e\id3tag.c" claims that the length is including the footer
      // but for "WUMPSCUT - Preferential Legacy\Music For A German Tribe (re-release)\01 - All Cried Out (1st -W- cover).mpc"
      // this is not true. The footer size must be added!
      nOverlayOffset = m_pBuffer->GetFileSize () - (APETAG_F_SIZE + aApeTag.nLength);
    }
  }
}
