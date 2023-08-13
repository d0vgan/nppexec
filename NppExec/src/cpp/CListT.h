/***********************************************
 *  
 *  CListT ver. 1.3.4
 *  --------------------------------  
 *  (C) DV, Nov 2006 - Aug 2023
 *  --------------------------------
 *
 *  Template:
 *  >>  CListT<type T>
 *
 *  Pre-defined types: none
 *  >>  Example:  typedef CListT<CStr>  CStrList;
 *  >>  Example:  typedef CListT<CWStr> CWStrList;
 *
 ***********************************************/

#ifndef _list_t_h_
#define _list_t_h_
//----------------------------------------------------------------------------
#include <utility>

template <class T> class CListT;

template <class T> class CListItemT 
{
private:
  CListItemT<T>* _pPrevItem;
  CListItemT<T>* _pNextItem;
  CListT<T>*     _pListOwner;
  T              _item;

public:
  CListItemT(CListT<T>* pListOwner) noexcept : _pPrevItem(NULL), _pNextItem(NULL), _pListOwner(pListOwner), _item() { }
  CListItemT(CListT<T>* pListOwner, const T& item) : _pPrevItem(NULL), _pNextItem(NULL), _pListOwner(pListOwner), _item(item) { }
  CListItemT(CListT<T>* pListOwner, T&& item) noexcept : _pPrevItem(NULL), _pNextItem(NULL), _pListOwner(pListOwner), _item(std::forward<T>(item)) { }

  inline const T&       GetItem() const noexcept  { return _item; }
  inline T&             GetItem() noexcept  { return _item; }
  inline CListItemT<T>* GetNext() const noexcept  { return _pNextItem; }
  inline CListItemT<T>* GetPrev() const noexcept  { return _pPrevItem; }
  inline CListT<T>*     GetOwner() const noexcept  { return _pListOwner; }
  inline void           SetItem(const T& item)  { _item = item; }
  inline void           SetItem(T&& item) noexcept  { _item = std::forward<T>(item); }

protected:
  friend class CListT<T>;
  inline void           SetNext(CListItemT<T>* pNextItem) noexcept  { _pNextItem = pNextItem; }
  inline void           SetPrev(CListItemT<T>* pPrevItem) noexcept  { _pPrevItem = pPrevItem; }

};

template <class T> class CListT 
{
public:
  typedef T value_type;
  typedef T CValueType;
  typedef CListItemT<T>* CListItemPtr;

private:
  CListItemT<T>* m_pFirstItem;
  CListItemT<T>* m_pLastItem;
  int            m_nCount;

  CListItemT<T>* itemAdd(CListItemT<T>* pNewItem) noexcept;
  CListItemT<T>* itemInsert(CListItemT<T>* pListItemPtr, bool bAfterListItem, CListItemT<T>* pNewItem) noexcept;
  CListItemT<T>* itemCreate(const T& item);
  CListItemT<T>* itemCreate(T&& item);
  void           itemDestroy(CListItemT<T>* pItemPtr) noexcept;
  void           itemQuickSort(CListItemPtr* pListItems, int lower, int upper) noexcept;
  void           itemSwap(CListItemT<T>* p1, CListItemT<T>* p2) noexcept;

  bool           areItemsEqual(const CListT<T>& List) const noexcept;

public:
  CListT() noexcept;
  CListT(const CListT& List);
  CListT(CListT&& List) noexcept;
  explicit CListT(const T& item);
  explicit CListT(T&& item);
  ~CListT() noexcept;
  CListItemT<T>* Add(const T& item);
  CListItemT<T>* Add(T&& item);
  int            AddItems(const CListT<T>& List);  // returns items count
  int            Assign(const CListT<T>& List);    // returns items count
  int            Assign(CListT<T>&& List) noexcept; // returns items count
  void           Clear() noexcept  { DeleteAll(); }
  bool           Delete(CListItemT<T>* pItemPtr) noexcept;
  void           DeleteAll() noexcept;
  bool           DeleteFirst() noexcept;
  bool           DeleteLast() noexcept;
  CListItemT<T>* FindExact(const T& item, const CListItemT<T>* pStartItemPtr = NULL) const noexcept;
  template<class Pred> CListItemT<T>* Find(Pred pred) const;
  int            GetCount() const noexcept  { return m_nCount; }
  CListItemT<T>* GetFirst() const noexcept  { return m_pFirstItem; }
  CListItemT<T>* GetLast() const noexcept  { return m_pLastItem; }
  CListItemT<T>* Insert(CListItemT<T>* pListItemPtr, bool bAfterListItem, const T& item);
  CListItemT<T>* Insert(CListItemT<T>* pListItemPtr, bool bAfterListItem, T&& item);
  CListItemT<T>* InsertFirst(const T& item);
  CListItemT<T>* InsertFirst(T&& item);
  bool           IsEmpty() const noexcept  { return (m_nCount == 0); }
  bool           Sort();
  bool           Swap(CListItemT<T>* pItemPtr1, CListItemT<T>* pItemPtr2) noexcept;
  void           Swap(CListT<T>& List) noexcept;

  CListT&        operator=(const CListT<T>& List)  { Assign(List); return *this; }
  CListT&        operator=(CListT<T>&& List) noexcept  { Assign(std::forward<CListT>(List)); return *this; }

  bool           operator==(const CListT<T>& List) const noexcept { return areItemsEqual(List); }
  bool           operator!=(const CListT<T>& List) const noexcept { return !areItemsEqual(List); }

};

