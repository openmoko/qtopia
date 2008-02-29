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
#ifndef LAUNCHERVIEW_H
#define LAUNCHERVIEW_H

#include <QWidget>
#include <QListView>
#include <QContentSet>
#include <QCategoryManager>

class QContentSetMultiColumnProxyModel;
class QAbstractMessageBox;
class TypeDialog;
class QCategoryDialog;
class QLabel;
class QLauncherProxyModel;

class LauncherViewListView : public QListView
{
    Q_OBJECT
public:
    LauncherViewListView( QWidget *parent );

signals:
    void currentIndexChanged( const QModelIndex &current, const QModelIndex &previous );

protected slots:
    virtual void currentChanged( const QModelIndex &current, const QModelIndex &previous );

protected:
    virtual void focusOutEvent(QFocusEvent *);
    virtual bool viewportEvent(QEvent *e);
};

class LauncherView : public QWidget
{
Q_OBJECT
public:
    LauncherView( QWidget* parent = 0, Qt::WFlags fl = 0 );
    virtual ~LauncherView();

    void resetSelection();
    void setBusy(bool);
    void setBusy(const QModelIndex &, bool);
    void setViewMode( QListView::ViewMode m );
    QListView::ViewMode viewMode() const;

    void removeAllItems();
    void addItem(QContent* app, bool resort=true);
    void removeItem(const QContent &);

    void setColumns(int);
    virtual void setFilter(const QContentFilter &filter);
    const QContent currentItem() const;

    enum SortingStyle { NoSorting, LanguageAwareSorting };

    void setSorting(SortingStyle style);
protected:
    friend class QLauncherProxyModel;
    LauncherViewListView *icons;
    QContentSet *contentSet;
    QContentSetModel *model;
    QContentFilter mainFilter;
    QContentFilter categoryFilter;
    QContentFilter typeFilter;
    int nColumns;
    int busyTimer;
    QLauncherProxyModel *bpModel;

    virtual void changeEvent(QEvent *e);
    virtual void showEvent(QShowEvent *e);
    virtual void calculateGridSize();
    virtual void timerEvent( QTimerEvent * event );

signals:
    void clicked( QContent );
    void rightPressed( QContent );

protected slots:
    void returnPressed(const QModelIndex &item);
    void itemClicked(const QModelIndex & index);
    void itemPressed(const QModelIndex &);
    void resizeEvent(QResizeEvent *);
public slots:
    void showType( const QContentFilter & );
    void showCategory( const QContentFilter & );
};

class ApplicationLauncherView : public LauncherView
{
    Q_OBJECT
    public:
        ApplicationLauncherView(QWidget * = 0);
        ApplicationLauncherView(const QString &, QWidget * = 0);
    private slots:
#ifdef QTOPIA_PHONE
    void addSpeedDial();
#endif
    void launcherRightPressed(QContent);
    private:
        QMenu *rightMenu;
};

#endif // LAUNCHERVIEW_H
