MAKE_ANALYZER (Text);

/*! text only
 */
//--------------------------------------------------------------------
MAKE_IS_A (Text)
//--------------------------------------------------------------------
{
  if (!aGI.m_pBuffer->IsText ())
    return false;

  DirEntry aDE (aGI.m_pBuffer->GetFileName ());
  return aDE.HasFileNameExtension (_T ("txt")) ||
         aDE.HasFileNameExtension (_T ("dok"));
}

//--------------------------------------------------------------------
MAKE_DISPLAY (Text)
//--------------------------------------------------------------------
{
  out_format (rc (HRC_GENLIB_TEXT_HEADER),
              m_pBuffer->IsUnixText ()
               ? _T ("UNIX")
               : m_pBuffer->IsMacText ()
                 ? _T ("Mac")
                 : _T ("DOS"));
}
