/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCONTENTFILTERSELECTORPRIVATE_H
#define QCONTENTFILTERSELECTORPRIVATE_H

#include <QAbstractProxyModel>
#include <qcontentfiltermodel.h>
#include <QListView>
#include <QItemDelegate>

class QTreePageProxyModelPrivate;

class QTreePageProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
public:

    enum ItemType
    {
        SelectionItem,
        ReturnItem,
        ParentItem
    };

    explicit QTreePageProxyModel( QObject *parent = 0 );

    virtual ~QTreePageProxyModel();

    virtual QModelIndex mapFromSource( const QModelIndex &index ) const;

    virtual QModelIndex mapToSource( const QModelIndex &index ) const;

    virtual void setSourceModel( QAbstractItemModel *sourceModel );

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual bool hasChildren( const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex &index ) const;

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QVariant data( const QModelIndex &index, int role ) const;

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    virtual QSize span( const QModelIndex &index ) const;

    virtual QModelIndex buddy( const QModelIndex &index ) const;

    virtual void fetchMore(const QModelIndex &parent);

    virtual bool canFetchMore(const QModelIndex &parent) const;

    bool sourceHasChildren( const QModelIndex &index ) const;

    bool atRoot() const;

public slots:

    void browseToIndex( const QModelIndex &index );

    void back();

signals:
    void ensureVisible( const QModelIndex &index );
    void select( const QModelIndex &index );

private slots:
    void _columnsAboutToBeInserted( const QModelIndex &parent, int start, int end );

    void _columnsAboutToBeRemoved( const QModelIndex &parent, int start, int end );

    void _columnsInserted( const QModelIndex &parent, int start, int end );

    void _columnsRemoved( const QModelIndex &parent, int start, int end );

    void _dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );

    void _modelReset();

    void _rowsAboutToBeInserted( const QModelIndex &parent, int start, int end );

    void _rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );

    void _rowsInserted( const QModelIndex &parent, int start, int end );

    void _rowsRemoved( const QModelIndex &parent, int start, int end );

private:
    QTreePageProxyModelPrivate *d;
};

class QContentFilterView : public QListView
{
    Q_OBJECT
public:
    explicit QContentFilterView( QWidget *parent = 0 );
    virtual ~QContentFilterView();

    QContentFilter checkedFilter( const QModelIndex &parent = QModelIndex() ) const;
    QString checkedLabel() const;

    QContentFilterModel *model() const;
    void setModel( QContentFilterModel *model );

signals:
    void filterSelected( const QContentFilter &filter );

private slots:
    void indexSelected( const QModelIndex &index );
    void ensureVisible( const QModelIndex &index );


protected:
#ifdef QTOPIA_KEYPAD_NAVIGATION
    void keyPressEvent( QKeyEvent *e );
#endif
    void focusInEvent( QFocusEvent *e );

private:
    QTreePageProxyModel *m_proxyModel;
    QContentFilterModel *m_filterModel;
};

class QTreePageItemDelegate : public QItemDelegate
{
public:
    explicit QTreePageItemDelegate( QObject *parent = 0 );
    virtual ~QTreePageItemDelegate();

    virtual void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    virtual QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;

protected:
    QRect arrow( const QStyleOptionViewItem &option, const QRect &bounding, const QVariant &value ) const;

    void doLayout( const QStyleOptionViewItem &option, QRect *checkRect, QRect *pixmapRect, QRect *textRect, QRect *arrowRect, bool hint ) const;
    void drawArrow(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, QTreePageProxyModel::ItemType itemType ) const;
};

#endif
