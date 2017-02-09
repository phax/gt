MAKE_ANALYZER (TPU);

namespace TPU {

//--------------------------------------------------------------------
enum Version
//--------------------------------------------------------------------
{
  NONE = 0,
  TPU4,
  TPU5,
  TPU55,
  TPU6,
  TPU7,
  DELPHI1,
};

/*! Convert TPU identifier to enum value.
 */
//--------------------------------------------------------------------
static Version _SubTPU (const gtuint32 l0)
//--------------------------------------------------------------------
{
  if (l0 == 0x30555054) return TPU4;
  if (l0 == 0x35555054) return TPU5;
  if (l0 == 0x36555054) return TPU55;
  if (l0 == 0x39555054) return TPU6;
  if (l0 == 0x51555054) return TPU7;
  if (l0 == 0x31554344) return DELPHI1;
  return NONE;
}

}  // namespace

/*! Only binary!
 */
//--------------------------------------------------------------------
MAKE_IS_A (TPU)
//--------------------------------------------------------------------
{
  if (aGI.m_pBuffer->IsText ())
    return false;

  return TPU::_SubTPU (aGI.m_pBuffer->GetInt32 (0)) != TPU::NONE;
}

//--------------------------------------------------------------------
MAKE_DISPLAY (TPU)
//--------------------------------------------------------------------
{
  TPU::Version eType = TPU::_SubTPU (m_pBuffer->GetInt32 (0));

  out_format (rc (HRC_GENLIB_TPU_HEADER),
              eType == TPU::TPU4    ? _T ("4.0") :
              eType == TPU::TPU5    ? _T ("5.0") :
              eType == TPU::TPU55   ? _T ("5.5") :
              eType == TPU::TPU6    ? _T ("6.0") :
              eType == TPU::TPU7    ? _T ("7.0") :
              eType == TPU::DELPHI1 ? _T ("Delphi 1") : _T ("???"));
}
