#pragma once

#include "Logging/LogAssert.h"

class EXPORT_COREMODULE ListElement
{
public:
    // Element in a linked list. A common use case is to derive your type T from ListElement and use a List<T>.
    // If you can't use a derived type, for example because an object can be in multiple lists, then use ListNode<T>.
    // A ListElement can also act as its own list root, if you are very concerned about memory and don't need empty lists.
    inline ListElement();

    // RemoveFromList is done explicitly now.
    // We had a lot of issues where these destructors were hiding global modifications happening in the object destructor.
    // Since object destruction happens on another thread this lead to a lot of random crashes. Thus we enforce this to be explicit.
    inline ~ListElement();

    inline bool IsInList() const;
    inline bool RemoveFromList();
    inline void InsertInList(ListElement* pos);

    // Check against List::end(), not NULL
    ListElement* GetPrev() const { return m_Prev; }
    ListElement* GetNext() const { return m_Next; }

protected:
    // For using linked ListElements independently of a List.
    // This feature is hidden by default and types need to opt in by making SetAsListRoot() accessible.
    inline void SetAsListRoot();

private:
    // Non copyable
    ListElement(const ListElement&);
    ListElement& operator=(const ListElement&);

    ListElement* m_Prev;
    ListElement* m_Next;

    template<class T> friend class List;

#if !UNITY_RELEASE
    inline void ValidateLinks() const;

    // Iterator debugging only
    template<class T> friend class ListIterator;
    template<class T> friend class ListConstIterator;
    void SetList(void* l) { m_List = l; }
    void* m_List;
#else
    void SetList(void*) {}
#endif
};

template<class T>
class ListNode : public ListElement
{
public:
    ListNode(T* data = NULL) : m_Data(data) {}
    T& operator*() const  { return *m_Data; }
    T* operator->() const { return m_Data; }
    T* GetData() const { return m_Data; }
    void SetData(T* data) { m_Data = data; }

    // We know the type of prev and next element
    ListNode* GetPrev() const { return static_cast<ListNode*>(ListElement::GetPrev()); }
    ListNode* GetNext() const { return static_cast<ListNode*>(ListElement::GetNext()); }

private:
    T* m_Data;
};

template<class T>
class List;

template<class T>
class ListIterator
{
public:
    ListIterator(T* node = NULL) : m_Node(node) {}

    // Pre- and post-increment operator
    ListIterator& operator++()    { m_Node = m_Node->GetNext(); return *this; }
    ListIterator  operator++(int) { ListIterator ret(*this); ++(*this); return ret; }

    // Pre- and post-decrement operator
    ListIterator& operator--()    { m_Node = m_Node->GetPrev(); return *this; }
    ListIterator  operator--(int) { ListIterator ret(*this); --(*this); return ret; }

    T& operator*() const  { return static_cast<T&>(*m_Node); }
    T* operator->() const { return static_cast<T*>(m_Node); }

    friend bool operator!=(const ListIterator& x, const ListIterator& y) { return x.m_Node != y.m_Node; }
    friend bool operator==(const ListIterator& x, const ListIterator& y) { return x.m_Node == y.m_Node; }

private:
    friend class List<T>;
    ListIterator(ListElement* node) : m_Node(node) {}
    ListElement* m_Node;
};


template<class T>
class ListConstIterator
{
public:
    ListConstIterator(const T* node = NULL) : m_Node(node) {}

    // Pre- and post-increment operator
    ListConstIterator& operator++()    { m_Node = m_Node->GetNext(); return *this; }
    ListConstIterator  operator++(int) { ListConstIterator ret(*this); ++(*this); return ret; }

    // Pre- and post-decrement operator
    ListConstIterator& operator--()    { m_Node = m_Node->GetPrev(); return *this; }
    ListConstIterator  operator--(int) { ListConstIterator ret(*this); --(*this); return ret; }

    const T& operator*() const  { return static_cast<const T&>(*m_Node); }
    const T* operator->() const { return static_cast<const T*>(m_Node); }

