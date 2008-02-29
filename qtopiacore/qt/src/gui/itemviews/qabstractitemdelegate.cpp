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

#include "qabstractitemdelegate.h"

#ifndef QT_NO_ITEMVIEWS
#include <qabstractitemmodel.h>
#include <qfontmetrics.h>
#include <qstring.h>
#include <qdebug.h>
#include <private/qtextengine_p.h>

/*!
    \class QAbstractItemDelegate

    \brief The QAbstractItemDelegate class is used to display and edit
    data items from a model.

    \ingroup model-view
    \mainclass

    A QAbstractItemDelegate provides the interface and common functionality
    for delegates in the model/view architecture. Delegates display
    individual items in views, and handle the editing of model data.

    The QAbstractItemDelegate class is one of the \l{Model/View Classes}
    and is part of Qt's \l{Model/View Programming}{model/view framework}.

    To render an item in a custom way, you must implement paint() and
    sizeHint(). The QItemDelegate class provides default implementations for
    these functions; if you do not need custom rendering, subclass that
    class instead.

    To provide custom editing, there are two approaches that can be
    used. The first approach is to create an editor widget and display
    it directly on top of the item. To do this you must reimplement
    createEditor() to provide an editor widget, setEditorData() to populate
    the editor with the data from the model, and setModelData() so that the
    delegate can update the model with data from the editor.

    The second approach is to handle user events directly by reimplementing
    editorEvent().

    \sa {model-view-programming}{Model/View Programming}, QItemDelegate,
        {Pixelator Example}
*/

/*!
    \enum QAbstractItemDelegate::EndEditHint

    This enum describes the different hints that the delegate can give to the
    model and view components to make editing data in a model a comfortable
    experience for the user.

    \value NoHint           There is no recommended action to be performed.

    These hints let the delegate influence the behavior of the view:

    \value EditNextItem     The view should use the delegate to open an
                            editor on the next item in the view.
    \value EditPreviousItem The view should use the delegate to open an
                            editor on the previous item in the view.

    Note that custom views may interpret the concepts of next and previous
    differently.

    The following hints are most useful when models are used that cache
    data, such as those that manipulate date locally in order to increase
    performance or conserve network bandwidth.

    \value SubmitModelCache If the model caches data, it should write out
                            cached data to the underlying data store.
    \value RevertModelCache If the model caches data, it should discard
                            cached data and replace it with data from the
                            underlying data store.

    Although models and views should respond to these hints in appropriate
    ways, custom components may ignore any or all of them if they are not
    relevant.
*/

/*!
  \fn void QAbstractItemDelegate::commitData(QWidget *editor)

  This signal must be emitted when the \a editor widget has completed
  editing the data, and wants to write it back into the model.
*/

/*!
    \fn void QAbstractItemDelegate::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)

    This signal is emitted when the user has finished editing an item using
    the specified \a editor.

    The \a hint provides a way for the delegate to influence how the model and
    view behave after editing is completed. It indicates to these components
    what action should be performed next to provide a comfortable editing
    experience for the user. For example, if \c EditNextItem is specified,
    the view should use a delegate to open an editor on the next item in the
    model.

    \sa EndEditHint
*/

/*!
    Creates a new abstract item delegate with the given \a parent.
*/
QAbstractItemDelegate::QAbstractItemDelegate(QObject *parent)
    : QObject(parent)
{

}

/*!
    \internal

    Creates a new abstract item delegate with the given \a parent.
*/
QAbstractItemDelegate::QAbstractItemDelegate(QObjectPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{

}

/*!
    Destroys the abstract item delegate.
*/
QAbstractItemDelegate::~QAbstractItemDelegate()
{

}

/*!
    \fn void QAbstractItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const = 0;

    This pure abstract function must be reimplemented if you want to
    provide custom rendering. Use the \a painter and style \a option to
    render the item specified by the item \a index.

    If you reimplement this you must also reimplement sizeHint().
*/

/*!
    \fn QSize QAbstractItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const = 0

    This pure abstract function must be reimplemented if you want to
    provide custom rendering. The options are specified by \a option
    and the model item by \a index.

    If you reimplement this you must also reimplement paint().
*/

/*!
    Returns the editor to be used for editing the data item with the
    given \a index. Note that the index contains information about the
    model being used. The editor's parent widget is specified by \a parent,
    and the item options by \a option.

    The base implementation returns 0. If you want custom editing you
    will need to reimplement this function.

    \sa setModelData() setEditorData()
*/
QWidget *QAbstractItemDelegate::createEditor(QWidget *,
                                             const QStyleOptionViewItem &,
                                             const QModelIndex &) const
{
    return 0;
}

/*!
    Sets the contents of the given \a editor to the data for the item
    at the given \a index. Note that the index contains information
    about the model being used.

    The base implementation does nothing. If you want custom editing
    you will need to reimplement this function.

    \sa setModelData()
*/
void QAbstractItemDelegate::setEditorData(QWidget *,
                                          const QModelIndex &) const
{
    // do nothing
}

/*!
    Sets the data for the item at the given \a index in the \a model
    to the contents of the given \a editor.

    The base implementation does nothing. If you want custom editing
    you will need to reimplement this function.

    \sa setEditorData()
*/
void QAbstractItemDelegate::setModelData(QWidget *,
                                         QAbstractItemModel *,
                                         const QModelIndex &) const
{
    // do nothing
}

/*!
    Updates the geometry of the \a editor for the item with the given
    \a index, according to the rectangle specified in the \a option.
    If the item has an internal layout, the editor will be laid out
    accordingly. Note that the index contains information about the
    model being used.

    The base implementation does nothing. If you want custom editing
    you must reimplement this function.
*/
void QAbstractItemDelegate::updateEditorGeometry(QWidget *,
                                                 const QStyleOptionViewItem &,
                                                 const QModelIndex &) const
{
    // do nothing
}

/*!
    Whenever an event occurs, this function is called with the \a event
    \a model \a option and the \a index that corresponds to the item being edited.

    The base implementation returns false (indicating that it has not
    handled the event).
*/
bool QAbstractItemDelegate::editorEvent(QEvent *,
                                        QAbstractItemModel *,
                                        const QStyleOptionViewItem &,
                                        const QModelIndex &)
{
    // do nothing
    return false;
}

/*!
    \obsolete 

    Use QFontMetrics::elidedText() instead.
    
    \oldcode
        QFontMetrics fm = ...
        QString str = QAbstractItemDelegate::elidedText(fm, width, mode, text);
    \newcode
        QFontMetrics fm = ...
        QString str = fm.elidedText(text, mode, width);
    \endcode
*/

QString QAbstractItemDelegate::elidedText(const QFontMetrics &fontMetrics, int width,
                                          Qt::TextElideMode mode, const QString &text)
{
    return fontMetrics.elidedText(text, mode, width);
}
#endif // QT_NO_ITEMVIEWS
