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
#ifndef FILESELECTOR_H
#define FILESELECTOR_H

#include <qtopia/qpeglobal.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qtoolbutton.h>
#include <qlistview.h>

#include "filemanager.h"
#include "applnk.h"

class QPopupMenu;
class QPushButton;
class FileSelectorView;

class QTOPIA_EXPORT FileSelectorItem : public QListViewItem
{
public:
    FileSelectorItem( QListView *parent, const DocLnk& f );
    ~FileSelectorItem();

    DocLnk file() const { return fl; }

private:
    friend class FileSelector;
    DocLnk fl;
};

class FileSelectorPrivate;
class QTOPIA_EXPORT FileSelector : public QVBox
{
    Q_OBJECT

public:
    FileSelector( const QString &mimefilter, QWidget *parent, const char *name=0, bool newVisible = TRUE, bool closeVisible = TRUE );
    ~FileSelector();
    void setNewVisible( bool b );
    void setCloseVisible( bool b );
    int fileCount();
    DocLnk selectedDocument() const;	    // libqtopia
    QValueList<DocLnk> fileList() const;    // libqtopia
    const DocLnk *selected(); // use selectedDocument() instead

    void reread();

signals:
    void fileSelected( const DocLnk & );
    void newSelected( const DocLnk & );
    void closeMe();
    void typeChanged(void); // not available in 1.5.0
    void categoryChanged(void); // not available in 1.5.0

private:
    void showEvent( QShowEvent * );

private slots:
    void createNew();
    void fileClicked( int, QListViewItem *, const QPoint &, int );
    // pressed to get 'right down'
    void filePressed( int, QListViewItem *, const QPoint &, int );
    void fileClicked( QListViewItem *);
    void typeSelected( const QString &type );
    void catSelected( int );
    void cardChanged();
    void linkChanged( const QString & );
    void slotReread();

private:
    void updateView();
    void updateWhatsThis();

private:
    FileSelectorView *view;
    QString filter;
    QToolButton *buttonNew, *buttonClose;
    FileSelectorPrivate *d;
};

#endif