    friend bool operator!=(const ListConstIterator& x, const ListConstIterator& y) { return x.m_Node != y.m_Node; }
    friend bool operator==(const ListConstIterator& x, const ListConstIterator& y) { return x.m_Node == y.m_Node; }

private:
    friend class List<T>;
    ListConstIterator(const ListElement* node) : m_Node(node) {}
    const ListElement* m_Node;
};


template<class T>
class List
{
public:
    typedef ListConstIterator<T> const_iterator;
    typedef ListIterator<T> iterator;
    typedef T value_type;

    inline  List();
    inline  ~List();

    void    push_back(T& node)            { node.InsertInList(&m_Root); }
    void    push_front(T& node)           { node.InsertInList(m_Root.m_Next); }
    void    insert(iterator pos, T& node) { node.InsertInList(&(*pos)); }
    void    erase(iterator pos)           { pos->RemoveFromList(); }

    void    pop_back()                    { if (m_Root.m_Prev != &m_Root) m_Root.m_Prev->RemoveFromList(); }
    void    pop_front()                   { if (m_Root.m_Next != &m_Root) m_Root.m_Next->RemoveFromList(); }

    iterator       begin()                { return iterator(m_Root.m_Next); }
    iterator       end()                  { return iterator(&m_Root); }

    const_iterator begin() const          { return const_iterator(m_Root.m_Next); }
    const_iterator end() const            { return const_iterator(&m_Root); }

    T&             front()                { DebugAssert(!empty()); return static_cast<T&>(*m_Root.m_Next); }
    T&             back()                 { DebugAssert(!empty()); return static_cast<T&>(*m_Root.m_Prev); }

    const T&       front() const          { DebugAssert(!empty()); return static_cast<const T&>(*m_Root.m_Next); }
    const T&       back() const           { DebugAssert(!empty()); return static_cast<const T&>(*m_Root.m_Prev); }

    bool           empty() const          { return begin() == end(); }

    size_t         size_slow() const;
    inline void    clear();
    inline void    swap(List& other);

    // Insert list into list (removes elements from source)
    inline void    insert(iterator pos, List& src);
    inline void    append(List& src);

private:
    ListElement m_Root;
};


template<class T>
List<T>::List()
{
    m_Root.SetAsListRoot();
    m_Root.SetList(this);
}

template<class T>
List<T>::~List()
{
    DebugAssert(empty());
}

template<class T>
size_t List<T>::size_slow() const
{
    size_t size = 0;
    ListElement* node = m_Root.m_Next;
    while (node != &m_Root)
    {
        node = node->m_Next;
        size++;
    }
    return size;
}

template<class T>
void List<T>::clear()
{
    ListElement* node = m_Root.m_Next;
    while (node != &m_Root)
    {
        ListElement* next = node->m_Next;
        node->m_Prev = NULL;
        node->m_Next = NULL;
        node->SetList(NULL);
        node = next;
    }
    m_Root.m_Next = &m_Root;
    m_Root.m_Prev = &m_Root;
}

template<class T>
void List<T>::swap(List<T>& other)
{
    DebugAssert(this != &other);

    std::swap(other.m_Root.m_Prev, m_Root.m_Prev);
    std::swap(other.m_Root.m_Next, m_Root.m_Next);

    if (other.m_Root.m_Prev == &m_Root)
        other.m_Root.m_Prev = &other.m_Root;
    if (m_Root.m_Prev == &other.m_Root)
        m_Root.m_Prev = &m_Root;
    if (other.m_Root.m_Next == &m_Root)
        other.m_Root.m_Next = &other.m_Root;
    if (m_Root.m_Next == &other.m_Root)
        m_Root.m_Next = &m_Root;

    other.m_Root.m_Prev->m_Next = &other.m_Root;
    other.m_Root.m_Next->m_Prev = &other.m_Root;

    m_Root.m_Prev->m_Next = &m_Root;
    m_Root.m_Next->m_Prev = &m_Root;

#if !UNITY_RELEASE
    iterator my_it, my_end = end();
    for (my_it = begin(); my_it != my_end; ++my_it)
        my_it->m_List = this;
    iterator other_it, other_end = other.end();
    for (other_it = other.begin(); other_it != other_end; ++other_it)
        other_it->m_List = &other;
#endif
}

