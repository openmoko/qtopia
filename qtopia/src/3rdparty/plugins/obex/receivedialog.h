/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
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
    
public slots:
    void progress( int );
    void receiving( int size, const QString &filename, const QString &mimetype );
    void done();
    void cancelPressed();
    void discardPressed();
    void savePressed();
    void openPressed();
    void statusMsg(const QString &);

signals:
    void cancel();

private slots:
    void fileComplete();

private:
    void setInfo( int size, const QString &filename, const QString &mimetype );
    void save(bool open);

private:
    QStringList catFilesForType( const QStringList &inputFiles, const QString &mimeType, const QString &outFile );
    QStringList receivedFiles;
    void reset();
    QIrServer *server;
    int totalSize;
    QString application;

    ReceiveDialogBase *w;
};

#endif
