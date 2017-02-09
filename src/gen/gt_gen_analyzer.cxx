#include "gt_gen_analyzer.hxx"
#include "gt_gen_lister.hxx"
#include "gt_gen_structs.hxx"

#include <set>
#include <vector>

#include "gtc_data.hxx"
#include "gt_cmdline_params.hxx"
#include "gt_compare.hxx"
#include "gt_direntry.hxx"
#include "gt_exception.hxx"
#include "gt_filebuffer.hxx"
#include "gt_exe_pe.hxx"
#include "gt_output.hxx"
#include "gt_table.hxx"
#include "gt_utils.hxx"

namespace GT {

#include "gtgen_ape.hxx"
#include "gtgen_apetag.hxx"
#include "gtgen_basic.hxx"
#include "gtgen_bmf.hxx"
#include "gtgen_bmp.hxx"
#include "gtgen_ccdlng.hxx"
#include "gtgen_chm.hxx"
#include "gtgen_class.hxx"
#include "gtgen_crypt.hxx"
#include "gtgen_cue.hxx"
#include "gtgen_dbf.hxx"
#include "gtgen_elf.hxx"
#include "gtgen_fli.hxx"
#include "gtgen_fpk.hxx"
#include "gtgen_gbx.hxx"
#include "gtgen_gif.hxx"
#include "gtgen_grp.hxx"
#include "gtgen_headtext.hxx"
#include "gtgen_hlp.hxx"
#include "gtgen_html.hxx"
#include "gtgen_ico.hxx"
#include "gtgen_id3.hxx"
#include "gtgen_iff.hxx"
#include "gtgen_ini.hxx"
#include "gtgen_it.hxx"              // Impulse Tracker
#include "gtgen_jpg.hxx"
#include "gtgen_lib.hxx"
#include "gtgen_m4a.hxx"
#include "gtgen_mac.hxx"
#include "gtgen_map.hxx"
#include "gtgen_midi.hxx"
#include "gtgen_mp3.hxx"
#include "gtgen_mp3tag.hxx"
#include "gtgen_mpc.hxx"
#include "gtgen_mpeg.hxx"
#include "gtgen_obj.hxx"
#include "gtgen_pcx.hxx"
#include "gtgen_pgp.hxx"
#include "gtgen_php.hxx"
#include "gtgen_pif.hxx"
#include "gtgen_pkg.hxx"
#include "gtgen_pmwlite.hxx"
#include "gtgen_png.hxx"
#include "gtgen_poifs.hxx"
#include "gtgen_psd.hxx"
#include "gtgen_res.hxx"
#include "gtgen_riff.hxx"
#include "gtgen_shell.hxx"
#include "gtgen_source.hxx"
#include "gtgen_sys.hxx"
#include "gtgen_text.hxx"
#include "gtgen_tiff.hxx"
#include "gtgen_tpu.hxx"
#include "gtgen_ttf.hxx"
#include "gtgen_tws.hxx"
#include "gtgen_unicode.hxx"
#include "gtgen_voc.hxx"
#include "gtgen_wmf.hxx"
#include "gtgen_wpc.hxx"
#include "gtgen_xml.hxx"
#include "gtgen_xpt.hxx"

#define IS_A(__N)  if (__N##_Analyzer::IsA (aGI)) { \
                     return new __N##_Analyzer (aGI); \
                   }

//--------------------------------------------------------------------
static GenericLister* __GetGenericLister (const GenericInput& aGI)
//--------------------------------------------------------------------
{
  ASSERT (aGI.m_pBuffer);

  // file with less than 16 bytes are not analyzed!
  if (aGI.m_pBuffer->GetFileSize () < 16)
    return NULL;

  IS_A (APE);
  IS_A (APETAG);
  IS_A (BMF);
  IS_A (BMP);
  IS_A (CCDLNG);
  IS_A (CHM);
  IS_A (CLASS);
  IS_A (Crypt);
  IS_A (CUE);
  IS_A (DBF);
  IS_A (ELF);
  IS_A (FLI);
  IS_A (FPK);
  IS_A (GBX);
  IS_A (GIF);
  IS_A (GRP);
  IS_A (HLP);
  IS_A (Source);  // Note: Source before HTML!
  IS_A (PHP);     // Note: PHP before HTML!
  IS_A (HTML);
  IS_A (ICO);
  IS_A (ID3);
  IS_A (IFF);
  IS_A (ImpulseTracker);
  IS_A (INI);
  IS_A (JPG);
  IS_A (LIB);
  IS_A (M4A);
  IS_A (MAC);
  IS_A (MAP);
  IS_A (XML);          // Note: before UnicodeText
  IS_A (UnicodeText);  // Note: before MP3!
  IS_A (MP3);
  IS_A (MP3TAG);
  IS_A (MPC);
  IS_A (MPEG);
  IS_A (OBJ);
  IS_A (PCX);
  IS_A (PGP);
  IS_A (PIF);
  IS_A (PKG);
  IS_A (PMWLite);
  IS_A (PNG);
  IS_A (POIFS);
  IS_A (PSD);
  IS_A (RES);
  IS_A (RIFF);
  IS_A (Shell);
  IS_A (SYS);
  IS_A (TIFF);
  IS_A (TPU);
  IS_A (TTF);
  IS_A (TWS);
  IS_A (VOC);
  IS_A (WMF);
  IS_A (WPC);
  IS_A (XPT);

  // basic identification after all (why? just an idea :)
  IS_A (Basic);
  IS_A (Text);

  // headtext as very last because it is just a heuristic!
  IS_A (HeadText);

  // unknown :(
  return NULL;
}

//--------------------------------------------------------------------
GenericAnalyzer::GenericAnalyzer (const GenericInput &aGI)
//--------------------------------------------------------------------
  : BasicAnalyzer (aGI.m_pBuffer)
{
  m_pLister = __GetGenericLister (aGI);
}

//--------------------------------------------------------------------
GenericAnalyzer::~GenericAnalyzer ()
//--------------------------------------------------------------------
{
  delete m_pLister;
}

//--------------------------------------------------------------------
void GT_CALL GenericAnalyzer::_ShowResults ()
//--------------------------------------------------------------------
{
  try
  {
    // nOverlayOffset is a out param
    m_pLister->Display (m_nOverlayOffset);
  }
  catch (const Excpt& e)
  {
    // show error
    e.Display ();
  }
}

}  // Namespace GT
