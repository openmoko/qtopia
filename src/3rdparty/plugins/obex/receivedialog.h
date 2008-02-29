/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef RECEIVEDIALOG_H
#define RECEIVEDIALOG_H

#include <qmainwindow.h>

#include "receivedialogbase.h"

class QIrServer;

class ReceiveDialog : public QMainWindow
{
    Q_OBJECT
public:
    ReceiveDialog( QIrServer* irserver, QWidget *parent = 0, const char *name = 0);
    ~ReceiveDialog();
    
    void failed();
    void finished();
    
public slots:
    void progress( int );
    void receiving( int size, const QString &filename, const QString &mimetype );
    void received();
    void cancelPressed();
    void discardPressed();
    void savePressed();
    void openPressed();
    void statusMsg(const QString &);

signals:
    void cancel();
    
private:
    void setInfo( int size, const QString &filename, const QString &mimetype );
    void save(bool open);

private:
    QIrServer *server;
    int totalSize;
    QString application;

    ReceiveDialogBase *w;
};

#endif
