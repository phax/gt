#ifndef _GT_GEN_LISTER_HXX_
#define _GT_GEN_LISTER_HXX_

namespace GT {

//--------------------------------------------------------------------
class GenericLister
//--------------------------------------------------------------------
{
private:
  GenericLister ();
  GenericLister (const GenericLister&);
  GenericLister& operator = (const GenericLister&);

protected:
  FileBuffer* m_pBuffer;
  bool        m_bListMode;

public:
  //------------------------------------------------------------------
  explicit GenericLister (const GenericInput& aGI)
  //------------------------------------------------------------------
    : m_pBuffer   (aGI.m_pBuffer),
      m_bListMode (aGI.m_bListMode)
  {}

  //! pure virtual method
  virtual void Display (file_t& nOverlayOffset) = 0;
};

// create a new analyzer
#define MAKE_ANALYZER(__N) class __N##_Analyzer : public GenericLister { \
                           public: \
                             __N##_Analyzer (const GenericInput& aGI) \
                               : GenericLister (aGI) {} \
                             static bool IsA (const GenericInput& aGI); \
                             virtual void Display (file_t& nOverlayOffset); \
                           }

#define MAKE_IS_A(__N)       bool __N##_Analyzer::IsA (const GenericInput& aGI)
#define MAKE_DISPLAY(__N)    void __N##_Analyzer::Display (file_t& nOverlayOffset)

}  // namespace

#endif
