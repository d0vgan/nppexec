/***********************************************
 *  
 *  CListT ver. 1.3.1
 *  --------------------------------  
 *  (C) DV, Nov 2006 - Jun 2021
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
  CListItemT(CListT<T>* pListOwner) : _pPrevItem(NULL), _pNextItem(NULL), _pListOwner(pListOwner), _item() { }
  CListItemT(CListT<T>* pListOwner, const T& item) : _pPrevItem(NULL), _pNextItem(NULL), _pListOwner(pListOwner), _item(item) { }
  CListItemT(CListT<T>* pListOwner, T&& item) : _pPrevItem(NULL), _pNextItem(NULL), _pListOwner(pListOwner), _item(std::forward<T>(item)) { }

  inline const T&       GetItem() const  { return _item; }
  inline T&             GetItem()  { return _item; }
  inline CListItemT<T>* GetNext() const  { return _pNextItem; }
  inline CListItemT<T>* GetPrev() const  { return _pPrevItem; }
  inline CListT<T>*     GetOwner() const  { return _pListOwner; }
  inline void           SetItem(const T& item)  { _item = item; }
  inline void           SetItem(T&& item)  { _item = std::forward<T>(item); }

protected:
  friend class CListT<T>;
  inline void           SetNext(CListItemT<T>* pNextItem)  { _pNextItem = pNextItem; }
  inline void           SetPrev(CListItemT<T>* pPrevItem)  { _pPrevItem = pPrevItem; }

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

  CListItemT<T>* itemAdd(CListItemT<T>* pNewItem);
  CListItemT<T>* itemInsert(CListItemT<T>* pListItemPtr, bool bAfterListItem, CListItemT<T>* pNewItem);
  CListItemT<T>* itemCreate(const T& item);
  CListItemT<T>* itemCreate(T&& item);
  void           itemDestroy(CListItemT<T>* pItemPtr);
  void           itemQuickSort(CListItemPtr* pListItems, int lower, int upper);
  void           itemSwap(CListItemT<T>* p1, CListItemT<T>* p2);

  bool           areItemsEqual(const CListT<T>& List) const;

public:
  CListT();
  CListT(const CListT& List);
  CListT(CListT&& List);
  explicit CListT(const T& item);
  explicit CListT(T&& item);
  ~CListT();
  CListItemT<T>* Add(const T& item);
  CListItemT<T>* Add(T&& item);
  int            AddItems(const CListT<T>& List);  // returns items count
  int            Assign(const CListT<T>& List);    // returns items count
  int            Assign(CListT<T>&& List);         // returns items count
  void           Clear()  { DeleteAll(); }
  bool           Delete(CListItemT<T>* pItemPtr);
  void           DeleteAll();
  bool           DeleteFirst();
  bool           DeleteLast();
  CListItemT<T>* FindExact(const T& item, const CListItemT<T>* pStartItemPtr = NULL) const;
  template<class Pred> CListItemT<T>* Find(Pred pred) const;
  int            GetCount() const  { return m_nCount; }
  CListItemT<T>* GetFirst() const  { return m_pFirstItem; }
  CListItemT<T>* GetLast() const  { return m_pLastItem; }
  CListItemT<T>* Insert(CListItemT<T>* pListItemPtr, bool bAfterListItem, const T& item);
  CListItemT<T>* Insert(CListItemT<T>* pListItemPtr, bool bAfterListItem, T&& item);
  CListItemT<T>* InsertFirst(const T& item);
  CListItemT<T>* InsertFirst(T&& item);
  bool           IsEmpty() const  { return (m_nCount == 0); }
  bool           Sort();
  bool           Swap(CListItemT<T>* pItemPtr1, CListItemT<T>* pItemPtr2);
  void           Swap(CListT<T>& List);

  CListT&        operator=(const CListT<T>& List)  { Assign(List); return *this; }
  CListT&        operator=(CListT<T>&& List)       { Assign(std::forward<CListT>(List)); return *this; }

  bool           operator==(const CListT<T>& List) const { return areItemsEqual(List); }
  bool           operator!=(const CListT<T>& List) const { return !areItemsEqual(List); }

};

//----------------------------------------------------------------------------

template <class T> CListT<T>::CListT() : m_pFirstItem(NULL), m_pLastItem(NULL), m_nCount(0)
{
}

template <class T> CListT<T>::CListT(const CListT& List) : m_pFirstItem(NULL), m_pLastItem(NULL), m_nCount(0)
{
    AddItems(List);
}

template <class T> CListT<T>::CListT(CListT&& List) : m_pFirstItem(NULL), m_pLastItem(NULL), m_nCount(0)
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

template <class T> CListT<T>::~CListT()
{
    DeleteAll();
}

template <class T> CListItemT<T>* CListT<T>::itemAdd(CListItemT<T>* pNewItem)
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
  CListItemT<T>* pNewItem)
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

template <class T> void CListT<T>::itemDestroy(CListItemT<T>* pItemPtr)
{
    delete pItemPtr;
}

template <class T> void CListT<T>::itemQuickSort(
  CListItemPtr* pListItems, 
  int           lower, 
  int           upper)
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

template <class T> void CListT<T>::itemSwap(CListItemT<T>* p1, CListItemT<T>* p2)
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

template <class T> int CListT<T>::Assign(CListT<T>&& List)
{
    DeleteAll();
    Swap(List);
    return m_nCount;
}

template <class T> bool CListT<T>::Delete(CListItemT<T>* pItemPtr)
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

template <class T> void CListT<T>::DeleteAll()
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

template <class T> bool CListT<T>::DeleteFirst()
{
    return Delete(m_pFirstItem);
}

template <class T> bool CListT<T>::DeleteLast()
{
    return Delete(m_pLastItem);
}

template <class T> CListItemT<T>* CListT<T>::FindExact(
  const T&             item, 
  const CListItemT<T>* pStartItemPtr ) const
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
  CListItemT<T>* pItemPtr2)
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

template <class T> void CListT<T>::Swap(CListT<T>& List)
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

template <class T> bool CListT<T>::areItemsEqual(const CListT<T>& List) const
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

