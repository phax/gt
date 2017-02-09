#include "gt_crc32.hxx"

namespace GT {

static const gtuint32 GT_MAXCRC32_ENTRIES = 256;

static bool     g_bInited = false;
static gtuint32 g_aTable[GT_MAXCRC32_ENTRIES];

//--------------------------------------------------------------------
void CRC32::_Init ()
//--------------------------------------------------------------------
{
  // init the complete CRC32 table only once
  if (!g_bInited)
  {
    for (gtuint32 i = 0; i < GT_MAXCRC32_ENTRIES; ++i)
    {
      gtuint32 nCRC = i;
      for (int n = 0; n < 8; ++n)
        nCRC = (nCRC & 1)
                 ? 0xEDB88320L ^ (nCRC >> 1)
                 : nCRC >> 1;
      g_aTable[i] = nCRC;
    }
    g_bInited = true;
  }
}

//--------------------------------------------------------------------
gtuint32 CRC32::Get (void *pData, gtuint32 nDataLen)
//--------------------------------------------------------------------
{
  return CRC32::Update (0xFFFFFFFF, pData, nDataLen);
}

//--------------------------------------------------------------------
gtuint32 CRC32::Update (gtuint32 nStartCRC32, void *pData, gtuint32 nDataLen)
//--------------------------------------------------------------------
{
  _Init ();

  gtuint8 *src = (gtuint8*) pData;
  gtuint8 *end = (gtuint8*) pData + nDataLen;
  gtuint32 nCRC32 = nStartCRC32;
  for (; src < end; src++)
    nCRC32 = g_aTable [(nCRC32 ^ *src) & 0xFF] ^ (nCRC32 >> 8);
  return nCRC32;
}

}  // namespace