template<class T>
void List<T>::insert(iterator pos, List<T>& src)
{
    DebugAssert(this != &src);
    if (src.empty())
        return;

#if !UNITY_RELEASE
    iterator src_it, src_end = src.end();
    for (src_it = src.begin(); src_it != src_end; ++src_it)
        src_it->m_List = this;
#endif
    // Insert source before pos
    ListElement* a = pos.m_Node->m_Prev;
    ListElement* b = pos.m_Node;
    a->m_Next = src.m_Root.m_Next;
    b->m_Prev = src.m_Root.m_Prev;
    a->m_Next->m_Prev = a;
    b->m_Prev->m_Next = b;
    // Clear source list
    src.m_Root.m_Next = &src.m_Root;
    src.m_Root.m_Prev = &src.m_Root;
}

template<class T>
void List<T>::append(List& src)
{
    insert(end(), src);
    DebugAssert(src.empty());
}

ListElement::ListElement()
{
    m_Prev = NULL;
    m_Next = NULL;
    SetList(NULL);
}

inline ListElement::~ListElement()
{
    DebugAssertMsg(m_Prev == NULL || m_Prev == this, "List element is still in a list!");
}

bool ListElement::IsInList() const
{
    // Returns true for a list root, even if the list is empty
    return m_Prev != NULL;
}

bool ListElement::RemoveFromList()
{
    if (!IsInList())
        return false;

#if !UNITY_RELEASE
    ValidateLinks();
#endif
    m_Prev->m_Next = m_Next;
    m_Next->m_Prev = m_Prev;
    m_Prev = NULL;
    m_Next = NULL;
    SetList(NULL);
    return true;
}

void ListElement::InsertInList(ListElement* pos)
{
    if (this == pos)
        return;

    if (IsInList())
        RemoveFromList();

#if !UNITY_RELEASE
    m_List = pos->m_List;
    pos->m_Prev->ValidateLinks();
    pos->ValidateLinks();
#endif
    m_Prev = pos->m_Prev;
    m_Next = pos;
    m_Prev->m_Next = this;
    m_Next->m_Prev = this;
#if !UNITY_RELEASE
    ValidateLinks();
#endif
}

void ListElement::SetAsListRoot()
{
    DebugAssert(!IsInList());
    m_Prev = m_Next = this;
    SetList(NULL);
}

#if !UNITY_RELEASE
void ListElement::ValidateLinks() const
{
    DebugAssert(m_Prev != NULL && m_Next != NULL);
    DebugAssert(m_Prev->m_Next == this && m_Next->m_Prev == this);
    DebugAssert(m_Prev->m_List == m_List && m_Next->m_List == m_List);
}

#endif

/// Allows for iterating a linked list, even if you add / remove any node during traversal.
template<class T>
class SafeIterator
{
public:
    SafeIterator(T& list)
        : m_SourceList(list)
    {
        m_CurrentNode = NULL;
        m_ExecuteList.swap(m_SourceList);
    }

    ~SafeIterator()
    {
        // Call Complete if you abort the iteration!
        DebugAssert(m_ExecuteList.empty());
    }

    // You must call complete if you are in some way aborting list iteration.
    // If you dont call Complete, the source list will lose nodes that have not yet been iterated permanently.
    //
    /// SafeIterator<Behaviour*> i(myList);
    /// i =0;
    /// while(i.GetNext() && ++i != 3)
    ///   (**i).Update();
    /// i.Complete();
    void Complete()
    {
        m_SourceList.append(m_ExecuteList);
    }

    typename T::value_type* Next()
    {
        if (!m_ExecuteList.empty())
        {
            typename T::iterator it = m_ExecuteList.begin();
            m_CurrentNode = &*it;
            m_ExecuteList.erase(it);
            m_SourceList.push_back(*m_CurrentNode);
        }
        else
        {
            m_CurrentNode = NULL;
        }
        return m_CurrentNode;
    }

    typename T::value_type& operator*() const { return *m_CurrentNode; }
    typename T::value_type* operator->() const  { return m_CurrentNode; }

private:
    T m_ExecuteList;
    T& m_SourceList;
    typename T::value_type* m_CurrentNode;
};
