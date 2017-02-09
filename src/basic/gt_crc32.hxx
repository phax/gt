#ifndef _GT_CRC32_HXX_
#define _GT_CRC32_HXX_

namespace GT {

//--------------------------------------------------------------------
class GT_EXPORT_BASIC CRC32
//--------------------------------------------------------------------
{
private:
  static void _Init ();

public:
  static gtuint32 Get (void *pData, gtuint32 nDataLen);
  static gtuint32 Update (gtuint32 nStartCRC32, void *pData, gtuint32 nDataLen);
};

}  // namespace

#endif
