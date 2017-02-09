#ifndef _GT_TABLE_HXX_
#define _GT_TABLE_HXX_

#include "gt_stringhelper.hxx"
#include "gt_utils.hxx"

#include <vector>

namespace GT {

const int NO_ALIGN     = -1;
const int ALIGN_LEFT   = -2;
const int ALIGN_CENTER = -3;
const int ALIGN_RIGHT  = -4;

const size_t TABLE_AUTO_WIDTH = 0;

const int DEFAULT_FORMATWIDTH_DOUBLE = 6;

// all string alígnments must be < NO_ALIGN!
GT_COMPILER_ASSERT (ALIGN_LEFT   < NO_ALIGN);
GT_COMPILER_ASSERT (ALIGN_CENTER < NO_ALIGN);
GT_COMPILER_ASSERT (ALIGN_RIGHT  < NO_ALIGN);

/*! This enum represents the type of a table column.
 */
//--------------------------------------------------------------------
enum TableColumnType
//--------------------------------------------------------------------
{
  TABLE_INVALID = 99,
  NUM_ZERO,
  NUM_SPACE,
  NUM_DOUBLE,
  HEX,
  STR,
};

/*! This enum contains all possibe cell types.
 */
//--------------------------------------------------------------------
enum CellType
//--------------------------------------------------------------------
{
  CELL_NONE,
  CELL_STRING,
  CELL_INT32,
  CELL_INT64,
  CELL_DOUBLE,
};

//--------------------------------------------------------------------
class TableCell
//--------------------------------------------------------------------
{
private:
  CellType m_eType;    //!< type
  size_t   m_nColSpan; //!< col span
  EColor   m_eColor;   //!< color
  union
  {
    pstring* pText;    //!< content as string
    gtuint32 n32;      //!< content as uint32
    gtint64  n64;      //!< content as int64
    double   dbl;      //!< content as double
  };

  TableCell (const TableCell&);  //!< declared only

public:
  TableCell ()
    : m_eType    (CELL_NONE),
      m_nColSpan (1),
      m_eColor   (GT_COLOR_INVALID)
  {}

  explicit TableCell (const pstring& val)
    : m_eType    (CELL_STRING),
      m_nColSpan (1),
      m_eColor   (GT_COLOR_INVALID),
      pText      (new pstring (val))
  {}

  explicit TableCell (const gtuint32 val)
    : m_eType    (CELL_INT32),
      m_nColSpan (1),
      m_eColor   (GT_COLOR_INVALID),
      n32        (val)
  {}

  explicit TableCell (const gtint64 val)
    : m_eType    (CELL_INT64),
      m_nColSpan (1),
      m_eColor   (GT_COLOR_INVALID),
      n64        (val)
  {}

  explicit TableCell (const double val)
    : m_eType    (CELL_DOUBLE),
      m_nColSpan (1),
      m_eColor   (GT_COLOR_INVALID),
      dbl        (val)
  {}

  ~TableCell ()
  {
    if (m_eType == CELL_STRING)
      delete pText;
  }

  //! get type of cell
  CellType GetType () const
  {
    return m_eType;
  }

  //! get column span
  size_t GetColSpan () const
  {
    return m_nColSpan;
  }

  //! set column span
  void SetColSpan (const size_t nColSpan)
  {
    m_nColSpan = nColSpan;
  }

  //! get color
  EColor GetColor () const
  {
    return m_eColor;
  }

  //! set color
  void SetColor (const EColor eColor)
  {
    m_eColor = eColor;
  }

  //! checker
  bool HasCustomColor () const
  {
    return (m_eColor != GT_COLOR_INVALID);
  }

  //! set content as string
  void SetStr (const pstring& s)
  {
    ASSERT (m_eType == CELL_NONE);
    m_eType = CELL_STRING;
    pText = new pstring (s);
  }

  //! get content as string
  pstring* GetStr () const
  {
    ASSERT (m_eType == CELL_STRING);
    return pText;
  }

  //! set content as uint32
  void SetInt32 (gtuint32 n)
  {
    ASSERT (m_eType == CELL_NONE);
    m_eType = CELL_INT32;
    n32 = n;
  }

  //! get content as uint32
  gtuint32 GetInt32 () const
  {
    ASSERT (m_eType == CELL_INT32);
    return n32;
  }

  //! set content as int64
  void SetInt64 (gtint64 n)
  {
    ASSERT (m_eType == CELL_NONE);
    m_eType = CELL_INT64;
    n64 = n;
  }

  //! get content as int64
  gtint64 GetInt64 () const
  {
    ASSERT (m_eType == CELL_INT64);
    return n64;
  }

  //! set content as double
  void SetDouble (double d)
  {
    ASSERT (m_eType == CELL_NONE);
    m_eType = CELL_DOUBLE;
    dbl = d;
  }

  //! get content as double
  double GetDouble () const
  {
    ASSERT (m_eType == CELL_DOUBLE);
    return dbl;
  }

  //! get the width
  size_t GetWidth () const
  {
    return m_eType == CELL_STRING ? pText->length () : TABLE_AUTO_WIDTH;
  }
};

//--------------------------------------------------------------------
class TableCellVector : public std::vector<TableCell*>
//--------------------------------------------------------------------
{
public:
  //------------------------------------------------------------------
  virtual ~TableCellVector ()
  //------------------------------------------------------------------
  {
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
      delete *cit;
  }
};

//--------------------------------------------------------------------
class TableColumn
//--------------------------------------------------------------------
{
private:
  size_t          m_nWidth;
  int             m_nFormatWidth;  //!< determine between string and int
  TableColumnType m_eType;
  pstring         m_sName;
  TableCellVector m_aCells;

  friend class Table;

public:
  //------------------------------------------------------------------
  TableColumn ()
  //------------------------------------------------------------------
    : m_nWidth       (TABLE_AUTO_WIDTH),
      m_nFormatWidth (0),
      m_eType        (TABLE_INVALID)
  {}

  //------------------------------------------------------------------
  void Init (      LPCTSTR         sName,
             const TableColumnType eType,
             const size_t          nWidth,
             const int             nFormatWidth)
  //------------------------------------------------------------------
  {
    ASSERT (nWidth < 128);
    ASSERT (m_eType == TABLE_INVALID);
    ASSERT (m_aCells.empty ());

    // auto resolve res_id string
    str_auto_resolve_resid (sName);

    // ensure that the width of the table is at least as big as the name is long
    m_nWidth = nWidth != TABLE_AUTO_WIDTH
                 ? _max (nWidth, _tcslen (sName))
                 : TABLE_AUTO_WIDTH;
    m_nFormatWidth = nFormatWidth;
    m_eType = eType;
    m_sName = sName;
  }

  //------------------------------------------------------------------
  TableCell* AddInt (const gtuint16 n)
  //------------------------------------------------------------------
  {
    TableCell* pCell = new TableCell (gtuint32 (n));
    m_aCells.push_back (pCell);
    return pCell;
  }

  //------------------------------------------------------------------
  TableCell* AddInt (const gtuint32 n)
  //------------------------------------------------------------------
  {
    TableCell* pCell = new TableCell (n);
    m_aCells.push_back (pCell);
    return pCell;
  }

  //------------------------------------------------------------------
  TableCell* AddInt (const gtint64 n)
  //------------------------------------------------------------------
  {
    TableCell* pCell = new TableCell (n);
    m_aCells.push_back (pCell);
    return pCell;
  }

  //------------------------------------------------------------------
  TableCell* AddDbl (const double d)
  //------------------------------------------------------------------
  {
    TableCell* pCell = new TableCell (d);
    m_aCells.push_back (pCell);
    return pCell;
  }

  //------------------------------------------------------------------
  TableCell* AddStr (const pstring &s)
  //------------------------------------------------------------------
  {
    TableCell* pCell = new TableCell (s);
    m_aCells.push_back (pCell);
    return pCell;
  }

  //------------------------------------------------------------------
  TableCell* GetCell (const size_t n)
  //------------------------------------------------------------------
  {
    ASSERT (n < m_aCells.size ());
    return m_aCells[n];
  }

  bool            IsAligned      () const { return m_nFormatWidth != NO_ALIGN; }
  LPCTSTR         GetpName       () const { return m_sName.c_str (); }
  int             GetFormatWidth () const { return m_nFormatWidth; }
  TableColumnType GetType        () const { return m_eType; }
  size_t          GetRowCount    () const { return m_aCells.size (); }

  // get the width
  size_t GetWidth ()
  {
    if (m_nWidth == TABLE_AUTO_WIDTH)
    {
      // static int i = 0; if (!i++) __asm int 3

      // iterate all entries and determine the maximum width
      // -> consider the colspan
      // Add 1 char as space
      TableCellVector::const_iterator cit = m_aCells.begin ();
      for (; cit != m_aCells.end (); ++cit)
        m_nWidth = _max (m_nWidth, ((*cit)->GetWidth () / (*cit)->GetColSpan ()) + 1);
    }
    return m_nWidth;
  }
};

//--------------------------------------------------------------------
class TableColumnVector : public std::vector<TableColumn*>
//--------------------------------------------------------------------
{
public:
  //------------------------------------------------------------------
  virtual ~TableColumnVector ()
  //------------------------------------------------------------------
  {
    const_iterator cit = begin ();
    for (; !(cit == end ()); ++cit)
      delete *cit;
  }
};

//--------------------------------------------------------------------
class Table
//--------------------------------------------------------------------
{
private:
  bool              m_bHeadline;
  TableColumnVector m_aColumns;

  Table (const Table&);
  Table& operator = (const Table&);

public:
  //------------------------------------------------------------------
  explicit Table ()
  //------------------------------------------------------------------
    : m_bHeadline (true)
  {}

  //------------------------------------------------------------------
  void EnableHeadline (const bool bEnable)
  //------------------------------------------------------------------
  {
    m_bHeadline = bEnable;
  }

  /*! Add a new column.
      \param sName headline string
      \param eType value type of the column
      \param nWidth number chars.
      \param nFormatWidth for numbers this is the printf param, for
                          strings it is either ALIGN_LEFT, ALIGN_CENTER
                          or ALIGN_RIGHT
   */
  //------------------------------------------------------------------
  void AddColumn (      LPCTSTR         sName,
                  const TableColumnType eType,
                  const size_t          nWidth,
                  const int             nFormatWidth)
  //------------------------------------------------------------------
  {
    ASSERT (sName);
    ASSERT (eType != STR || nFormatWidth < NO_ALIGN); // check for alignment!
    TableColumn *pTC = new TableColumn;
    pTC->Init (sName, eType, nWidth, nFormatWidth);
    m_aColumns.push_back (pTC);
  }

  /*! Add an int32 value.
   */
  //------------------------------------------------------------------
  TableCell* AddInt (const size_t   nColIndex,
                     const gtuint16 n)
  //------------------------------------------------------------------
  {
    return m_aColumns[nColIndex]->AddInt (n);
  }

  /*! Add an int32 value.
   */
  //------------------------------------------------------------------
  TableCell* AddInt (const size_t   nColIndex,
                     const gtuint32 n)
  //------------------------------------------------------------------
  {
    return m_aColumns[nColIndex]->AddInt (n);
  }

  /*! Add an int64 value.
   */
  //------------------------------------------------------------------
  TableCell* AddInt (const size_t nColIndex,
                     const file_t n)
  //------------------------------------------------------------------
  {
    return m_aColumns[nColIndex]->AddInt (n);
  }

  /*! Add a double value.
   */
  //------------------------------------------------------------------
  TableCell* AddDbl (const size_t nColIndex,
                     const double d)
  //------------------------------------------------------------------
  {
    return m_aColumns[nColIndex]->AddDbl (d);
  }

  //------------------------------------------------------------------
  TableCell* AddStr (const size_t  nColIndex,
                           resid_t nResId)
  //------------------------------------------------------------------
  {
    pstring sTmp;
    str_assign_from_res (sTmp, nResId);
    return m_aColumns[nColIndex]->AddStr (sTmp);
  }

  //------------------------------------------------------------------
  TableCell* AddStr (const size_t   nColIndex,
                     const pstring& s)
  //------------------------------------------------------------------
  {
    return m_aColumns[nColIndex]->AddStr (s);
  }

  bool IsHeadlineEnabled () const
  {
    return m_bHeadline;
  }

  size_t GetRowCount () const
  {
    // we need to scan all columns, in case "colspan" is used
    size_t nMax = 0;
    TableColumnVector::const_iterator cit = m_aColumns.begin ();
    for (; cit != m_aColumns.end (); ++cit)
      nMax = _max (nMax, (*cit)->GetRowCount ());
    return nMax;
  }

  size_t             GetColumnCount    () const { return m_aColumns.size (); }
  TableColumnVector& GetColumns        ()       { return m_aColumns; }
};

}  // namespace

#endif
