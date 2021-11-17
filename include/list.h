/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <initializer_list>

#include <core/linked_list.h>
#include <container.h>
#include <serialization.h>

namespace spl {

/**
 * @brief A singly-linked list.
 * 
 * @tparam T The type of list elements.
 */
template <typename T>
class List
:   protected __LinkedList::ListBase<T, __LinkedList::SinglyLinkedNode<T>, size_t>,
    public ForwardIterableContainer<List<T>>,
    public Serializable
{

    template <typename ListType> friend struct ListTester;

protected:
    using base = typename __LinkedList::ListBase<T, __LinkedList::SinglyLinkedNode<T>, size_t>;
    using node = typename base::node;
    using base::_head;
    using base::_tail;
    using base::_mkNode;

public:

    using Iterator = typename base::template ListForwardIterator<T>;
    using ConstIterator = typename base::template ListForwardIterator<const T>;

    /**
     * @brief Construct a new List object.
     */
    List()
    :   base()
    { }

    List(const List &rhs)
    :   base(rhs)
    { }

    List(List &&rhs)
    :   base(std::move(rhs))
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @param list An initializer list of objects of type T.
     */
    List(const std::initializer_list<T> &list)
    :   base(list)
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @param seq An iterable container of type T objects.
     */
    template <typename Sequence>
    List(const Sequence &seq)
    :   base(seq)
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @param seq An iterable container of type T objects.
     */
    template <typename Sequence>
    List(Sequence &&seq)
    :   base(std::move(seq))
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @param begin A beginning iterator over type T objects.
     * @param end An end iterator over type T objects.
     */
    template <typename Begin, typename End>
    List(const Begin &begin, const End &end)
    :   base(begin, end)
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @param begin A beginning iterator over type T objects.
     * @param end An end iterator over type T objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    List(const Begin &begin, const End &end, size_t size)
    :   base(begin, end)
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new List object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static List<typename It::value_type> create(const It &begin, const EndIt &end) {
        return List<typename It::value_type>(begin, end);
    }

    /**
     * @brief Construct a new List object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @param size The number of elements between begin and end.
     * @return A new List object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static List<typename It::value_type> create(const It &begin, const EndIt &end, size_t size) {
        return List<typename It::value_type>(begin, end, size);
    }

    ~List() = default;

    List & operator=(const List &rhs) {
        base::operator=(rhs);
        return *this;
    }

    List & operator=(List &&rhs) {
        base::operator=(std::move(rhs));
        return *this;
    }

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        base::_serialize(serializer);
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        base::_deserialize(serializer);
    }

    /**
     * @return The size of this container.
     */
    size_t size() const {
        return base::size();
    }

    /**
     * @return A boolean indicating whether this container is empty.
     */
    bool empty() const {
        return base::empty();
    }

    /**
     * @return A boolean indicating whether this container is non-empty.
     */
    bool nonEmpty() const {
        return ! base::empty();
    }

    /**
     * @return A constant iterator pointing to the beginning of this container.
     */
    ConstIterator cbegin() const {
        return base::cbegin();
    }

    /**
     * @return A constant iterator pointing to a past-the-end position.
     */
    ConstIterator cend() const {
        return base::cend();
    }

    /**
     * @return An iterator pointing to the beginning of this container.
     */
    Iterator begin() {
        return base::begin();
    }

    /**
     * @return A constant iterator pointing to the beginning of this container.
     */
    ConstIterator begin() const {
        return base::begin();
    }

    /**
     * @return An iterator pointing to a past-the-end position.
     */
    Iterator end() {
        return base::end();
    }

    /**
     * @return A constant iterator pointing to a past-the-end position.
     */
    ConstIterator end() const {
        return base::end();
    }

    /**
     * @return A reference to the first element of this container.
     */
    T & front() {
        return static_cast<node *>(_head)->data;
    }

    /**
     * @return A constant reference to the first element of this container.
     */
    const T & front() const {
        return static_cast<node *>(_head)->data;
    }

    /**
     * @return A reference to the last element of this container.
     */
    T & back() {
        return static_cast<node *>(_tail)->data;
    }

    /**
     * @return A constant reference to the last element of this container.
     */
    const T & back() const {
        return static_cast<node *>(_tail)->data;
    }

    /**
     * @brief Erases all elements in this container.
     * 
     * @return A reference to this container for chaining.
     */
    List & clear() {
        base::clear();
        return *this;
    }

    /**
     * @brief Inserts an element to the front of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & prepend(const T &elem) {
        base::prepend(_mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element to the front of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & prepend(T &&elem) {
        base::prepend(_mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Inserts an element to the front of a list.
     * 
     * @param[in] elem The element to insert.
     * @param[in] list The target list.
     * @return A reference to the list for chaining.
     */
    friend List & operator>>(const T &elem, List &list) {
        return list.prepend(elem);
    }

    /**
     * @brief Inserts an element to the front of a list.
     * 
     * @param[in] elem The element to insert.
     * @param[in] list The target list.
     * @return A reference to the list for chaining.
     */
    friend List & operator>>(T &&elem, List &list) {
        return list.prepend(std::move(elem));
    }

    /**
     * @brief Inserts an element to the back of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & append(const T &elem) {
        base::append(_mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element to the back of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & append(T &&elem) {
        base::append(_mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Inserts an element to the back of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & operator<<(const T &elem) {
        base::append(_mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element to the back of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & operator<<(T &&elem) {
        base::append(_mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Inserts an element.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & insert(const T &elem) {
        base::prepend(_mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & insert(T &&elem) {
        base::prepend(_mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Inserts an element directly before an iterator position.
     * 
     * @param[in] pos An iterator marking point before which the inserted
     * element will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    List & insertBefore(const Iterator &pos, const T &elem) {
        base::insertBefore(pos, _mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element directly before an iterator position.
     * 
     * @param[in] pos An iterator marking point before which the inserted
     * element will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    List & insertBefore(const Iterator &pos, T &&elem) {
        base::insertBefore(pos, _mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Inserts an element directly after an iterator position.
     * 
     * @param[in] pos An iterator marking point after which the inserted element
     * will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    List & insertAfter(const Iterator &pos, const T &elem) {
        base::insertAfter(pos, _mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element directly after an iterator position.
     * 
     * @param[in] pos An iterator marking point after which the inserted element
     * will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    List & insertAfter(const Iterator &pos, T &&elem) {
        base::insertAfter(pos, _mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Erases an element at the indicated position.
     * 
     * @param pos An iterator pointing to the element to be erased. The
     * iterator will be moved to the next element before the element is erased.
     * @return A reference to this container for chaining.
     */
    List & erase(Iterator &pos) {
        base::erase(pos);
        return *this;
    }

    /**
     * @brief Erases an element at the indicated position.
     * 
     * @param pos An iterator pointing to the element to be erased. The
     * iterator will be moved to the next element before the element is erased.
     * @return A reference to this container for chaining.
     */
    List & erase(Iterator &&pos) {
        base::erase(pos);
        return *this;
    }

    /**
     * @brief Removes an element from the indicated position and returns it.
     * 
     * @param pos An iterator pointing to the element to be removed. The
     * iterator will be moved to the next element before the element is removed.
     * @return The removed element.
     */
    T remove(Iterator &pos) {
        return base::remove(pos);
    }

    /**
     * @brief Removes an element from the indicated position and returns it.
     * 
     * @param pos An iterator pointing to the element to be removed. The
     * iterator will be moved to the next element before the element is removed.
     * @return The removed element.
     */
    T remove(Iterator &&pos) {
        return base::remove(pos);
    }
};

namespace parallel
{

/**
 * @brief A thread-safe singly-linked list.
 * 
 * @tparam T The type of list elements.
 */
template <typename T>
class List
:   protected __LinkedList::ListBase<T, __LinkedList::AtomicSinglyLinkedNode<T>, std::atomic<size_t>>,
    public ForwardIterableContainer<List<T>>,
    public Serializable
{

    template <typename ListType> friend struct ListTester;

protected:
    using base = typename __LinkedList::ListBase<T, __LinkedList::AtomicSinglyLinkedNode<T>, std::atomic<size_t>>;
    using node = typename base::node;
    using base::_head;
    using base::_tail;
    using base::_mkNode;

public:

    using Iterator = typename base::template ListForwardIterator<T>;
    using ConstIterator = typename base::template ListForwardIterator<const T>;

    /**
     * @brief Construct a new List object.
     */
    List()
    :   base()
    { }

    List(const List &rhs)
    :   base(rhs)
    { }

    List(List &&rhs)
    :   base(std::move(rhs))
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @param list An initializer list of objects of type T.
     */
    List(const std::initializer_list<T> &list)
    :   base(list)
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @param seq An iterable container of type T objects.
     */
    template <typename Sequence>
    List(const Sequence &seq)
    :   base(seq)
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @param seq An iterable container of type T objects.
     */
    template <typename Sequence>
    List(Sequence &&seq)
    :   base(std::move(seq))
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @param begin A beginning iterator over type T objects.
     * @param end An end iterator over type T objects.
     */
    template <typename Begin, typename End>
    List(const Begin &begin, const End &end)
    :   base(begin, end)
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @param begin A beginning iterator over type T objects.
     * @param end An end iterator over type T objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    List(const Begin &begin, const End &end, size_t size)
    :   base(begin, end)
    { }

    /**
     * @brief Construct a new List object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new List object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static List<typename It::value_type> create(const It &begin, const EndIt &end) {
        return List<typename It::value_type>(begin, end);
    }

    /**
     * @brief Construct a new List object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @param size The number of elements between begin and end.
     * @return A new List object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static List<typename It::value_type> create(const It &begin, const EndIt &end, size_t size) {
        return List<typename It::value_type>(begin, end, size);
    }

    ~List() = default;

    List & operator=(const List &rhs) {
        base::operator=(rhs);
        return *this;
    }

    List & operator=(List &&rhs) {
        base::operator=(std::move(rhs));
        return *this;
    }

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        base::_serialize(serializer);
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        base::_deserialize(serializer);
    }

    /**
     * @return The size of this container.
     */
    size_t size() const {
        return base::size();
    }

    /**
     * @return A boolean indicating whether this container is empty.
     */
    bool empty() const {
        return base::empty();
    }

    /**
     * @return A boolean indicating whether this container is non-empty.
     */
    bool nonEmpty() const {
        return ! base::empty();
    }

    /**
     * @return A constant iterator pointing to the beginning of this container.
     */
    ConstIterator cbegin() const {
        return base::cbegin();
    }

    /**
     * @return A constant iterator pointing to a past-the-end position.
     */
    ConstIterator cend() const {
        return base::cend();
    }

    /**
     * @return An iterator pointing to the beginning of this container.
     */
    Iterator begin() {
        return base::begin();
    }

    /**
     * @return A constant iterator pointing to the beginning of this container.
     */
    ConstIterator begin() const {
        return base::begin();
    }

    /**
     * @return An iterator pointing to a past-the-end position.
     */
    Iterator end() {
        return base::end();
    }

    /**
     * @return A constant iterator pointing to a past-the-end position.
     */
    ConstIterator end() const {
        return base::end();
    }

    /**
     * @return A reference to the first element of this container.
     */
    T & front() {
        return static_cast<node *>(_head)->data;
    }

    /**
     * @return A constant reference to the first element of this container.
     */
    const T & front() const {
        return static_cast<node *>(_head)->data;
    }

    /**
     * @return A reference to the last element of this container.
     */
    T & back() {
        return static_cast<node *>(_tail)->data;
    }

    /**
     * @return A constant reference to the last element of this container.
     */
    const T & back() const {
        return static_cast<node *>(_tail)->data;
    }

    /**
     * @brief Erases all elements in this container.
     * 
     * @return A reference to this container for chaining.
     */
    List & clear() {
        base::clear();
        return *this;
    }

    /**
     * @brief Inserts an element to the front of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & prepend(const T &elem) {
        base::prepend(_mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element to the front of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & prepend(T &&elem) {
        base::prepend(_mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Inserts an element to the front of a list.
     * 
     * @param[in] elem The element to insert.
     * @param[in] list The target list.
     * @return A reference to the list for chaining.
     */
    friend List & operator>>(const T &elem, List &list) {
        return list.prepend(elem);
    }

    /**
     * @brief Inserts an element to the front of a list.
     * 
     * @param[in] elem The element to insert.
     * @param[in] list The target list.
     * @return A reference to the list for chaining.
     */
    friend List & operator>>(T &&elem, List &list) {
        return list.prepend(std::move(elem));
    }

    /**
     * @brief Inserts an element to the back of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & append(const T &elem) {
        base::append(_mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element to the back of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & append(T &&elem) {
        base::append(_mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Inserts an element to the back of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & operator<<(const T &elem) {
        base::append(_mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element to the back of this list.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & operator<<(T &&elem) {
        base::append(_mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Inserts an element.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & insert(const T &elem) {
        base::prepend(_mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element.
     * 
     * @param[in] elem The element to insert.
     * @return A reference to this container for chaining.
     */
    List & insert(T &&elem) {
        base::prepend(_mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Inserts an element directly before an iterator position.
     * 
     * @param[in] pos An iterator marking point before which the inserted
     * element will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    List & insertBefore(const Iterator &pos, const T &elem) {
        base::insertBefore(pos, _mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element directly before an iterator position.
     * 
     * @param[in] pos An iterator marking point before which the inserted
     * element will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    List & insertBefore(const Iterator &pos, T &&elem) {
        base::insertBefore(pos, _mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Inserts an element directly after an iterator position.
     * 
     * @param[in] pos An iterator marking point after which the inserted element
     * will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    List & insertAfter(const Iterator &pos, const T &elem) {
        base::insertAfter(pos, _mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element directly after an iterator position.
     * 
     * @param[in] pos An iterator marking point after which the inserted element
     * will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    List & insertAfter(const Iterator &pos, T &&elem) {
        base::insertAfter(pos, _mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Erases an element at the indicated position.
     * Note: this function is not thread-safe.
     * 
     * @param pos An iterator pointing to the element to be erased. The
     * iterator will be moved to the next element before the element is erased.
     * @return A reference to this container for chaining.
     */
    List & erase(Iterator &pos) {
        base::erase(pos);
        return *this;
    }

    /**
     * @brief Erases an element at the indicated position.
     * Note: this function is not thread-safe.
     * 
     * @param pos An iterator pointing to the element to be erased. The
     * iterator will be moved to the next element before the element is erased.
     * @return A reference to this container for chaining.
     */
    List & erase(Iterator &&pos) {
        base::erase(pos);
        return *this;
    }

    /**
     * @brief Removes an element from the indicated position and returns it.
     * Note: this function is not thread-safe.
     * 
     * @param pos An iterator pointing to the element to be removed. The
     * iterator will be moved to the next element before the element is removed.
     * @return The removed element.
     */
    T remove(Iterator &pos) {
        return base::remove(pos);
    }

    /**
     * @brief Removes an element from the indicated position and returns it.
     * Note: this function is not thread-safe.
     * 
     * @param pos An iterator pointing to the element to be removed. The
     * iterator will be moved to the next element before the element is removed.
     * @return The removed element.
     */
    T remove(Iterator &&pos) {
        return base::remove(pos);
    }};

}   // namespace parallel

}   // namespace spl