//----------------------------------------------------------------------------

template <class T> CListT<T>::CListT() noexcept : m_pFirstItem(NULL), m_pLastItem(NULL), m_nCount(0)
{
}

template <class T> CListT<T>::CListT(const CListT& List) : m_pFirstItem(NULL), m_pLastItem(NULL), m_nCount(0)
{
    AddItems(List);
}

template <class T> CListT<T>::CListT(CListT&& List) noexcept : m_pFirstItem(NULL), m_pLastItem(NULL), m_nCount(0)
{
    Swap(List);
}

template <class T> CListT<T>::CListT(const T& item) : m_pFirstItem(NULL), m_pLastItem(NULL), m_nCount(0)
{
    Add(item);
}

template <class T> CListT<T>::CListT(T&& item) : m_pFirstItem(NULL), m_pLastItem(NULL), m_nCount(0)
{
    Add(std::forward<T>(item));
}

template <class T> CListT<T>::~CListT() noexcept
{
    DeleteAll();
}

template <class T> CListItemT<T>* CListT<T>::itemAdd(CListItemT<T>* pNewItem) noexcept
{
    if ( pNewItem )
    {
        if ( m_nCount == 0 )
        {
            m_pFirstItem = pNewItem;
        }
        else
        {
            pNewItem->SetPrev(m_pLastItem);
            m_pLastItem->SetNext(pNewItem);
        }
        m_pLastItem = pNewItem;
        ++m_nCount;
    }
    return pNewItem;
}

template <class T> CListItemT<T>* CListT<T>::itemInsert(
  CListItemT<T>* pListItemPtr, 
  bool           bAfterListItem,
  CListItemT<T>* pNewItem) noexcept
{
    if ( pNewItem )
    {
        CListItemT<T>* pItem;
        if ( bAfterListItem )
        {
            pItem = pListItemPtr->GetNext();
            pListItemPtr->SetNext(pNewItem);
            pNewItem->SetPrev(pListItemPtr);
            pNewItem->SetNext(pItem);
            if ( pItem )
                pItem->SetPrev(pNewItem);
            if ( m_pLastItem == pListItemPtr )
                m_pLastItem = pNewItem;
        }
        else
        {
            pItem = pListItemPtr->GetPrev();
            pListItemPtr->SetPrev(pNewItem);
            pNewItem->SetNext(pListItemPtr);
            pNewItem->SetPrev(pItem);
            if ( pItem )
                pItem->SetNext(pNewItem);
            if ( m_pFirstItem == pListItemPtr )
                m_pFirstItem = pNewItem;
        }
        ++m_nCount;
    }
    return pNewItem;
}

template <class T> CListItemT<T>* CListT<T>::itemCreate(const T& item)
{
    CListItemT<T>* pNewItem = new CListItemT<T>(this, item);
    return pNewItem;
}

