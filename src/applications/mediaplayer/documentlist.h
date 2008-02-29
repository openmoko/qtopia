/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef APP_DOCUMENT_LIST_H 
#define APP_DOCUMENT_LIST_H 


#include <qtopia/applnk.h>
#include <qtopia/storage.h>
#include <qcopchannel_qws.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qlist.h>


class AppDocumentListPrivate;


class AppDocumentList : public QObject {
    Q_OBJECT
public:
    AppDocumentList( const QString &mimefilter, QObject *parent, const char *name = 0 );
    ~AppDocumentList();

    void start();
    void pause();
    void resume();
    void rescan();
    void resend();
signals:
    void added( const DocLnk& doc );
    void removed( const DocLnk& doc );
    void changed( const DocLnk& oldDoc, const DocLnk& newDoc );
    void allRemoved();
    void doneForNow();

private slots:
    void systemMessage( const QCString &msg, const QByteArray &);
    void storageChanged();
    void resendWorker();
    
protected:
    void timerEvent( QTimerEvent *te );

private:
    void add( const DocLnk& doc );
    AppDocumentListPrivate *d;
};


#endif

