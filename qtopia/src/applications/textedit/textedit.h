/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#define QTEXTEDIT_OPEN_API

#include <qtopia/filemanager.h>
#include <qtopia/fileselector.h>
#include <qtopia/contextmenu.h>

#include <qmainwindow.h>
#include <qmultilineedit.h>
#include <qlist.h>
#include <qmap.h>
#include <qtoolbar.h>
#include <qwidgetstack.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qaction.h>

class QpeEditor;

class TextEdit : public QMainWindow
{
    Q_OBJECT

public:
    TextEdit( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~TextEdit();

protected:
    void closeEvent( QCloseEvent* );

public slots:
    void setDocument(const QString&);

private slots:
    void message(const QCString& msg, const QByteArray& data);

    void fileNew();
    void fileRevert();
    void fileOpen();
    void fileName();

    void editCut();
    void editCopy();
    void editPaste();
    void editFind(bool);

    void search();
    void searchNext();
    void findWrapped();
    void findNotFound();
    void findFound();

    void accept();

    void newFile( const DocLnk & );
    void openFile( const DocLnk & );
    void showEditTools();

    void zoomIn();
    void zoomOut();
    void setWordWrap(bool y);
    void setFixedWidth(bool y);

    void clipboardChanged();
    void linkChanged( const QString & );
    
private:
    bool save();
    void clear();
    void colorChanged( const QColor &c );
    void updateCaption( const QString &name=QString::null );
    void setFontSize(int sz, bool round_down_not_up);
    void setupFontSizes(void);
    void setReadOnly(bool);

private:
    QWidgetStack *editorStack;
    FileSelector *fileSelector;
    QpeEditor* editor;
#ifndef QTOPIA_PHONE
    QToolBar *menu, *editBar;
#else
    QToolButton *findTb;
#endif
    QToolBar *searchBar;
    QLineEdit *searchEdit;
    QAction *pasteAction;
    QAction *fixedAction;
    QAction *findAction;
    DocLnk *doc;
#ifdef QTOPIA_PHONE
    QString backup;
    bool qCopActivated, canceled, saved;
#endif
    bool wasCreated;
    bool searchVisible;
    bool lastSaveCancelled;
    QAction *zin, *zout;
    bool zinE,zoutE,zoomOutLast;
    int variableFontSize;
    QString calculateName(QString);
#ifdef QTOPIA_PHONE
    ContextMenu *contextMenu;
    ContextMenu *fileContextMenu;
#endif
};

#endif