template <class T> CListItemT<T>* CListT<T>::itemCreate(T&& item)
{
    CListItemT<T>* pNewItem = new CListItemT<T>(this, std::forward<T>(item));
    return pNewItem;
}

template <class T> void CListT<T>::itemDestroy(CListItemT<T>* pItemPtr) noexcept
{
    delete pItemPtr;
}

template <class T> void CListT<T>::itemQuickSort(
  CListItemPtr* pListItems, 
  int           lower, 
  int           upper) noexcept
{
    if ( lower < upper ) 
    {
        itemSwap( pListItems[lower], pListItems[(upper + lower)/2] );
    
        T   pivot = pListItems[lower]->_item;
        int m = lower;
        int i = lower + 1;
        while ( i <= upper ) {
            if ( pListItems[i]->_item < pivot ) {
                ++m;
                itemSwap( pListItems[m], pListItems[i] );
            }
            ++i;
        }
    
        itemSwap( pListItems[lower], pListItems[m] );
        itemQuickSort( pListItems, lower, m - 1 );
        itemQuickSort( pListItems, m + 1, upper );
    }
}

template <class T> void CListT<T>::itemSwap(CListItemT<T>* p1, CListItemT<T>* p2) noexcept
{
    if ( p1 != p2 )
    {
        T item = p2->_item;
        p2->_item = p1->_item;
        p1->_item = item;
    }
}

template <class T> CListItemT<T>* CListT<T>::Add(const T& item)
{
    CListItemT<T>* pNewItem = itemCreate(item);
    return itemAdd(pNewItem);
}

template <class T> CListItemT<T>* CListT<T>::Add(T&& item)
{
    CListItemT<T>* pNewItem = itemCreate(std::forward<T>(item));
    return itemAdd(pNewItem);
}

template <class T> int CListT<T>::AddItems(const CListT<T>& List)
{
    CListItemT<T>* pItem = (CListItemT<T> *) List.GetFirst();
    while ( pItem )
    {
        Add( pItem->GetItem() );
        pItem = pItem->GetNext();
    }
    return m_nCount;
}

template <class T> int CListT<T>::Assign(const CListT<T>& List)
{
    DeleteAll();
    return AddItems(List);
}

template <class T> int CListT<T>::Assign(CListT<T>&& List) noexcept
{
    DeleteAll();
    Swap(List);
    return m_nCount;
}

template <class T> bool CListT<T>::Delete(CListItemT<T>* pItemPtr) noexcept
{
    if ( !pItemPtr )
        return false;

    CListT<T>* pListOwner = pItemPtr->GetOwner();
    if ( pListOwner != this )
        return false;
  
    CListItemT<T>* pPrev = pItemPtr->GetPrev();
    CListItemT<T>* pNext = pItemPtr->GetNext();
    
    --(pListOwner->m_nCount);
    if ( pItemPtr == pListOwner->m_pFirstItem )
        pListOwner->m_pFirstItem = pNext;
    if ( pItemPtr == pListOwner->m_pLastItem )
        pListOwner->m_pLastItem = pPrev;
  
    if ( pPrev )
        pPrev->SetNext(pNext);
    if ( pNext )
        pNext->SetPrev(pPrev);
    itemDestroy(pItemPtr);

    return true;  
}

template <class T> void CListT<T>::DeleteAll() noexcept
{
    CListItemT<T>* pNext;
    CListItemT<T>* pItem = m_pFirstItem;
    while ( pItem )
    {
        pNext = pItem->GetNext();
        itemDestroy(pItem);
        pItem = pNext;
    }
    m_pFirstItem = NULL;
    m_pLastItem = NULL;
    m_nCount = 0;
}

template <class T> bool CListT<T>::DeleteFirst() noexcept
{
    return Delete(m_pFirstItem);
}

template <class T> bool CListT<T>::DeleteLast() noexcept
{
    return Delete(m_pLastItem);
}

template <class T> CListItemT<T>* CListT<T>::FindExact(
  const T&             item, 
  const CListItemT<T>* pStartItemPtr ) const noexcept
{
    if ( m_pFirstItem )
    {
        if ( !pStartItemPtr )  pStartItemPtr = m_pFirstItem;
        if ( pStartItemPtr->GetOwner() == this )
        {
            while ( pStartItemPtr )
            {
                if ( pStartItemPtr->GetItem() == item )
                    break;
                else
                    pStartItemPtr = pStartItemPtr->GetNext();
            }
            return ( (CListItemT<T>*) pStartItemPtr );
        }
    }
    return NULL;
}

