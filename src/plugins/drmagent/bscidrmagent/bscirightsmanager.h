/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef BSCIRIGHTSMODEL_H
#define BSCIRIGHTSMODEL_H

#include "bscidrm.h"
#include <QThread>
#include <QAbstractItemModel>
#include <QContent>
#include <QTreeView>
#include <QValueSpaceObject>

class BSciRightsModelPrivate;

/*
    Provides a complete list of all rights available in the drm agent's database.
*/
class BSciRightsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    BSciRightsModel( QObject *parent );
    ~BSciRightsModel();

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual QModelIndex parent( const QModelIndex &index ) const;
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    QContent content( const QModelIndex &index ) const;
    QList< QDrmRights > rights( const QModelIndex &index ) const;

public slots:
    void remove( const QModelIndex &index );

protected:
    virtual QContent content( qint32 index ) const;
    virtual QList< QDrmRights > rights( qint32 index ) const;
    virtual int rightsCount() const;
    virtual void remove( qint32 index );

private slots:
    void init();

private:
    struct ContentItem
    {
        QContent content;

        QList< QDrmRights > rights;

        int rightsId;

        int count;
    };


    const ContentItem *contentItem( int row ) const;

    void verifyList() const;
    void loadList();

    QMap< int, ContentItem* > m_contentItems;

    uint m_rightsCount;
    SBSciRightsInfo **m_rightsInfoList;

private:
    BSciRightsModelPrivate *d;
};

class BSciRightsView : public QTreeView
{
    Q_OBJECT
public:
    explicit BSciRightsView( QWidget *parent );
    ~BSciRightsView();

public slots:
    void back();

protected slots:
    void currentChanged( const QModelIndex &current, const QModelIndex &previous );

#ifdef QTOPIA_KEYPAD_NAVIGATION
protected:
    void keyPressEvent( QKeyEvent *e );
    void focusInEvent( QFocusEvent *e );
#endif
private slots:
    void licenseSelected( const QModelIndex &index );
    void selectCurrent();
    void deleteCurrent();
    void ensureVisible();

private:
    bool canSelect( const QModelIndex &index ) const;
    bool canDelete( const QModelIndex &index ) const;

    QAction *viewAction;
    QAction *deleteAction;
};


class QFileSystem;
class QStorageMetaInfo;
class QWaitWidget;
class QPushButton;

class BSciSettings : public QWidget
{
    Q_OBJECT
public:
    BSciSettings( QWidget *parent = 0, Qt::WindowFlags f = 0 );
private slots:

    void enableSilentRoap( int enable );
    void enableTransactionTracking( int enable );

    void backup();
    void restore();

    void backupComplete( int result );
    void restoreComplete( int result );

private:
    QList< QFileSystem * > backupLocations();
    QList< QFileSystem * > restoreLocations();

    QPushButton *m_backupButton;
    QPushButton *m_restoreButton;
    QDialog *m_backupDialog;
    QThread *m_backupThread;
    QWaitWidget *m_waitWidget;
    QStorageMetaInfo *m_storage;
    QValueSpaceObject m_agentSettings;
};

class BSciBackupThread : public QThread
{
    Q_OBJECT
public:
    BSciBackupThread( const QString &fileName, QObject *parent = 0 );

    void run();

signals:
    void complete( int result );

private:
    QString m_fileName;
};

class BSciRestoreThread : public QThread
{
    Q_OBJECT
public:
    BSciRestoreThread( const QString &fileName, QObject *parent = 0 );

    void run();

signals:
    void complete( int result );

private:
    QString m_fileName;
};

#endif
