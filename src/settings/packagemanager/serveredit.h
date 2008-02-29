/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef SERVEREDIT_H
#define SERVEREDIT_H

#include <QDialog>
#include <QHash>

#include "ui_serveredit.h"

class QTreeWidgetItem;
class ServerItem;

class ServerEdit : public QDialog, private Ui_ServerEditBase
{
    Q_OBJECT
public:
    ServerEdit( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~ServerEdit();
    bool wasModified() const;
    QHash<QString,QString> serverList() const;

public slots:
    void accept();

private slots:
    void init();
    void addNewServer();
    void removeServer();
    void editServer( QTreeWidgetItem*  );
    void activateServer();
    void deactivateServer();
    void serverChanged( QTreeWidgetItem * item, int column );
    void serverSelected( QTreeWidgetItem* current, QTreeWidgetItem* previous );
    void nameChanged( const QString & );
    void urlChanged( const QString & );
private:
    void updateIcons();
    QHash<QString,QString> activeServers;
    bool modified;
    ServerItem *editedServer;
    QAction *removeServerAction;
    QAction *activateServerAction;
    QAction *deactivateServerAction;
    QStringList serversToRemove;
};

////////////////////////////////////////////////////////////////////////
/////
///// inline ServerEdit implementations
/////
inline bool ServerEdit::wasModified() const
{
    return modified;
}

/**
  \internal
  return the list of possibly updated servers
  note that the list is only non-empty after the call to done( int )
*/
inline QHash<QString,QString> ServerEdit::serverList() const
{
    return activeServers;
}

#endif
