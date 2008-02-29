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

#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qmainwindow.h>
#include <qtextbrowser.h>
#include <qstringlist.h>
#include <qmap.h>

class QPopupMenu;
class QAction;

class HelpBrowser : public QMainWindow
{
    Q_OBJECT
public:
    HelpBrowser( QWidget* parent = 0, const char *name=0, WFlags f=0 );
    ~HelpBrowser();

public slots:
    void setDocument( const QString &doc );
    
private slots:
    void appMessage(const QCString& msg, const QByteArray& data);
    void textChanged();

    void pathSelected( const QString & );
    void bookmChosen( int );
    void addBookmark();
    void removeBookmark();
    
private:
    void init( const QString & );
    void readBookmarks();
    
    QTextBrowser* browser;
    QAction *backAction;
    QAction *forwardAction;
    QString selectedURL;
    struct Bookmark { 
	QString name;
	QString file;
    };
    QMap<int, Bookmark> mBookmarks;
    QMenuBar *menu;
    QPopupMenu *bookm;
};

#endif

