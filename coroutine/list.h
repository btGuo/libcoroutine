#ifndef __LIST_H
#define __LIST_H

#include <type_traits>
#include <utility>

namespace co
{

template <typename T>
class ListHook
{
public:
    void setNext(T *next){ m_next = next; }
    void setPrev(T *prev){ m_prev = prev; }
    T *getNext(){ return m_next; }
    T *getPrev(){ return m_prev; }
private:
    T *m_prev{static_cast<T*>(this)};
    T *m_next{static_cast<T*>(this)};
};

template <typename T>
class List
{
    static_assert(std::is_base_of<ListHook<T>, T>::value);
public:
    List(){ m_head = new T(); }
    ~List(){ delete m_head;   }
    void push_back(T *elem)
    {
        add(elem, m_head->getPrev(), m_head);
        m_size++;
    }
    void push_front(T *elem)
    {
        add(elem, m_head, m_head->getNext());
        m_size++;
    }
    void erase(T *elem)
    {
        elem->getPrev()->setNext(elem->getNext());
        elem->getNext()->setPrev(elem->getPrev());
        m_size--;
    }
    T *pop_front()
    {
        auto ret = m_head->getNext();
        erase(m_head->getNext());
        return ret;
    }
    T *pop_back()
    {
        auto ret = m_head->getPrev();
        erase(m_head->getPrev());
        return ret;
    }
    bool empty()
    {
        return m_size == 0;
    }
    std::size_t size()
    {
        return m_size;
    }
private:
    void add(T *newNode, T *prevNode, T *nextNode)
    {
        nextNode->setPrev(newNode);
        newNode->setNext(nextNode);
        newNode->setPrev(prevNode);
        prevNode->setNext(newNode);
    }
    T *m_head{nullptr};
    std::size_t m_size{0};
};

}//namespace co

#endif
