/*
 ISO DirEntry: nFileFlags:

*/

//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (ISO)
//--------------------------------------------------------------------
{
  // read primary volume descriptor
  ISO_PrimaryVolumeDesc aPVD;
  m_pBuffer->GetBuffer (m_nLocalOffset + 0x8000, &aPVD, GT_ISO_PVD_SIZE);

  // listing is open
  m_pBuffer->SetToEOF ();
}
