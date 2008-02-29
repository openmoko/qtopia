/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*!
    \class QStack
    \brief The QStack class is a template class that provides a stack.

    \ingroup tools
    \ingroup shared
    \mainclass
    \reentrant

    QStack\<T\> is one of Qt's generic \l{container classes}. It implements
    a stack data structure for items of a same type.

    A stack is a last in, first out (LIFO) structure. Items are added
    to the top of the stack using push() and retrieved from the top
    using pop(). The top() function provides access to the topmost
    item without removing it.

    Example:

    \quotefromfile snippets/qstack/main.cpp
    \skipto QStack
    \printuntil cout

    The example will output 3, 2, 1 in that order.

    QStack inherits from QVector. All of QVector's functionality also
    applies to QStack. For example, you can use isEmpty() to test
    whether the stack is empty, and you can traverse a QStack using
    QVector's iterator classes (for example, QVectorIterator). But in
    addition, QStack provides three convenience functions that make
    it easy to implement LIFO semantics: push(), pop(), and top().

    QStack's value type must be an \l{assignable data type}. This
    covers most data types that are commonly used, but the compiler
    won't let you, for example, store a QWidget as a value; instead,
    store a QWidget *.

    \sa QVector, QQueue
*/

/*!
    \fn QStack::QStack()

    Constructs an empty stack.
*/

/*!
    \fn QStack::~QStack()

    Destroys the stack. References to the values in the stack, and all
    iterators over this stack, become invalid.
*/

/*!
    \fn void QStack::push(const T& t)

    Adds element \a t to the top of the stack.

    This is the same as QVector::append().

    \sa pop(), top()
*/

/*!
    \fn T& QStack::top()

    Returns a reference to the stack's top item. This function
    assumes that the stack isn't empty.

    This is the same as QVector::last().

    \sa pop(), push(), isEmpty()
*/

/*!
    \fn const T& QStack::top() const

    \overload

    \sa pop(), push()
*/

/*!
    \fn T QStack::pop()

    Removes the top item from the stack and returns it. This function
    assumes that the stack isn't empty.

    \sa top(), push(), isEmpty()
*/