template <class T> template <class Pred> CListItemT<T>* CListT<T>::Find(Pred pred) const
{
    for ( CListItemT<T>* pItem = m_pFirstItem; pItem != NULL; pItem = pItem->GetNext() )
    {
        if ( pred(pItem->GetItem()) )
            return pItem;
    }
    return NULL;
}

template <class T> CListItemT<T>* CListT<T>::Insert(
  CListItemT<T>* pListItemPtr, 
  bool           bAfterListItem, 
  const T&       item)
{
    if ( (!pListItemPtr) || (pListItemPtr->GetOwner() != this) )
        return NULL;

    CListItemT<T>* pNewItem = itemCreate(item);
    return itemInsert(pListItemPtr, bAfterListItem, pNewItem);
}

template <class T> CListItemT<T>* CListT<T>::Insert(
    CListItemT<T>* pListItemPtr, 
    bool           bAfterListItem, 
    T&&            item)
{
    if ( (!pListItemPtr) || (pListItemPtr->GetOwner() != this) )
        return NULL;

    CListItemT<T>* pNewItem = itemCreate(std::forward<T>(item));
    return itemInsert(pListItemPtr, bAfterListItem, pNewItem);
}

template <class T> CListItemT<T>* CListT<T>::InsertFirst(const T& item)
{
    if ( m_nCount == 0 )
        return Add(item);
    else
        return Insert( m_pFirstItem, false, item );
}

template <class T> CListItemT<T>* CListT<T>::InsertFirst(T&& item)
{
    if ( m_nCount == 0 )
        return Add(std::forward<T>(item));
    else
        return Insert( m_pFirstItem, false, std::forward<T>(item) );
}

template <class T> bool CListT<T>::Sort()
{
    if ( m_nCount > 0 )
    {
        CListItemPtr* pListItems = new CListItemPtr[m_nCount];
        if ( pListItems )
        {
            int i = 0;
            CListItemT<T>* pItem = m_pFirstItem;
      
            while ( pItem )
            {
                pListItems[i++] = pItem;
                pItem = pItem->_pNextItem;
            }
            itemQuickSort( pListItems, 0, m_nCount - 1 );

            delete [] pListItems;

            return true;
        } // else out of memory: sorting is impossible
    } // else nothing to sort
    return false;
}

template <class T> bool CListT<T>::Swap(
  CListItemT<T>* pItemPtr1, 
  CListItemT<T>* pItemPtr2) noexcept
{
    if ( (!pItemPtr1) || (!pItemPtr2) )
        return false;

    if ( (pItemPtr1->_pListOwner != this) ||
         (pItemPtr2->_pListOwner != this) )
    {
        return false;
    }

    itemSwap( pItemPtr1, pItemPtr2 );
    return true;
}

template <class T> void CListT<T>::Swap(CListT<T>& List) noexcept
{
    CListItemT<T>* listFirstItem = List.m_pFirstItem;
    CListItemT<T>* listLastItem  = List.m_pLastItem;
    int            listCount     = List.m_nCount;

    List.m_pFirstItem = m_pFirstItem;
    List.m_pLastItem  = m_pLastItem;
    List.m_nCount     = m_nCount;

    m_pFirstItem = listFirstItem;
    m_pLastItem  = listLastItem;
    m_nCount     = listCount;
}

template <class T> bool CListT<T>::areItemsEqual(const CListT<T>& List) const noexcept
{
    if ( m_nCount != List.m_nCount )
        return false;

    CListItemT<T>* pItem1 = m_pFirstItem;
    CListItemT<T>* pItem2 = List.m_pFirstItem;
    while ( pItem1 && pItem2 )
    {
        if ( pItem1->GetItem() != pItem2->GetItem() )
            return false;

        pItem1 = pItem1->GetNext();
        pItem2 = pItem2->GetNext();
    }

    return true;
}

//----------------------------------------------------------------------------
#endif

