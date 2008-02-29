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
#ifndef MAIN_DOCUMENT_WIDGET_STACK_H
#define MAIN_DOCUMENT_WIDGET_STACK_H


#include <qwidgetstack.h>


class MainDocumentWidgetStack : public QWidgetStack
{
    Q_OBJECT

public:
    MainDocumentWidgetStack( QWidget *parent, const char *name = 0, WFlags fl = 0 );
    ~MainDocumentWidgetStack();
    void setLoading( bool b );
    bool havePendingSignals();

public slots:
    void createMediaPlayer();
    void setWaiting( bool );
    void setDocument( const QString& fileref );
    void appMessage( const QCString &msg, const QByteArray &data );

signals:
    void handleSetDocument( const QString& fileref );
    void openURL( const QString& url, const QString& mimetype );

protected:
    void closeEvent( QCloseEvent *ce );

private:
    bool loading;
    bool waiting;
    QString docString;
    QString url, mimetype;
};


extern MainDocumentWidgetStack *mainDocumentWindow;


#endif

